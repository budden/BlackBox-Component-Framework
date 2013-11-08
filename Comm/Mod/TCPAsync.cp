MODULE CommTCPAsync;

(*
	Author: Stewart Greenhill	3/7/1998
*)

IMPORT SYSTEM, WinApi, WinNet, CommStreams, Log := StdLog, Strings;

CONST
	moduleName = "CommTCPAsync";
	
	(* name of window and class of notification message window *)
	windowClassName = moduleName + "MessageClass";
	windowName = moduleName + " Message Window";
		
	(* Error conditions returned by NewListener / NewStream *)
	
	errNoLibrary* = 1;	(* cannot initialise networking library *)
	errNoMemory* = 2;	(* cannot allocate structure *)
	errNoSocket* = 3;	(* cannot create socket *)
	errFNBIO* = 4;	(* cannot make socket non-blocking *)
	errBind* = 5;	(* cannot bind socket *)
	errLocalAdr* = 6;	(* error in specification of local address *)
	errRemoteAdr* = 7;	(* error in specification of remote address *)	
	errRemoteAdrSystem* = 8;	(* inet_addr  reported bad address *)
	errMessagePort* = 9;	(* no message port to receive notification *)
	errAsync* = 10;	(* could not start Asyncronous process *)
	errHostName* = 11;	(* could not resolve host name *)
	errSelect* = 12;	(* error in select *)
	errConnect* = 13;	(* error in connect *)
	errListen* = 14;	(* error in listen *)

	(* Internal state for stream *)
	
	error = 0;	(* stream error - cannot connect *)
	waitHost = 1;	(* waiting for WSAAsyncGetHostByName *)
	waitSelect = 2;	(* waiting for WSAAsyncSelect *)
	connected = 3;	(* connected *)
	closed = 4;	(* closed *)
	
	(* Windows Messages sent by WSAAsync tasks *)
			
	WM_HOSTNAME = WinApi.WM_USER;
	WM_SELECT = WinApi.WM_USER + 1;

	logDebug = TRUE;
					
TYPE
	Listener* = POINTER TO RECORD (CommStreams.Listener)
		socket : WinNet.SOCKET;
		remoteAdr : WinNet.sockaddr_in;
	END;

	Stream* = POINTER TO RECORD (CommStreams.Stream)
		next : Stream;	(* next Stream in streamQueue *)
		taskHandle : WinApi.HANDLE;	(* identifies this stream to Async task *)
		buffer : POINTER TO ARRAY OF SHORTCHAR;	(* for hostent structure *)
		name : POINTER TO ARRAY OF SHORTCHAR;	(* for host name *)
		remoteAdr : WinNet.sockaddr_in;
		socket : WinNet.SOCKET;
		state : INTEGER;
		res : INTEGER;
		reason : INTEGER;
	END;

VAR
	streamQueue : Stream;	(* queue of connecting streams *)
	netActive : BOOLEAN;	(* Network subsystem is active *)
	instance : WinApi.HINSTANCE;	(* Win32 application instance handle *)
	wndMessage : WinApi.HWND;	(* Win32 window for receiving events from Async tasks *)
	atomClass : WinApi.ATOM;	(* Win32 "message" class *)

(* LogError - print an error message string to the log *)

PROCEDURE LogError (IN message : ARRAY OF CHAR);
BEGIN
	Log.String(moduleName); Log.String(": ERROR: ");
	Log.String(message); Log.Ln;
END LogError;
	
(* ReportError - Report an error from the Winsock library *)

PROCEDURE ReportError(IN message : ARRAY OF CHAR);
BEGIN
	Log.String(moduleName); Log.String(": Winsock error code "); 
	Log.Int(WinNet.WSAGetLastError()); 
	Log.String(" : "); Log.String(message); Log.Ln;
END ReportError;

(* ReportWinError - Report an error from Win32 *)

PROCEDURE ReportWinError(IN message : ARRAY OF CHAR);
BEGIN
	Log.String(moduleName); Log.String("Win32 error code ");
	Log.Int(WinApi.GetLastError()); 
	Log.String(" : "); Log.String(message); Log.Ln;
END ReportWinError;

(* CancelHost - Cancel the AsyncGetHostByName task associated with stream <s> *)

