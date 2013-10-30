MODULE DevLinker;
(**
	project	= "BlackBox"
	organization	= "www.oberon.ch"
	contributors	= "Oberon microsystems"
	version	= "System/Rsrc/About"
	copyright	= "System/Rsrc/About"
	license	= "Docu/BB-License"
	changes	= ""
	issues	= ""

**)
	
	IMPORT
		Kernel, Files, Dates, Dialog, Strings,
		TextModels, TextViews, TextMappers,
		Log := StdLog, DevCommanders;
	
	CONST
		NewRecFP = 4E27A847H;
		NewArrFP = 76068C78H;

		ImageBase = 00400000H;
		ObjAlign = 1000H;
		FileAlign = 200H;
		HeaderSize = 400H;

		FixLen = 30000;
		
		OFdir = "Code";
		SYSdir = "System";
		RsrcDir = "Rsrc";
		WinDir = "Win";

		(* meta interface consts *)
		mConst = 1; mTyp = 2; mVar = 3; mProc = 4; mField = 5;
		mInternal = 1; mReadonly = 2; mExported = 4;

		(* fixup types *)
		absolute = 100; relative = 101; copy = 102; table = 103; tableend = 104;
		
		(* mod desc fields *)
		modOpts = 4; modRefcnt = 8; modTerm = 40; modNames = 84; modImports = 92; modExports = 96;
		
	TYPE
		Name = ARRAY 40 OF SHORTCHAR;
		Export = POINTER TO RECORD
			next: Export;
			name: Name;
			adr: INTEGER
		END;
		Resource = POINTER TO RECORD
			next, local: Resource;
			typ, id, lid, size, pos, x, y: INTEGER;
			opts: SET;
			file: Files.File;
			name: Files.Name
		END;
		Module = POINTER TO RECORD
			next: Module;
			name: Files.Name;
			file: Files.File;
			hs, ms, ds, cs, vs, ni, ma, ca, va: INTEGER;
			dll, intf: BOOLEAN;
			exp: Export;
			imp: POINTER TO ARRAY OF Module;
			data: POINTER TO ARRAY OF BYTE;
		END;
		
	VAR
		W: TextMappers.Formatter;
		Out: Files.File;
		R: Files.Reader;
		Ro: Files.Writer;
		error, isDll, isStatic, comLine: BOOLEAN;
		modList, kernel, main, last, impg, impd: Module;
		numMod, lastTerm: INTEGER;
		resList: Resource;
		numType, resHSize: INTEGER;
		numId: ARRAY 32 OF INTEGER;
		rsrcName: ARRAY 16 OF CHAR;	(* name of resource file *)
		firstExp, lastExp: Export;
		entryPos, isPos, fixPos, himpPos, hexpPos, hrsrcPos, termPos: INTEGER;
		codePos, dataPos, conPos, rsrcPos, impPos, expPos, relPos: INTEGER;
		CodeSize, DataSize, ConSize, RsrcSize, ImpSize, ImpHSize, ExpSize, RelocSize, DllSize: INTEGER;
		CodeRva, DataRva, ConRva, RsrcRva, ImpRva, ExpRva, RelocRva, ImagesSize: INTEGER;
		CodeBase, DataBase, ConBase, maxCode, numImp, numExp, noffixup, timeStamp: INTEGER;
		newRec, newArr: Name;
		fixups: POINTER TO ARRAY OF INTEGER;
		code: POINTER TO ARRAY OF BYTE;
		atab: POINTER TO ARRAY OF INTEGER;
		ntab: POINTER TO ARRAY OF SHORTCHAR;
	
	PROCEDURE TimeStamp (): INTEGER;	(* seconds since 1.1.1970 00:00:00 *)
		VAR a: INTEGER; t: Dates.Time; d, epoch: Dates.Date;
	BEGIN
		Dates.GetUTCTime(t); Dates.GetUTCDate(d);
		epoch.year := 1970; epoch.month := 1; epoch.day := 1;
		a := Dates.Day(d) - Dates.Day(epoch);
		RETURN ((a * 24 + t.hour) * 60 + t.minute) * 60 + t.second;
	END TimeStamp;

	PROCEDURE ThisFile (modname: ARRAY OF CHAR): Files.File;
		VAR dir, name: Files.Name; loc: Files.Locator; f: Files.File;
	BEGIN
		Kernel.SplitName(modname, dir, name);
		Kernel.MakeFileName(name, Kernel.objType);
		loc := Files.dir.This(dir); loc := loc.This(OFdir);
		f := Files.dir.Old(loc, name, TRUE);
		IF (f = NIL) & (dir = "") THEN
			loc := Files.dir.This(SYSdir); loc := loc.This(OFdir);
			f := Files.dir.Old(loc, name, TRUE)
		END;
		RETURN f
	END ThisFile;
	
	PROCEDURE ThisResFile (VAR name: Files.Name): Files.File;
		VAR loc: Files.Locator; f: Files.File;
	BEGIN
		f := Files.dir.Old(Files.dir.This(RsrcDir), name, TRUE);
		IF f = NIL THEN
			loc := Files.dir.This(WinDir); loc := loc.This(RsrcDir);
			f := Files.dir.Old(loc, name, TRUE);
			IF f = NIL THEN
				f := Files.dir.Old(Files.dir.This(""), name, TRUE)
			END
		END;
		RETURN f
	END ThisResFile;
	
	PROCEDURE Read2 (VAR x: INTEGER);
		VAR b: BYTE;
	BEGIN
		R.ReadByte(b); x := b MOD 256;
		R.ReadByte(b); x := x + 100H * (b MOD 256)
	END Read2;
	
	PROCEDURE Read4 (VAR x: INTEGER);
		VAR b: BYTE;
	BEGIN
		R.ReadByte(b); x := b MOD 256;
		R.ReadByte(b); x := x + 100H * (b MOD 256);
		R.ReadByte(b); x := x + 10000H * (b MOD 256);
		R.ReadByte(b); x := x + 1000000H * b
	END Read4;
	
	PROCEDURE ReadName (VAR name: ARRAY OF SHORTCHAR);
		VAR i: INTEGER; b: BYTE;
	BEGIN i := 0;
		REPEAT
			R.ReadByte(b); name[i] := SHORT(CHR(b)); INC(i)
		UNTIL b = 0
	END ReadName;
		
	PROCEDURE RNum (VAR i: INTEGER);
		VAR b: BYTE; s, y: INTEGER;
	BEGIN
		s := 0; y := 0; R.ReadByte(b);
		WHILE b < 0 DO INC(y, ASH(b + 128, s)); INC(s, 7); R.ReadByte(b) END;
		i := ASH((b + 64) MOD 128 - 64, s) + y
	END RNum;
	
	PROCEDURE WriteCh (ch: SHORTCHAR);
	BEGIN
		Ro.WriteByte(SHORT(ORD(ch)))
	END WriteCh;
	
	PROCEDURE Write2 (x: INTEGER);
	BEGIN
		Ro.WriteByte(SHORT(SHORT(x MOD 256))); x := x DIV 256;
		Ro.WriteByte(SHORT(SHORT(x MOD 256)))
	END Write2;
	
	PROCEDURE Write4 (x: INTEGER);
	BEGIN
		Ro.WriteByte(SHORT(SHORT(x MOD 256))); x := x DIV 256;
		Ro.WriteByte(SHORT(SHORT(x MOD 256))); x := x DIV 256;
		Ro.WriteByte(SHORT(SHORT(x MOD 256))); x := x DIV 256;
		Ro.WriteByte(SHORT(SHORT(x MOD 256)))
	END Write4;
	
	PROCEDURE WriteName (s: ARRAY OF SHORTCHAR; len: SHORTINT);
		VAR i: SHORTINT;
	BEGIN i := 0;
		WHILE s[i] # 0X DO Ro.WriteByte(SHORT(ORD(s[i]))); INC(i) END;
		WHILE i < len DO Ro.WriteByte(0); INC(i) END
	END WriteName;
	
	PROCEDURE Reloc (a: INTEGER);
		VAR p: POINTER TO ARRAY OF INTEGER; i: INTEGER;
	BEGIN
		IF noffixup >= LEN(fixups) THEN
			NEW(p, 2 * LEN(fixups));
			i := 0; WHILE i < LEN(fixups) DO p[i] := fixups[i]; INC(i) END;
			fixups := p
		END;
		fixups[noffixup] := a; INC(noffixup)
