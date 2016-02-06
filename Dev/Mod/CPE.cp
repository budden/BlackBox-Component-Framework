MODULE DevCPE;
(* code file emiter *)
(**
	project	= "BlackBox"
	organization	= "www.oberon.ch"
	contributors	= "Oberon microsystems, Robert Campbell"
	version	= "System/Rsrc/About"
	copyright	= "System/Rsrc/About"
	license	= "Docu/BB-License"
	references	= "ftp://ftp.inf.ethz.ch/pub/software/Oberon/OberonV4/Docu/OP2.Paper.ps"
	changes	= "
	- 20070123, bh, support for signatures added
	- 20070816, bh, ProcTyp inserted in base type handling in PrepDesc
	- 20080202, bh, Real comparison corrected in AllocConst (proposed by Robert Campbell)
	"
	issues	= ""

**)

	IMPORT SYSTEM, Dates, DevCPM, DevCPT;


	CONST
		(* item base modes (=object modes) *)
		Var = 1; VarPar = 2; Con = 3; LProc = 6; XProc = 7; CProc = 9; IProc = 10; TProc = 13;
	
		(* structure forms *)
		Undef = 0; Byte = 1; Bool = 2; Char8 = 3; Int8 = 4; Int16 = 5; Int32 = 6;
		Real32 = 7; Real64 = 8; Set = 9; String8 = 10; NilTyp = 11; NoTyp = 12;
		Pointer = 13; ProcTyp = 14; Comp = 15;
		Char16 = 16; String16 = 17; Int64 = 18; Guid = 23;
		
		(* composite structure forms *)
		Basic = 1; Array = 2; DynArr = 3; Record = 4;
		
		(* object modes *)
		Fld = 4; Typ = 5; Head = 12;
		
		(* module visibility of objects *)
		internal = 0; external = 1; externalR = 2; inPar = 3; outPar = 4;

		(* history of imported objects *)
		inserted = 0; same = 1; pbmodified = 2; pvmodified = 3; removed = 4; inconsistent = 5;
		
		(* attribute flags (attr.adr, struct.attribute, proc.conval.setval)*)
		newAttr = 16; absAttr = 17; limAttr = 18; empAttr = 19; extAttr = 20;
		
		(* meta interface consts *)
		mConst = 1; mTyp = 2; mVar = 3; mProc = 4; mField = 5;
		mBool = 1; mChar8 = 2; mChar16 = 3; mInt8 = 4; mInt16 = 5; mInt32 = 6;
		mReal32 = 7; mReal64 = 8; mSet = 9; mInt64 = 10; mAnyRec = 11; mAnyPtr = 12; mSysPtr = 13;
		mProctyp = 0; mRecord = 1; mArray = 2; mPointer = 3;
		mInternal = 1; mReadonly = 2; mPrivate = 3; mExported = 4;
		mValue = 10; mInPar = 11; mOutPar = 12; mVarPar = 13;
		mInterface = 32; mGuid = 33; mResult = 34;

		(* sysflag *)
		untagged = 1; noAlign = 3; union = 7; interface = 10;
		
		(* fixup types *)
		absolute = 100; relative = 101; copy = 102; table = 103; tableend = 104; short = 105;
		
		(* kernel flags *)
		iptrs = 30;
		
		expAllFields = TRUE;
		
		(* implementation restrictions *)
		CodeBlocks = 512;
		CodeLength = 16384;
		MaxNameTab = 800000H;
		
		useAllRef = FALSE;
		outSignatures = TRUE;
	
	TYPE
		CodeBlock = POINTER TO ARRAY CodeLength OF SHORTCHAR;
	
	VAR
		pc*: INTEGER;
		dsize*: INTEGER;	(* global data size *)
		KNewRec*, KNewArr*: DevCPT.Object;
		closeLbl*: INTEGER;
		CaseLinks*: DevCPT.LinkList;
		
		processor: INTEGER;
		bigEndian: BOOLEAN;
		procVarIndirect: BOOLEAN;
		idx8, idx16, idx32, idx64, namex, nofptrs, headSize: INTEGER;
		Const8, Const16, Const32, Const64, Code, Data, Meta, Mod, Proc, nameList, descList, untgd: DevCPT.Object;
		outRef, outAllRef, outURef, outSrc, outObj: BOOLEAN;
		codePos, srcPos: INTEGER;
		options: SET;
		code: ARRAY CodeBlocks OF CodeBlock;
		actual: CodeBlock;
		actIdx, blkIdx: INTEGER;
		CodeOvF: BOOLEAN;
		zero: ARRAY 16 OF SHORTCHAR;	(* all 0X *)
		imports: INTEGER;
		dllList, dllLast: DevCPT.Object;
		
		
	PROCEDURE GetLongWords* (con: DevCPT.Const; OUT hi, low: INTEGER);
		CONST N = 4294967296.0; (* 2^32 *)
		VAR rh, rl: REAL;
	BEGIN
		rl := con.intval; rh := con.realval / N;
		IF rh >= MAX(INTEGER) + 1.0 THEN rh := rh - 1; rl := rl + N
		ELSIF rh < MIN(INTEGER) THEN rh := rh + 1; rl := rl - N
		END;
		hi := SHORT(ENTIER(rh));
		rl := rl + (rh - hi) * N;
		IF rl < 0 THEN hi := hi - 1; rl := rl + N
		ELSIF rl >= N THEN hi := hi + 1; rl := rl - N
		END;
		IF rl >= MAX(INTEGER) + 1.0 THEN rl := rl - N END;
		low := SHORT(ENTIER(rl))