PROCEDURE CancelHost (s : Stream);
BEGIN
	ASSERT(s.state = waitHost, 20);
	IF WinNet.WSACancelAsyncRequest(s.taskHandle) = WinNet.SOCKET_ERROR THEN
		ReportError("Cannot cancel AsyncGetHostByName");
	END;
END CancelHost;

(* CancelSelect - Cancel the AsyncSelect task associated with stream <s> *)

PROCEDURE CancelSelect (s : Stream);
BEGIN
	ASSERT(s.state = waitSelect, 20);
	IF WinNet.WSAAsyncSelect(s.socket, wndMessage, 0, 0) = WinNet.SOCKET_ERROR THEN
		ReportError("Cannot cancel AsynSelect");
	END;
END CancelSelect;

(* CancelTask - Cancel any Async task associated with stream <s> *)

PROCEDURE CancelTask (s : Stream);
BEGIN
	IF s.state = waitHost THEN
		CancelHost(s)
	ELSIF s.state = waitSelect THEN
		CancelSelect(s)
	END;
END CancelTask;

(* CloseSocket - Close socket <s> if it is not already closed. Report any errors that occur. We mark a socket as closed by setting the handle value to SOCKET_ERROR *)

PROCEDURE CloseSocket(VAR s : WinNet.SOCKET);
BEGIN
	IF s # WinNet.SOCKET_ERROR THEN
		IF WinNet.closesocket(s) = WinNet.SOCKET_ERROR THEN
			ReportError("Error closing socket")
		END;
	END;
	s := WinNet.SOCKET_ERROR
END CloseSocket;

(* CreateNonBlockingSocket - create a new non-blocking socket <s>. If <portDefined> is TRUE, bind the socket to the address given in <addr>. If the process fails at any stage, <s> will be closed (if it has been opened).

Post:
	Socket could not be created
		res = errNoSocket
	Socket could not be made non-blocking
		res = errFNBIO
	Socket could not be bound
		res = errBind
*)

PROCEDURE SetNonBlocking (s : WinNet.SOCKET; OUT res : INTEGER);
VAR
	param : WinNet.u_long;
BEGIN
	param := 1;
	(* FNBIO is a nasty macro in winsock.h. The following is equivalient *)
	IF WinNet.ioctlsocket(s, 8004667EH, param) # WinNet.SOCKET_ERROR THEN
	ELSE 
		ReportError("Cannot make socket non-blocking"); res := errFNBIO 
	END;
END SetNonBlocking;

PROCEDURE CreateNonBlockingSocket(OUT s : WinNet.SOCKET; 
	portDefined : BOOLEAN; IN addr : WinNet.sockaddr_in; OUT res : INTEGER);
BEGIN
	s := WinNet.socket(WinNet.AF_INET, WinNet.SOCK_STREAM, 0);
	IF s # WinNet.SOCKET_ERROR THEN
		SetNonBlocking(s, res);
		IF res = 0 THEN
			IF portDefined THEN
				IF WinNet.bind(s, 
					SYSTEM.VAL(WinNet.Ptrsockaddr, SYSTEM.ADR(addr)), SIZE(WinNet.sockaddr_in)) # 0 THEN
					ReportError("Cannot bind socket");
					res := errBind
				END
			END;
		END;
		IF res # 0 THEN CloseSocket(s) END;
	ELSE 
		ReportError("Cannot create socket"); res := errNoSocket; 
	END
END CreateNonBlockingSocket;

(* FINALIZE - release all resources associated with Streams and Listeners. *)

PROCEDURE (s : Stream) FINALIZE-;
BEGIN
	CloseSocket(s.socket)
END FINALIZE;

PROCEDURE (l : Listener) FINALIZE-;
BEGIN
	CloseSocket(l.socket)
END FINALIZE;

PROCEDURE (l : Listener) Accept* (OUT sResult : CommStreams.Stream);
VAR
	set : WinNet.fd_set;
	res, len : INTEGER;
	new : WinNet.SOCKET;
	remoteAdr : WinNet.sockaddr_in;
	s : Stream;
	tv : WinNet.timeval;
