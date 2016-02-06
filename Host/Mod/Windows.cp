MODULE HostWindows;
(**
	project	= "BlackBox"
	organization	= "www.oberon.ch"
	contributors	= "Oberon microsystems, Alexander Iljin, Josef Templ"
	version	= "System/Rsrc/About"
	copyright	= "System/Rsrc/About"
	license	= "Docu/BB-License"
	changes	= "
	- 20060531, ai, Updated GetThisWindow to allow parameter p = NIL
	- 20060608, ai, Updated WMPaint processing to RETURN 0
	- 20070131, bh, Unicode support
	- 20070205, bh, Win32s handling removed
	- 20070209, bh, Windows character mapping removed
	- 20070210, jt, hookDocWinHandler added
	- 20070308, bh, handling of minimized windows corrected
	- 20080119, bh, handling of minimized windows corrected
	"
	issues	= ""

**)

	IMPORT
		SYSTEM,
		GLib:=LibsGlib, Gdk := LibsGdk, Gtk := LibsGtk, GtkU:=Gtk2Util, Key:=Gtk2Keysyms,

		Kernel, Meta, Services, Log,
		Files, HostFiles, Ports, HostRegistry, HostPorts, Properties,
		Views, Controllers, Containers, Dialog, Converters, Documents, Windows,
		Utf8 := HostUtf8,
		HostMechanisms (* don't remove *);

	CONST
		inPlace* = 31;	(* flag for in place windows *)
		untitledKey = "#System:untitled";
		allocKey = "#Host:AllocatedMemory";
		totalKey = "#Host:Total";
		byteKey = "#Host:Bytes";
		scrollRange = 16384;
		lineInc = 1;
		pageInc = 100;
		defThumbSize = scrollRange DIV 20;
		borderW = 5 * Ports.point;
		guardCheck = 4;
		useSeparators = TRUE; noSeparators = FALSE; 	(* parameters to be used with AppendInt *)

		ENTER = 0DX; ESC = 1BX;
		TAB = 09X; LTAB = 0AX; RDEL = 07X; LDEL = 08X;
		PL = 10X; PR = 11X; PU = 12X; PD = 13X;
		DL = 14X; DR = 15X; DU = 16X; DD = 17X;
		AL = 1CX; AR = 1DX; AU = 1EX; AD = 1FX;

		(* values for the type parameter for HandleMouse *)
		press = 0; move = 1;

	TYPE
		Window* = POINTER TO RECORD (Windows.Window)
			wnd-: Gtk.GtkWindow; (* gtk window *)
			da: Gtk.GtkDrawingArea; 	(* gtk drawing area *)
			dlg: BOOLEAN;	(* if window has a 3d dialog border *)
			fix: BOOLEAN;	(* window is opened with fix coordinates *)
			next: Window;	(* window ring, to prevent garbage collection of windows *)
			trapped: BOOLEAN;	(* if window caused trap, it won't get idle messages anymore *)
			used: BOOLEAN;	(* window received at least on message *)
			destroyed: BOOLEAN;
			vBar: Gtk.GtkVScrollbar;
			hBar: Gtk.GtkHScrollbar; (* scrollbars of the window, may be NIL *)
			fixed: Gtk.GtkFixed;
			vBarSize, hBarSize: INTEGER; (* width of the vertical scrollbar,  height of the horizontal scrollbar *)
			oldScrollVPos, oldScrollHPos: INTEGER; (* used to determine wheather page or line increments should be scrolled *)
			open: BOOLEAN;
			title: Views.Title
		END;

		Directory* = POINTER TO RECORD (Windows.Directory) END;

	VAR
		dir: Directory;
		main-: Gtk.GtkWindow;
		unit: INTEGER;	(* resolution of main window *)
		scW-, scH-: INTEGER;	(* screen width and height *)
		mainVBox: Gtk.GtkContainer; (* container to hold the menu and the statusbar *)
		menuBar: Gtk.GtkWidget;
		statusBar, infoBar: Gtk.GtkStatusbar;
		winAnchor: Window;	(* list of all windows, from top to bottom, first is dumy header *)

		tWindow, fWindow: Window;	(* target and front focus windows *)
		aWindow: Window;	(* activated child window *)
		newNumber: INTEGER;	(* number for next untitled document *)
		statusId: INTEGER; (* used for statusbar *)
		infoId: INTEGER; (* used for info bar*)

		alloc, total: INTEGER;
		allocStr, totalStr, byteStr: ARRAY 256 OF CHAR;
		idleTraped: BOOLEAN;


	(* auxiliary portable procedures (scrolling) *)

	PROCEDURE GetSection (w: Window; focus, vertical: BOOLEAN;
								VAR size, sect, pos: INTEGER; VAR valid: BOOLEAN);
		VAR msg: Controllers.PollSectionMsg;
	BEGIN	(* portable *)
		msg.focus := focus; msg.vertical := vertical;
		msg.wholeSize := 1; msg.partSize := 0; msg.partPos := 0;
		msg.valid := FALSE; msg.done := FALSE;
		w.ForwardCtrlMsg(msg);
		IF msg.done THEN
			size := msg.wholeSize; sect := msg.partSize; pos := msg.partPos;
			IF size < 0 THEN size := 0 END;
			IF sect < 0 THEN sect := 0 ELSIF sect > size THEN sect := size END;
			IF pos > size - sect THEN pos := size - sect END;
			IF pos < 0 THEN pos := 0 END
		ELSE size := 1; sect := 0; pos := 0
		END;
		valid := msg.valid
	END GetSection;

	PROCEDURE SetOrigin (w: Window; focus, vertical: BOOLEAN; pos: INTEGER);
	(* set origin of window's view *)
		VAR msg: Controllers.ScrollMsg;
	BEGIN	(* portable *)
		msg.focus := focus; msg.vertical := vertical;
		msg.op := Controllers.gotoPos; msg.pos := pos;
		msg.done := FALSE;
		w.ForwardCtrlMsg(msg)
	END SetOrigin;

	PROCEDURE Scroll (w: Window; focus, vertical: BOOLEAN; dir: INTEGER);
	(* scroll relative, by line or page increment or decrement *)
		VAR msg: Controllers.ScrollMsg; c: Containers.Controller; v: Views.View;
	BEGIN	(* portable *)
		c := w.doc.ThisController(); v := c.ThisFocus();
		IF (v # NIL) & (v IS Containers.View) THEN
			Containers.FadeMarks(v(Containers.View).ThisController(), FALSE)
		END;
		msg.focus := focus; msg.vertical := vertical;
		msg.op := dir;
		msg.done := FALSE;
		w.ForwardCtrlMsg(msg)
	END Scroll;


	(** miscellaneous procedures **)

	PROCEDURE AppendInt (VAR s: ARRAY OF CHAR; n: INTEGER; useSeparators: BOOLEAN);
		VAR len: INTEGER; i, j: INTEGER; d: ARRAY 32 OF CHAR;
	BEGIN
		ASSERT(n >= 0, 20);
		i := 0; REPEAT
			d[i] := CHR(30H + n MOD 10); INC(i); n := n DIV 10;
			IF useSeparators & (i MOD 4 = 3) & (n # 0) THEN d[i] := "'"; INC(i) END
		UNTIL n = 0;
		len := LEN(s) - 1;
		j := 0; WHILE s[j] # 0X DO INC(j) END;
		IF j + i < len THEN
			REPEAT DEC(i); s[j] := d[i]; INC(j) UNTIL i = 0;
			s[j] := 0X
		END
	END AppendInt;

	PROCEDURE Append (VAR s: ARRAY OF CHAR; t: ARRAY OF CHAR);
		VAR len: INTEGER; i, j: INTEGER; ch: CHAR;
	BEGIN
		len := LEN(s);
		i := 0; WHILE s[i] # 0X DO INC(i) END;
		j := 0; REPEAT ch := t[j]; s[i] := ch; INC(j); INC(i) UNTIL (ch = 0X) OR (i = len);
		s[len - 1] := 0X
	END Append;

	PROCEDURE StripTitle (VAR s: Views.Title);
		VAR i: INTEGER;
	BEGIN
		IF s[0] = "<" THEN
			i := 1; WHILE (s[i] # ">") & (s[i] # 0X) DO s[i - 1] := s[i]; INC(i) END;
			DEC(i); s[i] := 0X
		END
	END StripTitle;

	PROCEDURE GenTitle (w: Window; name: ARRAY OF CHAR; VAR title: ARRAY OF CHAR);
	(* generate window title for a document *)
		VAR newName: ARRAY 64 OF CHAR; i: INTEGER;
	BEGIN
		IF w.sub THEN title[0] := "<"; title[1] := 0X ELSE title[0] := 0X END;
		IF name # "" THEN
			i := 0;
			WHILE name[i] # 0X DO INC(i) END;
			IF (i > 4) & (name[i-4] = ".") &
				(CAP(name[i-3]) = "O") & (CAP(name[i-2]) = "D") & (CAP(name[i-1]) = "C")
			THEN
				name[i-4] := 0X
			END;
			Append(title, name)
		ELSE
			Dialog.MapString(untitledKey, newName);
			Append(title, newName);
			AppendInt(title, newNumber, noSeparators);
			INC(newNumber)
		END;
		IF w.sub THEN Append(title, ">") END
	END GenTitle;

	PROCEDURE GenPathTitle (w: Window; OUT title: ARRAY OF CHAR);
		VAR loc: Files.Locator; ch: CHAR; s1, s2: Views.Title; i, j: INTEGER;
	BEGIN
		loc := w.loc; title := "";
		WITH loc: HostFiles.Locator DO
			i := 0; ch := loc.path[0];
			j := 0; s2 := "";
			WHILE ch # 0X DO
				IF (ch = "\") OR (ch = "/") THEN
					s1[j] := 0X; s2 := s1$; j := 0
				ELSE
					s1[j] := ch; INC(j)
				END;
				INC(i); ch := loc.path[i]
			END;
			s1[j] := 0X;
			IF ((CAP(s1[0]) = "M") & (CAP(s1[1]) = "O") & (CAP(s1[2]) = "D") & (s1[3] = 0X) OR
				(CAP(s1[0]) = "D") & (CAP(s1[1]) = "O") & (CAP(s1[2]) = "C") & (CAP(s1[3]) = "U") & (s1[4] = 0X) OR
				(CAP(s1[0]) = "R") & (CAP(s1[1]) = "S") & (CAP(s1[2]) = "R") & (CAP(s1[3]) = "C") & (s1[4] = 0X))
				& (s2 # "") THEN
				title := "("; Append(title, s2); Append(title, ")")
			END
		ELSE
		END;
		Append(title, w.name)
	END GenPathTitle;


	(* Window creation *)
(*
	PROCEDURE OpenDoc (w: Window; l, t, r, b: INTEGER; min, max: BOOLEAN);
	BEGIN
	END OpenDoc;

	PROCEDURE OpenDlg (w: Window; l, t, r, b: INTEGER; min, max: BOOLEAN);
		(* first part of Open, called from directory.open *)
	BEGIN
	END OpenDlg;
*)

	(** Window **)

	PROCEDURE^ (w: Window) UpdateScrollbars (focus, grow: BOOLEAN), NEW;

	PROCEDURE (w: Window) ForwardCtrlMsg* (VAR msg: Controllers.Message), EXTENSIBLE;
		VAR d: BOOLEAN; res: INTEGER;
	BEGIN
		IF w.frame # NIL THEN
			Views.SetRoot(w.frame, w.frame.view, w = fWindow, w.flags);
			w.ForwardCtrlMsg^(msg);
			WITH msg: Controllers.ScrollMsg DO
				w.UpdateScrollbars(FALSE, FALSE)
			ELSE
			END;
		END
	END ForwardCtrlMsg;

	PROCEDURE (w: Window) SetSize* (width, height: INTEGER);
		VAR res, x, y, dw, dh: INTEGER;
	BEGIN
		IF w.port # NIL THEN
			w.SetSize^(width, height);
			dw := width - w.da.allocation.width; dh := height - w.da.allocation.height;
			IF ~(inPlace IN w.flags) & ((dw # 0) OR (dh # 0)) THEN
				Gtk.gtk_widget_set_usize(w.wnd, width + w.vBarSize, height + w.hBarSize);
				IF w.fixed # NIL THEN
					Gtk.gtk_widget_set_usize(w.fixed, width + w.vBarSize, height + w.hBarSize);
					IF w.vBarSize > 0 THEN
						Gtk.gtk_fixed_move(w.fixed, w.vBar, SHORT(width), 0);
						Gtk.gtk_widget_set_usize(w.vBar, w.vBarSize, height);
					END;
					IF w.hBarSize > 0 THEN
						Gtk.gtk_fixed_move(w.fixed, w.hBar, 0, SHORT(height));
						Gtk.gtk_widget_set_usize(w.hBar, width, w.hBarSize);
					END;
				END;
				Gtk.gtk_drawing_area_size(w.da, width, height)
			END
		END
	END SetSize;

	PROCEDURE (w: Window) SetTitle2 (title: Views.Title), NEW;
	(* assign name of window, generate title out of name, and update window title bar *)
		VAR res: INTEGER; h: Window; t: ARRAY 256 OF CHAR;
			us: GLib.PString;
	BEGIN
		ASSERT(w.wnd # NIL, 20);
		StripTitle(title);
		h := w;
		REPEAT
			GenTitle(h, title, t);
			us := GLib.g_utf16_to_utf8(t, -1, NIL, NIL, NIL);
			Gtk.gtk_window_set_title(w.wnd, us);
			GLib.g_free(SYSTEM.VAL(GLib.gpointer, us));
			h := h.link(Window)
		UNTIL h = w
	END SetTitle2;

	PROCEDURE (w: Window) SetTitle* (title: Views.Title);
	BEGIN
		ASSERT(w.wnd # NIL, 20);
		w.title := title; Dialog.MapString(w.title, title);
		w.SetTitle2(title)
	END SetTitle;

	PROCEDURE (w: Window) RefreshTitle* ;
		VAR title: Views.Title;
	BEGIN
		ASSERT(w.wnd # NIL, 20);
		Dialog.MapString(w.title, title);
		w.SetTitle2(title)
	END RefreshTitle;

	PROCEDURE (w: Window) GetTitle* (OUT title: Views.Title);
	(* get name of window *)
		VAR us: GLib.PString;
	BEGIN
		ASSERT(w.wnd # NIL, 20);
		title := w.title;
		IF title = "" THEN
			us := Gtk.gtk_window_get_title(w.wnd);
			Utf8.Long(us$, title);
			StripTitle(title)
		END
	END GetTitle;

	PROCEDURE (w: Window) SetSpec* (loc: Files.Locator; name: Files.Name; conv: Converters.Converter);
		VAR title: Views.Title;
	BEGIN
		IF name # "" THEN Kernel.MakeFileName(name, "") END;
		w.SetSpec^ (loc, name, conv);
		IF (loc # NIL) & (w.wnd # NIL) THEN GenPathTitle(w, title); w.SetTitle(title) END
	END SetSpec;

	PROCEDURE (w: Window) Mark (do, wk: BOOLEAN), NEW;
		VAR mark: Controllers.MarkMsg;
	BEGIN
		mark.show := do;
		mark.focus := ~wk;
		w.ForwardCtrlMsg(mark);
		Properties.IncEra
	END Mark;

	PROCEDURE (w: Window) MouseDown* (x, y, time: INTEGER; modifiers: SET);
	(* handle a mouse down event in window *)
		VAR pw, ph: INTEGER; track: Controllers.TrackMsg;
	BEGIN
		track.modifiers := modifiers;
		w.port.GetSize(pw, ph); track.x := x * w.port.unit; track.y := y * w.port.unit;
		w.ForwardCtrlMsg(track);
		Properties.IncEra
	END MouseDown;

	PROCEDURE (w: Window) KeyDown* (ch: CHAR; buttons: SET);
	BEGIN
		w.KeyDown^(ch, buttons);
		Properties.IncEra
	END KeyDown;


	PROCEDURE UpdateScrollbar (w: Window; vertical, focus: BOOLEAN);
		VAR res, size, sect, pos: INTEGER; valid: BOOLEAN;
			msg: Controllers.PollSectionMsg; f: Views.Frame;
			adj: Gtk.GtkAdjustment; trans: REAL;
	BEGIN
		IF w.frame = NIL THEN RETURN END;
		GetSection(w, focus, vertical, size, sect, pos, valid);
		IF valid THEN
			IF size = 0 THEN size := 1 END;
			IF vertical THEN
				adj := Gtk.gtk_range_get_adjustment( w.vBar);
				Gtk.gtk_widget_show(w.vBar);
				w.vBarSize := w.vBar.requisition.width
			ELSE
				adj := Gtk.gtk_range_get_adjustment(w.hBar);
				Gtk.gtk_widget_show(w.hBar);
				w.hBarSize := w.hBar.requisition.height
			END;
			trans := scrollRange / (size - sect);
			adj.value := SHORT(ENTIER(pos * trans));
			IF sect > 0 THEN
				adj.page_size := SHORT(ENTIER(sect * trans))
			ELSE
				adj.page_size := defThumbSize
			END;
			adj.lower := 0;
			adj.upper := scrollRange + adj.page_size;
			Gtk.gtk_adjustment_changed(adj);
			IF vertical THEN w.oldScrollVPos := SHORT(ENTIER(adj.value))
			ELSE w.oldScrollHPos := SHORT(ENTIER(adj.value)) END;
		ELSIF ~focus THEN
			msg.focus := FALSE; msg.vertical := vertical; msg.done := FALSE;
			f := Views.ThisFrame(w.frame, w.doc.ThisView());
			IF f # NIL THEN
				Views.ForwardCtrlMsg(f, msg);
				IF msg.done THEN
					IF vertical THEN
						adj := Gtk.gtk_range_get_adjustment(w.vBar);
						Gtk.gtk_widget_show(w.vBar);
						w.vBarSize := w.vBar.requisition.width
					ELSE
						adj := Gtk.gtk_range_get_adjustment(w.hBar);
						Gtk.gtk_widget_show(w.hBar);
						w.hBarSize := w.hBar.requisition.height
					END;
					adj.page_size := adj.upper;
					Gtk.gtk_adjustment_changed(adj);
				ELSE
					IF vertical THEN
						w.vBarSize := 0;
						Gtk.gtk_widget_hide(w.vBar)
					ELSE
						w.hBarSize := 0;
						Gtk.gtk_widget_hide(w.hBar)
					END
				END
			ELSE
				IF vertical THEN
					w.vBarSize := 0;
					Gtk.gtk_widget_hide(w.vBar)
				ELSE
					w.hBarSize := 0;
					Gtk.gtk_widget_hide(w.hBar)
				END
			END
		END
	END UpdateScrollbar;

	PROCEDURE (w: Window) ScrollDir (sdir: INTEGER; value: REAL; focus, vertical: BOOLEAN), NEW;
		VAR  size, sect, pos, type: INTEGER; valid: BOOLEAN;
	BEGIN
		GetSection(w, focus, vertical, size, sect, pos, valid);
		IF valid THEN
			IF sdir = Controllers.gotoPos THEN
				value := SHORT(ENTIER((value * (size - sect)) / scrollRange));
				ASSERT(value >= 0, 100);
				SetOrigin(w, focus, vertical, SHORT(ENTIER(value)))
			ELSE
				Scroll(w, focus, vertical, sdir)
			END;
			dir.Update(w);
		END
	END ScrollDir;

	PROCEDURE (w: Window) Scroll (adj: Gtk.GtkAdjustment; focus, vertical: BOOLEAN), NEW;
		VAR sdir, osp: INTEGER;
	BEGIN
		IF vertical THEN osp := w.oldScrollVPos ELSE osp := w.oldScrollHPos END;
		IF adj.value = osp - lineInc THEN
			sdir := Controllers.decLine
		ELSIF adj.value = osp + lineInc THEN
			sdir := Controllers.incLine
		ELSIF adj.value = osp - pageInc THEN
			sdir := Controllers.decPage
		ELSIF adj.value = osp + pageInc THEN
			sdir := Controllers.incPage
		ELSE
			sdir := Controllers.gotoPos
		END;
		IF sdir = Controllers.gotoPos THEN
			w.ScrollDir(sdir, adj.value, focus, vertical)
		ELSE
			w.ScrollDir(sdir, 0, focus, vertical)
		END
	END Scroll;

	PROCEDURE (w: Window) UpdateScrollbars (focus, grow: BOOLEAN), NEW;
		VAR v, h, width, height: INTEGER;
	BEGIN
		v := w.vBarSize; h := w.hBarSize;
		IF ~(Windows.noHScroll IN w.flags) THEN UpdateScrollbar(w, FALSE, focus) END;
		IF ~(Windows.noVScroll IN w.flags) THEN UpdateScrollbar(w, TRUE, focus) END;
		IF (v # w.vBarSize) OR (h # w.hBarSize) THEN
			IF grow THEN
				w.GetSize(width, height);
				w.SetSize(width + w.vBarSize, height + w.hBarSize)
			ELSE
				w.GetSize(width, height);
				width := width - (w.vBarSize - v);
				height := height - (w.hBarSize - h);
				w.SetSize(width, height)
			END
		END
	END UpdateScrollbars;

	PROCEDURE (w: Window) UpdateCursor (x, y: INTEGER; modifiers: SET), NEW;
		VAR pw, ph: INTEGER; msg: Controllers.PollCursorMsg; cur: INTEGER;
	BEGIN
		w.port.GetSize(pw, ph);
		IF ((w = fWindow) OR (w = tWindow) (* TODO: OR ~w.child*))
			& (x >= 0) & (x < pw) & (y >= 0) & (y < ph) THEN
			msg.x := x * w.frame.unit;
			msg.y := y * w.frame.unit;
			msg.cursor := Ports.arrowCursor;
			msg.modifiers := modifiers;
			w.ForwardCtrlMsg(msg); cur := msg.cursor
		ELSE cur := Ports.arrowCursor
		END;
		IF cur >= 0 THEN w.frame.SetCursor(cur) END
	END UpdateCursor;

	PROCEDURE (w: Window) PutOnTop, NEW;
		VAR v: Window;
	BEGIN
		v := winAnchor;
		WHILE (v # NIL) & (v.next # w) DO v := v.next END;
		IF v # NIL THEN
			v.next := w.next; w.next := winAnchor.next; winAnchor.next := w
		END
	END PutOnTop;

	PROCEDURE (w: Window) Close*;
		VAR res: INTEGER; h: Window;
	BEGIN
		ASSERT(w.frame # NIL, 20);
		IF fWindow = w THEN
			(*w.Mark(FALSE, FALSE);*) fWindow := NIL;
			IF tWindow = w THEN tWindow := NIL END
		ELSIF tWindow = w THEN
			(*w.Mark(FALSE, FALSE);*) tWindow := NIL
		END;
(*
		(* remove all shown marks in all windows *)
		IF fWindow # NIL THEN
			ASSERT(fWindow.frame # NIL, 125); ASSERT(fWindow.wnd # 0, 126);
			mark.show := FALSE; fWindow.ForwardCtrlMsg(mark)
		END;
		tWindow := NIL; fWindow := NIL;
*)
(*
		IF w = fWindow THEN fWindow := NIL END;
		IF w = tWindow THEN tWindow := NIL END;
*)
		h := winAnchor;
		WHILE (h.next # NIL) & (h.next # w) DO h := h.next END;
		ASSERT(h.next = w, 21);
		h.next := w.next; w.next := NIL;
(*		HostMechanisms.RemoveDropTarget(w.wnd);*)
		w.Close^;
		IF ~w.destroyed THEN
			w.destroyed := TRUE;
			Gtk.gtk_container_remove(w.wnd, w.fixed);
			Gtk.gtk_widget_destroy(w.wnd);
		END;
		ASSERT(w.frame = NIL, 60)
	END Close;

	PROCEDURE CallHeapShow* (a: INTEGER);
		TYPE P = PROCEDURE(a: INTEGER; t: ARRAY OF CHAR);
			V = RECORD (Meta.Value) p: P END;
		VAR i: Meta.Item; ok: BOOLEAN; v: V;
	BEGIN
		Meta.Lookup("DevDebug", i);
		IF i.obj = Meta.modObj THEN i.Lookup("ShowHeapObject", i);
			IF i.obj = Meta.procObj THEN i.GetVal(v, ok);
				IF ok THEN v.p(a, "") END
			END
		END
	END CallHeapShow;


	(** window handlers **)

	PROCEDURE HandleChar ( w: Window; key: INTEGER; mod: SET);
		VAR ch: CHAR;
	BEGIN
		IF (key >= 20H) & (key # 7FH) THEN
			w.KeyDown(CHR(key), mod)
		END
	END HandleChar;

	PROCEDURE HandleKey (wndHandle, eventHandle, null: INTEGER);
		VAR
			w: Window;
			c: Containers.Controller;
			pmsg: Controllers.PollFocusMsg;
			scroll: BOOLEAN;
			event: Gdk.GdkEventKey;
			code: INTEGER;
			b: SET;
	BEGIN
		w := SYSTEM.VAL(Window, wndHandle);
		event := SYSTEM.VAL(Gdk.GdkEventKey, eventHandle);
		b := {};
		IF Gdk.GDK_SHIFT_BIT IN event.state THEN
			b := b + {HostPorts.shift, Controllers.extend}
			END;
		IF Gdk.GDK_CONTROL_BIT IN event.state THEN
			b := b + {HostPorts.ctrl, Controllers.modify}
		END;
		IF Gdk.GDK_MOD1_BIT IN event.state THEN INCL(b, HostPorts.alt) END;
		scroll := Gdk.GDK_MOD5_BIT IN event.state; (* scroll lock *)

		w.used := TRUE;
		pmsg.focus := NIL; w.ForwardCtrlMsg(pmsg);
		IF (pmsg.focus # NIL) & (pmsg.focus.view IS Containers.View) THEN
			c := pmsg.focus.view(Containers.View).ThisController();
			IF (c # NIL) & (Containers.noCaret IN c.opts) THEN scroll := TRUE END
		END;

		code := event.keyval;
		CASE code OF
		| Key.GDK_Pause:
		| Key.GDK_Shift_L..Key.GDK_Caps_Lock:
		| Key.GDK_F1 .. Key.GDK_F12:
		| Key.GDK_Delete: w.KeyDown(RDEL, b)	(* delete -> right delete *)
		| Key.GDK_BackSpace: w.KeyDown(LDEL, b)	(* backspace -> left delete *)
		| Key.GDK_Tab:
			IF Controllers.extend IN b THEN w.KeyDown(LTAB, b)	(* left tab *)
			ELSE w.KeyDown(TAB, b)	(* right tab *)
			END
		| Key.GDK_Return: w.KeyDown(ENTER, b)
		| Key.GDK_Escape: w.KeyDown(ESC, b)
		| Key.GDK_Page_Up:
			IF scroll THEN
				w.ScrollDir(Controllers.decPage, 0, TRUE, ~(Controllers.modify IN b))
			ELSIF Controllers.modify IN b THEN (* move caret left one page *)
				w.KeyDown(PL, b - {Controllers.modify})
			ELSE (* move caret up one page *)
				w.KeyDown(PU, b)
			END
		| Key.GDK_Page_Down:
			IF scroll THEN
				w.ScrollDir(Controllers.incPage, 0, TRUE, ~(Controllers.modify IN b))
			ELSIF Controllers.modify IN b THEN (* move caret right one page *)
				w.KeyDown(PR, b - {Controllers.modify})
			ELSE (* move caret down one page *)
				w.KeyDown(PD, b)
			END
		| Key.GDK_End:
			IF scroll THEN
				w.ScrollDir(Controllers.gotoPos, scrollRange, TRUE, Controllers.modify IN b)
			ELSIF Controllers.modify IN b THEN (* move caret to doc end *)
				w.KeyDown(DD, b - {Controllers.modify})
			ELSE (* move caret to line end *)
				w.KeyDown(DR, b)
			END
		| Key.GDK_Home:
			IF scroll THEN
				w.ScrollDir(Controllers.gotoPos, 0, TRUE, Controllers.modify IN b)
			ELSIF Controllers.modify IN b THEN (* move caret to doc start *)
				w.KeyDown(DU, b - {Controllers.modify})
			ELSE (* move caret to line start *)
				w.KeyDown(DL, b)
			END
		| Key.GDK_Left:
			IF scroll THEN
				w.ScrollDir(Controllers.decLine, 0, TRUE, FALSE)
			ELSE
				w.KeyDown(AL, b)
			END
		| Key.GDK_Up:
			IF scroll THEN
				w.ScrollDir(Controllers.decLine, 0, TRUE, TRUE)
			ELSE
				w.KeyDown(AU, b)
			END
		| Key.GDK_Right:
			IF scroll THEN
				w.ScrollDir(Controllers.incLine, 0, TRUE, FALSE)
			ELSE
				w.KeyDown(AR, b)
			END
		| Key.GDK_Down:
			IF scroll THEN
				w.ScrollDir(Controllers.incLine, 0, TRUE, TRUE)
			ELSE
				w.KeyDown(AD, b)
			END
		ELSE
			code := Gdk.gdk_keyval_to_unicode(code); (* =ORD(event.string[0]); *)
			IF code # 0 THEN HandleChar(w, code, b) END
		END;
		Properties.IncEra
	END HandleKey;

	PROCEDURE HandleScroll (wndHandle, eventHandle, null: INTEGER);
		VAR
			w: Window;
			c: Containers.Controller;
			pmsg: Controllers.PollFocusMsg;
			event: Gdk.GdkEventScroll;
			a: INTEGER;
	BEGIN
		w := SYSTEM.VAL(Window, wndHandle);

		event := SYSTEM.VAL(Gdk.GdkEventScroll, eventHandle);

		IF event.direction = 1 THEN
			w.ScrollDir(Controllers.incLine, 0, TRUE,  TRUE)
		ELSE
			w.ScrollDir(Controllers.decLine, 0, TRUE,  TRUE)
		END;

		Properties.IncEra
	END HandleScroll;

	PROCEDURE HandleMouse (wndHandle, eventHandle, type: INTEGER);
		VAR
			w: Window;
			isDown: BOOLEAN;
			x, y: INTEGER;
			b: SET;
			f, g: Views.Frame;
			eventB: Gdk.GdkEventButton;
			eventM: Gdk.GdkEventMotion;
			e: Gdk.GdkEvent;
			ex, ey: REAL;
			button: INTEGER;
			state: SET;
	BEGIN
		b := {};
		w := SYSTEM.VAL(Window, wndHandle);
		IF type = press THEN
			eventB := SYSTEM.VAL(Gdk.GdkEventButton, eventHandle);
			ASSERT(~(eventB.type IN {Gdk.GDK_2BUTTON_PRESS, Gdk.GDK_3BUTTON_PRESS}), 100);
			button := eventB.button; state := eventB.state; ex := eventB.x; ey := eventB.y;
		ELSE
			eventM := SYSTEM.VAL(Gdk.GdkEventMotion, eventHandle);
			button := 0; state := eventM.state; ex := eventM.x; ey := eventM.y;
		END;
		w.used := TRUE;
		IF button = 1 THEN INCL(b, HostPorts.left) END;
		IF button = 2 THEN INCL(b, HostPorts.middle) END;
		IF button = 3 THEN INCL(b, HostPorts.right) END;
		isDown := b # {};
		IF Gdk.GDK_SHIFT_BIT IN state THEN INCL(b, HostPorts.shift); INCL(b, Controllers.extend) END;
		IF Gdk.GDK_CONTROL_BIT IN state THEN INCL(b, HostPorts.ctrl); INCL(b, Controllers.modify) END;
		IF Gdk.GDK_MOD1_BIT IN state THEN INCL(b, HostPorts.alt) END;
		e := Gdk.gdk_event_peek();
		IF (e # NIL) & (e.type = Gdk.GDK_2BUTTON_PRESS) THEN
			INCL(b, Controllers.doubleClick);
			Gdk.gdk_event_free(e);
		(*	e := Gdk.gdk_event_get()*)
		END;
		(*	IF e # NIL THEN Gdk.gdk_event_free(e) END;*)
		x := SHORT(ENTIER(ex));
		y := SHORT(ENTIER(ey));
		HostPorts.SetMouseState(x, y, b, isDown);

		(**)
		IF type = press THEN
			w.MouseDown(x, y, 0, b)
		END;
		IF w.port # NIL THEN (* window could have been closed by MouseDown *)
			w.UpdateCursor(x, y, b)
		END;
(*		IF ~isDown THEN activating := FALSE END;*)
		Properties.IncEra
	END HandleMouse;

(*
	PROCEDURE ActivateWindow* (w: Windows.Window; do: BOOLEAN);
	BEGIN
		IF debug THEN
			IF do THEN Log.String("Activate ") ELSE Log.String("Deactivate ") END;
			Log.IntForm(SYSTEM.VAL(INTEGER, w), 16, 8, "0", FALSE);
			Log.Char(" ");
			Log.IntForm(SYSTEM.VAL(INTEGER, fWindow), 16, 8, "0", FALSE);
			IF inPlace IN w.flags THEN Log.String(" ip") END;
			IF Windows.isTool IN w.flags THEN Log.String(" tool") END;
			Log.Ln
		END;

		IF do THEN ActivateWin(w(Window).wnd, 0, 0)
		ELSE DeactivateWin(w(Window).wnd, 0, 0)
		END
	END ActivateWindow;
*)



	(* Signal handlers *)

	(** window signals **)
	(* return 0 -> close ok. return 1 -> don't close *)

	PROCEDURE [ccall] DeleteHandler(widget: Gtk.GtkWidget; event: Gdk.GdkEvent; user_data: INTEGER): INTEGER;
		VAR res: INTEGER; w: Window;
	BEGIN
		w := SYSTEM.VAL(Window, user_data);
		IF w.destroyed THEN
			RETURN 0
		ELSE
			Dialog.Call("HostCmds.Close", "", res);
			RETURN 1
		END
	END DeleteHandler;

	PROCEDURE DeactivateWin (w: Window);
	BEGIN
		IF fWindow = w THEN
			w.Mark(FALSE, TRUE); fWindow := NIL;
			IF (inPlace IN w.flags) OR ~(Windows.isTool IN w.flags) THEN
				w.Mark(TRUE, TRUE);
				IF (w # aWindow) & ~(inPlace IN w.flags) THEN tWindow := NIL END
			END
		END
	END DeactivateWin;

	PROCEDURE [ccall] ChildActivateHandler (window: Gtk.GtkWindow; event: Gdk.GdkEventFocus; user_data: INTEGER);
		VAR w: Window;
	BEGIN
		w := SYSTEM.VAL(Window, user_data);
		IF fWindow # w THEN
			IF fWindow # NIL THEN DeactivateWin(fWindow) END;
			w.PutOnTop;
			IF (inPlace IN w.flags) OR ~(Windows.isTool IN w.flags) THEN
				w.Mark(FALSE, TRUE);
				tWindow := w; aWindow := w
			END;
			fWindow := w; w.Mark(TRUE, TRUE);
			Properties.IncEra;
			Dialog.Notify(0, 0, {guardCheck})
		END
	END ChildActivateHandler;

	PROCEDURE [ccall] ScrollHandler(widget: Gtk.GtkWidget; event: Gdk.GdkEventKey; user_data: INTEGER): INTEGER;
	BEGIN
		Kernel.Try(HandleScroll, user_data, SYSTEM.VAL(INTEGER, event), 0);
		RETURN 1
	END ScrollHandler;

	PROCEDURE [ccall] KeyHandler(widget: Gtk.GtkWidget; event: Gdk.GdkEventKey; user_data: INTEGER): INTEGER;
	BEGIN
		Controllers.SetCurrentPath(Controllers.targetPath);
		Kernel.Try(HandleKey, user_data, SYSTEM.VAL(INTEGER, event), 0);
		Controllers.ResetCurrentPath();
		RETURN 1
	END KeyHandler;

	PROCEDURE [ccall] MouseHandler(widget: Gtk.GtkWidget; event: Gdk.GdkEventButton; user_data: INTEGER): INTEGER;
		VAR w: Window;
	BEGIN
		IF event.type = Gdk.GDK_BUTTON_PRESS THEN
			w := SYSTEM.VAL(Window, user_data);
			Controllers.SetCurrentPath(Controllers.targetPath);
			Gtk.gtk_grab_add(w.da);
			Kernel.Try(HandleMouse, user_data, SYSTEM.VAL(INTEGER, event), press);
			(*	IF Gtk.gtk_grab_get_current() = w.da THEN Gtk.gtk_grab_remove(w.da) END; *)
			IF Gtk.gtk_grab_get_current() # NIL THEN Gtk.gtk_grab_remove(Gtk.gtk_grab_get_current()) END;
			Controllers.ResetCurrentPath()
		END;
		RETURN 1
	END MouseHandler;

	PROCEDURE [ccall] MouseMove(widget: Gtk.GtkWidget; event: Gdk.GdkEventMotion; user_data: INTEGER): INTEGER;
		VAR pw, ph, x, y: INTEGER; w: Window;
	BEGIN
		w := SYSTEM.VAL(Window, user_data);
		w.port.GetSize(pw, ph);
		Gdk.gdk_window_get_position(w.wnd.window, x, y);
		IF (event.x_root < x + pw) & (event.y_root < y + ph) THEN
			Controllers.SetCurrentPath(Controllers.targetPath);
			Gtk.gtk_grab_add(widget);
			Kernel.Try(HandleMouse, user_data, SYSTEM.VAL(INTEGER, event), move);
			IF Gtk.gtk_grab_get_current() = widget THEN Gtk.gtk_grab_remove(widget) END;
			Controllers.ResetCurrentPath();
		END;
		RETURN 1
	END MouseMove;


	PROCEDURE [ccall] WinMouseHandler(widget: Gtk.GtkWidget; event: Gdk.GdkEventButton; user_data: INTEGER): INTEGER;
		VAR w: Window; wx, wy: INTEGER;
	BEGIN
		w := SYSTEM.VAL(Window, user_data);
		Gdk.gdk_window_get_position(w.wnd.window, wx, wy);
		event.x := event.x_root - wx;
		event.y := event.y_root - wy;
		RETURN MouseHandler(w.da, event, user_data);
		RETURN 1
	END WinMouseHandler;

	PROCEDURE [ccall] ConfigureHandler(widget: Gtk.GtkWidget; event: Gdk.GdkEventConfigure; user_data: INTEGER): INTEGER;
		VAR w, h, nw, nh: INTEGER; win: Window;
	BEGIN
		win := SYSTEM.VAL(Window, user_data);
		IF win.open THEN
			win.GetSize(w, h);
			nw := event.width - win.vBarSize;
			nh := event.height - win.hBarSize;
			IF (w # nw) OR (h # nh) THEN win.SetSize(nw, nh) END;
			Gtk.gtk_grab_add(win.da);
			Gtk.gtk_grab_remove(win.da)
		END;
		RETURN 1
	END ConfigureHandler;

	PROCEDURE [ccall] DestroyHandler (object: Gtk.GtkObject; func_data: INTEGER);
		VAR w: Window;
	BEGIN
		w := SYSTEM.VAL(Window, func_data);
		IF ~w.destroyed THEN
			w.destroyed := TRUE;
			w.Close
		END
	END DestroyHandler;

	PROCEDURE [ccall] ExposeEvent (widget: Gtk.GtkWidget; event: Gdk.GdkEventExpose; user_data: INTEGER): INTEGER;
		VAR w: Window; p: HostPorts.Port;
	BEGIN
		w := SYSTEM.VAL(Window, user_data);
		w.open := TRUE;
		w.Restore(event.area.x, event.area.y, event.area.x + event.area.width, event.area.y + event.area.height);
		IF event.count = 0 THEN w.Update END;
		RETURN 1
	END ExposeEvent;

	PROCEDURE [ccall] ShowHandler (object: Gtk.GtkObject; func_data: INTEGER);
		VAR w: Window;
	BEGIN
		w := SYSTEM.VAL(Window, func_data);
		w.Restore(0, 0, w.wnd.allocation.width, w.wnd.allocation.height);
		w.Update
	END ShowHandler;

	(* Scrollbars "value_changed" *)

	PROCEDURE [ccall] VScrollChanged (adjustment: Gtk.GtkAdjustment; user_data: INTEGER);
		VAR w: Window;
	BEGIN
		w := SYSTEM.VAL(Window, user_data);
		w.Scroll(adjustment, FALSE, TRUE) (* TODO: If ctrl is pressed TRUE should be passed for focus. *)
	END VScrollChanged;

	PROCEDURE [ccall] HScrollChanged (adjustment: Gtk.GtkAdjustment; user_data: INTEGER);
		VAR w: Window;
	BEGIN
		w := SYSTEM.VAL(Window, user_data);
		w.Scroll(adjustment, FALSE, FALSE) (* TODO: If ctrl is pressed TRUE should be passed for focus. *)
	END HScrollChanged;

	PROCEDURE ConnectSignals (w: Window);
		VAR res,wval: INTEGER;
	BEGIN
		wval := SYSTEM.VAL(INTEGER, w);
		(* window signals *)
		res := GtkU.gtk_signal_connect(w.wnd, "delete-event", SYSTEM.ADR(DeleteHandler), wval);
		res := GtkU.gtk_signal_connect(w.wnd, "focus-in-event", SYSTEM.ADR(ChildActivateHandler), wval);
		res := GtkU.gtk_signal_connect(w.wnd, "key-press-event", SYSTEM.ADR(KeyHandler), wval);
		(* DIA: Scroll *)
		res := GtkU.gtk_signal_connect(w.wnd, "scroll-event", SYSTEM.ADR(ScrollHandler), wval);
		(*?_after?*)
		res := GtkU.gtk_signal_connect(w.wnd, "button-press-event", SYSTEM.ADR(WinMouseHandler), wval);
		res := GtkU.gtk_signal_connect(w.wnd, "configure-event", SYSTEM.ADR(ConfigureHandler), wval);

		res := GtkU.gtk_signal_connect(w.fixed, "button-press-event", SYSTEM.ADR(WinMouseHandler), wval);
		(* drawing area signals *)
		res := GtkU.gtk_signal_connect(w.da, "destroy", SYSTEM.ADR(DestroyHandler), wval);
		res := GtkU.gtk_signal_connect(w.da, "button-press-event", SYSTEM.ADR(MouseHandler), wval);
		res := GtkU.gtk_signal_connect(w.da, "motion-notify-event", SYSTEM.ADR(MouseMove), wval);
		res := GtkU.gtk_signal_connect(w.da, "expose_event", SYSTEM.ADR(ExposeEvent), wval);
		res := GtkU.gtk_signal_connect(w.da, "show", SYSTEM.ADR(ShowHandler), wval);

		(* set the event masks ??? *)
		Gtk.gtk_widget_set_events(w.da,
			Gdk.GDK_EXPOSURE_MASK + Gdk.GDK_BUTTON_PRESS_MASK +
			Gdk.GDK_BUTTON_RELEASE_MASK + Gdk.GDK_BUTTON_MOTION_MASK +
			Gdk.GDK_KEY_PRESS_MASK + Gdk.GDK_KEY_RELEASE_MASK +
			Gdk.GDK_POINTER_MOTION_MASK + Gdk.GDK_SCROLL_MASK);
		Gtk.gtk_widget_set_events(w.wnd,  Gdk.GDK_BUTTON_PRESS_MASK + Gdk.GDK_BUTTON_RELEASE_MASK +  Gdk.GDK_SCROLL_MASK);
		Gtk.gtk_widget_set_events(w.fixed,  Gdk.GDK_BUTTON_PRESS_MASK +Gdk.GDK_BUTTON_RELEASE_MASK +  Gdk.GDK_SCROLL_MASK);
	END ConnectSignals;


	(** clipboard handling **)
(*
	PROCEDURE OpenClipboard* (doc: Documents.Document);
	BEGIN
	END OpenClipboard;
*)


	(* Directory *)

	PROCEDURE (d: Directory) Open* (w: Windows.Window; doc: Documents.Document; flags: SET; name: Views.Title;
													loc: Files.Locator; fname: Files.Name; conv: Converters.Converter);
		VAR p: HostPorts.Port; c: Containers.Controller; cw, ch: INTEGER;


		PROCEDURE AddScrollbars (w: Window);
			VAR  vadj, hadj: Gtk.GtkAdjustment; res: INTEGER;
			req: Gtk.GtkRequisition;
		BEGIN
			w.fixed := Gtk.gtk_fixed_new();
			Gtk.gtk_fixed_put(w.fixed, w.da, 0, 0);

			IF  (Windows.noHScroll IN w.flags) & (Windows.noVScroll IN w.flags) THEN
				w.vBarSize := 0;
				w.hBarSize := 0;
			ELSE
				vadj := Gtk.gtk_adjustment_new(0, 0, scrollRange, lineInc, pageInc, defThumbSize);
				res := GtkU.gtk_signal_connect(vadj, "value_changed", SYSTEM.ADR(VScrollChanged),SYSTEM.VAL(INTEGER, w));
				hadj := Gtk.gtk_adjustment_new(0, 0, scrollRange, lineInc, pageInc, defThumbSize);
				res := GtkU.gtk_signal_connect(hadj, "value_changed",SYSTEM.ADR(HScrollChanged),SYSTEM.VAL(INTEGER, w));

				w.vBar := Gtk.gtk_vscrollbar_new(vadj);			EXCL(w.vBar.flags, 11);
				w.hBar := Gtk.gtk_hscrollbar_new(hadj);			EXCL(w.hBar.flags, 11);

				Gtk.gtk_fixed_put(w.fixed, w.vBar, 100, 0);
				Gtk.gtk_fixed_put(w.fixed, w.hBar, 0, w.wnd.allocation.height - w.hBarSize);

				Gtk.gtk_widget_size_request(w.vBar, req);			w.vBarSize := req.width;
				Gtk.gtk_widget_size_request(w.hBar, req);			w.hBarSize := req.height;
			END;
			Gtk.gtk_container_add(w.wnd, w.fixed)
		END AddScrollbars;

		PROCEDURE IsDialog (w: Window; c: Containers.Controller ): BOOLEAN;
			VAR dlg: BOOLEAN; v: Views.View; f: SET; col: Ports.Color;
		BEGIN
			IF Windows.isTool IN w.flags THEN dlg := TRUE
			ELSE
				v := w.doc.ThisView(); f := {};
				WITH v: Containers.View DO
					c := v.ThisController();
					IF c # NIL THEN f := c.opts END
				ELSE
				END;
				col := Views.transparent; v.GetBackground(col);
				dlg := ({Containers.noCaret, Containers.noSelection} - f = {})	(* mask mode *)
						& (col = Ports.dialogBackground);	(* dialog background *)
			END;
			RETURN dlg
		END IsDialog;

		PROCEDURE SetSizePos (p: HostPorts.Port; w: Window);
			VAR
				res, size, sect, pos: INTEGER;
				u, dl, dt, dr, db: INTEGER;
		BEGIN
			IF (d.l >= 0) & (d.t >= 0) & ~((d.l = 0) & (d.t = 0) & (d.r = 0) & (d.b = 0)) THEN
				Gtk.gtk_widget_set_uposition(w.wnd, d.l, d.t);
				IF (d.r > d.l) & (d.b > d.t) THEN
					cw := d.r - d.l; ch := d.b - d.t; w.fix := TRUE
				END
			ELSE
				cw := w.wnd.allocation.width; ch := w.wnd.allocation.height;
			END;
			u := w.frame.unit;
			w.port.SetSize(0, 0);
			w.doc.PollRect(dl, dt, dr, db);
			IF w.fix THEN
				IF w.dlg THEN w.doc.SetRect(0, 0, cw * u, ch * u)
				ELSE w.doc.SetRect(borderW, borderW, cw * u - borderW, ch * u - borderW)
				END
			ELSIF w.dlg THEN
				cw := (dr - dl) DIV u;
				ch := (db - dt) DIV u;
				w.doc.SetRect(0, 0, dr - dl, db - dt)
			ELSE
				cw := (dr - dl + 2 * borderW) DIV u + 1;
				ch := (db - dt + 2 * borderW) DIV u + 1;
				IF ~(Windows.noHScroll IN w.flags) & (cw > scW - 40) THEN cw := scW - 80 END;
				IF ~(Windows.noVScroll IN w.flags) & (ch > scH - 40) THEN ch := scH - 160 END;
				w.doc.SetRect(borderW, borderW, borderW + dr - dl, borderW + db - dt)
			END;
			IF cw < 0 THEN cw := 0 END;
			IF ch < 0 THEN ch := 0 END;
			Gtk.gtk_drawing_area_size(w.da, cw, ch);
			w.SetSize(cw, ch);
			IF (Windows.isTool IN w.flags) THEN
				Gtk.gtk_window_set_position(w.wnd, Gtk.GTK_WIN_POS_CENTER)
			ELSE
			END
		END SetSizePos;

		PROCEDURE ShowWindow (w: Window);
		BEGIN
			Gtk.gtk_widget_show(w.wnd);
			IF w.fixed # NIL THEN Gtk.gtk_widget_show(w.fixed) END;
			IF w.vBar # NIL THEN Gtk.gtk_widget_show(w.vBar) END;
			IF w.hBar # NIL THEN Gtk.gtk_widget_show(w.hBar) END;
			Gtk.gtk_widget_show(w.da);
			(*
			w.Restore(0, 0, cw, ch);
			w.Update;
			*)
			w.UpdateScrollbars(FALSE, TRUE);
		END ShowWindow;

	BEGIN
		WITH w: Window DO
			w.open := FALSE;
			NEW(p); p.Init(unit, FALSE);
			w.Init(p);
			w.wnd := Gtk.gtk_window_new(Gtk.GTK_WINDOW_TOPLEVEL);
			IF w.wnd = NIL THEN
				Log.String("Could not create window"); Log.Ln
			ELSE
				d.Open^(w, doc, flags, name, loc, fname, conv);
				w.next := winAnchor.next; winAnchor.next := w;	(* new top window *)
				w.da := Gtk.gtk_drawing_area_new();
				AddScrollbars(w);
				ConnectSignals(w);
				c := w.doc.ThisController();
				c.SetFocus(w.doc.ThisView());
				w.dlg := IsDialog(w, c);
				SetSizePos(p, w);
				p.SetDA(w.da);
				p.SetFW(w.fixed);
				IF (loc # NIL) & (name = fname) THEN GenPathTitle(w, name) END;
				w.SetTitle(name);
				ShowWindow(w);
				d.l := -1; d.t := -1; d.r := -1; d.b := -1;
				IF w.dlg THEN
					Gtk.gtk_window_set_policy(w.wnd, 0, 0, 1)
				ELSE
					Gtk.gtk_window_set_policy(w.wnd, 1, 1, 1)
				END;
				w.SetSize(cw, ch)
			END
		END
	END Open;

	PROCEDURE (d: Directory) First* (): Window;
	BEGIN
		RETURN winAnchor.next
	END First;

	PROCEDURE (d: Directory) Next* (w: Windows.Window): Window;
	BEGIN
		IF w # NIL THEN RETURN w(Window).next ELSE RETURN NIL END
	END Next;

	PROCEDURE (d: Directory) New* (): Windows.Window;
		VAR w: Window;
	BEGIN
		NEW(w); RETURN w
	END New;

	PROCEDURE (d: Directory) Focus* (target: BOOLEAN): Window;
	BEGIN
		IF target THEN RETURN tWindow ELSE RETURN fWindow END
	END Focus;

	PROCEDURE (d: Directory) Select* (w: Windows.Window; lazy: BOOLEAN);
	BEGIN
		WITH w: Window DO
			Gdk.gdk_window_raise(w.wnd.window);
			Gtk.gtk_widget_grab_focus(w.wnd);
		(* On windows the raised window gets focus, but under X this is left to the Window manager to decide. *)
		END
	END Select;

	PROCEDURE (d: Directory) GetThisWindow* (p: Ports.Port; px, py: INTEGER; OUT x, y: INTEGER; OUT w: Windows.Window);
	BEGIN
		Kernel.Beep;

		(* TODO: Implement this when implementing drag and drop. Use: Gdk.gdk_window_at_pointer *)

	END GetThisWindow;

	PROCEDURE (d: Directory) Close* (w: Windows.Window);
		VAR v, u: Windows.Window; h: Window;
	BEGIN
		h := winAnchor; WHILE (h.next # NIL) & (h.next # w) DO h := h.next END;
		IF h.next = w THEN
			IF ~w.sub THEN
				v := w.link;
				WHILE v # w DO u := v.link; v.Close; v := u END
			END;
			w.Close
		END
	END Close;

	PROCEDURE (d: Directory) GetBounds* (OUT w, h: INTEGER);
	BEGIN
		w := scW; h := scH
	END GetBounds;


	(** miscellaneous **)

	PROCEDURE UpdateInfo;
		VAR msgId: INTEGER;
			str: ARRAY 256 OF CHAR;
			us: GLib.PString;
	BEGIN
		IF (alloc # Kernel.Allocated()) OR (total # Kernel.Used()) THEN
			alloc := Kernel.Allocated(); total := Kernel.Used();
			str := allocStr$; AppendInt(str, alloc, useSeparators); Append(str, byteStr);
			us:=GLib.g_utf16_to_utf8(str,-1,NIL,NIL,NIL);
			Gtk.gtk_statusbar_pop(infoBar, infoId);
			msgId := Gtk.gtk_statusbar_push(infoBar, infoId, us);
			GLib.g_free(SYSTEM.VAL(GLib.gpointer, us))
		END
	END UpdateInfo;

	PROCEDURE Idle*;
		VAR w: Window; tick: Controllers.TickMsg; focus: BOOLEAN;
	BEGIN
		w := dir.Focus(FALSE);
		IF (w # NIL) & ~w.trapped THEN
			w.trapped := TRUE;
			IF w.frame # NIL THEN
				tick.tick := SHORT(Services.Ticks()); (* FIXME: see commit 9c28d7017c *)
				w.ForwardCtrlMsg(tick)
			END;
			w.trapped := FALSE
		END;
		(* focus := ScrollModPressed(); TODO: Should check if control key is pressed *)
		focus := FALSE;
		w := dir.First();
		WHILE w # NIL DO
			IF ~w.trapped THEN
				w.trapped := TRUE;
				w.UpdateScrollbars(focus & (w = fWindow), FALSE);
				w.trapped := FALSE
			END;
			w := dir.Next(w)
		END;
		IF ~idleTraped THEN
			idleTraped := TRUE;
			UpdateInfo;
			idleTraped := FALSE
		END;
		Services.actionHook.Step
	END Idle;

	PROCEDURE ShowMain*;
	BEGIN
		Gtk.gtk_widget_show_all(main);
		HostPorts.ResetColors(main)
	END ShowMain;

	PROCEDURE RaiseMain*;
	BEGIN
		Gdk.gdk_window_raise(main.window)
	END RaiseMain;

	PROCEDURE CreateMainWindows*;
		VAR msgId: INTEGER; hBox: Gtk.GtkContainer;
	BEGIN
		main :=  Gtk.gtk_window_new(Gtk.GTK_WINDOW_TOPLEVEL);
		Gtk.gtk_window_set_title(main, "BlackBox");
		Gtk.gtk_widget_set_uposition(main, 1, 1);

		mainVBox := Gtk.gtk_vbox_new(0, 0);
		Gtk.gtk_container_add(main, mainVBox);

		statusBar := Gtk.gtk_statusbar_new();
		statusId := Gtk.gtk_statusbar_get_context_id(statusBar, "BlackBoxStatus");

		infoBar := Gtk.gtk_statusbar_new();
		infoId := Gtk.gtk_statusbar_get_context_id(infoBar, "BlackBoxInfo");

		hBox := Gtk.gtk_hbox_new(1, 0);
		Gtk.gtk_container_add(hBox, statusBar);
		Gtk.gtk_container_add(hBox, infoBar);
		Gtk.gtk_container_add(mainVBox, hBox);

		Dialog.MapString(allocKey, allocStr);
		Dialog.MapString(totalKey, totalStr);
		Dialog.MapString(byteKey, byteStr);
(*
		IF Dialog.appName = "BlackBox" THEN
			str := Dialog.appName$;
			ver := " x.y";
			ver[1] := CHR(Dialog.version DIV 10 + ORD("0"));
			ver[3] := CHR(Dialog.version MOD 10 + ORD("0"));
			Append(str, ver);
			Dialog.ShowStatus(str)
		END;
*)
	END CreateMainWindows;

	PROCEDURE SetMenu* (newMenuBar: Gtk.GtkWidget);
	BEGIN
		IF menuBar # NIL THEN
			Gtk.gtk_container_remove(mainVBox, menuBar);
		END;
		menuBar := newMenuBar;
		Gtk.gtk_container_add(mainVBox, menuBar);
		Gtk.gtk_box_reorder_child(mainVBox, menuBar, 0)
	END SetMenu;

	PROCEDURE SetStatusText* (IN str: ARRAY OF CHAR);
	VAR msgId: INTEGER;
			us: GLib.PString;
	BEGIN
		Gtk.gtk_statusbar_pop(statusBar, statusId);
		us := GLib.g_utf16_to_utf8(str,-1,NIL,NIL,NIL);
		msgId := Gtk.gtk_statusbar_push(statusBar, statusId, us);
		GLib.g_free(SYSTEM.VAL(GLib.gpointer, us))
	END SetStatusText;

	PROCEDURE Init;
	BEGIN
		scW := Gdk.gdk_screen_width();
		scH := Gdk.gdk_screen_height();
		unit := (Ports.mm * Gdk.gdk_screen_height_mm()) DIV scH;
		NEW(winAnchor);
			winAnchor.next := NIL;	(* dummy header *)
		tWindow := NIL; fWindow := NIL; aWindow := NIL;
		NEW(dir); Windows.SetDir(dir)
	END Init;

BEGIN
	Init
END HostWindows.