(*
		ELSE
			IF ~error THEN W.WriteSString("  too many fixups") END;
			error := TRUE
		END
*)
	END Reloc;
	
	PROCEDURE Put (mod: Module; a, x: INTEGER);
	BEGIN
		mod.data[a] := SHORT(SHORT(x)); INC(a); x := x DIV 256;
		mod.data[a] := SHORT(SHORT(x)); INC(a); x := x DIV 256;
		mod.data[a] := SHORT(SHORT(x)); INC(a); x := x DIV 256;
		mod.data[a] := SHORT(SHORT(x))
	END Put;
	
	PROCEDURE Get (mod: Module; a: INTEGER; VAR x: INTEGER);
	BEGIN
		x := ((mod.data[a + 3] * 256 +
			(mod.data[a + 2] MOD 256)) * 256 +
			(mod.data[a + 1] MOD 256)) * 256 +
			(mod.data[a] MOD 256)
	END Get;
	
	PROCEDURE GenName (VAR from, to: ARRAY OF SHORTCHAR; ext: ARRAY OF SHORTCHAR);
		VAR i, j: INTEGER;
	BEGIN
		i := 0;
		WHILE from[i] # 0X DO to[i] := from[i]; INC(i) END;
		IF ext # "" THEN
			to[i] := "."; INC(i); j := 0;
			WHILE ext[j] # 0X DO to[i] := ext[j]; INC(i); INC(j) END
		END;
		to[i] := 0X
	END GenName;
	
	PROCEDURE Fixup0 (link, adr: INTEGER);
		VAR offset, linkadr, t, n, x: INTEGER;
	BEGIN
		WHILE link # 0 DO
			RNum(offset);
			WHILE link # 0 DO
				IF link > 0 THEN
					n := (code[link] MOD 256) + (code[link+1] MOD 256) * 256 + code[link+2] * 65536;
					t := code[link+3]; linkadr := CodeBase + impg.ca + link
				ELSE
					n := (impg.data[-link] MOD 256) + (impg.data[-link+1] MOD 256) * 256 + impg.data[-link+2] * 65536;
					t := impg.data[-link+3]; linkadr := ConBase + impg.ma - link
				END;
				IF t = absolute THEN x := adr + offset
				ELSIF t = relative THEN x := adr + offset - linkadr - 4
				ELSIF t = copy THEN Get(impd, adr + offset - ConBase - impd.ma, x)
				ELSIF t = table THEN x := adr + n; n := link + 4
				ELSIF t = tableend THEN x := adr + n; n := 0
				ELSE HALT(99)
				END;
				IF link > 0 THEN
					code[link] := SHORT(SHORT(x));
					code[link+1] := SHORT(SHORT(x DIV 100H));
					code[link+2] := SHORT(SHORT(x DIV 10000H));
					code[link+3] := SHORT(SHORT(x DIV 1000000H))
				ELSE
					link := -link;
					impg.data[link] := SHORT(SHORT(x));
					impg.data[link+1] := SHORT(SHORT(x DIV 100H));
					impg.data[link+2] := SHORT(SHORT(x DIV 10000H));
					impg.data[link+3] := SHORT(SHORT(x DIV 1000000H))
				END;
				IF (t # relative) & ((t # copy) OR (x DIV 65536 # 0)) THEN Reloc(linkadr) END;
				link := n
			END;
			RNum(link)
		END
	END Fixup0;
	
	PROCEDURE Fixup (adr: INTEGER);
		VAR link: INTEGER;
	BEGIN
		RNum(link); Fixup0(link, adr)
	END Fixup;
	
	PROCEDURE CheckDllImports (mod: Module);
		VAR i, x, y: INTEGER; name: Name; imp: Module; exp: Export;
		
		PROCEDURE SkipLink;
			VAR a: INTEGER;
		BEGIN
			RNum(a);
			WHILE a # 0 DO RNum(a); RNum(a) END
		END SkipLink;

	BEGIN
		R := mod.file.NewReader(R);
		R.SetPos(mod.hs + mod.ms + mod.ds + mod.cs);
		SkipLink; SkipLink; SkipLink; SkipLink; SkipLink; SkipLink; i := 0;
		WHILE i < mod.ni DO
			imp := mod.imp[i];
			IF imp # NIL THEN
				RNum(x);
				WHILE x # 0 DO
					ReadName(name); RNum(y);
					IF x = mVar THEN SkipLink;
						IF imp.dll THEN
							W.WriteString("variable (");
							W.WriteString(imp.name); W.WriteChar(".");
							W.WriteSString(name);
							W.WriteString(") imported from DLL in ");
							W.WriteString(mod.name);
							W.WriteLn; Log.text.Append(Log.buf); error := TRUE;
							RETURN
						END
					ELSIF x = mTyp THEN RNum(y);
						IF imp.dll THEN
							RNum(y);
							IF y # 0 THEN
								W.WriteString("type descriptor (");
								W.WriteString(imp.name); W.WriteChar(".");
								W.WriteSString(name);
								W.WriteString(") imported from DLL in ");
								W.WriteString(mod.name);
								W.WriteLn; Log.text.Append(Log.buf); error := TRUE;
								RETURN
							END
						ELSE SkipLink
						END
					ELSIF x = mProc THEN
						IF imp.dll THEN
							SkipLink; exp := imp.exp;
							WHILE (exp # NIL) & (exp.name # name) DO exp := exp.next END;
							IF exp = NIL THEN
								NEW(exp); exp.name := name$;
								exp.next := imp.exp; imp.exp := exp; INC(DllSize, 6)
							 END
						END
					END;
					RNum(x)
				END
			END;
			INC(i)
		END
	END CheckDllImports;
	
	PROCEDURE ReadHeaders;
		VAR mod, im, t: Module; x, i: INTEGER; impdll: BOOLEAN; exp: Export; name: Name;
	BEGIN
		mod := modList; modList := NIL; numMod := 0;
		WHILE mod # NIL DO	(* reverse mod list & count modules *)
			IF ~mod.dll THEN INC(numMod) END;
			t := mod; mod := t.next; t.next := modList; modList := t
		END;
		IF isStatic THEN
			IF isDll THEN
				(* push ebx; cmp [12, esp], 1; jne L1; mov ebx, modlist; { call body; } jp L2 *)
				(* L1: cmp [12, esp], 0; jne L2; { call term; } *)
				(* L2: pop ebx; mov aex,1; ret 12 *)
				CodeSize := 42 + 10 * numMod
			ELSE
				(* push ebx; push ebx; push ebx; mov ebx, modlist; { call body; } { call term; } *)
				(* pop ebx; pop ebx; pop ebx; ret *)
				CodeSize := 12 + 10 * numMod
			END
		ELSE
			IF isDll THEN
				(* push ebx; cmp [12, esp], 1; jne L1; mov ebx, modlist; call main; jp L2 *)
				(* L1: cmp [12, esp], 0; jne L2; call mainTerm; *)
				(* L2: pop ebx; mov aex,1; ret 12 *)
				CodeSize := 41
			ELSE
				(* mov ebx, modlist; jmp main *)
				CodeSize := 10
			END
		END;
(*
		IF isDll THEN
			CodeSize := 24	(* push ebx, esi, edi; mov bx, modlist; call main; pop edi, esi, ebx; mov aex,1; ret 12 *)
		ELSE
			CodeSize := 10	(* mov bx, modlist; jmp main *)
		END
*)
		DataSize := 0; ConSize := 0;
		ImpSize := 0; ImpHSize := 0; ExpSize := 0;
		RelocSize := 0; DllSize := 0; noffixup := 0; maxCode := 0; numImp := 0; numExp := 0;
		mod := modList;
		WHILE mod # NIL DO
			IF ~mod.dll THEN
				mod.file := ThisFile(mod.name);
				IF mod.file # NIL THEN
					R := mod.file.NewReader(R); R.SetPos(0); Read4(x);
					IF x = 6F4F4346H THEN
						Read4(x);
						Read4(mod.hs); Read4(mod.ms); Read4(mod.ds); Read4(mod.cs);
						Read4(mod.vs); RNum(mod.ni); ReadName(name); impdll := FALSE;
						IF mod.ni > 0 THEN
							NEW(mod.imp, mod.ni); x := 0;
							WHILE x < mod.ni DO
								ReadName(name);
								IF name = "$$" THEN
									IF (mod # kernel) & (kernel # NIL) THEN
										mod.imp[x] := kernel
									ELSE
										W.WriteSString("no kernel"); W.WriteLn;
										Log.text.Append(Log.buf); error := TRUE
									END
								ELSIF name[0] = "$" THEN
									i := 1;
									WHILE name[i] # 0X DO name[i-1] := name[i]; INC(i) END;
									name[i-1] := 0X; impdll := TRUE; im := modList;
									WHILE (im # mod) & (im.name # name) DO im := im.next END;
									IF (im = NIL) OR ~im.dll THEN
										NEW(im); im.next := modList; modList := im;
										im.name := name$;
										im.dll := TRUE
									END;
									mod.imp[x] := im;
								ELSE
									im := modList;
									WHILE (im # mod) & (im.name # name) DO im := im.next END;
									IF im # mod THEN
										mod.imp[x] := im;
									ELSE
										W.WriteSString(name);
										W.WriteString(" not present (imported in ");
										W.WriteString(mod.name); W.WriteChar(")");
										W.WriteLn; Log.text.Append(Log.buf); error := TRUE
									END
								END;
								INC(x)
							END
						END;
						IF impdll & ~error THEN CheckDllImports(mod) END;
						mod.ma := ConSize; INC(ConSize, mod.ms + mod.ds);
						mod.va := DataSize; INC(DataSize, mod.vs);
						mod.ca := CodeSize; INC(CodeSize, mod.cs);
						IF mod.cs > maxCode THEN maxCode := mod.cs END
					ELSE
						W.WriteString(mod.name); W.WriteString(": wrong file type"); 
						W.WriteLn; Log.text.Append(Log.buf); error := TRUE
					END;
					mod.file.Close; mod.file := NIL
				ELSE
					W.WriteString(mod.name); W.WriteString(" not found"); 
					W.WriteLn; Log.text.Append(Log.buf); error := TRUE
				END;
				last := mod
			END;
			mod := mod.next
		END;
		IF ~isStatic & (main = NIL) THEN
			W.WriteSString("no main module specified"); W.WriteLn;
			Log.text.Append(Log.buf); error := TRUE
		END;
		(* calculate rva's *)
		IF DataSize = 0 THEN DataSize := 1 END;
		CodeRva := ObjAlign;
		DataRva := CodeRva + (CodeSize + DllSize + (ObjAlign - 1)) DIV ObjAlign * ObjAlign;
		ConRva := DataRva + (DataSize + (ObjAlign - 1)) DIV ObjAlign * ObjAlign;
		RsrcRva := ConRva + (ConSize + (ObjAlign - 1)) DIV ObjAlign * ObjAlign;
		CodeBase := ImageBase + CodeRva;
		DataBase := ImageBase + DataRva;
		ConBase := ImageBase + ConRva;
		(* write dll export adresses *)
		mod := modList; x := 0;
		WHILE mod # NIL DO
			IF mod.dll THEN
				exp := mod.exp; INC(ImpSize, 20);
				WHILE exp # NIL DO exp.adr := x; INC(x, 6); exp := exp.next END
			END;
			mod := mod.next
		END;
		ASSERT(x = DllSize); INC(ImpSize, 20); (* sentinel *)
	END ReadHeaders;
	
	PROCEDURE MenuSize (r: Resource): INTEGER;
		VAR s, i: INTEGER;
	BEGIN
		s := 0;
		WHILE r # NIL DO
			INC(s, 2);
			IF r.local = NIL THEN INC(s, 2) END;
			i := 0; WHILE r.name[i] # 0X DO INC(s, 2); INC(i) END;
			INC(s, 2);
			s := s + MenuSize(r.local);
			r := r.next
		END;
		RETURN s
	END MenuSize;
	
	PROCEDURE PrepResources;
		VAR res, r, s: Resource; n, i, j, t, x: INTEGER; loc: Files.Locator;
	BEGIN
		r := resList;
		WHILE r # NIL DO
			IF r.lid = 0 THEN r.lid := 1033 END;
			IF r.name = "MENU" THEN
				r.typ := 4; r.size := 4 + MenuSize(r.local);
			ELSIF r.name = "ACCELERATOR" THEN
				r.typ := 9; r.size := 0; s := r.local;
				WHILE s # NIL DO INC(r.size, 8); s := s.next END;
			ELSE
				r.file := ThisResFile(r.name);
				IF r.file # NIL THEN
					IF r.typ = -1 THEN	(* typelib *)
						r.typ := 0; r.size := r.file.Length(); r.pos := 0; rsrcName := "TYPELIB"
					ELSE
						R := r.file.NewReader(R); R.SetPos(0); Read2(n);
						IF n = 4D42H THEN	(* bitmap *)
							Read4(n); r.typ := 2; r.size := n - 14; r.pos := 14;
						ELSE
							Read2(x);
							IF x = 1 THEN	(* icon *)
								Read2(n); r.typ := 14; r.size := 6 + 14 * n; r.pos := 0; i := 0;
								WHILE i < n DO
									NEW(s); s.typ := 3; s.id := 10 * r.id + i; s.lid := r.lid; s.name := r.name$; 
									Read4(x); Read4(x); Read4(s.size); Read2(s.pos); Read2(x);
									s.next := resList; resList := s;
									INC(i)
								END
							ELSIF x = 2 THEN	(* cursor *)
								Read2(n); r.typ := 12; r.size := 6 + 14 * n; r.pos := 0; i := 0;
								WHILE i < n DO
									NEW(s); s.typ := 1; s.id := 10 * r.id + i; s.lid := r.lid; s.name := r.name$; 
									Read4(x); Read2(s.x); Read2(s.y); Read4(s.size); INC(s.size, 4); Read2(s.pos); Read2(x);
									s.next := resList; resList := s;
									INC(i)
								END
							ELSE
								Read4(n);
								IF (x = 0) & (n = 20H) THEN	(* resource file *)
									Read4(n); Read4(n); Read4(n); Read4(n); Read4(n); Read4(n);	(* 32 bit marker *)
									Read4(r.size); Read4(n); Read2(i); 
									IF i = 0FFFFH THEN
										Read2(j);
										IF (j >= 4) & ((j <= 11) OR (j = 16)) THEN
											r.typ := j; r.pos := n + 32;
										ELSE
											W.WriteString(r.name); W.WriteString(": invalid type"); W.WriteLn;
											Log.text.Append(Log.buf); error := TRUE
										END
									ELSE
										j := 0;
										WHILE i # 0 DO rsrcName[j] := CHR(i); INC(j); Read2(i) END;
										rsrcName[j] := 0X;
										r.typ := 0; r.pos := n + 32
									END
								ELSE
									W.WriteString(r.name); W.WriteString(": unknown type"); W.WriteLn;
									Log.text.Append(Log.buf); error := TRUE
								END
							END
						END
					END;
					r.file.Close; r.file := NIL
				ELSE
					W.WriteString(r.name); W.WriteString(" not found"); W.WriteLn;
					Log.text.Append(Log.buf); error := TRUE
				END
			END;
			r := r.next
		END;
		res := resList; resList := NIL;	(* sort resources *)
		WHILE res # NIL DO
			r := res; res := res.next;
			IF (resList = NIL) OR (r.typ < resList.typ) OR (r.typ = resList.typ) & ((r.id < resList.id) OR (r.id = resList.id) & (r.lid < resList.lid))
			THEN
				r.next := resList; resList := r
			ELSE
				s := resList;
				WHILE (s.next # NIL) & (r.typ >= s.next.typ)
					& ((r.typ # s.next.typ) OR (r.id >= s.next.id) & ((r.id # s.next.id) OR (r.lid >= s.next.lid))) DO s := s.next END;
				r.next := s.next; s.next := r
			END
		END;
		r := resList; numType := 0; resHSize := 16; t := 0; n := 0;	(* get resource size *)
		WHILE t < LEN(numId) DO numId[t] := 0; INC(t) END;
		WHILE r # NIL DO
			INC(numType); INC(resHSize, 24); t := r.typ;
			WHILE (r # NIL) & (r.typ = t) DO
				INC(numId[t]); INC(resHSize, 24); i := r.id;
				WHILE (r # NIL) & (r.typ = t) & (r.id = i) DO
					INC(resHSize, 24); INC(n, (r.size + 3) DIV 4 * 4); r := r.next
				END
			END
		END;
		IF numId[0] > 0 THEN INC(n, (LEN(rsrcName$) + 1) * 2) END;
		RsrcSize := resHSize + n;
		ImpRva := RsrcRva + (RsrcSize + (ObjAlign - 1)) DIV ObjAlign * ObjAlign
	END PrepResources;
	
	PROCEDURE WriteHeader(VAR name: Files.Name);
	BEGIN
		Out := Files.dir.New(Files.dir.This(""), Files.ask); Ro := Out.NewWriter(Ro); Ro.SetPos(0);

		(* DOS header *)
		Write4(905A4DH); Write4(3); Write4(4); Write4(0FFFFH);
		Write4(0B8H); Write4(0); Write4(40H); Write4(0);
		Write4(0); Write4(0); Write4(0); Write4(0);
		Write4(0); Write4(0); Write4(0); Write4(80H);
		Write4(0EBA1F0EH); Write4(0CD09B400H); Write4(4C01B821H); Write2(21CDH);
		WriteName("This program cannot be run in DOS mode.", 39);
		WriteCh(0DX); WriteCh(0DX); WriteCh(0AX);
		Write4(24H); Write4(0);

		(* Win32 header *)
		WriteName("PE", 4); (* signature bytes *)
		Write2(014CH); (* cpu type (386) *)
		IF isDll THEN
			Write2(7); (* 7 objects *)
		ELSE
			Write2(6); (* 6 objects *)
		END;
		Write4(timeStamp); (* time/date *)
		Write4(0); Write4(0);
		Write2(0E0H); (* NT header size *)
		IF isDll THEN
			Write2(0A38EH); (* library image flags *)
		ELSE
			Write2(838EH); (* program image flags *)
		END;
		Write2(10BH); (* magic (normal ececutable file) *)
		Write2(0502H); (* linker version !!! *) (* BdT: if value < 2.5, Microsoft AppLocker may report "not a valid Win32 application" *)
		Write4(CodeSize); (* code size *)
		Write4(ConSize); (* initialized data size *)
		Write4(DataSize); (* uninitialized data size *)
		entryPos := Ro.Pos();
		Write4(0); (* entry point *)	(* !!! *)
		Write4(CodeRva); (* base of code *)
		Write4(ConRva); (* base of data *)
		Write4(400000H); (* image base *)
		Write4(ObjAlign); (* object align *)
		Write4(FileAlign); (* file align *)
		Write4(3); (* OS version *)
		Write4(4); (* user version *)
		Write4(4); (* subsys version *) (* mf 14.3.04: value changed from 0A0003H to 4. Corrects menubar pixel bug on Windows XP *)
		Write4(0);
		isPos := Ro.Pos();
		Write4(0); (* image size *)	(* !!! *)
		Write4(HeaderSize); (* header size !!! *)
		Write4(0); (* checksum *)
		IF comLine THEN
			Write2(3) (* dos subsystem *)
		ELSE
			Write2(2) (* gui subsystem *)
		END;
		Write2(0); (* dll flags *)
		Write4(200000H); (* stack reserve size *)
		Write4(10000H); (* stack commit size *)
		IF isDll THEN
			Write4(00100000H); (* heap reserve size *)
		ELSE
			Write4(00400000H); (* heap reserve size *)
		END;
		Write4(10000H); (* heap commit size *)
		Write4(0);
		Write4(16); (* num of rva/sizes *)
		hexpPos := Ro.Pos();
		Write4(0); Write4(0); (* export table *)
		himpPos := Ro.Pos();
		Write4(0); Write4(0); (* import table *)	(* !!! *)
		hrsrcPos := Ro.Pos();
		Write4(0); Write4(0); (* resource table *)	(* !!! *)
		Write4(0); Write4(0); (* exception table *)
		Write4(0); Write4(0); (* security table *)
		fixPos := Ro.Pos();
		Write4(0); Write4(0); (* fixup table *)	(* !!! *)
		Write4(0); Write4(0); (* debug table *)
		Write4(0); Write4(0); (* image description *)
		Write4(0); Write4(0); (* machine specific *)
		Write4(0); Write4(0); (* thread local storage *)
		Write4(0); Write4(0); (* ??? *)
		Write4(0); Write4(0); (* ??? *)
		Write4(0); Write4(0); (* ??? *)
		Write4(0); Write4(0); (* ??? *)
		Write4(0); Write4(0); (* ??? *)
		Write4(0); Write4(0); (* ??? *)

		(* object directory *)
		WriteName(".text", 8); (* code object *)
		Write4(0); (* object size (always 0) *)
		codePos := Ro.Pos();
		Write4(0); (* object rva *)
		Write4(0); (* physical size *)
		Write4(0); (* physical offset *)
		Write4(0); Write4(0); Write4(0);
		Write4(60000020H); (* flags: code, exec, read *)

		WriteName(".var", 8); (* variable object *)
		Write4(0); (* object size (always 0) *)
		dataPos := Ro.Pos();
		Write4(0); (* object rva *)
		Write4(0); (* physical size *)
		Write4(0); (* physical offset *)	(* zero! (noinit) *)
		Write4(0); Write4(0); Write4(0);
		Write4(0C0000080H); (* flags: noinit, read, write *)

		WriteName(".data", 8); (* constant object *)
		Write4(0); (* object size (always 0) *)
		conPos := Ro.Pos();
		Write4(0); (* object rva *)
		Write4(0); (* physical size *)
		Write4(0); (* physical offset *)
		Write4(0); Write4(0); Write4(0);
		Write4(0C0000040H); (* flags: data, read, write *)

		WriteName(".rsrc", 8); (* resource object *)
		Write4(0); (* object size (always 0) *)
		rsrcPos := Ro.Pos();
		Write4(0); (* object rva *)
		Write4(0); (* physical size *)
		Write4(0); (* physical offset *)
		Write4(0); Write4(0); Write4(0);
		Write4(0C0000040H); (* flags: data, read, write *)

		WriteName(".idata", 8); (* import object *)
		Write4(0); (* object size (always 0) *)
		impPos := Ro.Pos();
		Write4(0); (* object rva *)
		Write4(0); (* physical size *)
		Write4(0); (* physical offset *)
		Write4(0); Write4(0); Write4(0);
		Write4(0C0000040H); (* flags: data, read, write *)

		IF isDll THEN
			WriteName(".edata", 8); (* export object *)
			Write4(0); (* object size (always 0) *)
			expPos := Ro.Pos();
			Write4(0); (* object rva *)
			Write4(0); (* physical size *)
			Write4(0); (* physical offset *)
			Write4(0); Write4(0); Write4(0);
			Write4(0C0000040H); (* flags: data, read, write *)
		END;

		WriteName(".reloc", 8); (* relocation object *)
		Write4(0); (* object size (always 0) *)
		relPos := Ro.Pos();
		Write4(0); (* object rva *)
		Write4(0); (* physical size *)
		Write4(0); (* physical offset *)
		Write4(0); Write4(0); Write4(0);
		Write4(42000040H); (* flags: data, read, ? *)
	END WriteHeader;
	
	PROCEDURE SearchObj (mod: Module; VAR name: ARRAY OF SHORTCHAR; m, fp, opt: INTEGER; VAR adr: INTEGER);
		VAR dir, len, ntab, f, id, l, r, p, n, i, j: INTEGER; nch, och: SHORTCHAR;
	BEGIN
		Get(mod, mod.ms + modExports, dir); DEC(dir, ConBase + mod.ma); Get(mod, dir, len); INC(dir, 4);
		Get(mod, mod.ms + modNames, ntab); DEC(ntab, ConBase + mod.ma);
		IF name # "" THEN
			l := 0; r := len;
			WHILE l < r DO	(* binary search *)
				n := (l + r) DIV 2; p := dir + n * 16;
				Get(mod, p + 8, id);
				i := 0; j := ntab + id DIV 256; nch := name[0]; och := SHORT(CHR(mod.data[j]));
				WHILE (nch = och) & (nch # 0X) DO INC(i); INC(j); nch := name[i]; och := SHORT(CHR(mod.data[j])) END;
				IF och = nch THEN
					IF id MOD 16 = m THEN Get(mod, p, f);
						IF m = mTyp THEN
							IF ODD(opt) THEN Get(mod, p + 4, f) END;
							IF (opt > 1) & (id DIV 16 MOD 16 # mExported) THEN
								W.WriteString(mod.name); W.WriteChar("."); W.WriteSString(name);
								W.WriteString(" imported from "); W.WriteString(impg.name);
								W.WriteString(" has wrong visibility"); W.WriteLn; error := TRUE
							END;
							Get(mod, p + 12, adr)
						ELSIF m = mVar THEN
							Get(mod, p + 4, adr); INC(adr, DataBase + mod.va)
						ELSIF m = mProc THEN
							Get(mod, p + 4, adr); INC(adr, CodeBase + mod.ca)
						END;
						IF f # fp THEN
							W.WriteString(mod.name); W.WriteChar("."); W.WriteSString(name);
							W.WriteString(" imported from "); W.WriteString(impg.name);
							W.WriteString(" has wrong fprint"); W.WriteLn; error := TRUE
						END
					ELSE
						W.WriteString(mod.name); W.WriteChar("."); W.WriteSString(name);
						W.WriteString(" imported from "); W.WriteString(impg.name);
						W.WriteString(" has wrong class"); W.WriteLn; error := TRUE
					END;
					RETURN
				END;
				IF och < nch THEN l := n + 1 ELSE r := n END
			END;
			W.WriteString(mod.name); W.WriteChar("."); W.WriteSString(name);
			W.WriteString(" not found (imported from "); W.WriteString(impg.name);
			W.WriteChar(")"); W.WriteLn; error := TRUE
		ELSE (* anonymous type *)
			WHILE len > 0 DO
				Get(mod, dir + 4, f); Get(mod, dir + 8, id);
				IF (f = fp) & (id MOD 16 = mTyp) & (id DIV 256 = 0) THEN
					Get(mod, dir + 12, adr); RETURN
				END;
				DEC(len); INC(dir, 16)
			END;
			W.WriteString("anonymous type in "); W.WriteString(mod.name);
			W.WriteString(" not found"); W.WriteLn; error := TRUE
		END
	END SearchObj;
	
	PROCEDURE CollectExports (mod: Module);
		VAR dir, len, ntab, id, i, j, n: INTEGER; e, exp: Export;
	BEGIN
		Get(mod, mod.ms + modExports, dir); DEC(dir, ConBase + mod.ma); Get(mod, dir, len); INC(dir, 4);
		Get(mod, mod.ms + modNames, ntab); DEC(ntab, ConBase + mod.ma); n := 0;
		WHILE n < len DO
			Get(mod, dir + 8, id);
			IF (id DIV 16 MOD 16 # mInternal) & ((id MOD 16 = mProc) OR (id MOD 16 = mVar))THEN	(* exported procedure & var *)
				NEW(exp);
				i := 0; j := ntab + id DIV 256;
				WHILE mod.data[j] # 0 DO exp.name[i] := SHORT(CHR(mod.data[j])); INC(i); INC(j) END;
				exp.name[i] := 0X;
				Get(mod, dir + 4, exp.adr);
				IF id MOD 16 = mProc THEN INC(exp.adr, CodeRva + mod.ca)
				ELSE ASSERT(id MOD 16 = mVar); INC(exp.adr, DataRva + mod.va)
				END;
				IF (firstExp = NIL) OR (exp.name < firstExp.name) THEN
					exp.next := firstExp; firstExp := exp;
					IF lastExp = NIL THEN lastExp := exp END
				ELSE
					e := firstExp;
					WHILE (e.next # NIL) & (exp.name > e.next.name) DO e := e.next END;
					exp.next := e.next; e.next := exp;
					IF lastExp = e THEN lastExp := exp END
				END;
				INC(numExp);
			END;
			INC(n); INC(dir, 16)
		END
	END CollectExports;

	PROCEDURE WriteTermCode (m: Module; i: INTEGER);
		VAR x: INTEGER;
	BEGIN
		IF m # NIL THEN
			IF m.dll THEN WriteTermCode(m.next, i)
			ELSE
				IF isStatic THEN WriteTermCode(m.next, i + 1) END;
				Get(m, m.ms + modTerm, x);	(* terminator address in mod desc*)
				IF x = 0 THEN
					WriteCh(005X); Write4(0)	(* add EAX, 0 (nop) *)
				ELSE
					WriteCh(0E8X); Write4(x - lastTerm + 5 * i - CodeBase)	(* call term *)
				END
			END
		END
	END WriteTermCode;
	
	PROCEDURE WriteCode;
		VAR mod, m: Module; i, x, a, fp, opt: INTEGER; exp: Export; name: Name;
	BEGIN
		IF isStatic THEN
			WriteCh(053X);	(* push ebx *)
			a := 1;
			IF isDll THEN
				WriteCh(083X); WriteCh(07CX); WriteCh(024X); WriteCh(00CX); WriteCh(001X);	(* cmp [12, esp], 1 *)
				WriteCh(00FX); WriteCh(085X); Write4(10 + 5 * numMod);	(* jne L1 *)
				INC(a, 11)
			ELSE
				WriteCh(053X); WriteCh(053X);	(* push ebx; push ebx *)
				INC(a, 2)
			END;
			WriteCh(0BBX); Write4(ConBase + last.ma + last.ms); Reloc(CodeBase + a + 1);	(* mov bx, modlist *)
			INC(a, 5); m := modList;
			WHILE m # NIL DO
				IF ~m.dll THEN
					WriteCh(0E8X); INC(a, 5); Write4(m.ca - a)	(* call body *)
				END;
				m := m.next
			END;
			IF isDll THEN
				WriteCh(0E9X); Write4(11 + 5 * numMod);	(* jp L2 *)
				WriteCh(083X); WriteCh(07CX); WriteCh(024X); WriteCh(00CX); WriteCh(000X);	(* L1: cmp [12, esp], 0 *)
				WriteCh(00FX); WriteCh(085X); Write4(5 * numMod);	(* jne L2 *)
				INC(a, 16)
			END;
			termPos := Ro.Pos(); i := 0;
			WHILE i < numMod DO	(* nop for call terminator *)
				WriteCh(02DX); Write4(0);	(* sub EAX, 0 *)
				INC(i); INC(a, 5)
			END;
			lastTerm := a;
			WriteCh(05BX); 	(* L2: pop ebx *)
			IF isDll THEN
				WriteCh(0B8X); Write4(1);	(* mov eax,1 *)
				WriteCh(0C2X); Write2(12)	(* ret 12 *)
			ELSE
				WriteCh(05BX); WriteCh(05BX);	(* pop ebx; pop ebx *)
				WriteCh(0C3X)	(* ret *)
			END
		ELSIF isDll THEN
			WriteCh(053X);	(* push ebx *)
			WriteCh(083X); WriteCh(07CX); WriteCh(024X); WriteCh(00CX); WriteCh(001X);	(* cmp [12, esp], 1 *)
			WriteCh(075X); WriteCh(SHORT(CHR(12)));	(* jne L1 *)
			WriteCh(0BBX); Write4(ConBase + last.ma + last.ms); Reloc(CodeBase + 9);	(* mov bx, modlist *)
			WriteCh(0E8X); Write4(main.ca - 18);	(* call main *)
			WriteCh(0EBX); WriteCh(SHORT(CHR(12)));	(* jp L2 *)
			WriteCh(083X); WriteCh(07CX); WriteCh(024X); WriteCh(00CX); WriteCh(000X);	(* L1: cmp [12, esp], 0 *)
			WriteCh(075X); WriteCh(SHORT(CHR(5)));	(* jne L2 *)
			termPos := Ro.Pos();
			WriteCh(02DX); Write4(0);	(* sub EAX, 0 *)	(* nop for call terminator *)
			lastTerm := 32;
			WriteCh(05BX); 	(* L2: pop ebx *)
			WriteCh(0B8X); Write4(1);	(* mov eax,1 *)
			WriteCh(0C2X); Write2(12)	(* ret 12 *)
		ELSE
			WriteCh(0BBX); Write4(ConBase + last.ma + last.ms); Reloc(CodeBase + 1);	(* mov bx, modlist *)
			WriteCh(0E9X); Write4(main.ca - 10);	(* jmp main *)
		END;
		NEW(code, maxCode);
		mod := modList;
		WHILE mod # NIL DO impg := mod; impd := mod;
			IF ~mod.dll THEN
				mod.file := ThisFile(mod.name);
				R := mod.file.NewReader(R); R.SetPos(mod.hs);
				NEW(mod.data, mod.ms + mod.ds);
				R.ReadBytes(mod.data^, 0, mod.ms + mod.ds);
				R.ReadBytes(code^, 0, mod.cs);
				RNum(x);
				IF x # 0 THEN
					IF (mod # kernel) & (kernel # NIL) THEN
						SearchObj(kernel, newRec, mProc, NewRecFP, -1, a); Fixup0(x, a)
					ELSE
						W.WriteSString("no kernel"); W.WriteLn;
						Log.text.Append(Log.buf); error := TRUE; RETURN
					END
				END;
				RNum(x);
				IF x # 0 THEN
					IF (mod # kernel) & (kernel # NIL) THEN
						SearchObj(kernel, newArr, mProc, NewArrFP, -1, a); Fixup0(x, a)
					ELSE
						W.WriteSString("no kernel"); W.WriteLn;
						Log.text.Append(Log.buf); error := TRUE; RETURN
					END
				END;
				Fixup(ConBase + mod.ma);
				Fixup(ConBase + mod.ma + mod.ms);
				Fixup(CodeBase + mod.ca);
				Fixup(DataBase + mod.va); i := 0;
				WHILE i < mod.ni DO
					m := mod.imp[i]; impd := m; RNum(x);
					WHILE x # 0 DO
						ReadName(name); RNum(fp); opt := 0;
						IF x = mTyp THEN RNum(opt) END;
						IF m.dll THEN
							IF x = mProc THEN exp := m.exp;
								WHILE exp.name # name DO exp := exp.next END;
								a := exp.adr + CodeBase + CodeSize
							END
						ELSE
							SearchObj(m, name, x, fp, opt, a)
						END;
						IF x # mConst THEN Fixup(a) END;
						RNum(x)
					END;
					IF ~m.dll THEN
						Get(mod, mod.ms + modImports, x); DEC(x, ConBase + mod.ma); INC(x, 4 * i);
						Put(mod, x, ConBase + m.ma + m.ms);	(* imp ref *)
						Reloc(ConBase + mod.ma + x);
						Get(m, m.ms + modRefcnt, x); Put(m, m.ms + modRefcnt, x + 1)	(* inc ref count *)
					END;
					INC(i)
				END;
				Ro.WriteBytes(code^, 0, mod.cs);
				IF mod.intf THEN CollectExports(mod) END;
				mod.file.Close; mod.file := NIL
			END;
			mod := mod.next
		END;
		(* dll links *)
		mod := modList; ImpHSize := ImpSize;
		WHILE mod # NIL DO
			IF mod.dll THEN
				exp := mod.exp; 
				WHILE exp # NIL DO
					WriteCh(0FFX); WriteCh(25X); Write4(ImageBase + ImpRva + ImpSize);	(* JMP indirect *)
					Reloc(CodeBase + CodeSize + exp.adr + 2);
					INC(ImpSize, 4); INC(numImp); exp := exp.next
				END;
				INC(ImpSize, 4); INC(numImp) (* sentinel *)
			END;
			mod := mod.next
		END
	END WriteCode;
	
	PROCEDURE WriteConst;
		VAR mod, last: Module; x: INTEGER;
	BEGIN
		mod := modList; last := NIL;
		WHILE mod # NIL DO
			IF ~mod.dll THEN
				IF last # NIL THEN
					Put(mod, mod.ms, ConBase + last.ma + last.ms);	(* mod list *)
					Reloc(ConBase + mod.ma + mod.ms);
				END;
				Get(mod, mod.ms + modOpts, x);
				IF isStatic THEN INC(x, 10000H) END;	(* set init bit (16) *)
				IF isDll THEN INC(x, 1000000H) END;	(* set dll bit (24) *)
				Put(mod, mod.ms + modOpts, x);
				Ro.WriteBytes(mod.data^, 0, mod.ms + mod.ds);
				last := mod
			END;
			mod := mod.next
		END
	END WriteConst;
	
	PROCEDURE WriteResDir (n, i: INTEGER);
	BEGIN
		Write4(0);	(* flags *)
		Write4(timeStamp);
		Write4(0);	(* version *)
		Write2(n);	(* name entries *)
		Write2(i);	(* id entries *)
	END WriteResDir;
	
	PROCEDURE WriteResDirEntry (id, adr: INTEGER; dir: BOOLEAN);
	BEGIN
		IF id = 0 THEN id := resHSize + 80000000H END;	(* name Rva *)
		Write4(id);
		IF dir THEN Write4(adr + 80000000H) ELSE Write4(adr) END
	END WriteResDirEntry;
	
	PROCEDURE WriteMenu (res: Resource);
		VAR f, i: INTEGER;
	BEGIN
		WHILE res # NIL DO
			IF res.next = NIL THEN f := 80H ELSE f := 0 END;
			IF 29 IN res.opts THEN INC(f, 1) END;	(* = grayed *)
			IF 13 IN res.opts THEN INC(f, 2) END;	(* - inctive *)
			IF 3 IN res.opts THEN INC(f, 4) END;	(* # bitmap *)
			IF 10 IN res.opts THEN INC(f, 8) END;	(* * checked *)
			IF 1 IN res.opts THEN INC(f, 20H) END;	(* ! menubarbreak *)
			IF 15 IN res.opts THEN INC(f, 40H) END;	(* / menubreak *)
			IF 31 IN res.opts THEN INC(f, 100H) END;	(* ? ownerdraw *)
			IF res.local # NIL THEN Write2(f + 10H) ELSE Write2(f); Write2(res.id) END;
			i := 0; WHILE res.name[i] # 0X DO Write2(ORD(res.name[i])); INC(i) END;
			Write2(0);
			WriteMenu(res.local);
			res := res.next
		END
	END WriteMenu;
	
	PROCEDURE WriteResource;
		VAR r, s: Resource; i, t, a, x, n, nlen, nsize: INTEGER;
	BEGIN
		IF numId[0] > 0 THEN WriteResDir(1, numType - 1); nlen := LEN(rsrcName$); nsize := (nlen + 1) * 2;
		ELSE WriteResDir(0, numType)
		END;
		a := 16 + 8 * numType; t := 0;
		WHILE t < LEN(numId) DO
			IF numId[t] > 0 THEN WriteResDirEntry(t, a, TRUE); INC(a, 16 + 8 * numId[t]) END;
			INC(t)
		END;
		r := resList; t := -1;
		WHILE r # NIL DO
			IF t # r.typ THEN t := r.typ; WriteResDir(0, numId[t]) END;
			WriteResDirEntry(r.id, a, TRUE); INC(a, 16); i := r.id;
			WHILE (r # NIL) & (r.typ = t) & (r.id = i) DO INC(a, 8); r := r.next END
		END;
		r := resList;
		WHILE r # NIL DO
			n := 0; s := r;
			WHILE (s # NIL) & (s.typ = r.typ) & (s.id = r.id) DO INC(n); s := s.next END;
			WriteResDir(0, n);
			WHILE r # s DO WriteResDirEntry(r.lid, a, FALSE); INC(a, 16); r := r.next END
		END;
		ASSERT(a = resHSize);
		IF numId[0] > 0 THEN INC(a, nsize) END;	(* TYPELIB string *)
		r := resList;
		WHILE r # NIL DO
			Write4(a + RsrcRva); INC(a, (r.size + 3) DIV 4 * 4);
			Write4(r.size);
			Write4(0); Write4(0);
			r := r.next
		END;
		ASSERT(a = RsrcSize);
		IF numId[0] > 0 THEN
			Write2(nlen); i := 0;
			WHILE rsrcName[i] # 0X DO Write2(ORD(rsrcName[i])); INC(i) END
		END;
		r := resList;
		WHILE r # NIL DO
			IF r.typ = 4 THEN	(* menu *)
				Write2(0); Write2(0);
				WriteMenu(r.local);
				WHILE Ro.Pos() MOD 4 # 0 DO WriteCh(0X) END
			ELSIF r.typ = 9 THEN	(* accelerator *)
				s := r.local;
				WHILE s # NIL DO
					i := 0; a := 0;
					IF 10 IN s.opts THEN INC(a, 4) END;	(* * shift *)
					IF 16 IN s.opts THEN INC(a, 8) END;	(* ^ ctrl *)
					IF 0 IN s.opts THEN INC(a, 16) END;	(* @ alt *)
					IF 13 IN s.opts THEN INC(a, 2) END;	(* - noinv *)
					IF s.next = NIL THEN INC(a, 80H) END;
					IF (s.name[0] = "v") & (s.name[1] # 0X) THEN
						s.name[0] := " "; Strings.StringToInt(s.name, x, n); INC(a, 1)
					ELSE x := ORD(s.name[0])
					END;
					Write2(a); Write2(x); Write2(s.id); Write2(0); s := s.next
				END
			ELSE
				r.file := ThisResFile(r.name);
				IF r.file # NIL THEN
					R := r.file.NewReader(R); R.SetPos(r.pos); i := 0;
					IF r.typ = 12 THEN	(* cursor group *)
						Read4(x); Write4(x); Read2(n); Write2(n);
						WHILE i < n DO
							Read4(x); Write2(x MOD 256); Write2(x DIV 256 MOD 256 * 2);
							Write2(1); Write2(1); Read4(x);	(* ??? *)
							Read4(x); Write4(x + 4); Read4(x); Write2(r.id * 10 + i); INC(i)
						END;
						IF ~ODD(n) THEN Write2(0) END
					ELSIF r.typ = 14 THEN	(* icon group *)
						Read4(x); Write4(x); Read2(n); Write2(n);
						WHILE i < n DO
							Read2(x); Write2(x); Read2(x);
							IF (13 IN r.opts) & (x = 16) THEN x := 4 END;
							Write2(x);
							a := x MOD 256; Read4(x); Write2(1);
							IF a <= 2 THEN Write2(1)
							ELSIF a <= 4 THEN Write2(2)
							ELSIF a <= 16 THEN Write2(4)
							ELSE Write2(8)
							END;
							Read4(x);
							IF (13 IN r.opts) & (x = 744) THEN x := 440 END;
							IF (13 IN r.opts) & (x = 296) THEN x := 184 END;
							Write4(x); Read4(x); Write2(r.id * 10 + i); INC(i)
						END;
						IF ~ODD(n) THEN Write2(0) END
					ELSE
						IF r.typ = 1 THEN Write2(r.x); Write2(r.y); i := 4 END;	(* cursor hot spot *)
						WHILE i < r.size DO Read4(x); Write4(x); INC(i, 4) END
					END;
					r.file.Close; r.file := NIL
				END
			END;
			r := r.next
		END
	END WriteResource;

	PROCEDURE Insert(VAR name: ARRAY OF SHORTCHAR; VAR idx: INTEGER; hint: INTEGER);
		VAR i: INTEGER;
	BEGIN
		IF hint >= 0 THEN
			ntab[idx] := SHORT(CHR(hint)); INC(idx);
			ntab[idx] := SHORT(CHR(hint DIV 256)); INC(idx);
		END;
		i := 0;
		WHILE name[i] # 0X DO ntab[idx] := name[i]; INC(idx); INC(i) END;
		IF (hint = -1) & ((ntab[idx-4] # ".") OR (CAP(ntab[idx-3]) # "D") OR (CAP(ntab[idx-2]) # "L") OR (CAP(ntab[idx-1]) # "L")) THEN
			ntab[idx] := "."; INC(idx);
			ntab[idx] := "d"; INC(idx);
			ntab[idx] := "l"; INC(idx);
			ntab[idx] := "l"; INC(idx);
		END;
		ntab[idx] := 0X; INC(idx);
		IF ODD(idx) THEN ntab[idx] := 0X; INC(idx) END
	END Insert;

	PROCEDURE WriteImport;
		VAR i, lt, at, nt, ai, ni: INTEGER; mod: Module; exp: Export; ss: ARRAY 256 OF SHORTCHAR;
	BEGIN
		IF numImp > 0 THEN NEW(atab, numImp) END;
		IF numExp > numImp THEN i := numExp ELSE i := numImp END;
		IF i > 0 THEN NEW(ntab, 40 * i) END;
		at := ImpRva + ImpHSize; ai := 0; ni := 0;
		lt := ImpRva + ImpSize; nt := lt + ImpSize - ImpHSize;
		mod := modList;
		WHILE mod # NIL DO
			IF mod.dll THEN
				Write4(lt); (* lookup table rva *)
				Write4(0); (* time/data (always 0) *)
				Write4(0); (* version (always 0) *)
				Write4(nt + ni); (* name rva *)
				ss := SHORT(mod.name$); Insert(ss, ni, -1);
				Write4(at); (* addr table rva *)
				exp := mod.exp;
				WHILE exp # NIL DO
					atab[ai] := nt + ni; (* hint/name rva *)
					Insert(exp.name, ni, 0);
					INC(lt, 4); INC(at, 4); INC(ai); exp := exp.next
				END;
				atab[ai] := 0; INC(lt, 4); INC(at, 4); INC(ai)
			END;
			mod := mod.next
		END;
		Write4(0); Write4(0); Write4(0); Write4(0); Write4(0);
		i := 0;
		WHILE i < ai DO Write4(atab[i]); INC(i) END; (* address table *)
		i := 0;
		WHILE i < ai DO Write4(atab[i]); INC(i) END; (* lookup table *)
		i := 0;
		WHILE i < ni DO WriteCh(ntab[i]); INC(i) END;
		ASSERT(ai * 4 = ImpSize - ImpHSize);
		INC(ImpSize, ai * 4 + ni);
		ExpRva := ImpRva + (ImpSize + (ObjAlign - 1)) DIV ObjAlign * ObjAlign;
		RelocRva := ExpRva;
	END WriteImport;
	
	PROCEDURE WriteExport (VAR name: ARRAY OF CHAR);
		VAR i, ni: INTEGER; e: Export; ss: ARRAY 256 OF SHORTCHAR;
	BEGIN
		Write4(0);	(* flags *)
		Write4(timeStamp);	(* time stamp *)
		Write4(0);	(* version *)
		Write4(ExpRva + 40 + 10 * numExp);	(* name rva *)
		Write4(1);	(* ordinal base *)
		Write4(numExp);	(* # entries *)
		Write4(numExp);	(* # name ptrs *)
		Write4(ExpRva + 40);	(* address table rva *)
		Write4(ExpRva + 40 + 4 * numExp);	(* name ptr table rva *)
		Write4(ExpRva + 40 + 8 * numExp);	(* ordinal table rva *)
		ExpSize := 40 + 10 * numExp;
		(* adress table *)
		e := firstExp;
		WHILE e # NIL DO Write4(e.adr); e := e.next END;
		(* name ptr table *)
		ni := 0; e := firstExp;
		ss := SHORT(name$); Insert(ss, ni, -2);
		WHILE e # NIL DO
			Write4(ExpRva + ExpSize + ni); Insert(e.name, ni, -2); e := e.next
		END;
		(* ordinal table *)
		i := 0;
		WHILE i < numExp DO Write2(i); INC(i) END;
		(* name table *)
		i := 0;
		WHILE i < ni DO WriteCh(ntab[i]); INC(i) END;
		ExpSize := (ExpSize + ni + 15) DIV 16 * 16;
		RelocRva := ExpRva + (ExpSize + (ObjAlign - 1)) DIV ObjAlign * ObjAlign;
	END WriteExport;

	PROCEDURE Sort (l, r: INTEGER);
		VAR i, j, x, t: INTEGER;
	BEGIN
		i := l; j := r; x := fixups[(l + r) DIV 2];
		REPEAT
			WHILE fixups[i] < x DO INC(i) END;
			WHILE fixups[j] > x DO DEC(j) END;
			IF i <= j THEN t := fixups[i]; fixups[i] := fixups[j]; fixups[j] := t; INC(i); DEC(j) END
		UNTIL i > j;
		IF l < j THEN Sort(l, j) END;
		IF i < r THEN Sort(i, r) END
	END Sort;

	PROCEDURE WriteReloc;
		VAR i, j, h, a, p: INTEGER;
	BEGIN
		Sort(0, noffixup - 1); i := 0;
		WHILE i < noffixup DO
			p := fixups[i] DIV 4096 * 4096; j := i; a := p + 4096;
			WHILE (j < noffixup) & (fixups[j] < a) DO INC(j) END;
			Write4(p - ImageBase); (* page rva *)
			h := 8 + 2 * (j - i);
			Write4(h + h MOD 4); (* block size *)
			INC(RelocSize, h);
			WHILE i < j DO Write2(fixups[i] - p + 3 * 4096); INC(i) END; (* long fix *)
			IF h MOD 4 # 0 THEN Write2(0); INC(RelocSize, 2) END
		END;
		Write4(0); Write4(0); INC(RelocSize, 8);
		ImagesSize := RelocRva + (RelocSize + (ObjAlign - 1)) DIV ObjAlign * ObjAlign;
	END WriteReloc;
	
	PROCEDURE Align(VAR pos: INTEGER);
	BEGIN
		WHILE Ro.Pos() MOD FileAlign # 0 DO WriteCh(0X) END;
		pos := Ro.Pos()
	END Align;
	
	PROCEDURE WriteOut (VAR name: Files.Name);
		VAR res, codepos, conpos, rsrcpos, imppos, exppos, relpos, relend, end: INTEGER;
	BEGIN
		IF ~error THEN Align(codepos); WriteCode END;
		IF ~error THEN Align(conpos); WriteConst END;
		IF ~error THEN Align(rsrcpos); WriteResource END;
		IF ~error THEN Align(imppos); WriteImport END;
		IF ~error & isDll THEN Align(exppos); WriteExport(name) END;
		IF ~error THEN Align(relpos); WriteReloc END;
		relend := Ro.Pos() - 8; Align(end);
		
		IF ~error THEN
			Ro.SetPos(entryPos); Write4(CodeRva);
			Ro.SetPos(isPos); Write4(ImagesSize);
			IF isDll THEN
				Ro.SetPos(hexpPos); Write4(ExpRva); Write4(ExpSize);
			END;
			Ro.SetPos(himpPos); Write4(ImpRva); Write4(ImpHSize);
			Ro.SetPos(hrsrcPos); Write4(RsrcRva); Write4(RsrcSize);
			Ro.SetPos(fixPos); Write4(RelocRva); Write4(relend - relpos);
	
			Ro.SetPos(codePos); Write4(CodeRva); Write4(conpos - HeaderSize); Write4(HeaderSize);
			Ro.SetPos(dataPos); Write4(DataRva); Write4((DataSize + (FileAlign-1)) DIV FileAlign * FileAlign);
			Ro.SetPos(conPos); Write4(ConRva); Write4(rsrcpos - conpos); Write4(conpos);
			Ro.SetPos(rsrcPos); Write4(RsrcRva); Write4(imppos - rsrcpos); Write4(rsrcpos);
			IF isDll THEN
				Ro.SetPos(impPos); Write4(ImpRva); Write4(exppos - imppos); Write4(imppos);
				Ro.SetPos(expPos); Write4(ExpRva); Write4(relpos - exppos); Write4(exppos)
			ELSE
				Ro.SetPos(impPos); Write4(ImpRva); Write4(relpos - imppos); Write4(imppos);
			END;
			Ro.SetPos(relPos); Write4(RelocRva); Write4(end - relpos); Write4(relpos);
			IF isStatic THEN
				Ro.SetPos(termPos); WriteTermCode(modList, 0)
			ELSIF isDll THEN
				Ro.SetPos(termPos); WriteTermCode(main, 0)
			END
		END;
		
		IF ~error THEN
			Out.Register(name, "exe", Files.ask, res);
			IF res # 0 THEN error := TRUE END
		END
	END WriteOut;
	
	PROCEDURE ScanRes (VAR S: TextMappers.Scanner; end: INTEGER; VAR list: Resource);
		VAR res, tail: Resource; n: INTEGER;
	BEGIN
		tail := NIL;
		WHILE (S.start < end) & (S.type = TextMappers.int) DO
			NEW(res); res.id := S.int; S.Scan;
			IF (S.type = TextMappers.char) & (S.char = "[") THEN
				S.Scan;
				IF S.type = TextMappers.int THEN res.lid := S.int; S.Scan END;
				IF (S.type = TextMappers.char) & (S.char = "]") THEN S.Scan
				ELSE W.WriteSString("missing ']'"); error := TRUE
				END
			END;
			WHILE S.type = TextMappers.char DO
				IF S.char = "@" THEN n := 0
				ELSIF S.char = "^" THEN n := 16
				ELSIF S.char = "~" THEN n := 17
				ELSIF S.char <= "?" THEN n := ORD(S.char) - ORD(" ")
				END;
				INCL(res.opts, n); S.Scan
			END;
			IF S.type = TextMappers.string THEN
				res.name := S.string$; S.Scan;
				IF (S.type = TextMappers.char) & (S.char = ".") THEN S.Scan;
					IF S.type = TextMappers.string THEN
						IF (S.string = "tlb") OR (S.string = "TLB") THEN res.typ := -1 END;
						Kernel.MakeFileName(res.name, S.string); S.Scan
					END
				END;
				IF (S.type = TextMappers.char) & (S.char = "(") THEN S.Scan;
					ScanRes(S, end, res.local);
					IF (S.type = TextMappers.char) & (S.char = ")") THEN S.Scan
					ELSE W.WriteSString("missing ')'"); error := TRUE
					END
				END;
				IF tail = NIL THEN list := res ELSE tail.next := res END;
				tail := res
			ELSE
				W.WriteSString("wrong resource name"); error := TRUE
			END
		END;
	END ScanRes;

	PROCEDURE LinkIt;
		VAR S: TextMappers.Scanner; name: Files.Name; mod: Module; end: INTEGER;
	BEGIN
		comLine := FALSE;
		modList := NIL; kernel := NIL; main := NIL;
		last := NIL; impg := NIL; impd := NIL; resList := NIL;
		firstExp := NIL; lastExp := NIL;
		NEW(fixups, FixLen);
		Dialog.ShowStatus("linking");
		timeStamp := TimeStamp();
		error := FALSE; modList := NIL; resList := NIL;
		IF DevCommanders.par = NIL THEN RETURN END;
		S.ConnectTo(DevCommanders.par.text);
		S.SetPos(DevCommanders.par.beg);
		end := DevCommanders.par.end;
		DevCommanders.par := NIL;
		W.ConnectTo(Log.buf); S.Scan;
		IF S.type = TextMappers.string THEN
			IF S.string = "dos" THEN comLine := TRUE; S.Scan END;
			name := S.string$; S.Scan;
			IF (S.type = TextMappers.char) & (S.char = ".") THEN S.Scan;
				IF S.type = TextMappers.string THEN
					Kernel.MakeFileName(name, S.string); S.Scan
				END
			ELSE Kernel.MakeFileName(name, "EXE");
			END;
			IF (S.type = TextMappers.char) & (S.char = ":") THEN S.Scan;
				IF (S.type = TextMappers.char) & (S.char = "=") THEN S.Scan;
					WHILE (S.start < end) & (S.type = TextMappers.string) DO
						NEW(mod); mod.name := S.string$;
						mod.next := modList; modList := mod;
						S.Scan;
						WHILE (S.start < end) & (S.type = TextMappers.char) &
							((S.char = "*") OR (S.char = "+") OR (S.char = "$") OR (S.char = "#")) DO
							IF S.char = "*" THEN mod.dll := TRUE
							ELSIF S.char = "+" THEN kernel := mod
							ELSIF S.char = "$" THEN main := mod
							ELSE mod.intf := TRUE;
								IF ~isDll THEN
									W.WriteSString("Exports from Exe not possible. Use LinkDll or LinkDynDll.");
									W.WriteLn; Log.text.Append(Log.buf); error := TRUE
								END
							END;
							S.Scan
						END
					END;
					ScanRes(S, end, resList);
					ReadHeaders;
					PrepResources;
					IF ~error THEN WriteHeader(name) END;
					IF ~error THEN WriteOut(name) END;
					IF ~error THEN	
						W.WriteString(name); W.WriteString(" written  ");
						W.WriteInt(Out.Length()); W.WriteString("  "); W.WriteInt(CodeSize)
					END
				ELSE W.WriteString(" := missing")
				END
			ELSE W.WriteString(" := missing")
			END;
			W.WriteLn; Log.text.Append(Log.buf)
		END;
		IF error THEN Dialog.ShowStatus("failed") ELSE Dialog.ShowStatus("ok") END;
		W.ConnectTo(NIL); S.ConnectTo(NIL);
		modList := NIL; kernel := NIL; main := NIL; firstExp := NIL; lastExp := NIL;
		last := NIL; impg := NIL; impd := NIL; resList := NIL; code := NIL; atab := NIL; ntab := NIL;
		fixups := NIL
	END LinkIt;
	
	PROCEDURE Link*;
	BEGIN
		isDll := FALSE; isStatic := FALSE;
		LinkIt
	END Link;
	
	PROCEDURE LinkExe*;
	BEGIN
		isDll := FALSE; isStatic := TRUE;
		LinkIt
	END LinkExe;
	
	PROCEDURE LinkDll*;
	BEGIN
		isDll := TRUE; isStatic := TRUE;
		LinkIt
	END LinkDll;
	
	PROCEDURE LinkDynDll*;
	BEGIN
		isDll := TRUE; isStatic := FALSE;
		LinkIt
	END LinkDynDll;
	
(*
	PROCEDURE Show*;
		VAR S: TextMappers.Scanner; name: Name; mod: Module; t: TextModels.Model;
	BEGIN
		t := TextViews.FocusText(); IF t = NIL THEN RETURN END;
		W.ConnectTo(Log.buf); S.ConnectTo(t); S.Scan;
		IF S.type = TextMappers.string THEN
			mod := modList;
			WHILE (mod # NIL) & (mod.name # S.string) DO mod := mod.next END;
			IF mod # NIL THEN
				W.WriteString(S.string);
				W.WriteString(" ca = ");
				W.WriteIntForm(CodeBase + mod.ca, TextMappers.hexadecimal, 8, "0", TRUE);
				W.WriteLn; Log.text.Append(Log.buf)
			END
		END;
		W.ConnectTo(NIL); S.ConnectTo(NIL)
	END Show;
*)
		
BEGIN
	newRec := "NewRec"; newArr := "NewArr"
END DevLinker.


DevLinker.Link Usekrnl.exe := TestKernel$+ Usekrnl ~	"DevDecExe.Decode('', 'Usekrnl.exe')"

DevLinker.LinkDynDll MYDLL.dll := TestKernel+ MYDLL$# ~	"DevDecExe.Decode('', 'MYDLL.dll')"

DevLinker.LinkExe Usekrnl.exe := TestKernel+ Usekrnl ~	"DevDecExe.Decode('', 'Usekrnl.exe')"

DevLinker.LinkDll MYDLL.dll := TestKernel+ MYDLL# ~	"DevDecExe.Decode('', 'MYDLL.dll')"


MODULE TestKernel;
	IMPORT KERNEL32;

	PROCEDURE Beep*;
	BEGIN
		KERNEL32.Beep(500, 200)
	END Beep;
	
BEGIN
CLOSE
	KERNEL32.ExitProcess(0)
END TestKernel.

MODULE Usekrnl;
(* empty windows application using BlackBox Kernel *)
(* Ominc  *)

	IMPORT KERNEL32, USER32, GDI32, S := SYSTEM, Kernel := TestKernel;
	
	VAR Instance, MainWnd: USER32.Handle;
		
	PROCEDURE WndHandler (wnd, message, wParam, lParam: INTEGER): INTEGER;
		VAR res: INTEGER; ps: USER32.PaintStruct; dc: GDI32.Handle;
	BEGIN
		IF message = USER32.WMDestroy THEN
			USER32.PostQuitMessage(0)
		ELSIF message = USER32.WMPaint THEN
			dc := USER32.BeginPaint(wnd, ps);
			res := GDI32.TextOutA(dc, 50, 50, "Hello World", 11);
			res := USER32.EndPaint(wnd, ps)
		ELSIF message = USER32.WMChar THEN
			Kernel.Beep
		ELSE
			RETURN USER32.DefWindowProcA(wnd, message, wParam, lParam)
		END;
		RETURN 0
	END WndHandler;
	
	PROCEDURE OpenWindow;
		VAR class: USER32.WndClass; res: INTEGER;
	BEGIN
		class.cursor := USER32.LoadCursorA(0, USER32.MakeIntRsrc(USER32.IDCArrow));
		class.icon := USER32.LoadIconA(Instance, USER32.MakeIntRsrc(1));
		class.menuName := NIL;
		class.className := "Simple";
		class.backgnd := GDI32.GetStockObject(GDI32.WhiteBrush);
		class.style := {0, 1, 5, 7};
		class.instance := Instance;
		class.wndProc := WndHandler;
		class.clsExtra := 0;
		class.wndExtra := 0;
		USER32.RegisterClassA(class);
		MainWnd := USER32.CreateWindowExA({}, "Simple", "Empty Windows Application",
														{16..19, 22, 23, 25},
														USER32.CWUseDefault, USER32.CWUseDefault,
														USER32.CWUseDefault, USER32.CWUseDefault,
														0, 0, Instance, 0);
		res := USER32.ShowWindow(MainWnd, 10);
		res := USER32.UpdateWindow(MainWnd);
	END OpenWindow;
	
	PROCEDURE MainLoop;
		VAR msg: USER32.Message; res: INTEGER;
	BEGIN
		WHILE USER32.GetMessageA(msg, 0, 0, 0) # 0 DO
			res := USER32.TranslateMessage(msg);
			res := USER32.DispatchMessageA(msg);
		END;
(*
		KERNEL32.ExitProcess(msg.wParam)
*)
	END MainLoop;
	
BEGIN
	Instance := KERNEL32.GetModuleHandleA(NIL);
	OpenWindow;
	MainLoop
CLOSE
	Kernel.Beep
END Usekrnl.


MODULE MYDLL;
(* sample module to be linked into a dll *)
(* Ominc  *)

	IMPORT SYSTEM, KERNEL32;
	
	VAR expVar*: INTEGER;
	
	PROCEDURE GCD* (a, b: INTEGER): INTEGER;
	BEGIN
		WHILE a # b DO
			IF a < b THEN b := b - a ELSE a := a - b END
		END;
		expVar := a;
		RETURN a
	END GCD;

	PROCEDURE Beep*;
	BEGIN
		KERNEL32.Beep(500, 200)
	END Beep;
	
CLOSE
	Beep
END MYDLL.



Resource = Id [ "[" Language "]" ] Options name [ "." ext ] [ "(" { Resource } ")" ]
Id = number
Language = number
Options = { "@" | "!" .. "?" | "^" | "~" }

names

MENU
	1 MENU (0 File (11 New 12 Open 13 Save 0 "" 14 Exit) 0 Edit (21 Cut 22 Copy 23 Paste))
		= grayed
		- inctive
		# bitmap
		* checked
		! menuBarBreak
		/ menuBreak
		? ownerDraw

ACCELERATOR
	1 ACCELERATOR (11 ^N 12 ^O 13 ^S 21 ^X 22 ^C 23 ^V)
		* shift
		^ ctrl
		@ alt
		- noInvert

filename.ico

filename.cur

filname.bmp

filename.res

filename.tlb