BEGIN
	sResult := NIL;
	IF l.socket = WinNet.SOCKET_ERROR THEN RETURN END;

	set.fd_count := 1;
	set.fd_array[0] := l.socket;
	tv.tv_sec := 0; tv.tv_usec := 0;
	IF WinNet.select(1, set, NIL, NIL, tv) = 1 THEN
		len := SIZE(WinNet.sockaddr_in);
		new := WinNet.accept(l.socket, SYSTEM.VAL(WinNet.Ptrsockaddr, SYSTEM.ADR(remoteAdr)), len);
		IF new # WinNet.SOCKET_ERROR THEN
			NEW(s);
			s.res := 0;
			s.state := connected;
			s.remoteAdr := remoteAdr;
			s.socket := new;
			sResult := s;
			res := 0;
			SetNonBlocking(s.socket, res);
			ASSERT(res = 0);
		ELSE
			ReportError("Accept");
			HALT(60);
		END
	END
END Accept;

PROCEDURE (l : Listener) Close*;
BEGIN
	CloseSocket(l.socket);
END Close;

PROCEDURE (l : Listener) LocalAdr* () : CommStreams.Adr;
BEGIN
	RETURN NIL
END LocalAdr;

PROCEDURE (s : Stream) Close*;
BEGIN
	CancelTask(s);
	CloseSocket(s.socket);
	s.state := closed;
END Close;

PROCEDURE (s : Stream) IsConnected* () : BOOLEAN;
VAR
	set : WinNet.fd_set;
	tv : WinNet.timeval;
BEGIN
	CASE s.state OF
	| closed, error :
		RETURN FALSE
	| waitHost, waitSelect :
		RETURN TRUE
	| connected :
		set.fd_count := 1;
		set.fd_array[0] := s.socket;
		tv.tv_sec := 0; tv.tv_usec := 0;
		IF WinNet.select(1, NIL, NIL, set, tv) = 1 THEN
			s.state := closed;
		END;
		RETURN s.state = connected;
	END;
END IsConnected;

PROCEDURE (s : Stream) HasConnected* () : BOOLEAN, NEW;
BEGIN
	RETURN (s.state = connected) OR (s.state = closed)
END HasConnected;

PROCEDURE (s: Stream) ReadBytes* (VAR x: ARRAY OF BYTE; beg, len: INTEGER; OUT read: INTEGER);
BEGIN
	ASSERT(beg >= 0, 20);
	ASSERT(len > 0, 21);
	ASSERT(beg + len <= LEN(x), 22);
	IF s.state = connected THEN
		read := WinNet.recv(s.socket, SYSTEM.VAL(WinApi.PtrSTR, SYSTEM.ADR(x[beg])), len, {});
		IF read = 0 THEN
			s.state := closed
		ELSIF (read = WinNet.SOCKET_ERROR) THEN
			read := 0;
			CASE WinNet.WSAGetLastError() OF
			| WinNet.WSAEWOULDBLOCK:
			| WinNet.WSAENETRESET,
			WinNet.WSAENETDOWN,
			WinNet.WSAECONNRESET,
			WinNet.WSAECONNABORTED:
				s.state := closed;
			ELSE
				ReportError("ReadBytes") 
			END
		END;
	ELSE
		read := 0
	END
END ReadBytes;

PROCEDURE (s: Stream) RemoteAdr* (): CommStreams.Adr;
BEGIN
	RETURN NIL
END RemoteAdr;

PROCEDURE (s: Stream) WriteBytes* (IN x: ARRAY OF BYTE; beg, len: INTEGER; OUT written: INTEGER);
VAR result : INTEGER;
BEGIN
	ASSERT(beg >= 0, 20);
	ASSERT(len > 0, 21);
	ASSERT(beg + len <= LEN(x), 22);
	IF s.state = connected THEN
		written := WinNet.send(s.socket, SYSTEM.VAL(WinApi.PtrSTR, SYSTEM.ADR(x[beg])), len, {});
		IF written = WinNet.SOCKET_ERROR THEN
			written := 0;
			CASE WinNet.WSAGetLastError() OF
			| WinNet.WSAEWOULDBLOCK:
			| WinNet.WSAENETRESET,
			WinNet.WSAENETDOWN,
			WinNet.WSAECONNRESET,
			WinNet.WSAECONNABORTED:
				s.state := closed;
			ELSE
				ReportError("WriteBytes") 
			END
		END;
	ELSE
		written := 0
	END