(*
		hi := SHORT(ENTIER((con.realval + con.intval) / 4294967296.0));
		r := con.realval + con.intval - hi * 4294967296.0;
		IF r > MAX(INTEGER) THEN r := r - 4294967296.0 END;
		low := SHORT(ENTIER(r))
*)
	END GetLongWords;
	
	PROCEDURE GetRealWord* (con: DevCPT.Const; OUT x: INTEGER);
		VAR r: SHORTREAL;
	BEGIN
		r := SHORT(con.realval); x := SYSTEM.VAL(INTEGER, r)
	END GetRealWord;
		
	PROCEDURE GetRealWords* (con: DevCPT.Const; OUT hi, low: INTEGER);
		TYPE A = ARRAY 2 OF INTEGER;
		VAR a: A;
	BEGIN
		a := SYSTEM.VAL(A, con.realval);
		IF DevCPM.LEHost THEN hi := a[1]; low := a[0] ELSE hi := a[0]; low := a[1] END
	END GetRealWords;
		
	PROCEDURE IsSame (x, y: REAL): BOOLEAN;
	BEGIN
		RETURN  (x = y) & ((x #  0.) OR (1. / x = 1. / y))
	END IsSame;
	
	PROCEDURE AllocConst* (con: DevCPT.Const; form: BYTE; VAR obj: DevCPT.Object; VAR adr: INTEGER);
		VAR c: DevCPT.Const;
	BEGIN
		INCL(con.setval, form);
		CASE form OF
		| String8:
			obj := Const8; c := obj.conval;
			WHILE (c # NIL) & ((con.setval # c.setval) OR (con.intval2 # c.intval2) OR (con.ext^ # c.ext^)) DO c := c.link END;
			IF c = NIL THEN adr := idx8; INC(idx8, (con.intval2 + 3) DIV 4 * 4) END
		| String16:
			obj := Const16; c := obj.conval;
			WHILE (c # NIL) & ((con.setval # c.setval) OR (con.intval2 # c.intval2) OR (con.ext^ # c.ext^)) DO c := c.link END;
			IF c = NIL THEN adr := idx16; INC(idx16, (con.intval2 + 1) DIV 2 * 4) END
		| Int64:
			obj := Const64; c := obj.conval;
			WHILE (c # NIL) & ((con.setval # c.setval) OR (con.intval # c.intval2) OR (con.realval # c.realval)) DO
				c := c.link
			END;
			IF c = NIL THEN con.intval2 := con.intval; adr := idx64; INC(idx64, 8) END
		| Real32:
			obj := Const32; c := obj.conval;
			WHILE (c # NIL) & ((con.setval # c.setval) OR ~IsSame(con.realval, c.realval)) DO c := c.link END;
			IF c = NIL THEN adr := idx32; INC(idx32, 4) END
		| Real64:
			obj := Const64; c := obj.conval;
			WHILE (c # NIL) & ((con.setval # c.setval) OR ~IsSame(con.realval, c.realval)) DO c := c.link END;
			IF c = NIL THEN adr := idx64; INC(idx64, 8) END
		| Guid:
			obj := Const32; c := obj.conval;
			WHILE (c # NIL) & ((con.setval # c.setval) OR (con.intval2 # c.intval2) OR (con.ext^ # c.ext^)) DO c := c.link END;
			IF c = NIL THEN adr := idx32; INC(idx32, 16) END
		END;
		IF c = NIL THEN con.link := obj.conval; obj.conval := con ELSE adr := c.intval END;
		con.intval := adr
	END AllocConst;


	PROCEDURE AllocTypDesc* (typ: DevCPT.Struct);	 (* typ.comp = Record *)
		VAR obj: DevCPT.Object; name: DevCPT.Name;
	BEGIN
		IF typ.strobj = NIL THEN
			name := "@"; DevCPT.Insert(name, obj); obj.name := DevCPT.null;	(* avoid err 1 *)
			obj.mode := Typ; obj.typ := typ; typ.strobj := obj
		END
	END AllocTypDesc; 


	PROCEDURE PutByte* (a, x: INTEGER);
	BEGIN
		code[a DIV CodeLength]^[a MOD CodeLength] := SHORT(CHR(x MOD 256))
	END PutByte;
	
	PROCEDURE PutShort* (a, x: INTEGER);
	BEGIN
		IF bigEndian THEN
			PutByte(a, x DIV 256); PutByte(a + 1, x)
		ELSE
			PutByte(a, x); PutByte(a + 1, x DIV 256)
		END
	END PutShort;
	
	PROCEDURE PutWord* (a, x: INTEGER);
	BEGIN
		IF bigEndian THEN
			PutByte(a, x DIV 1000000H); PutByte(a + 1, x DIV 10000H);
			PutByte(a + 2, x DIV 256); PutByte(a + 3, x)
		ELSE
			PutByte(a, x); PutByte(a + 1, x DIV 256);
			PutByte(a + 2, x DIV 10000H); PutByte(a + 3, x DIV 1000000H)
		END
	END PutWord;
	
	PROCEDURE ThisByte* (a: INTEGER): INTEGER;
	BEGIN
		RETURN ORD(code[a DIV CodeLength]^[a MOD CodeLength])
	END ThisByte;
	
	PROCEDURE ThisShort* (a: INTEGER): INTEGER;
	BEGIN
		IF bigEndian THEN
			RETURN ThisByte(a) * 256 + ThisByte(a+1)
		ELSE
			RETURN ThisByte(a+1) * 256 + ThisByte(a)
		END
	END ThisShort;
	
	PROCEDURE ThisWord* (a: INTEGER): INTEGER;
	BEGIN
		IF bigEndian THEN
			RETURN ((ThisByte(a) * 256 + ThisByte(a+1)) * 256 + ThisByte(a+2)) * 256 + ThisByte(a+3)
		ELSE
			RETURN ((ThisByte(a+3) * 256 + ThisByte(a+2)) * 256 + ThisByte(a+1)) * 256 + ThisByte(a)
		END
	END ThisWord;
	
	PROCEDURE GenByte* (x: INTEGER);
	BEGIN
		IF actIdx >= CodeLength THEN
			IF blkIdx < CodeBlocks THEN
				NEW(actual); code[blkIdx] := actual; INC(blkIdx); actIdx := 0
			ELSE
				IF ~CodeOvF THEN DevCPM.err(210); CodeOvF := TRUE END;
				actIdx := 0; pc := 0
			END
		END;
		actual^[actIdx] := SHORT(CHR(x MOD 256)); INC(actIdx); INC(pc)
	END GenByte;
	
	PROCEDURE GenShort* (x: INTEGER);
	BEGIN
		IF bigEndian THEN
			GenByte(x DIV 256); GenByte(x)
		ELSE
			GenByte(x); GenByte(x DIV 256)
		END
	END GenShort;
	
	PROCEDURE GenWord* (x: INTEGER);
	BEGIN
		IF bigEndian THEN
			GenByte(x DIV 1000000H); GenByte(x DIV 10000H); GenByte(x DIV 256); GenByte(x)
		ELSE
			GenByte(x); GenByte(x DIV 256); GenByte(x DIV 10000H); GenByte(x DIV 1000000H)
		END
	END GenWord;
	
	PROCEDURE WriteCode;
		VAR i, j, k, n: INTEGER; b: CodeBlock;
	BEGIN
		j := 0; k := 0;
		WHILE j < pc DO
			n := pc - j; i := 0; b := code[k];
			IF n > CodeLength THEN n := CodeLength END;
			WHILE i < n DO DevCPM.ObjW(b^[i]); INC(i) END;
			INC(j, n); INC(k)
		END
	END WriteCode;


	PROCEDURE OffsetLink* (obj: DevCPT.Object; offs: INTEGER): DevCPT.LinkList;
		VAR link: DevCPT.LinkList; m: DevCPT.Object;
	BEGIN
		ASSERT((obj.mode # Typ) OR (obj.typ # DevCPT.int32typ));
		ASSERT((obj.mode # Typ) OR (obj.typ # DevCPT.iunktyp) & (obj.typ # DevCPT.guidtyp));
		IF obj.mnolev >= 0 THEN	(* not imported *)
			CASE obj.mode OF
			| Typ: IF obj.links = NIL THEN obj.link := descList; descList := obj END
			| TProc: IF obj.adr = -1 THEN obj := obj.nlink ELSE offs := offs + obj.adr; obj := Code END
			| Var: offs := offs + dsize; obj := Data
			| Con, IProc, XProc, LProc:
			END
		ELSIF obj.mode = Typ THEN
			IF obj.typ.untagged THEN	(* add desc for imported untagged types *)
				IF obj.links = NIL THEN obj.link := descList; descList := obj END
			ELSE
				m := DevCPT.GlbMod[-obj.mnolev];
				IF m.library # NIL THEN RETURN NIL END	(* type import from dll *)
			END
		END;
		link := obj.links;
		WHILE (link # NIL) & (link.offset # offs) DO link := link.next END;
		IF link = NIL THEN
			NEW(link); link.offset := offs; link.linkadr := 0;
			link.next := obj.links; obj.links := link
		END;
		RETURN link
	END OffsetLink;


	PROCEDURE TypeObj* (typ: DevCPT.Struct): DevCPT.Object;
		VAR obj: DevCPT.Object;
	BEGIN
		obj := typ.strobj;
		IF obj = NIL THEN
			obj := DevCPT.NewObj(); obj.leaf := TRUE; obj.mnolev := 0;
			obj.name := DevCPT.null; obj.mode := Typ; obj.typ := typ; typ.strobj := obj
		END;
		RETURN obj
	END TypeObj;


	PROCEDURE Align (n: INTEGER);
		VAR p: INTEGER;
	BEGIN
		p := DevCPM.ObjLen();
		DevCPM.ObjWBytes(zero, (-p) MOD n)
	END Align;
	
	PROCEDURE OutName (VAR name: ARRAY OF SHORTCHAR);
		VAR ch: SHORTCHAR; i: SHORTINT;
	BEGIN i := 0;
		REPEAT ch := name[i]; DevCPM.ObjW(ch); INC(i) UNTIL ch = 0X
	END OutName;
	
	PROCEDURE Out2 (x: INTEGER);	(* byte ordering must correspond to target machine *)
	BEGIN
		IF bigEndian THEN
			DevCPM.ObjW(SHORT(CHR(x DIV 256))); DevCPM.ObjW(SHORT(CHR(x)))
		ELSE
			DevCPM.ObjW(SHORT(CHR(x))); DevCPM.ObjW(SHORT(CHR(x DIV 256)))
		END
	END Out2;
	
	PROCEDURE Out4 (x: INTEGER);	(* byte ordering must correspond to target machine *)
	BEGIN
		IF bigEndian THEN
			DevCPM.ObjW(SHORT(CHR(x DIV 1000000H))); DevCPM.ObjW(SHORT(CHR(x DIV 10000H)));
			DevCPM.ObjW(SHORT(CHR(x DIV 256))); DevCPM.ObjW(SHORT(CHR(x)))
		ELSE
			DevCPM.ObjWLInt(x)
		END
	END Out4;

	PROCEDURE OutReference (obj: DevCPT.Object; offs, typ: INTEGER);
		VAR link: DevCPT.LinkList;
	BEGIN
		link := OffsetLink(obj, offs);
		IF link # NIL THEN
			Out4(typ * 1000000H + link.linkadr MOD 1000000H);
			link.linkadr := -(DevCPM.ObjLen() - headSize - 4)
		ELSE Out4(0)
		END
	END OutReference;
	
	PROCEDURE FindPtrs (typ: DevCPT.Struct; adr: INTEGER; ip: BOOLEAN; VAR num: INTEGER);
		VAR fld: DevCPT.Object; btyp: DevCPT.Struct; i, n: INTEGER;
	BEGIN
		IF typ.form = Pointer THEN
			IF ip & (typ.sysflag = interface)
				OR ~ip & ~typ.untagged THEN Out4(adr); INC(num) END
		ELSIF (typ.comp = Record) & (typ.sysflag # union) THEN
			btyp := typ.BaseTyp;
			IF btyp # NIL THEN FindPtrs(btyp, adr, ip, num) END ;
			fld := typ.link;
			WHILE (fld # NIL) & (fld.mode = Fld) DO
				IF ip & (fld.name^ = DevCPM.HdUtPtrName) & (fld.sysflag = interface)
					OR ~ip & (fld.name^ = DevCPM.HdPtrName) THEN Out4(fld.adr + adr); INC(num)
				ELSE FindPtrs(fld.typ, fld.adr + adr, ip, num)
				END;
				fld := fld.link
			END
		ELSIF typ.comp = Array THEN
			btyp := typ.BaseTyp; n := typ.n;
			WHILE btyp.comp = Array DO n := btyp.n * n; btyp := btyp.BaseTyp END ;
			IF (btyp.form = Pointer) OR (btyp.comp = Record) THEN
				i := num; FindPtrs(btyp, adr, ip, num);
				IF num # i THEN i := 1;
					WHILE i < n DO
						INC(adr, btyp.size); FindPtrs(btyp, adr, ip, num); INC(i)
					END
				END
			END
		END
	END FindPtrs;
	

	PROCEDURE OutRefName* (VAR name: ARRAY OF SHORTCHAR);
	BEGIN
		DevCPM.ObjW(0FCX); DevCPM.ObjWNum(pc); OutName(name)
	END OutRefName;

	PROCEDURE OutRefs* (obj: DevCPT.Object);
		VAR f: BYTE;
	BEGIN
		IF outRef & (obj # NIL) THEN
			OutRefs(obj.left);
			IF ((obj.mode = Var) OR (obj.mode = VarPar)) & (obj.history # removed) & (obj.name[0] # "@") THEN
				f := obj.typ.form;
				IF (f IN {Byte .. Set, Pointer, ProcTyp, Char16, Int64}) 
						OR outURef & (obj.typ.comp # DynArr)
						OR outAllRef & ~obj.typ.untagged
						OR (obj.typ.comp = Array) & (obj.typ.BaseTyp.form = Char8) THEN
					IF obj.mode = Var THEN DevCPM.ObjW(0FDX) ELSE DevCPM.ObjW(0FFX) END;
					IF obj.typ = DevCPT.anyptrtyp THEN DevCPM.ObjW(SHORT(CHR(mAnyPtr)))
					ELSIF obj.typ = DevCPT.anytyp THEN DevCPM.ObjW(SHORT(CHR(mAnyRec)))
					ELSIF obj.typ = DevCPT.sysptrtyp THEN DevCPM.ObjW(SHORT(CHR(mSysPtr)))
					ELSIF f = Char16 THEN DevCPM.ObjW(SHORT(CHR(mChar16)))
					ELSIF f = Int64 THEN DevCPM.ObjW(SHORT(CHR(mInt64)))
					ELSIF obj.typ = DevCPT.guidtyp THEN DevCPM.ObjW(SHORT(CHR(mGuid)))
					ELSIF obj.typ = DevCPT.restyp THEN DevCPM.ObjW(SHORT(CHR(mResult)))
					ELSIF f = Pointer THEN
						IF obj.typ.sysflag = interface THEN DevCPM.ObjW(SHORT(CHR(mInterface)))
						ELSIF obj.typ.untagged THEN DevCPM.ObjW(SHORT(CHR(mSysPtr)))
						ELSE DevCPM.ObjW(10X); OutReference(TypeObj(obj.typ), 0, absolute)
						END
					ELSIF (f = Comp) & outAllRef & (~obj.typ.untagged OR outURef & (obj.typ.comp # DynArr)) THEN
						DevCPM.ObjW(10X); OutReference(TypeObj(obj.typ), 0, absolute)
					ELSIF f < Int8 THEN DevCPM.ObjW(SHORT(CHR(f - 1)))
					ELSE DevCPM.ObjW(SHORT(CHR(f)))
					END;
					IF obj.mnolev = 0 THEN DevCPM.ObjWNum(obj.adr + dsize) ELSE DevCPM.ObjWNum(obj.adr) END;
					OutName(obj.name^)
				END
			END ;
			OutRefs(obj.right)
		END
	END OutRefs;
	
	PROCEDURE OutSourceRef* (pos: INTEGER);
	BEGIN
		IF outSrc & (pos # 0) & (pos # srcPos) & (pc > codePos) THEN
			WHILE pc > codePos + 250 DO
				DevCPM.ObjW(SHORT(CHR(250)));
				INC(codePos, 250);
				DevCPM.ObjWNum(0)
			END;
			DevCPM.ObjW(SHORT(CHR(pc - codePos)));
			codePos := pc;
			DevCPM.ObjWNum(pos - srcPos);
			srcPos := pos
		END
	END OutSourceRef;

	
	PROCEDURE OutPLink (link: DevCPT.LinkList; adr: INTEGER);
	BEGIN
		WHILE link # NIL DO
			ASSERT(link.linkadr # 0);
			DevCPM.ObjWNum(link.linkadr);
			DevCPM.ObjWNum(adr + link.offset);
			link := link.next
		END
	END OutPLink;

	PROCEDURE OutLink (link: DevCPT.LinkList);
	BEGIN
		OutPLink(link, 0); DevCPM.ObjW(0X)
	END OutLink;
	
	PROCEDURE OutNames;
		VAR a, b, c: DevCPT.Object;
	BEGIN
		a := nameList; b := NIL;
		WHILE a # NIL DO c := a; a := c.nlink; c.nlink := b; b := c END;
		DevCPM.ObjW(0X);	(* names[0] = 0X *)
		WHILE b # NIL DO
			OutName(b.name^);
			b := b.nlink
		END;
	END OutNames;
	
	PROCEDURE OutGuid* (VAR str: ARRAY OF SHORTCHAR);
		
		PROCEDURE Copy (n: INTEGER);
			VAR x, y: INTEGER;
		BEGIN
			x := ORD(str[n]); y := ORD(str[n + 1]);
			IF x >= ORD("a") THEN DEC(x, ORD("a") - 10)
			ELSIF x >= ORD("A") THEN DEC(x, ORD("A") - 10)
			ELSE DEC(x, ORD("0"))
			END;
			IF y >= ORD("a") THEN DEC(y, ORD("a") - 10)
			ELSIF y >= ORD("A") THEN DEC(y, ORD("A") - 10)
			ELSE DEC(y, ORD("0"))
			END;
			DevCPM.ObjW(SHORT(CHR(x * 16 + y)))
		END Copy;
		
	BEGIN
		IF bigEndian THEN
			Copy(1); Copy(3); Copy(5); Copy(7); Copy(10); Copy(12); Copy(15); Copy(17)
		ELSE
			Copy(7); Copy(5); Copy(3); Copy(1); Copy(12); Copy(10); Copy(17); Copy(15)
		END;
		Copy(20); Copy(22); Copy(25); Copy(27); Copy(29); Copy(31); Copy(33); Copy(35)
	END OutGuid;

	PROCEDURE OutConst (obj: DevCPT.Object);
		TYPE A4 = ARRAY 4 OF SHORTCHAR; A8 = ARRAY 8 OF SHORTCHAR;
		VAR a, b, c: DevCPT.Const; r: SHORTREAL; lr: REAL; a4: A4; a8: A8; ch: SHORTCHAR; i, x, hi, low: INTEGER;
	BEGIN
		a := obj.conval; b := NIL;
		WHILE a # NIL DO c := a; a := c.link; c.link := b; b := c END;
		WHILE b # NIL DO
			IF String8 IN b.setval THEN
				DevCPM.ObjWBytes(b.ext^, b.intval2);
				Align(4)
			ELSIF String16 IN b.setval THEN
				i := 0; REPEAT DevCPM.GetUtf8(b.ext^, x, i); Out2(x) UNTIL x = 0;
				Align(4)
			ELSIF Real32 IN b.setval THEN
				r := SHORT(b.realval); a4 := SYSTEM.VAL(A4, r);
				IF DevCPM.LEHost = bigEndian THEN
					ch := a4[0]; a4[0] := a4[3]; a4[3] := ch;
					ch := a4[1]; a4[1] := a4[2]; a4[2] := ch
				END;
				DevCPM.ObjWBytes(a4, 4)
			ELSIF Real64 IN b.setval THEN
				a8 := SYSTEM.VAL(A8, b.realval);
				IF DevCPM.LEHost = bigEndian THEN
					ch := a8[0]; a8[0] := a8[7]; a8[7] := ch;
					ch := a8[1]; a8[1] := a8[6]; a8[6] := ch;
					ch := a8[2]; a8[2] := a8[5]; a8[5] := ch;
					ch := a8[3]; a8[3] := a8[4]; a8[4] := ch
				END;
				DevCPM.ObjWBytes(a8, 8)
			ELSIF Int64 IN b.setval THEN
				(* intval moved to intval2 by AllocConst *)
				x := b.intval; b.intval := b.intval2; GetLongWords(b, hi, low); b.intval := x;
				IF bigEndian THEN Out4(hi); Out4(low) ELSE Out4(low); Out4(hi) END
			ELSIF Guid IN b.setval THEN
				OutGuid(b.ext^)
			END;
			b := b.link
		END
	END OutConst;
	
	PROCEDURE OutStruct (typ: DevCPT.Struct; unt: BOOLEAN);
	BEGIN
		IF typ = NIL THEN Out4(0)
		ELSIF typ = DevCPT.sysptrtyp THEN Out4(mSysPtr)
		ELSIF typ = DevCPT.anytyp THEN Out4(mAnyRec)
		ELSIF typ = DevCPT.anyptrtyp THEN Out4(mAnyPtr)
		ELSIF typ = DevCPT.guidtyp THEN Out4(mGuid)
		ELSIF typ = DevCPT.restyp THEN Out4(mResult)
		ELSE
			CASE typ.form OF
			| Undef, Byte, String8, NilTyp, NoTyp, String16: Out4(0)
			| Bool, Char8: Out4(typ.form - 1)
			| Int8..Set: Out4(typ.form)
			| Char16: Out4(mChar16)
			| Int64: Out4(mInt64)
			| ProcTyp: OutReference(TypeObj(typ), 0, absolute)
			| Pointer:
				IF typ.sysflag = interface THEN Out4(mInterface)
				ELSIF typ.untagged THEN Out4(mSysPtr)
				ELSE OutReference(TypeObj(typ), 0, absolute)
				END
			| Comp:
				IF ~typ.untagged OR (outURef & unt) THEN OutReference(TypeObj(typ), 0, absolute)
				ELSE Out4(0)
				END
			END
		END
	END OutStruct;
	
	PROCEDURE NameIdx (obj: DevCPT.Object): INTEGER;
		VAR n: INTEGER;
	BEGIN
		n := 0;
		IF obj.name # DevCPT.null THEN
			IF obj.num = 0 THEN
				obj.num := namex;
				WHILE obj.name[n] # 0X DO INC(n) END;
				INC(namex, n + 1);
				obj.nlink := nameList; nameList := obj
			END;
			n := obj.num;
		END;
		RETURN n
	END NameIdx;
	
	PROCEDURE OutSignature (par: DevCPT.Object; retTyp: DevCPT.Struct; OUT pos: INTEGER);
		VAR p: DevCPT.Object; n, m: INTEGER;
	BEGIN
		pos := DevCPM.ObjLen() - headSize;
		OutStruct(retTyp, TRUE);
		p := par; n := 0;
		WHILE p # NIL DO INC(n); p := p.link END;
		Out4(n); p := par;
		WHILE p # NIL DO
			IF p.mode # VarPar THEN m := mValue
			ELSIF p.vis = inPar THEN m := mInPar
			ELSIF p.vis = outPar THEN m := mOutPar
			ELSE m := mVarPar
			END;
			Out4(NameIdx(p) * 256 + m);
			OutStruct(p.typ, TRUE);
			p := p.link
		END
	END OutSignature;
	
	PROCEDURE PrepObject (obj: DevCPT.Object);
	BEGIN
		IF (obj.mode IN {LProc, XProc, IProc}) & outSignatures THEN	(* write param list *)
			OutSignature(obj.link, obj.typ, obj.conval.intval)
		END
	END PrepObject;
	
	PROCEDURE OutObject (mode, fprint, offs: INTEGER; typ: DevCPT.Struct; obj: DevCPT.Object);
		VAR vis: INTEGER;
	BEGIN
		Out4(fprint);
		Out4(offs);
		IF obj.vis = internal THEN vis := mInternal
		ELSIF obj.vis = externalR THEN vis := mReadonly
		ELSIF obj.vis = external THEN vis := mExported
		END;
		Out4(mode + vis * 16 + NameIdx(obj) * 256);
		IF (mode = mProc) & outSignatures THEN OutReference(Meta, obj.conval.intval, absolute)	(* ref to par list *)
		ELSE OutStruct(typ, mode = mField)
		END
	END OutObject;
	
	PROCEDURE PrepDesc (desc: DevCPT.Struct);
		VAR fld: DevCPT.Object; n: INTEGER; l: DevCPT.LinkList; b: DevCPT.Struct;
	BEGIN
		IF desc.comp = Record THEN	(* write field list *)
			desc.strobj.adr := DevCPM.ObjLen() - headSize;
			n := 0; fld := desc.link;
			WHILE (fld # NIL) & (fld.mode = Fld) DO
				IF expAllFields OR (fld.vis # internal) THEN INC(n) END;
				fld := fld.link
			END;
			Out4(n); fld := desc.link;
			WHILE (fld # NIL) & (fld.mode = Fld) DO
				IF expAllFields OR (fld.vis # internal) THEN
					OutObject(mField, 0, fld.adr, fld.typ, fld)
				END;
				fld := fld.link
			END
		ELSIF (desc.form = ProcTyp) & outSignatures THEN	(* write param list *)
			OutSignature(desc.link, desc.BaseTyp, desc.n)
		END;
		(* assert name and base type are included *)
		IF desc.untagged THEN n := NameIdx(untgd)
		ELSE n := NameIdx(desc.strobj)
		END;
		IF desc.form # ProcTyp THEN b := desc.BaseTyp;
			IF (b # NIL) & (b # DevCPT.anytyp) & (b # DevCPT.anyptrtyp) & (b.form IN {Pointer, Comp, ProcTyp})
					& (b.sysflag # interface) & (b # DevCPT.guidtyp)
					& (~b.untagged OR outURef & (b.form = Comp)) THEN
				l := OffsetLink(TypeObj(b), 0)
			END
		END
	END PrepDesc;
	
	PROCEDURE NumMeth (root: DevCPT.Object; num: INTEGER): DevCPT.Object;
		VAR obj: DevCPT.Object;
	BEGIN
		IF (root = NIL) OR (root.mode = TProc) & (root.num = num) THEN RETURN root END;
		obj := NumMeth(root.left, num);
		IF obj = NIL THEN obj := NumMeth(root.right, num) END;
		RETURN obj
	END NumMeth;
	
	PROCEDURE OutDesc (desc: DevCPT.Struct);
		VAR m: DevCPT.Object; i, nofptr, flddir, size: INTEGER; t, xb: DevCPT.Struct; form, lev, attr: BYTE;
			name: DevCPT.Name;
	BEGIN
		ASSERT(~desc.untagged);
		IF desc.comp = Record THEN
			xb := desc; flddir := desc.strobj.adr;
			REPEAT xb := xb.BaseTyp UNTIL (xb = NIL) OR (xb.mno # 0) OR xb.untagged;
			Out4(-1); i := desc.n;
			WHILE i > 0 DO DEC(i); t := desc;
				REPEAT
					m := NumMeth(t.link, i); t := t.BaseTyp
				UNTIL (m # NIL) OR (t = xb);
				IF m # NIL THEN 
					IF absAttr IN m.conval.setval THEN Out4(0)
					ELSE OutReference(m, 0, absolute)
					END
				ELSIF (xb = NIL) OR xb.untagged THEN Out4(0)	(* unimplemented ANYREC method *)
				ELSE OutReference(xb.strobj, -4 - 4 * i, copy)
				END
			END;
			desc.strobj.adr := DevCPM.ObjLen() - headSize;	(* desc adr *)
			Out4(desc.size);
			OutReference(Mod, 0, absolute);
			IF desc.untagged THEN m := untgd ELSE m := desc.strobj END;
			IF desc.attribute = extAttr THEN attr := 1
			ELSIF desc.attribute = limAttr THEN attr := 2
			ELSIF desc.attribute = absAttr THEN attr := 3
			ELSE attr := 0
			END;
			Out4(mRecord + attr * 4 + desc.extlev * 16 + NameIdx(m) * 256); i := 0;
			WHILE i <= desc.extlev DO
				t := desc;
				WHILE t.extlev > i DO t := t.BaseTyp END;
				IF t.sysflag = interface THEN Out4(0)
				ELSIF t.untagged THEN OutReference(TypeObj(t), 0, absolute)
				ELSIF (t.mno = 0) THEN OutReference(t.strobj, 0, absolute)
				ELSIF t = xb THEN OutReference(xb.strobj, 0, absolute)
				ELSE OutReference(xb.strobj, 12 + 4 * i, copy)
				END;
				INC(i)
			END;
			WHILE i <= DevCPM.MaxExts DO Out4(0); INC(i) END;
			OutReference(Meta, flddir, absolute);	(* ref to field list *)
			nofptr := 0; FindPtrs(desc, 0, FALSE, nofptr);
			Out4(-(4 * nofptr + 4));
			nofptr := 0; FindPtrs(desc, 0, TRUE, nofptr);
			Out4(-1)
		ELSE
			desc.strobj.adr := DevCPM.ObjLen() - headSize;
			lev := 0; size := 0;
			IF desc.comp = Array THEN
				size := desc.n; form := mArray
			ELSIF desc.comp = DynArr THEN
				form := mArray; lev := SHORT(SHORT(desc.n + 1))
			ELSIF desc.form = Pointer THEN
				form := mPointer
			ELSE ASSERT(desc.form = ProcTyp);
				DevCPM.FPrint(size, XProc); DevCPT.FPrintSign(size, desc.BaseTyp, desc.link); form := mProctyp;
			END;
			Out4(size);
			OutReference(Mod, 0, absolute);
			IF desc.untagged THEN m := untgd ELSE m := desc.strobj END;
			Out4(form + lev * 16 + NameIdx(m) * 256);
			IF desc.form # ProcTyp THEN OutStruct(desc.BaseTyp, TRUE)
			ELSIF outSignatures THEN OutReference(Meta, desc.n, absolute)	(* ref to par list *)
			END
		END
	END OutDesc;

	PROCEDURE OutModDesc (nofptr, refSize, namePos, ptrPos, expPos, impPos: INTEGER);
		VAR i: INTEGER; t: Dates.Time; d: Dates.Date;
	BEGIN
		Out4(0);	(* link *)
		Out4(ORD(options));	(* opts *)
		Out4(0);	(* refcnt *)
		Dates.GetDate(d); Dates.GetTime(t); 	(* compile time *)
		Out2(d.year); Out2(d.month); Out2(d.day);
		Out2(t.hour); Out2(t.minute); Out2(t.second);
		Out4(0); Out4(0); Out4(0); 	(* load time *)
		Out4(0);	(* ext *)
		IF closeLbl # 0 THEN OutReference(Code, closeLbl, absolute);	(* terminator *)
		ELSE Out4(0)
		END;
		Out4(imports);	(* nofimps *)
		Out4(nofptr);	(* nofptrs *)
		Out4(pc);	(* csize *)
		Out4(dsize);	(* dsize *)
		Out4(refSize);	(* rsize *)
		OutReference(Code, 0, absolute);	(* code *)
		OutReference(Data, 0, absolute);	(* data *)
		OutReference(Meta, 0, absolute);	(* refs *)
		IF procVarIndirect THEN
			OutReference(Proc, 0, absolute);	(* procBase *)
		ELSE
			OutReference(Code, 0, absolute);	(* procBase *)
		END;
		OutReference(Data, 0, absolute);	(* varBase *)
		OutReference(Meta, namePos, absolute);	(* names *)
		OutReference(Meta, ptrPos, absolute);	(* ptrs *)
		OutReference(Meta, impPos, absolute);	(* imports *)
		OutReference(Meta, expPos, absolute);	(* export *)
		i := 0;	(* name *)
		WHILE DevCPT.SelfName[i] # 0X DO DevCPM.ObjW(DevCPT.SelfName[i]); INC(i) END;
		DevCPM.ObjW(0X);
		Align(4)
	END OutModDesc;

	PROCEDURE OutProcTable (obj: DevCPT.Object);	(* 68000 *)
	BEGIN
		IF obj # NIL THEN
			OutProcTable(obj.left);
			IF obj.mode IN {XProc, IProc} THEN
				Out2(4EF9H); OutReference(Code, obj.adr, absolute); Out2(0);
			END;
			OutProcTable(obj.right);
		END;
	END OutProcTable;

	PROCEDURE PrepExport (obj: DevCPT.Object);
	BEGIN
		IF obj # NIL THEN
			PrepExport(obj.left);
			IF (obj.mode IN {LProc, XProc, IProc}) & (obj.history # removed) & (obj.vis # internal) THEN
				PrepObject(obj)
			END;
			PrepExport(obj.right)
		END
	END PrepExport;
	
	PROCEDURE OutExport (obj: DevCPT.Object);
		VAR num: INTEGER;
	BEGIN
		IF obj # NIL THEN
			OutExport(obj.left);
			IF (obj.history # removed) & ((obj.vis # internal) OR
						(obj.mode = Typ) & (obj.typ.strobj = obj) & (obj.typ.form = Comp)) THEN
				DevCPT.FPrintObj(obj);
				IF obj.mode IN {LProc, XProc, IProc} THEN
					IF procVarIndirect THEN
						ASSERT(obj.nlink = NIL);
						num := obj.num; obj.num := 0;
						OutObject(mProc, obj.fprint, num, NIL, obj);
						obj.num := num
					ELSE
						OutObject(mProc, obj.fprint, obj.adr, NIL, obj)
					END
				ELSIF obj.mode = Var THEN
					OutObject(mVar, obj.fprint, dsize + obj.adr, obj.typ, obj)
				ELSIF obj.mode = Typ THEN
					OutObject(mTyp, obj.typ.pbfp, obj.typ.pvfp, obj.typ, obj)
				ELSE ASSERT(obj.mode IN {Con, CProc});
					OutObject(mConst, obj.fprint, 0, NIL, obj)
				END
			END;
			OutExport(obj.right)
		END
	END OutExport;
	
	PROCEDURE OutCLinks (obj: DevCPT.Object);
	BEGIN
		IF obj # NIL THEN
			OutCLinks(obj.left);
			IF obj.mode IN {LProc, XProc, IProc} THEN OutPLink(obj.links, obj.adr) END;
			OutCLinks(obj.right)
		END
	END OutCLinks;

	PROCEDURE OutCPLinks (obj: DevCPT.Object; base: INTEGER);
	BEGIN
		IF obj # NIL THEN
			OutCPLinks(obj.left, base);
			IF obj.mode IN {LProc, XProc, IProc} THEN OutPLink(obj.links, obj.num + base) END;
			OutCPLinks(obj.right, base)
		END
	END OutCPLinks;

	PROCEDURE OutImport (obj: DevCPT.Object);
		VAR typ: DevCPT.Struct; strobj: DevCPT.Object; opt: INTEGER;
	BEGIN
		IF obj # NIL THEN
			OutImport(obj.left);
			IF obj.mode = Typ THEN typ := obj.typ;
				IF obj.used OR
					(typ.form IN {Pointer, Comp}) & (typ.strobj = obj) &
						((obj.links # NIL) OR (obj.name # DevCPT.null) & (typ.pvused OR typ.pbused)) THEN
					DevCPT.FPrintStr(typ);
					DevCPM.ObjW(SHORT(CHR(mTyp))); OutName(obj.name^);
					IF obj.used THEN opt := 2 ELSE opt := 0 END;
					IF (typ.form = Comp) & ((typ.pvused) OR (obj.name = DevCPT.null)) THEN
						DevCPM.ObjWNum(typ.pvfp); DevCPM.ObjW(SHORT(CHR(opt + 1)));
						IF obj.history = inconsistent THEN DevCPT.FPrintErr(obj, 249) END
					ELSE DevCPM.ObjWNum(typ.pbfp); DevCPM.ObjW(SHORT(CHR(opt)))
					END;
					OutLink(obj.links)
				END
			ELSIF obj.used THEN
				DevCPT.FPrintObj(obj);
				IF obj.mode = Var THEN
					DevCPM.ObjW(SHORT(CHR(mVar))); OutName(obj.name^);
					DevCPM.ObjWNum(obj.fprint); OutLink(obj.links)
				ELSIF obj.mode IN {XProc, IProc} THEN
					DevCPM.ObjW(SHORT(CHR(mProc))); OutName(obj.name^);
					DevCPM.ObjWNum(obj.fprint); OutLink(obj.links)
				ELSE ASSERT(obj.mode IN {Con, CProc});
					DevCPM.ObjW(SHORT(CHR(mConst))); OutName(obj.name^); DevCPM.ObjWNum(obj.fprint)
				END
			END;
			OutImport(obj.right)
		END
	END OutImport;
	
	PROCEDURE OutUseBlock;
		VAR m, obj: DevCPT.Object; i: INTEGER;
	BEGIN
		m := dllList;
		WHILE m # NIL DO
			obj := m.nlink;
			WHILE obj # NIL DO
				IF obj.mode = Var THEN DevCPM.ObjW(SHORT(CHR(mVar)))
				ELSE DevCPM.ObjW(SHORT(CHR(mProc)))
				END;
				IF obj.entry # NIL THEN OutName(obj.entry^)
				ELSE OutName(obj.name^);
				END;
				DevCPT.FPrintObj(obj); DevCPM.ObjWNum(obj.fprint); OutLink(obj.links);
				obj := obj.nlink
			END;
			DevCPM.ObjW(0X); m := m.link
		END;
		i := 1;
		WHILE i < DevCPT.nofGmod DO
			obj := DevCPT.GlbMod[i];
			IF obj.library = NIL THEN OutImport(obj.right); DevCPM.ObjW(0X) END;
			INC(i)
		END;
	END OutUseBlock;

	PROCEDURE CollectDll (obj: DevCPT.Object; mod: DevCPT.String);
		VAR name: DevCPT.String; dll: DevCPT.Object;
	BEGIN
		IF obj # NIL THEN
			CollectDll(obj.left, mod);
			IF obj.used & (obj.mode IN {Var, XProc, IProc}) THEN
				IF obj.library # NIL THEN name := obj.library
				ELSE name := mod
				END;
				dll := dllList;
				WHILE (dll # NIL) & (dll.library^ # name^) DO dll := dll.link END;
				IF dll = NIL THEN
					NEW(dll); dll.library := name; INC(imports);
					IF dllList = NIL THEN dllList := dll ELSE dllLast.link := dll END;
					dllLast := dll; dll.left := dll;
				END;
				dll.left.nlink := obj; dll.left := obj
			END;
			CollectDll(obj.right, mod)
		END
	END CollectDll;
	
	PROCEDURE EnumXProc(obj: DevCPT.Object; VAR num: INTEGER);
	BEGIN
		IF obj # NIL THEN
			EnumXProc(obj.left, num);
			IF obj.mode IN {XProc, IProc} THEN
				obj.num := num; INC(num, 8);
			END;
			EnumXProc(obj.right, num)
		END;
	END EnumXProc;
	
	PROCEDURE OutHeader*;
		VAR i: INTEGER; m: DevCPT.Object;
	BEGIN
		DevCPM.ObjWLInt(processor);	(* processor type *)
		DevCPM.ObjWLInt(0); DevCPM.ObjWLInt(0); DevCPM.ObjWLInt(0);
		DevCPM.ObjWLInt(0); DevCPM.ObjWLInt(0);	(* sizes *)
		imports := 0; i := 1;
		WHILE i < DevCPT.nofGmod DO
			m := DevCPT.GlbMod[i];
			IF m.library # NIL THEN	(* dll import *)
				CollectDll(m.right, m.library);
			ELSE INC(imports)	(* module import *)
			END;
			INC(i)
		END;
		DevCPM.ObjWNum(imports);	(* num of import *)
		OutName(DevCPT.SelfName); 
		m := dllList;
		WHILE m # NIL DO DevCPM.ObjW("$"); OutName(m.library^); m := m.link END;
		i := 1;
		WHILE i < DevCPT.nofGmod DO
			m := DevCPT.GlbMod[i];
			IF m.library = NIL THEN OutName(m.name^) END;
			INC(i)
		END;
		Align(16); headSize := DevCPM.ObjLen();
		IF procVarIndirect THEN
			i := 0; EnumXProc(DevCPT.topScope.right, i)
		END
	END OutHeader;

	PROCEDURE OutCode*;
		VAR i, j, refSize, expPos, ptrPos, impPos, namePos, procPos,
			con8Pos, con16Pos, con32Pos, con64Pos, modPos, codePos: INTEGER;
			m, obj, dlist: DevCPT.Object;
	BEGIN
	(* Ref *)
		DevCPM.ObjW(0X); (* end mark *)
		refSize := DevCPM.ObjLen() - headSize;
	(* Export *)
		Align(4);
		IF outSignatures THEN PrepExport(DevCPT.topScope.right) END;	(* procedure signatures *)
		Align(8); expPos := DevCPM.ObjLen(); 
		Out4(0);
		OutExport(DevCPT.topScope.right);	(* export objects *)
		i := DevCPM.ObjLen(); DevCPM.ObjSet(expPos); Out4((i - expPos - 4) DIV 16); DevCPM.ObjSet(i);
	(* Pointers *)
		ptrPos := DevCPM.ObjLen();
		obj := DevCPT.topScope.scope; nofptrs := 0;
		WHILE obj # NIL DO FindPtrs(obj.typ, dsize + obj.adr, FALSE, nofptrs); obj := obj.link END;
		obj := DevCPT.topScope.scope; i := 0;
		WHILE obj # NIL DO FindPtrs(obj.typ, dsize + obj.adr, TRUE, i); obj := obj.link END;
		IF i > 0 THEN Out4(-1); INCL(options, iptrs) END;
	(* Prepare Type Descriptors *)
		dlist := NIL;
		WHILE descList # NIL DO
			obj := descList; descList := descList.link;
			PrepDesc(obj.typ);
			obj.link := dlist; dlist := obj
		END;
	(* Import List *)
		impPos := DevCPM.ObjLen(); i := 0;
		WHILE i < imports DO Out4(0); INC(i) END;
	(* Names *)
		namePos := DevCPM.ObjLen(); OutNames;
	(* Const *)
		Align(4); con8Pos := DevCPM.ObjLen();
		OutConst(Const8); con16Pos := DevCPM.ObjLen();
		ASSERT(con16Pos MOD 4 = 0); ASSERT(con16Pos - con8Pos = idx8);
		OutConst(Const16); con32Pos := DevCPM.ObjLen();
		ASSERT(con32Pos MOD 4 = 0); ASSERT(con32Pos - con16Pos = idx16);
		OutConst(Const32); con64Pos := DevCPM.ObjLen();
		ASSERT(con64Pos MOD 4 = 0); ASSERT(con64Pos - con32Pos = idx32);
		IF ODD(con64Pos DIV 4) THEN Out4(0); INC(con64Pos, 4) END;
		OutConst(Const64); ASSERT(DevCPM.ObjLen() - con64Pos = idx64);
	(* Module Descriptor *)
		Align(16); modPos := DevCPM.ObjLen();
		OutModDesc(nofptrs, refSize, namePos - headSize, ptrPos - headSize, expPos - headSize, impPos - headSize);
	(* Procedure Table *)
		procPos := DevCPM.ObjLen();
		OutProcTable(DevCPT.topScope.right);
		Out4(0); Out4(0); (* at least one entry in ProcTable *)
		Out4(0); (* sentinel *)
	(* Type Descriptors *)
		obj := dlist;
		WHILE obj # NIL DO OutDesc(obj.typ); obj := obj.link END;
	(* Code *)
		codePos := DevCPM.ObjLen(); WriteCode;
		WHILE pc MOD 4 # 0 DO DevCPM.ObjW(90X); INC(pc) END;
	(* Fixups *)
		OutLink(KNewRec.links); OutLink(KNewArr.links);
		(* metalink *)
		OutPLink(Const8.links, con8Pos - headSize);
		OutPLink(Const16.links, con16Pos - headSize);
		OutPLink(Const32.links, con32Pos - headSize);
		OutPLink(Const64.links, con64Pos - headSize);
		OutLink(Meta.links);
		(* desclink *)
		obj := dlist; i := modPos - headSize;
		WHILE obj # NIL DO OutPLink(obj.links, obj.adr - i); obj.links := NIL; obj := obj.link END;
		IF procVarIndirect THEN
			OutPLink(Proc.links, procPos - modPos);
			OutCPLinks(DevCPT.topScope.right, procPos - modPos)
		END;
		OutLink(Mod.links);
		(* codelink *)
		IF ~procVarIndirect THEN OutCLinks(DevCPT.topScope.right) END;
		OutPLink(CaseLinks, 0); OutLink(Code.links);
		(* datalink *)
		OutLink(Data.links);
	(* Use *)
		OutUseBlock;
	(* Header Fixups *)
		DevCPM.ObjSet(8);
		DevCPM.ObjWLInt(headSize);
		DevCPM.ObjWLInt(modPos - headSize);
		DevCPM.ObjWLInt(codePos - modPos);
		DevCPM.ObjWLInt(pc);
		DevCPM.ObjWLInt(dsize);
		IF namex > MaxNameTab THEN DevCPM.err(242) END;
		IF DevCPM.noerr & outObj THEN DevCPM.RegisterObj END
	END OutCode;

	PROCEDURE Init* (proc: INTEGER; opt: SET);
		CONST obj = 3; ref = 4; allref = 5; srcpos = 6; bigEnd = 15; pVarInd = 14;
	BEGIN
		processor := proc;
		bigEndian := bigEnd IN opt; procVarIndirect := pVarInd IN opt;
		outRef := ref IN opt; outAllRef := allref IN opt; outObj := obj IN opt;
		outURef := useAllRef & outAllRef & (DevCPM.comAware IN DevCPM.options);
		outSrc := srcpos IN opt;
		pc := 0; actIdx := CodeLength; blkIdx := 0; 
		idx8 := 0; idx16 := 0; idx32 := 0; idx64 := 0; namex := 1;
		options := opt * {0..15}; CodeOvF := FALSE;
		KNewRec.links := NIL; KNewArr.links := NIL; CaseLinks := NIL;
		Const8.links := NIL; Const8.conval := NIL; Const16.links := NIL; Const16.conval := NIL;
		Const32.links := NIL; Const32.conval := NIL; Const64.links := NIL; Const64.conval := NIL;
		Code.links := NIL; Data.links := NIL; Mod.links := NIL; Proc.links := NIL; Meta.links := NIL;
		nameList := NIL; descList := NIL; dllList := NIL; dllLast := NIL;
		codePos := 0; srcPos := 0;
		NEW(untgd); untgd.name := DevCPT.NewName("!");
		closeLbl := 0
	END Init;

	PROCEDURE Close*;
	BEGIN
		KNewRec.links := NIL; KNewArr.links := NIL; CaseLinks := NIL;
		Const8.links := NIL; Const8.conval := NIL; Const16.links := NIL; Const16.conval := NIL;
		Const32.links := NIL; Const32.conval := NIL; Const64.links := NIL; Const64.conval := NIL;
		Code.links := NIL; Data.links := NIL; Mod.links := NIL; Proc.links := NIL; Meta.links := NIL;
		nameList := NIL; descList := NIL; dllList := NIL; dllLast := NIL;
		WHILE blkIdx > 0 DO DEC(blkIdx); code[blkIdx] := NIL END;
		actual := NIL; untgd := NIL;
	END Close;

BEGIN
	NEW(KNewRec); KNewRec.mnolev := -128;
	NEW(KNewArr); KNewArr.mnolev := -128;
	NEW(Const8); Const8.mode := Con; Const8.mnolev := 0;
	NEW(Const16); Const16.mode := Con; Const16.mnolev := 0;
	NEW(Const32); Const32.mode := Con; Const32.mnolev := 0;
	NEW(Const64); Const64.mode := Con; Const64.mnolev := 0;
	NEW(Code); Code.mode := Con; Code.mnolev := 0;
	NEW(Data); Data.mode := Con; Data.mnolev := 0;
	NEW(Mod); Mod.mode := Con; Mod.mnolev := 0;
	NEW(Proc); Proc.mode := Con; Proc.mnolev := 0;
	NEW(Meta); Meta.mode := Con; Mod.mnolev := 0;
END DevCPE.