END WriteBytes;

(* AllocateBuffer - make buffer point to an array of at least <size> SHORTCHARs. New storage is only allocated if sufficient space does not already exist. *)

PROCEDURE AllocateBuffer (VAR buffer : POINTER TO ARRAY OF SHORTCHAR; size : INTEGER);
BEGIN
	IF (buffer = NIL) OR (LEN(buffer) < size) THEN
		NEW(buffer, size)
	END;
END AllocateBuffer;

(* SetBuffer - convert a segment of CHAR string <a> to SHORTCHAR string in <buffer>. Size of <buffer> will be adjusted accordingly. The function will extract <length> characters starting at position <start> *)

PROCEDURE SetBuffer (VAR buffer : POINTER TO ARRAY OF SHORTCHAR; a : ARRAY OF CHAR; start, length : INTEGER);
VAR
	aLength, i : INTEGER;
	ch : CHAR;
BEGIN
	aLength := LEN(a$);
	ASSERT((start >= 0) & (length >= 0) & ((start + length) <= aLength), 20);
	AllocateBuffer(buffer, length + 1);
	FOR i := 0 TO length - 1 DO
		ch := a[start + i];
		buffer[i] := SHORT(ch);
	END;
	a[length] := 0X;
END SetBuffer;

(* StartNet - initialise the Windows Socket library if it has not already been initialised. *)

PROCEDURE StartNet() : BOOLEAN;
VAR
	wsadata : WinNet.WSADATA;
BEGIN
	IF netActive THEN 
		RETURN TRUE
	ELSE
		IF WinNet.WSAStartup(101H, wsadata) # 0 THEN
			ReportError("Cannot start Winsock library");
			RETURN FALSE
		ELSE
			(* ignore version information for now *)
			netActive := TRUE;
			RETURN TRUE
		END
	END
END StartNet;

(* StopNet - finish using the Windows Socket library if it has been initialised *)

PROCEDURE StopNet;
BEGIN
	IF netActive THEN
		netActive := FALSE;
		IF WinNet.WSACleanup() # 0 THEN 
			ReportError("Cannot close winsock library");
		END
	END
END StopNet;

(* ParseLocalAdr - interpret a CHAR string as a local network address. This function builds an internet address <addr>, using the port specified in the string <localAdr>. If a port was specified, <definied> will be set TRUE, indicating that a bind() should be done using <addr>. If none was specified, 0 is used, indicating that an arbitrary port may be assigned. The result <res> will be set as follows:

Post:
	<localAdr> is not a valid integer
		res = errLocalAdr
	<localAdr> is a valid integer
		defined = TRUE
	<localAdr> is empty
		defined = FALSE
*)

PROCEDURE ParseLocalAdr(localAdr : ARRAY OF CHAR;
	OUT defined : BOOLEAN; 
	OUT addr : WinNet.sockaddr_in; 
	OUT res : INTEGER);
VAR
	res2, port : INTEGER;
BEGIN
	addr.sin_family := WinNet.AF_INET;
	addr.sin_port := 0;
	addr.sin_addr.S_un.S_addr := 0;
	IF localAdr = "" THEN
		defined := FALSE
	ELSE
		Strings.StringToInt(localAdr, port, res2);
		IF res2 = 0 THEN
			defined := TRUE;
			addr.sin_port := WinNet.htons(SHORT(port));
		ELSE
			res := errLocalAdr
		END
	END;
END ParseLocalAdr;

(* ParseRemoteAdr - interpret CHAR string <remoteAdr> as a network address. The syntax is simlar to the host address specification in a URI:
	<host> [ ":" <port> ]
where
	<host> ::= [ <ident> { "." <ident> } ]
If the idents in the host part consists only of integers , the function will use inet_addr to convert the string to an IP address. If a non-digit is found, <hostName> is set TRUE, indicating that GetHostByName should be used to resolve the host name. Either way, <s.name> will be set to the name of the host. The resulting (possibly partial) internet address is output as <addr>.

Post:
	Invalid sequence of separators ('.' after ':') OR non-digit in <port> specification
		res = errRemoteAddr
	Invalid sequence of dot-separated integers (inet_addr failed)
		res = errRemoteAddrSystem
	Non-digit in <host> part
		hostName = TRUE
	Valid integer-only <host> part
		hostName = FALSE		
*)

PROCEDURE ParseRemoteAdr(s : Stream;
	remoteAdr : ARRAY OF CHAR; 
	OUT hostName : BOOLEAN;
	OUT addr : WinNet.sockaddr_in;
	OUT res : INTEGER);
VAR
	host : WinNet.u_long;
	nonDigit : BOOLEAN;
	lastSeparator, ch : CHAR;
	nSeparators, posLastSeparator, i, value : INTEGER;
BEGIN
	value := 0; i  := 0; ch := remoteAdr[i]; nonDigit := FALSE;
	lastSeparator := 0X; nSeparators := 0;
	addr.sin_family := WinNet.AF_INET;
	addr.sin_addr.S_un.S_addr := 0;
	WHILE (ch # 0X) DO
		CASE ch OF
		| '0'..'9' :
			value := value * 10 + ORD(ch) - ORD('0');
		| ':', '.' :
			IF lastSeparator = ':' THEN 
				res := errRemoteAdr; RETURN
			ELSE
				INC(nSeparators); lastSeparator := ch; posLastSeparator := i; value := 0;
			END;
		ELSE
			IF lastSeparator = ':' THEN 
				res := errRemoteAdr; RETURN
			ELSE
				nonDigit := TRUE
			END;
		END;
		INC(i); 
		ch := remoteAdr[i];
	END;
	IF res = 0 THEN
		IF lastSeparator = ':' THEN
			SetBuffer(s.name, remoteAdr, 0, posLastSeparator);
			addr.sin_port := WinNet.htons(SHORT(value));
		ELSE
			SetBuffer(s.name, remoteAdr, 0, LEN(remoteAdr$));
			addr.sin_port := WinNet.htons(0);
		END;
		IF nonDigit THEN
			hostName := TRUE;
		ELSE
			hostName := FALSE;
			host := WinNet.inet_addr(s.name^);
			IF host = WinNet.INADDR_NONE THEN
				res := errRemoteAdrSystem
			ELSE
				addr.sin_addr.S_un.S_addr := host
			END
		END
	END
END ParseRemoteAdr;

(* FindStream - find a stream in <streamQueue> with taskHandle of <task>. If found, the stream is removed from the queue and the function returns TRUE. Otherwise, it returns FALSE. *)
		
PROCEDURE FindStreamByTask (task : WinApi.HANDLE; OUT s : Stream) : BOOLEAN;
VAR
	this, prev : Stream;
BEGIN
	this := streamQueue; prev := NIL;
	WHILE this # NIL DO
		IF this.taskHandle = task THEN
			IF prev = NIL THEN
				streamQueue := this.next
			ELSE
				prev.next := this.next
			END;
			s := this;
			RETURN TRUE;
		END;
		prev := this; this := this.next
	END;
	RETURN FALSE;
END FindStreamByTask;

(* RemoveStream - remove stream <s> from the stream queue *)

PROCEDURE RemoveStream(s : Stream);
VAR
	this, prev : Stream;
BEGIN
	this := streamQueue; prev := NIL;
	WHILE this # NIL DO
		IF this = s THEN
			IF prev = NIL THEN
				streamQueue := this.next
			ELSE
				prev.next := this.next
			END;
		END;
		prev := this; this := this.next
	END;
	HALT(20);
END RemoveStream;

(* StartGetHostByName - Start an asynchronous GetHostByName task. Upon completion, a WM_HOSTNAME message will be sent to window wndMessage. This will be interpreted by our message handler. 

Post:
	Asynchronous task could not be started
		res = errAsync
*)

PROCEDURE StartGetHostByName(s : Stream; VAR res : INTEGER);
BEGIN
	(* allocate space for the result of this task *)
	AllocateBuffer(s.buffer, WinNet.MAXGETHOSTSTRUCT);
	
	(* AsyncGetHostByName _may_ complete immediately, so we must add our stream to the queue _now_ *)
	s.next := streamQueue;
	streamQueue := s;

	(* attempt to start asynchronous task *)	
	s.taskHandle := WinNet.WSAAsyncGetHostByName(wndMessage, WM_HOSTNAME, 
		s.name^, SYSTEM.VAL(WinApi.PtrSTR, SYSTEM.ADR(s.buffer[0])), LEN(s.buffer^));
		
	IF s.taskHandle = 0 THEN
		ReportError("Cannot start AsyncGetHostByName task");
		res := errAsync;
		RemoveStream(s);
	ELSE
		s.state := waitHost;
		IF logDebug THEN
			Log.String("GetHostByName Task Handle is "); Log.Int(SYSTEM.VAL(INTEGER, s.taskHandle)); Log.Ln;
		END;
	END
END StartGetHostByName;

(* StartConnect - Start an asynchronous Select task that waits for the socket associated with <s> to connect. Upon completion, a WM_SELECT message will be sent to window wndMessage. This will be interpreted by our message handler. Note that we expect connect() to return WSAEWOULDBLOCK since the socket is non-blocking. 

Post:
	Asynchronous task could not be started
		res = errAsync
	connect() returned an error other than WSAEWOULDBLOCK
		res = errConnect
*)

PROCEDURE StartConnect (s : Stream; VAR res : INTEGER);
BEGIN
	IF WinNet.connect(s.socket,  SYSTEM.VAL(WinNet.Ptrsockaddr, SYSTEM.ADR(s.remoteAdr)), 
			SIZE(WinNet.sockaddr_in)) = WinNet.SOCKET_ERROR THEN
		IF WinNet.WSAGetLastError() = WinNet.WSAEWOULDBLOCK THEN
			(* AsyncSelect _may_ complete immediately, so we must add our stream to the queue _now_ *)
			s.next := streamQueue;
			streamQueue := s;

			(* attempt to start asynchronous task *)
			s.taskHandle := s.socket;
			IF WinNet.WSAAsyncSelect(s.socket, wndMessage, WM_SELECT, WinNet.FD_CONNECT) = WinNet.SOCKET_ERROR THEN
				ReportError("Cannot start AsyncSelect task");
				res := errAsync;
				RemoveStream(s);
			ELSE
				s.state := waitSelect;
				IF logDebug THEN
					Log.String("AsyncSelect Started for socket"); Log.Int(s.socket); Log.Ln;
				END
			END
		ELSE
			ReportError("Cannot connect");
			res := errConnect;
		END
	ELSE
		(* something is not right - it should have reported an error *)
		LogError("Connect has probably blocked!");
		s.state := connected
	END;
END StartConnect;

PROCEDURE NewListener* (localAdr: ARRAY OF CHAR; OUT l: Listener; OUT res: INTEGER);
VAR
	lis : Listener;
	local : WinNet.sockaddr_in;
	portDefined : BOOLEAN;
BEGIN
	res := 0; l := NIL;
	ParseLocalAdr(localAdr, portDefined, local, res);
	IF res # 0 THEN RETURN END;
	
	IF StartNet() THEN
		NEW(lis);
		IF lis # NIL THEN
			CreateNonBlockingSocket(lis.socket, portDefined, local, res);
			IF res = 0 THEN
				IF WinNet.listen(lis.socket, 5) = WinNet.SOCKET_ERROR THEN
					ReportError("Cannot listen on socket");
					res := errListen;
				ELSE
					l := lis
				END
			END
		ELSE res := errNoMemory END
	ELSE res := errNoLibrary END;
END NewListener;

PROCEDURE NewStream* (localAdr, remoteAdr: ARRAY OF CHAR; OUT s: CommStreams.Stream; OUT res: INTEGER);
VAR
	str : Stream;
	hostent : WinNet.Ptrhostent;
	hostName, portDefined : BOOLEAN;
	param : WinNet.u_long;
	local : WinNet.sockaddr_in;
BEGIN
	res := 0; s := NIL;

	NEW(str);	
	str.res := 0;
	str.state := error;
	str.socket := WinNet.SOCKET_ERROR;
	
	ParseLocalAdr(localAdr, portDefined, local, res);
	IF res # 0 THEN RETURN END;
	
	ParseRemoteAdr(str, remoteAdr, hostName, str.remoteAdr, res);
	IF res # 0 THEN RETURN END;
	
	IF StartNet() THEN
		IF str # NIL THEN
			CreateNonBlockingSocket(str.socket, portDefined, local, res);
			IF res = 0 THEN
				IF hostName THEN
					IF wndMessage # WinApi.NULL THEN
						StartGetHostByName(str, res);
					ELSE
						res := errMessagePort
					END
				ELSE
					StartConnect(str, res);
				END;
				IF res = 0 THEN s := str ELSE CloseSocket(str.socket) END
			END
		ELSE res := errNoMemory END
	ELSE res := errNoLibrary END;
END NewStream;

(* WndHandler - handle messages from asynchonous tasks. Messages will be sent to window <wndMessage> to indicate completion of WSAAsyncSelect and WSAAsyncGetHostByName tasks. Here, they are interpreted and any necessary changes to the state of the associated streams will be handled.

All processing begins by locating the Stream associated with the task within the streamQueue. A stream is identified by its task handle (GetHostByName tasks) or its socket handle (Select tasks). These are placed in the stream's <taskHandle> by StartGetHostByName and StartConnect (respectively).

For GetHostByName tasks, the result is first checked. If an error occurred, the stream will be placed in an error state. Otherwise, the first host address will be used as the stream's remote address, and a connection will be initiated to this address using StartConnect (ie. a second asynchronous task will be started).

For Select tasks, the result if first checked. If an error occured, the stream will be placed in an error state. Otherwise, the stream will be placed in a connected state. (ie. it is ready for use)
*)

PROCEDURE WndHandler (wnd, msg, wParam, lParam: INTEGER): INTEGER;
VAR 
	i, result : INTEGER;
	s : Stream;
	h : WinNet.Ptrhostent;
	addr : WinApi.PtrSTR;
BEGIN
	IF msg = WM_HOSTNAME THEN
		IF logDebug THEN
			Log.String("Completed AsyncGetHostByName task "); Log.Int(wParam); Log.Ln;
		END;
		IF FindStreamByTask(SYSTEM.VAL(WinApi.HANDLE, wParam), s) THEN
			IF s.state = waitHost THEN
				result := lParam DIV 65536;
				IF result = 0 THEN
					h := SYSTEM.VAL(WinNet.Ptrhostent, SYSTEM.ADR(s.buffer[0]));
					addr := h.h_addr_list[0];
					s.remoteAdr.sin_addr.S_un.S_un_b.s_b1 := addr[0];
					s.remoteAdr.sin_addr.S_un.S_un_b.s_b2 := addr[1];
					s.remoteAdr.sin_addr.S_un.S_un_b.s_b3 := addr[2];
					s.remoteAdr.sin_addr.S_un.S_un_b.s_b4 := addr[3];
					StartConnect(s, s.res);
				ELSE
					IF logDebug THEN
						Log.String("Winsock Error Code :"); Log.Int(result); Log.Ln;
					END;
					s.res := errHostName; s.reason := result;
					s.state := error;
				END
			ELSE
				LogError("Stream was not waiting for GetHostByName")
			END
		ELSE
			LogError("Stream not found!")
		END;
	ELSIF msg = WM_SELECT THEN
		IF logDebug THEN
			Log.String("Completed Async Select for socket "); Log.Int(wParam); Log.Ln;
		END;
		IF FindStreamByTask(SYSTEM.VAL(WinApi.HANDLE, wParam), s) THEN
			IF s.state = waitSelect THEN
				CancelSelect(s);
				result := lParam DIV 65536;
				IF result = 0 THEN
					s.state := connected;
				ELSE
					IF logDebug THEN
						Log.String("Winsock Error Code :"); Log.Int(result); Log.Ln;
					END;
					s.res := errSelect; s.reason := result;
					s.state := error;
				END;
			ELSE
				LogError("Stream was not waiting for Select")
			END
		ELSE
			LogError("Stream not found!")
		END
	ELSE
		RETURN WinApi.DefWindowProcA(wnd, msg, wParam, lParam)
	END;
	RETURN 0
END WndHandler;

(* RegisterClass - register a Win32 window class named <name> for message processing *)

PROCEDURE RegisterClass (IN name : ARRAY OF SHORTCHAR) : WinApi.ATOM;
VAR 
	class: WinApi.WNDCLASSA; 
BEGIN
	class.hCursor := WinApi.NULL;
	class.hIcon := WinApi.NULL;
	class.lpszMenuName := NIL;
	class.lpszClassName := name;
	class.hbrBackground := WinApi.GetStockObject(WinApi.WHITE_BRUSH);
	class.style := {};
	class.hInstance := instance;
	class.lpfnWndProc := WndHandler;
	class.cbClsExtra := 0;
	class.cbWndExtra := 0;
	RETURN WinApi.RegisterClassA(class);
END RegisterClass;

(* OpenWindow - open a Win32 window with given <class> and <title> *)

PROCEDURE OpenWindow(IN class, title : ARRAY OF SHORTCHAR) : WinApi.HWND;
VAR
	wnd : WinApi.HWND;
BEGIN
	RETURN WinApi.CreateWindowExA({}, 
		class,
		title,
		WinApi.WS_OVERLAPPEDWINDOW + WinApi.WS_DISABLED,
		WinApi.CW_USEDEFAULT, WinApi.CW_USEDEFAULT,
		WinApi.CW_USEDEFAULT, WinApi.CW_USEDEFAULT,
		0, 0, instance, 0);
END OpenWindow;

(* CreateMessageWindow / DestroyMessageWindow. Create/Destroy a window for receiving Winsock notification messages.

Winsock functions use windows to receive asynchronous notifications. Thus, we must create a window to receive these notifications. This window will never be visible, and there should only ever be one such window (of class "message") per application. The window and its associated class is created automatically when this module is loaded, and will be destroyed when it is unloaded. *)

PROCEDURE CreateMessageWindow;
BEGIN
	atomClass := RegisterClass(windowClassName);
	IF atomClass # 0 THEN
		wndMessage := OpenWindow(windowClassName, windowName);
		IF wndMessage # WinApi.NULL THEN
		ELSE
			ReportWinError("Cannot open window");
		END
	ELSE
		ReportWinError("Cannot register class");
	END;
END CreateMessageWindow;

PROCEDURE DestroyMessageWindow;
BEGIN
	IF wndMessage # WinApi.NULL THEN
		IF WinApi.DestroyWindow(wndMessage) # WinApi.TRUE THEN
			ReportWinError("Cannot close window");
		END
	END;
	IF atomClass # 0 THEN
		IF WinApi.UnregisterClassA(windowClassName, instance) # WinApi.TRUE THEN 
			ReportWinError("UnregisterClass failed");
		END
	END;
END DestroyMessageWindow;

PROCEDURE TestNewStream(driver, localAdr, remoteAdr : ARRAY OF CHAR);
VAR
	s : CommStreams.Stream;
	res : INTEGER;
BEGIN
	Log.String('NewStream("'); Log.String(localAdr); 
	Log.String('", "'); Log.String(remoteAdr);
	Log.String('")'); Log.Ln;
	CommStreams.NewStream(driver, localAdr, remoteAdr, s, res);
	Log.String("   NewStream Result = "); Log.Int(res); Log.Ln;
END TestNewStream;

PROCEDURE Test* (a : ARRAY OF CHAR);
BEGIN
	TestNewStream("CommTCPAsync", "", a);
END Test;

PROCEDURE Test2* (a : ARRAY OF CHAR);
BEGIN
	TestNewStream("CommTCP", "", a);
END Test2;

BEGIN
	(* Winsock subsystem will be started later if required *)
	netActive := FALSE;
	
	(* create a notification window *)
	instance := WinApi.GetModuleHandleA(NIL);
	wndMessage := WinApi.NULL;
	CreateMessageWindow;
CLOSE
	(* cancel all async tasks and close their associated sockets *)
	WHILE streamQueue # NIL DO
		CancelTask(streamQueue);
		CloseSocket(streamQueue.socket);
		streamQueue := streamQueue.next
	END;
	(* remove notification window and close down Winsock *)
	DestroyMessageWindow;
	StopNet;
END CommTCPAsync.
