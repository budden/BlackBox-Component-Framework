MODULE DevCPV486;
(**
	project	= "BlackBox"
	organization	= "www.oberon.ch"
	contributors	= "Oberon microsystems"
	version	= "System/Rsrc/About"
	copyright	= "System/Rsrc/About"
	license	= "Docu/BB-License"
	references	= "ftp://ftp.inf.ethz.ch/pub/software/Oberon/OberonV4/Docu/OP2.Paper.ps"
	changes	= ""
	issues	= ""

**)

	(* bh 3.12.93 / 7.9.94 *)
	(* bh 25.9.95 COM support *)
	(* bh 10.10.95 return with jumps *)
	(* bh 6.12.95 COM support changed *)
	(* bh 9.12.95 longchar & largeint support *)
	(* bh 21.2.96 [new] & [iid] handling *)
	(* bh 28.5.96 union support *)
	(* bh 7.1.97 Checkpc calls chaged *)
	(* bh 17.12.97 Java frontend extensions *)
	(* bh 31.8.99 oveflow check in TypeSize *)
	(* cp 23.11.99 correction in CompStat (two assertions eliminated, according to bh) *)
	(* bh 30.4.00 correction in Dim *)
	(* bh 27.10.00 error for untagged arrays in Parameters *)
	(* bh 8.2.01 nil tests for var par & receiver (ActualPar) *)
	(* bh 8.2.01 FPU control register handling (CheckFpu, procs, Module) *)
	(* bh 8.2.01 check for wrong extensions (EnumTProcs, CountTProcs) *)

	IMPORT SYSTEM, DevCPM, DevCPT, DevCPE, DevCPH, DevCPL486, DevCPC486;
	
	CONST
		processor* = 10; (* for i386 *)

		(* object modes *)
		Var = 1; VarPar = 2; Con = 3; Fld = 4; Typ = 5; LProc = 6; XProc = 7;
		SProc = 8; CProc = 9; IProc = 10; Mod = 11; Head = 12; TProc = 13;
		
		(* item modes for i386 *)
		Ind = 14; Abs = 15; Stk = 16; Cond = 17; Reg = 18; DInd = 19;

		(* symbol values and ops *)
		times = 1; slash = 2; div = 3; mod = 4;
		and = 5; plus = 6; minus = 7; or = 8; eql = 9;
		neq = 10; lss = 11; leq = 12; gtr = 13; geq = 14;
		in = 15; is = 16; ash = 17; msk = 18; len = 19;
		conv = 20; abs = 21; cap = 22; odd = 23; not = 33;
		(*SYSTEM*)
		adr = 24; cc = 25; bit = 26; lsh = 27; rot = 28; val = 29;
		min = 34; max = 35; typfn = 36;
		thisrecfn = 45; thisarrfn = 46;
		shl = 50; shr = 51; lshr = 52; xor = 53;

		(* structure forms *)
		Undef = 0; Byte = 1; Bool = 2; Char8 = 3; Int8 = 4; Int16 = 5; Int32 = 6;
		Real32 = 7; Real64 = 8; Set = 9; String8 = 10; NilTyp = 11; NoTyp = 12;
		Pointer = 13; ProcTyp = 14; Comp = 15;
		Char16 = 16; String16 = 17; Int64 = 18;
		VString16to8 = 29; VString8 = 30; VString16 = 31;
		realSet = {Real32, Real64};

		(* composite structure forms *)
		Basic = 1; Array = 2; DynArr = 3; Record = 4;

		(* nodes classes *)
		Nvar = 0; Nvarpar = 1; Nfield = 2; Nderef = 3; Nindex = 4; Nguard = 5; Neguard = 6;
		Nconst = 7; Ntype = 8; Nproc = 9; Nupto = 10; Nmop = 11; Ndop = 12; Ncall = 13;
		Ninittd = 14; Nif = 15; Ncaselse = 16; Ncasedo = 17; Nenter = 18; Nassign = 19;
		Nifelse =20; Ncase = 21; Nwhile = 22; Nrepeat = 23; Nloop = 24; Nexit = 25;
		Nreturn = 26; Nwith = 27; Ntrap = 28; Ncomp = 30;
		Ndrop = 50; Nlabel = 51; Ngoto = 52; Njsr = 53; Nret = 54; Ncmp = 55;

		(*function number*)
		assign = 0; newfn = 1; incfn = 13; decfn = 14;
		inclfn = 15; exclfn = 16; copyfn = 18; assertfn = 32;

		(*SYSTEM function number*)
		getfn = 24; putfn = 25; getrfn = 26; putrfn = 27; sysnewfn = 30; movefn = 31;
		
		(* COM function number *)
		validfn = 40; queryfn = 42;
		
		(* procedure flags (conval.setval) *)
		hasBody = 1; isRedef = 2; slNeeded = 3; imVar = 4; isHidden = 29; isGuarded = 30; isCallback = 31;

		(* attribute flags (attr.adr, struct.attribute, proc.conval.setval) *)
		newAttr = 16; absAttr = 17; limAttr = 18; empAttr = 19; extAttr = 20;
		
		(* case statement flags (conval.setval) *)
		useTable = 1; useTree = 2;
		
		(* registers *)
		AX = 0; CX = 1; DX = 2; BX = 3; SP = 4; BP = 5; SI = 6; DI = 7; AH = 4; CH = 5; DH = 6; BH = 7;
		stk = 31; mem = 30; con = 29; float = 28; high = 27; short = 26; deref = 25; loaded = 24;
		wreg = {AX, BX, CX, DX, SI, DI};

		(* module visibility of objects *)
		internal = 0; external = 1; externalR = 2; inPar = 3; outPar = 4;

		(* sysflag *)
		untagged = 1; noAlign = 3; align2 = 4; align8 = 6; union = 7;
		interface = 10; guarded = 8; noframe = 16;
		nilBit = 1; enumBits = 8; new = 1; iid = 2;
		stackArray = 120;
		
		(* system trap numbers *)
		withTrap = -1; caseTrap = -2; funcTrap = -3; typTrap = -4;
		recTrap = -5; ranTrap = -6; inxTrap = -7; copyTrap = -8;
		
		ParOff = 8;
		interfaceSize = 16;	(* SIZE(Kernel.Interface) *)
		addRefFP = 4E27A847H;	(* fingerprint of AddRef and Release procedures *)
		intHandlerFP = 24B0EAE3H;	(* fingerprint of InterfaceTrapHandler *)
		numPreIntProc = 2;
		
		
	VAR
		Exit, Return: DevCPL486.Label;
		assert, sequential: BOOLEAN;
		nesting, actual: INTEGER;
		query, addRef, release, release2: DevCPT.Object;
		
	PROCEDURE Init*(opt: SET);
		CONST ass = 2;
	BEGIN
		DevCPL486.Init(opt); DevCPC486.Init(opt);
		assert := ass IN opt;
		DevCPM.breakpc := MAX(INTEGER);
		query := NIL; addRef := NIL; release := NIL; release2 := NIL; DevCPC486.intHandler := NIL;
	END Init;
	
	PROCEDURE Close*;
	BEGIN
		DevCPL486.Close
	END Close;

	PROCEDURE Align(VAR offset: INTEGER; align: INTEGER);
	BEGIN
		CASE align OF
		   1: (* ok *)
		| 2: INC(offset, offset MOD 2)
		| 4: INC(offset, (-offset) MOD 4)
		| 8: INC(offset, (-offset) MOD 8)
		END
	END Align;
	
	PROCEDURE NegAlign(VAR offset: INTEGER; align: INTEGER);
	BEGIN
		CASE align OF
		   1: (* ok *)
		| 2: DEC(offset, offset MOD 2)
		| 4: DEC(offset, offset MOD 4)
		| 8: DEC(offset, offset MOD 8)
		END
	END NegAlign;
	
	PROCEDURE Base(typ: DevCPT.Struct; limit: INTEGER): INTEGER;	(* typ.comp # DynArr *)
		VAR align: INTEGER;
	BEGIN
		WHILE typ.comp = Array DO typ := typ.BaseTyp END ;
		IF typ.comp = Record THEN
			align := typ.align
		ELSE
			align := typ.size;
		END;
		IF align > limit THEN RETURN limit ELSE RETURN align END
	END Base;

(* -----------------------------------------------------
	reference implementation of TypeSize for portable symbol files
	mandatory for all non-system structures

	PROCEDURE TypeSize (typ: DevCPT.Struct);
		VAR f, c: SHORTINT; offset: LONGINT; fld: DevCPT.Object; btyp: DevCPT.Struct;
	BEGIN
		IF typ.size = -1 THEN
			f := typ.form; c := typ.comp; btyp := typ.BaseTyp;
			IF c = Record THEN
				IF btyp = NIL THEN offset := 0 ELSE TypeSize(btyp); offset := btyp.size END;
				fld := typ.link;
				WHILE (fld # NIL) & (fld.mode = Fld) DO
					btyp := fld.typ; TypeSize(btyp);
					IF btyp.size >= 4 THEN INC(offset, (-offset) MOD 4)
					ELSIF btyp.size >= 2 THEN INC(offset, offset MOD 2)
					END;
					fld.adr := offset; INC(offset, btyp.size);
					fld := fld.link
				END;
				IF offset > 2 THEN INC(offset, (-offset) MOD 4) END;
				typ.size := offset; typ.align := 4;
				typ.n := -1  (* methods not counted yet *)
			ELSIF c = Array THEN
				TypeSize(btyp);
				typ.size := typ.n * btyp.size
			ELSIF f = Pointer THEN
				typ.size := DevCPM.PointerSize
			ELSIF f = ProcTyp THEN
				typ.size := DevCPM.ProcSize
			ELSE (* c = DynArr *)
				TypeSize(btyp);
				IF btyp.comp = DynArr THEN typ.size := btyp.size + 4
				ELSE typ.size := 8
				END
			END
		END
	END TypeSize;

----------------------------------------------------- *)

	PROCEDURE GTypeSize (typ: DevCPT.Struct; guarded: BOOLEAN);
		VAR f, c: BYTE; offset, align, falign, alignLimit: INTEGER;
			fld: DevCPT.Object; btyp: DevCPT.Struct; name: DevCPT.Name;
	BEGIN
		IF typ.untagged THEN guarded := TRUE END;
		IF typ = DevCPT.undftyp THEN DevCPM.err(58)
		ELSIF typ.size = -1 THEN
			f := typ.form; c := typ.comp; btyp := typ.BaseTyp;
			IF c = Record THEN
				IF btyp = NIL THEN offset := 0; align := 1;
				ELSE GTypeSize(btyp, guarded); offset := btyp.size; align := btyp.align
				END ;
				IF typ.sysflag = noAlign THEN alignLimit := 1
				ELSIF typ.sysflag = align2 THEN alignLimit := 2
				ELSIF typ.sysflag = align8 THEN alignLimit := 8
				ELSE alignLimit := 4
				END;
				fld := typ.link;
				WHILE (fld # NIL) & (fld.mode = Fld) DO
					btyp := fld.typ; GTypeSize(btyp, guarded);
					IF typ.sysflag > 0 THEN falign := Base(btyp, alignLimit)
					ELSIF btyp.size >= 4 THEN falign := 4
					ELSIF btyp.size >= 2 THEN falign := 2
					ELSE falign := 1
					END;
					IF typ.sysflag = union THEN
						fld.adr := 0;
						IF btyp.size > offset THEN offset := btyp.size END;
					ELSE
						Align(offset, falign);
						fld.adr := offset;
						IF offset <= MAX(INTEGER) - 4 - btyp.size THEN INC(offset, btyp.size)
						ELSE offset := 4; DevCPM.Mark(214, typ.txtpos)
						END						
					END;
					IF falign > align THEN align := falign END ;
					fld := fld.link
				END;
(*
				IF (typ.sysflag = interface) & (typ.BaseTyp = NIL) THEN
					fld := DevCPT.NewObj(); fld.name^ := DevCPM.HdPtrName; fld.mode := Fld;
					fld.typ := DevCPT.undftyp; fld.adr := 8;
					fld.right := typ.link; typ.link := fld;
					fld := DevCPT.NewObj(); fld.name^ := DevCPM.HdPtrName; fld.mode := Fld;
					fld.typ := DevCPT.undftyp; fld.adr := 12;
					typ.link.link := fld; typ.link.left := fld;
					offset := interfaceSize; align := 4
				END;
*)
				IF typ.sysflag <= 0 THEN align := 4 END;
				typ.align := align;
				IF (typ.sysflag > 0) OR (offset > 2) THEN Align(offset, align) END;
				typ.size := offset;
				typ.n := -1  (* methods not counted yet *)
			ELSIF c = Array THEN
				GTypeSize(btyp, guarded);
				IF (btyp.size = 0) OR (typ.n <= MAX(INTEGER) DIV btyp.size) THEN typ.size := typ.n * btyp.size
				ELSE typ.size := 4; DevCPM.Mark(214, typ.txtpos)
				END
			ELSIF f = Pointer THEN
				typ.size := DevCPM.PointerSize;
				IF guarded & ~typ.untagged THEN DevCPM.Mark(143, typ.txtpos) END
			ELSIF f = ProcTyp THEN
				typ.size := DevCPM.ProcSize
			ELSE (* c = DynArr *)
				GTypeSize(btyp, guarded);
				IF (typ.sysflag = untagged) OR typ.untagged THEN typ.size := 4
				ELSE
					IF btyp.comp = DynArr THEN typ.size := btyp.size + 4
					ELSE typ.size := 8
					END
				END
			END
		END
	END GTypeSize;
	
	PROCEDURE TypeSize*(typ: DevCPT.Struct);	(* also called from DevCPT.InStruct for arrays *)
	BEGIN
		GTypeSize(typ, FALSE)
	END TypeSize;
	
	PROCEDURE GetComKernel;
		VAR name: DevCPT.Name; mod: DevCPT.Object;
	BEGIN
		IF addRef = NIL THEN
			DevCPT.OpenScope(SHORT(SHORT(-DevCPT.nofGmod)), NIL);
			DevCPT.topScope.name := DevCPT.NewName("$$");
			name := "AddRef"; DevCPT.Insert(name, addRef);
			addRef.mode := XProc;
			addRef.fprint := addRefFP;
			addRef.fpdone := TRUE;
			name := "Release"; DevCPT.Insert(name, release);
			release.mode := XProc;
			release.fprint := addRefFP;
			release.fpdone := TRUE;
			name := "Release2"; DevCPT.Insert(name, release2);
			release2.mode := XProc;
			release2.fprint := addRefFP;
			release2.fpdone := TRUE;
			name := "InterfaceTrapHandler"; DevCPT.Insert(name, DevCPC486.intHandler);
			DevCPC486.intHandler.mode := XProc;
			DevCPC486.intHandler.fprint := intHandlerFP;
			DevCPC486.intHandler.fpdone := TRUE;
			DevCPT.GlbMod[DevCPT.nofGmod] := DevCPT.topScope;
			INC(DevCPT.nofGmod);
			DevCPT.CloseScope;
		END
	END GetComKernel;

	PROCEDURE EnumTProcs(rec: DevCPT.Struct);	(* method numbers in declaration order *)
		VAR btyp: DevCPT.Struct; obj, redef: DevCPT.Object;
	BEGIN
		IF rec.n = -1 THEN
			rec.n := 0; btyp := rec.BaseTyp;
			IF btyp # NIL THEN
				EnumTProcs(btyp); rec.n := btyp.n;
			END;
			obj := rec.strobj.link;
			WHILE obj # NIL DO
				DevCPT.FindBaseField(obj.name^, rec, redef);
				IF redef # NIL THEN obj.num := redef.num (*mthno*);
					IF ~(isRedef IN obj.conval.setval) OR (redef.conval.setval * {extAttr, absAttr, empAttr} = {}) THEN
						DevCPM.Mark(119, rec.txtpos)
					END
				ELSE obj.num := rec.n; INC(rec.n)
				END ;
				IF obj.conval.setval * {hasBody, absAttr, empAttr} = {} THEN DevCPM.Mark(129, obj.adr) END;
				obj := obj.nlink
			END
		END
	END EnumTProcs;

	PROCEDURE CountTProcs(rec: DevCPT.Struct);
		VAR btyp: DevCPT.Struct; comProc: INTEGER; m, rel: DevCPT.Object; name: DevCPT.Name;

		PROCEDURE TProcs(obj: DevCPT.Object);	(* obj.mnolev = 0, TProcs of base type already counted *)
			VAR redef: DevCPT.Object;
		BEGIN
			IF obj # NIL THEN
				TProcs(obj.left);
				IF obj.mode = TProc THEN
					DevCPT.FindBaseField(obj.name^, rec, redef);
					(* obj.adr := 0 *)
					IF redef # NIL THEN
						obj.num := redef.num (*mthno*);
						IF (redef.link # NIL) & (redef.link.typ.sysflag = interface) THEN
							obj.num := numPreIntProc + comProc - 1 - obj.num
						END;
						IF ~(isRedef IN obj.conval.setval) OR (redef.conval.setval * {extAttr, absAttr, empAttr} = {}) THEN
							DevCPM.Mark(119, rec.txtpos)
						END
					ELSE obj.num := rec.n; INC(rec.n)
					END ;
					IF obj.conval.setval * {hasBody, absAttr, empAttr} = {} THEN DevCPM.Mark(129, obj.adr) END
				END ;
				TProcs(obj.right)
			END
		END TProcs;

	BEGIN
		IF rec.n = -1 THEN
			comProc := 0;
			IF rec.untagged THEN rec.n := 0 ELSE rec.n := DevCPT.anytyp.n END;
			btyp := rec.BaseTyp;
			IF btyp # NIL THEN
				IF btyp.sysflag = interface THEN
					EnumTProcs(btyp); rec.n := btyp.n + numPreIntProc; comProc := btyp.n;
				ELSE
					CountTProcs(btyp); rec.n := btyp.n
				END
			END;
			WHILE (btyp # NIL) & (btyp # DevCPT.undftyp) & (btyp.sysflag # interface) DO btyp := btyp.BaseTyp END;
			IF (btyp # NIL) & (btyp.sysflag = interface) THEN
				IF comProc > 0 THEN
					name := "QueryInterface"; DevCPT.FindField(name, rec, m);
					IF m.link.typ.sysflag = interface THEN
						DevCPT.InsertField(name, rec, m); m.mode := TProc; m.typ := rec;
						m.conval := DevCPT.NewConst(); m.conval.setval := {isRedef, hasBody, isCallback, extAttr};
						m.nlink := query; query := m
					END;
					name := "AddRef";
					DevCPT.InsertField(name, rec, m); m.mode := TProc; m.mnolev := 0;
					m.conval := DevCPT.NewConst(); m.conval.setval := {isRedef, hasBody, isCallback, isHidden, extAttr};
					GetComKernel; addRef.used := TRUE; m.adr := -1; m.nlink := addRef;
				END;
				name := "RELEASE";
				DevCPT.FindField(name, rec, rel);
				IF (rel # NIL) & (rel.link.typ = DevCPT.anyptrtyp) THEN rel := NIL END;
				IF (comProc > 0) OR (rel # NIL) THEN
					name := "Release";
					DevCPT.InsertField(name, rec, m); m.mode := TProc; m.mnolev := 0;
					m.conval := DevCPT.NewConst(); m.conval.setval := {isRedef, hasBody, isCallback, isHidden, extAttr};
					GetComKernel; m.adr := -1;
					IF rel # NIL THEN release2.used := TRUE; m.nlink := release2
					ELSE release.used := TRUE; m.nlink := release
					END
				END
			END;
			TProcs(rec.link);
		END
	END CountTProcs;
	
	PROCEDURE ^Parameters(firstPar, proc: DevCPT.Object);

	PROCEDURE ^TProcedures(obj: DevCPT.Object);

	PROCEDURE TypeAlloc(typ: DevCPT.Struct);
		VAR f, c: SHORTINT; fld: DevCPT.Object; btyp: DevCPT.Struct;
	BEGIN
		IF ~typ.allocated THEN	(* not imported, not predefined, not allocated yet *)
			typ.allocated := TRUE;
			TypeSize(typ);
			f := typ.form; c := typ.comp; btyp := typ.BaseTyp;
			IF c = Record THEN
				IF typ.sysflag = interface THEN
					EnumTProcs(typ);
				ELSE
					CountTProcs(typ)
				END;
				IF typ.extlev > 14 THEN DevCPM.Mark(233, typ.txtpos) END;
				IF btyp # NIL THEN TypeAlloc(btyp) END;
				IF ~typ.untagged THEN DevCPE.AllocTypDesc(typ) END;
				fld := typ.link;
				WHILE (fld # NIL) & (fld.mode = Fld) DO
					TypeAlloc(fld.typ); fld := fld.link
				END;
				TProcedures(typ.link)
			ELSIF f = Pointer THEN
				IF btyp = DevCPT.undftyp THEN DevCPM.Mark(128, typ.txtpos)
				ELSE TypeAlloc(btyp);
				END
			ELSIF f = ProcTyp THEN
				TypeAlloc(btyp);
				Parameters(typ.link, NIL)
			ELSE (* c IN {Array, DynArr} *) 
				TypeAlloc(btyp);
				IF (btyp.comp = DynArr) & btyp.untagged & ~typ.untagged THEN DevCPM.Mark(225, typ.txtpos) END;
			END
		END
	END TypeAlloc;

	PROCEDURE NumOfIntProc (typ: DevCPT.Struct): INTEGER;
	BEGIN
		WHILE (typ # NIL) & (typ.sysflag # interface) DO typ := typ.BaseTyp END;
		IF typ # NIL THEN RETURN typ.n
		ELSE RETURN 0
		END
	END NumOfIntProc;
	
	PROCEDURE Parameters(firstPar, proc: DevCPT.Object);
	(* firstPar.mnolev = 0 *)
		VAR par: DevCPT.Object; typ: DevCPT.Struct; padr, vadr: INTEGER;
	BEGIN
		padr := ParOff; par := firstPar;
		WHILE par # NIL DO
			typ := par.typ; TypeAlloc(typ);
			par.adr := padr;
			IF (par.mode = VarPar) & (typ.comp # DynArr) THEN
				IF (typ.comp = Record) & ~typ.untagged THEN INC(padr, 8)
				ELSE INC(padr, 4)
				END
			ELSE
				IF (par.mode = Var) & (typ.comp = DynArr) & typ.untagged THEN DevCPM.err(145) END;
				INC(padr, typ.size); Align(padr, 4)
			END;
			par := par.link
		END;
		IF proc # NIL THEN
			IF proc.mode = XProc THEN
				INCL(proc.conval.setval, isCallback)
			ELSIF (proc.mode = TProc)
				& (proc.num >= numPreIntProc)
				& (proc.num < numPreIntProc + NumOfIntProc(proc.link.typ))
			THEN
				INCL(proc.conval.setval, isCallback);
				INCL(proc.conval.setval, isGuarded)
			END;
			IF proc.sysflag = guarded THEN INCL(proc.conval.setval, isGuarded) END;
			IF isGuarded IN proc.conval.setval THEN
				GetComKernel; vadr := -24
			ELSE
				vadr := 0;
				IF imVar IN proc.conval.setval THEN DEC(vadr, 4) END;
				IF isCallback IN proc.conval.setval THEN DEC(vadr, 8) END
			END;
			proc.conval.intval := padr; proc.conval.intval2 := vadr;
		END
	END Parameters;
	
	PROCEDURE Variables(var: DevCPT.Object; VAR varSize: INTEGER);
	(* allocates only offsets, regs allocated in DevCPC486.Enter *)
		VAR adr: INTEGER; typ: DevCPT.Struct;
	BEGIN
		adr := varSize;
		WHILE var # NIL DO
			typ := var.typ; TypeAlloc(typ);
			DEC(adr, typ.size); NegAlign(adr, Base(typ, 4));
			var.adr := adr;
			var := var.link
		END;
		NegAlign(adr, 4); varSize := adr
	END Variables;
	
	PROCEDURE ^Objects(obj: DevCPT.Object);

	PROCEDURE Procedure(obj: DevCPT.Object);
	(* obj.mnolev = 0 *)
		VAR oldPos: INTEGER;
	BEGIN
		oldPos := DevCPM.errpos; DevCPM.errpos := obj.scope.adr;
		TypeAlloc(obj.typ);
		Parameters(obj.link, obj);
		IF ~(hasBody IN obj.conval.setval) THEN DevCPM.Mark(129, obj.adr) END ;
		Variables(obj.scope.scope, obj.conval.intval2);	(* local variables *)
		Objects(obj.scope.right);
		DevCPM.errpos := oldPos
	END Procedure;

	PROCEDURE TProcedures(obj: DevCPT.Object);
	(* obj.mnolev = 0 *)
		VAR par: DevCPT.Object; psize: INTEGER;
	BEGIN
		IF obj # NIL THEN
			TProcedures(obj.left);
			IF (obj.mode = TProc) & (obj.scope # NIL) THEN
				TypeAlloc(obj.typ);
				Parameters(obj.link, obj);
				Variables(obj.scope.scope, obj.conval.intval2);	(* local variables *)
				Objects(obj.scope.right);
			END ;
			TProcedures(obj.right)
		END
	END TProcedures;

	PROCEDURE Objects(obj: DevCPT.Object);
	BEGIN
		IF obj # NIL THEN
			Objects(obj.left);
			IF obj.mode IN {Con, Typ, LProc, XProc, CProc, IProc} THEN
				IF (obj.mode IN {Con, Typ}) THEN TypeAlloc(obj.typ);
				ELSE Procedure(obj)
				END
			END ;
			Objects(obj.right)
		END
	END Objects;

	PROCEDURE Allocate*;
		VAR gvarSize: INTEGER; name: DevCPT.Name;
	BEGIN
		DevCPM.errpos := DevCPT.topScope.adr;	(* text position of scope used if error *)
		gvarSize := 0;
		Variables(DevCPT.topScope.scope, gvarSize); DevCPE.dsize := -gvarSize;
		Objects(DevCPT.topScope.right)
	END Allocate;
	
	(************************)

	PROCEDURE SameExp (n1, n2: DevCPT.Node): BOOLEAN;
	BEGIN
		WHILE (n1.class = n2.class) & (n1.typ = n2.typ) DO
			CASE n1.class OF
			| Nvar, Nvarpar, Nproc: RETURN n1.obj = n2.obj
			| Nconst: RETURN (n1.typ.form IN {Int8..Int32}) & (n1.conval.intval = n2.conval.intval)
			| Nfield: IF n1.obj # n2.obj THEN RETURN FALSE END
			| Nderef, Nguard:
			| Nindex: IF ~SameExp(n1.right, n2.right) THEN RETURN FALSE END
			| Nmop: IF (n1.subcl # n2.subcl) OR (n1.subcl = is) THEN RETURN FALSE END
			| Ndop: IF (n1.subcl # n2.subcl) OR ~SameExp(n1.right, n2.right) THEN RETURN FALSE END
			ELSE RETURN FALSE
			END ;
			n1 := n1.left; n2 := n2.left
		END;
		RETURN FALSE
	END SameExp;
	
	PROCEDURE Check (n: DevCPT.Node; VAR used: SET; VAR size: INTEGER);
		VAR ux, uy: SET; sx, sy, sf: INTEGER; f: BYTE;
	BEGIN
		used := {}; size := 0;
		WHILE n # NIL DO
			IF n.class # Ncomp THEN
				Check(n.left, ux, sx);
				Check(n.right, uy, sy)
			END;
			ux := ux + uy; sf := 0;
			CASE n.class OF
			| Nvar, Nvarpar:
					IF (n.class = Nvarpar) OR  (n.typ.comp = DynArr) OR
						(n.obj.mnolev > 0) &
						(DevCPC486.imLevel[n.obj.mnolev] < DevCPC486.imLevel[DevCPL486.level]) THEN sf := 1 END
			| Nguard: sf := 2
			| Neguard, Nderef: sf := 1
			| Nindex:
					IF (n.right.class # Nconst) OR (n.left.typ.comp = DynArr) THEN sf := 1 END;
					IF sx > 0 THEN INC(sy) END
			| Nmop:
					CASE n.subcl OF
					| is, adr, typfn, minus, abs, cap, val: sf := 1
					| bit: sf := 2; INCL(ux, CX)
					| conv:
							IF n.typ.form = Int64 THEN sf := 2
							ELSIF ~(n.typ.form IN realSet) THEN sf := 1;
								IF n.left.typ.form IN realSet THEN INCL(ux, AX) END
							END
					| odd, cc, not:
					END
			| Ndop:
					f := n.left.typ.form;
					IF f # Bool THEN
						CASE n.subcl OF
						| times:
								sf := 1;
								IF f = Int8 THEN INCL(ux, AX) END
						| div, mod:
								sf := 3; INCL(ux, AX);
								IF f > Int8 THEN INCL(ux, DX) END
						| eql..geq:
								IF f IN {String8, String16, Comp} THEN ux := ux + {AX, CX, SI, DI}; sf := 4
								ELSIF f IN realSet THEN INCL(ux, AX); sf := 1
								ELSE sf := 1
								END
						| ash, lsh, rot:
								IF n.right.class = Nconst THEN sf := 1 ELSE sf := 2; INCL(ux, CX) END
						| slash, plus, minus, msk, in, bit:
								sf := 1
						| len:
								IF f IN {String8, String16} THEN ux := ux + {AX, CX, DI}; sf := 3
								ELSE sf := 1
								END
						| min, max:
								sf := 1;
								IF f IN realSet THEN INCL(ux, AX) END
						| queryfn:
								ux := ux + {CX, SI, DI}; sf := 4
						END;
						IF sy > sx THEN INC(sx) ELSE INC(sy) END
					END
			| Nupto:
					IF (n.right.class = Nconst) OR (n.left.class = Nconst) THEN sf := 2
					ELSE sf := 3
					END;
					INCL(ux, CX); INC(sx)
			| Ncall, Ncomp:
					sf := 10; ux := wreg + {float}
			| Nfield, Nconst, Nproc, Ntype:
			END;
			used := used + ux;
			IF sx > size THEN size := sx END;
			IF sy > size THEN size := sy END;
			IF sf > size THEN size := sf END;
			n := n.link
		END;
		IF size > 10 THEN size := 10 END
	END Check;
	
	PROCEDURE^ expr (n: DevCPT.Node; VAR x: DevCPL486.Item; hint, stop: SET);
	
	PROCEDURE DualExp (left, right: DevCPT.Node; VAR x, y: DevCPL486.Item; hx, hy, stpx, stpy: SET);
		VAR ux, uy: SET; sx, sy: INTEGER;
	BEGIN
		Check(left, ux, sx); Check(right, uy, sy);
		IF sy > sx THEN
			expr(right, y, hy + stpy, ux + stpy * {AX, CX});
			expr(left, x, hx, stpx);
			DevCPC486.Assert(y, hy, stpy)
		ELSE
			expr(left, x, hx + stpx, uy);
			expr(right, y, hy, stpy);
			DevCPC486.Assert(x, hx, stpx)
		END;
	END DualExp;

	PROCEDURE IntDOp (n: DevCPT.Node; VAR x: DevCPL486.Item; hint: SET);
		VAR y: DevCPL486.Item; rev: BOOLEAN;
	BEGIN
		DualExp(n.left, n.right, x, y, hint, hint, {stk}, {stk});
		IF (x.mode = Reg) & DevCPC486.Fits(x, hint) THEN
			DevCPC486.IntDOp(x, y, n.subcl, FALSE)
		ELSIF (y.mode = Reg) & DevCPC486.Fits(y, hint) THEN
			DevCPC486.IntDOp(y, x, n.subcl, TRUE); x := y
		ELSIF x.mode # Reg THEN
			DevCPC486.Load(x, hint, {con}); DevCPC486.IntDOp(x, y, n.subcl, FALSE)
		ELSIF y.mode # Reg THEN
			DevCPC486.Load(y, hint, {con}); DevCPC486.IntDOp(y, x, n.subcl, TRUE); x := y
		ELSE
			DevCPC486.IntDOp(x, y, n.subcl, FALSE)
		END
	END IntDOp;
	
	PROCEDURE FloatDOp (n: DevCPT.Node; VAR x: DevCPL486.Item);
		VAR y: DevCPL486.Item; ux, uy, uf: SET; sx, sy: INTEGER;
	BEGIN
		Check(n.left, ux, sx); Check(n.right, uy, sy);
		IF (n.subcl = min) OR (n.subcl = max) THEN uf := {AX} ELSE uf := {} END;
		IF (sy > sx) OR (sy = sx) & ((n.subcl = mod) OR (n.subcl = ash)) THEN
			expr(n.right, x, {}, ux + {mem, stk});
			expr(n.left, y, {}, uf);
			DevCPC486.FloatDOp(x, y, n.subcl, TRUE)
		ELSIF float IN uy THEN (* function calls in both operands *)
			expr(n.left, y, {}, uy + {mem});
			expr(n.right, x, {}, {mem, stk});
			DevCPC486.FloatDOp(x, y, n.subcl, TRUE)
		ELSE
			expr(n.left, x, {}, uy + {mem, stk});
			expr(n.right, y, {}, uf);
			DevCPC486.FloatDOp(x, y, n.subcl, FALSE)
		END
	END FloatDOp;
	
	PROCEDURE design (n: DevCPT.Node; VAR x: DevCPL486.Item; hint, stop: SET);
		VAR obj: DevCPT.Object; y: DevCPL486.Item; ux, uy: SET; sx, sy: INTEGER;
	BEGIN
		CASE n.class OF
		  Nvar, Nvarpar:
				obj := n.obj; x.mode := obj.mode; x.obj := obj; x.scale := 0;
				IF obj.typ.comp = DynArr THEN x.mode := VarPar END;
				IF obj.mnolev < 0 THEN x.offset := 0; x.tmode := Con
				ELSIF x.mode = Var THEN x.offset := obj.adr; x.tmode := Con
				ELSE x.offset := 0; x.tmode := VarPar
				END
		| Nfield:
				design(n.left, x, hint, stop); DevCPC486.Field(x, n.obj)
		| Nderef:
				IF n.subcl # 0 THEN
					expr(n.left, x, hint, stop);
					IF n.typ.form = String8 THEN x.form := VString8 ELSE x.form := VString16 END
				ELSE
					expr(n.left, x, hint, stop + {mem} - {loaded}); DevCPC486.DeRef(x)
				END
		| Nindex:
				Check(n.left, ux, sx); Check(n.right, uy, sy);
				IF wreg - uy = {} THEN
					expr(n.right, y, hint + stop, ux);
					design(n.left, x, hint, stop);
					IF x.scale # 0 THEN DevCPC486.Index(x, y, {}, {}) ELSE DevCPC486.Index(x, y, hint, stop) END
				ELSE
					design(n.left, x, hint, stop + uy);
					IF x.scale # 0 THEN expr(n.right, y, {}, {}); DevCPC486.Index(x, y, {}, {})
					ELSE expr(n.right, y, hint, stop); DevCPC486.Index(x, y, hint, stop)
					END
				END
		| Nguard, Neguard:
				IF n.typ.form = Pointer THEN
					IF loaded IN stop THEN expr(n.left, x, hint, stop) ELSE expr(n.left, x, hint, stop + {mem}) END
				ELSE design(n.left, x, hint, stop)
				END;
				DevCPC486.TypTest(x, n.typ, TRUE, n.class = Neguard)
		| Nproc:
				obj := n.obj; x.mode := obj.mode; x.obj := obj;
				IF x.mode = TProc THEN x.offset := obj.num; (*mthno*) x.scale := n.subcl (* super *) END
		END;
		x.typ := n.typ
	END design;
	
	PROCEDURE IsAllocDynArr (x: DevCPT.Node): BOOLEAN;
	BEGIN
		IF (x.typ.comp = DynArr) & ~x.typ.untagged THEN
			WHILE x.class = Nindex DO x := x.left END;
			IF x.class = Nderef THEN RETURN TRUE END
		END;
		RETURN FALSE
	END IsAllocDynArr;
	
	PROCEDURE StringOp (left, right: DevCPT.Node; VAR x, y: DevCPL486.Item; useLen: BOOLEAN);
		VAR ax, ay: DevCPL486.Item; ux: SET; sx: INTEGER;
	BEGIN
		Check(left, ux, sx);
		expr(right, y, wreg - {SI} + ux, {});
		ay := y; DevCPC486.GetAdr(ay, wreg - {SI} + ux, {}); DevCPC486.Assert(ay, wreg - {SI}, ux);
		IF useLen & IsAllocDynArr(left) THEN	(* keep len descriptor *)
			design(left, x, wreg - {CX}, {loaded});
			DevCPC486.Prepare(x, wreg - {CX} + {deref}, {DI})
		ELSE
			expr(left, x, wreg - {DI}, {})
		END;
		ax := x; DevCPC486.GetAdr(ax, {}, wreg - {DI} + {stk, con});
		DevCPC486.Load(ay, {}, wreg - {SI} + {con});
		DevCPC486.Free(ax); DevCPC486.Free(ay)
	END StringOp;
	
	PROCEDURE AdrExpr (n: DevCPT.Node; VAR x: DevCPL486.Item; hint, stop: SET);
	BEGIN
		IF n.class < Nconst THEN
			design(n, x, hint + stop, {loaded}); DevCPC486.Prepare(x, hint + {deref}, stop)
		ELSE expr(n, x, hint, stop)
		END
	END AdrExpr;
	
	(* ---------- interface pointer reference counting ---------- *)
	
	PROCEDURE HandleIPtrs (typ: DevCPT.Struct; VAR x, y: DevCPL486.Item; add, rel, init: BOOLEAN);
	
		PROCEDURE FindPtrs (typ: DevCPT.Struct; adr: INTEGER);
			VAR fld: DevCPT.Object; btyp: DevCPT.Struct; i, n: INTEGER;
		BEGIN
			IF (typ.form = Pointer) & (typ.sysflag = interface) THEN
				IF add THEN DevCPC486.IPAddRef(y, adr, TRUE) END;
				IF rel THEN DevCPC486.IPRelease(x, adr, TRUE, init) END
			ELSIF (typ.comp = Record) & (typ.sysflag # union) THEN
				btyp := typ.BaseTyp;
				IF btyp # NIL THEN FindPtrs(btyp, adr) END ;
				fld := typ.link;
				WHILE (fld # NIL) & (fld.mode = Fld) DO
					IF (fld.sysflag = interface) & (fld.name^ = DevCPM.HdUtPtrName) THEN
						IF add THEN DevCPC486.IPAddRef(y, fld.adr + adr, TRUE) END;
						IF rel THEN DevCPC486.IPRelease(x, fld.adr + adr, TRUE, init) END
					ELSE FindPtrs(fld.typ, fld.adr + adr)
					END;
					fld := fld.link
				END
			ELSIF typ.comp = Array THEN
				btyp := typ.BaseTyp; n := typ.n;
				WHILE btyp.comp = Array DO n := btyp.n * n; btyp := btyp.BaseTyp END ;
				IF DevCPC486.ContainsIPtrs(btyp) THEN
					i := 0;
					WHILE i < n DO FindPtrs(btyp, adr); INC(adr, btyp.size); INC(i) END
				END
			ELSIF typ.comp = DynArr THEN 
				IF DevCPC486.ContainsIPtrs(typ) THEN DevCPM.err(221) END
			END
		END FindPtrs;
	
	BEGIN
		FindPtrs(typ, 0)
	END HandleIPtrs;
	
	PROCEDURE CountedPtr (n: DevCPT.Node): BOOLEAN;
	BEGIN
		RETURN (n.typ.form = Pointer) & (n.typ.sysflag = interface)
			& ((n.class = Ncall) OR (n.class = Ncomp) & (n.right.class = Ncall))
	END CountedPtr;
	
	PROCEDURE IPAssign (nx, ny: DevCPT.Node; VAR x, y: DevCPL486.Item; ux: SET);
		(* nx.typ.form = Pointer & nx.typ.sysflag = interface *)
	BEGIN
		expr(ny, y, {}, wreg - {SI} + {mem, stk});
		IF (ny.class # Nconst) & ~CountedPtr(ny) THEN
			DevCPC486.IPAddRef(y, 0, TRUE)
		END;
		IF nx # NIL THEN
			DevCPC486.Assert(y, {}, wreg - {SI} + ux);
			expr(nx, x, wreg - {DI}, {loaded});
			IF (x.mode = Ind) & (x.reg IN wreg - {SI, DI}) OR (x.scale # 0) THEN
				DevCPC486.GetAdr(x, {}, wreg - {DI} + {con});
				x.mode := Ind; x.offset := 0; x.scale := 0
			END;
			DevCPC486.IPRelease(x, 0, TRUE, FALSE);
		END
	END IPAssign;
	
	PROCEDURE IPStructAssign (typ: DevCPT.Struct);
		VAR x, y: DevCPL486.Item;
	BEGIN
		IF typ.comp = DynArr THEN DevCPM.err(270) END;
		(* addresses in SI and DI *)
		x.mode := Ind; x.reg := DI; x.offset := 0; x.scale := 0;
		y.mode := Ind; y.reg := SI; y.offset := 0; y.scale := 0;
		HandleIPtrs(typ, x, y, TRUE, TRUE, FALSE)
	END IPStructAssign;

	PROCEDURE IPFree (nx: DevCPT.Node; VAR x: DevCPL486.Item);
	BEGIN
		expr(nx, x, wreg - {DI}, {loaded}); DevCPC486.GetAdr(x, {}, wreg - {DI} + {con});
		x.mode := Ind; x.offset := 0; x.scale := 0;
		IF nx.typ.form = Comp THEN
			HandleIPtrs(nx.typ, x, x, FALSE, TRUE, TRUE)
		ELSE	(* nx.typ.form = Pointer & nx.typ.sysflag = interface *)
			DevCPC486.IPRelease(x, 0, TRUE, TRUE);
		END
	END IPFree;
	
	(* unchanged val parameters allways counted because of aliasing problems REMOVED! *)
	
	PROCEDURE InitializeIPVars (proc: DevCPT.Object);
		VAR x: DevCPL486.Item; obj: DevCPT.Object;
	BEGIN
		x.mode := Ind; x.reg := BP; x.scale := 0; x.form := Pointer;
		obj := proc.link;
		WHILE obj # NIL DO
			IF (obj.mode = Var) & obj.used THEN	(* changed value parameters *)
				x.offset := obj.adr;
				HandleIPtrs(obj.typ, x, x, TRUE, FALSE, FALSE)
			END;
			obj := obj.link
		END
	END InitializeIPVars;
	
	PROCEDURE ReleaseIPVars (proc: DevCPT.Object);
		VAR x, ax, dx, si, di: DevCPL486.Item; obj: DevCPT.Object;
	BEGIN
		obj := proc.link;
		WHILE (obj # NIL) & ((obj.mode # Var) OR ~obj.used OR ~DevCPC486.ContainsIPtrs(obj.typ)) DO
			obj := obj.link
		END;
		IF obj = NIL THEN
			obj := proc.scope.scope;
			WHILE (obj # NIL) & ~DevCPC486.ContainsIPtrs(obj.typ) DO obj := obj.link END;
			IF obj = NIL THEN RETURN END
		END;
		DevCPL486.MakeReg(ax, AX, Int32); DevCPL486.MakeReg(si, SI, Int32);
		DevCPL486.MakeReg(dx, DX, Int32); DevCPL486.MakeReg(di, DI, Int32);
		IF ~(proc.typ.form IN {Real32, Real64, NoTyp}) THEN DevCPL486.GenMove(ax, si) END;
		IF proc.typ.form = Int64 THEN DevCPL486.GenMove(dx, di) END;
		x.mode := Ind; x.reg := BP; x.scale := 0; x.form := Pointer;
		obj := proc.link;
		WHILE obj # NIL DO
			IF (obj.mode = Var) & obj.used THEN	(* value parameters *)
				x.offset := obj.adr;
				HandleIPtrs(obj.typ, x, x, FALSE, TRUE, FALSE)
			END;
			obj := obj.link
		END;
		obj := proc.scope.scope;
		WHILE obj # NIL DO	(* local variables *)
			IF obj.used THEN
				x.offset := obj.adr;
				HandleIPtrs(obj.typ, x, x, FALSE, TRUE, FALSE);
			END;
			obj := obj.link
		END;
		IF ~(proc.typ.form IN {Real32, Real64, NoTyp}) THEN DevCPL486.GenMove(si, ax) END;
		IF proc.typ.form = Int64 THEN DevCPL486.GenMove(di, dx) END
	END ReleaseIPVars;
	
	PROCEDURE CompareIntTypes (
		typ: DevCPT.Struct; VAR id: DevCPL486.Item; VAR exit: DevCPL486.Label; VAR num: INTEGER
	);
		VAR x, y: DevCPL486.Item; local: DevCPL486.Label;
	BEGIN
		local := DevCPL486.NewLbl;
		typ := typ.BaseTyp; num := 0;
		WHILE (typ # NIL) & (typ # DevCPT.undftyp) DO
			IF (typ.sysflag = interface) & (typ.ext # NIL) THEN
				IF num > 0 THEN DevCPC486.JumpT(x, local) END;
				DevCPC486.GuidFromString(typ.ext, y);
				x := id; DevCPC486.GetAdr(x, wreg - {SI}, {mem});
				x := y; DevCPC486.GetAdr(x, wreg - {DI}, {});
				x := id; DevCPC486.CmpString(x, y, eql, FALSE);
				INC(num)
			END;
			typ := typ.BaseTyp
		END;
		IF num > 0 THEN DevCPC486.JumpF(x, exit) END;
		IF num > 1 THEN DevCPL486.SetLabel(local) END
	END CompareIntTypes;
	
	PROCEDURE InstallQueryInterface (typ: DevCPT.Struct; proc: DevCPT.Object);
		VAR this, id, int, unk, c: DevCPL486.Item; nil, end: DevCPL486.Label; num: INTEGER;
	BEGIN
		nil := DevCPL486.NewLbl; end := DevCPL486.NewLbl;
		this.mode := Ind; this.reg := BP; this.offset := 8; this.scale := 0; this.form := Pointer; this.typ := DevCPT.anyptrtyp;
		id.mode := DInd; id.reg := BP; id.offset := 12; id.scale := 0; id.form := Pointer;
		int.mode := DInd; int.reg := BP; int.offset := 16; int.scale := 0; int.form := Pointer;
		DevCPC486.GetAdr(int, {}, {AX, CX, SI, DI, mem}); int.mode := Ind; int.offset := 0;
		DevCPL486.MakeConst(c, 0, Pointer); DevCPC486.Assign(int, c);
		unk.mode := Ind; unk.reg := BP; unk.offset := 8; unk.scale := 0; unk.form := Pointer; unk.typ := DevCPT.punktyp;
		DevCPC486.Load(unk, {}, {});
		unk.mode := Ind; unk.offset := 8;
		DevCPC486.Load(unk, {}, {});
		DevCPL486.GenComp(c, unk);
		DevCPL486.GenJump(4, nil, TRUE);
		DevCPL486.MakeReg(c, int.reg, Pointer);
		DevCPL486.GenPush(c);
		c.mode := Ind; c.reg := BP; c.offset := 12; c.scale := 0; c.form := Pointer;
		DevCPL486.GenPush(c);
		DevCPL486.GenPush(unk);
		c.mode := Ind; c.reg := unk.reg; c.offset := 0; c.scale := 0; c.form := Pointer;
		DevCPL486.GenMove(c, unk);
		unk.mode := Ind; unk.offset := 0; unk.scale := 0; unk.form := Pointer;
		DevCPL486.GenCall(unk);
		DevCPC486.Free(unk);
		DevCPL486.GenJump(-1, end, FALSE);
		DevCPL486.SetLabel(nil);
		DevCPL486.MakeConst(c, 80004002H, Int32);	(* E_NOINTERFACE *)
		DevCPC486.Result(proc, c);
		CompareIntTypes(typ, id, end, num);
		IF num > 0 THEN
			DevCPC486.Load(this, {}, {});
			DevCPC486.Assign(int, this);
			DevCPC486.IPAddRef(this, 0, FALSE);
			DevCPL486.MakeConst(c, 0, Int32);	(* S_OK *)
			DevCPC486.Result(proc, c);
		END;
		DevCPL486.SetLabel(end)
	END InstallQueryInterface;

	(* -------------------- *)

	PROCEDURE ActualPar (n: DevCPT.Node; fp: DevCPT.Object; rec: BOOLEAN; VAR tag: DevCPL486.Item);
		VAR ap: DevCPL486.Item; x: DevCPT.Node; niltest: BOOLEAN;
	BEGIN
		IF n # NIL THEN
			ActualPar(n.link, fp.link, FALSE, ap);
			niltest := FALSE;
			IF fp.mode = VarPar THEN
				IF (n.class = Ndop) & ((n.subcl = thisarrfn) OR (n.subcl = thisrecfn)) THEN
					expr(n.right, ap, {}, {}); DevCPC486.Push(ap);	(* push type/length *)
					expr(n.left, ap, {}, {}); DevCPC486.Push(ap);	(* push adr *)
					RETURN
				ELSIF (fp.vis = outPar) & DevCPC486.ContainsIPtrs(fp.typ) & (ap.typ # DevCPT.niltyp) THEN
					IPFree(n, ap)
				ELSE
					x := n;
					WHILE (x.class = Nfield) OR (x.class = Nindex) DO x := x.left END;
					niltest := x.class = Nderef;	(* explicit nil test needed *)
					AdrExpr(n, ap, {}, {})
				END
			ELSIF (n.class = Nmop) & (n.subcl = conv) THEN
				IF n.typ.form IN {String8, String16} THEN expr(n, ap, {}, {}); DevCPM.err(265)
				ELSIF (DevCPT.Includes(n.typ.form, n.left.typ.form) OR DevCPT.Includes(n.typ.form, fp.typ.form))
					& (n.typ.form # Set) & (fp.typ # DevCPT.bytetyp) THEN expr(n.left, ap, {}, {high});
				ELSE expr(n, ap, {}, {high});
				END
			ELSE expr(n, ap, {}, {high});
				IF CountedPtr(n) THEN DevCPM.err(270) END
			END;
			DevCPC486.Param(fp, rec, niltest, ap, tag)
		END
	END ActualPar;
	
	PROCEDURE Call (n: DevCPT.Node; VAR x: DevCPL486.Item);
		VAR tag: DevCPL486.Item; proc: DevCPT.Object; m: BYTE;
	BEGIN
		IF n.left.class = Nproc THEN
			proc := n.left.obj; m := proc.mode;
		ELSE proc := NIL; m := 0
		END;
		IF (m = CProc) & (n.right # NIL) THEN
			ActualPar(n.right.link, n.obj.link, FALSE, tag);
			expr(n.right, tag, wreg - {AX}, {});	(* tag = first param *)
		ELSE
			IF proc # NIL THEN DevCPC486.PrepCall(proc) END;
			ActualPar(n.right, n.obj, (m = TProc) & (n.left.subcl = 0), tag);
		END;
		IF proc # NIL THEN design(n.left, x, {}, {}) ELSE expr(n.left, x, {}, {}) END;
		DevCPC486.Call(x, tag)
	END Call;

	PROCEDURE Mem (n: DevCPT.Node; VAR x: DevCPL486.Item; typ: DevCPT.Struct; hint, stop: SET);
		VAR offset: INTEGER;
	BEGIN
		IF (n.class = Ndop) & (n.subcl IN {plus, minus}) & (n.right.class = Nconst) THEN
			expr(n.left, x, hint, stop + {mem}); offset := n.right.conval.intval;
			IF n.subcl = minus THEN offset := -offset END
		ELSE
			expr(n, x, hint, stop + {mem}); offset := 0
		END;
		DevCPC486.Mem(x, offset, typ)
	END Mem;
	
	PROCEDURE^ CompStat (n: DevCPT.Node);
	PROCEDURE^ CompRelease (n: DevCPT.Node; VAR res: DevCPL486.Item);

	PROCEDURE condition (n: DevCPT.Node; VAR x: DevCPL486.Item; VAR false, true: DevCPL486.Label);
		VAR local: DevCPL486.Label; y, z: DevCPL486.Item; ux: SET; sx, num: INTEGER; f: BYTE; typ: DevCPT.Struct;
	BEGIN
		IF n.class = Nmop THEN
			CASE n.subcl OF
			   not: condition(n.left, x, true, false); DevCPC486.Not(x)
			| is: IF n.left.typ.form = Pointer THEN expr(n.left, x, {}, {mem})
					ELSE design(n.left, x, {}, {})
					END;
					DevCPC486.TypTest(x, n.obj.typ, FALSE, FALSE)
			| odd: expr(n.left, x, {}, {}); DevCPC486.Odd(x)
			| cc: expr(n.left, x, {}, {}); x.mode := Cond; x.form := Bool
			| val: DevCPM.err(220)
			END
		ELSIF n.class = Ndop THEN
			CASE n.subcl OF
			   and: local := DevCPL486.NewLbl; condition(n.left, y, false, local);
					DevCPC486.JumpF(y, false);
					IF local # DevCPL486.NewLbl THEN DevCPL486.SetLabel(local) END;
					condition(n.right, x, false, true)
			| or: local := DevCPL486.NewLbl; condition(n.left, y, local, true);
					DevCPC486.JumpT(y, true);
					IF local # DevCPL486.NewLbl THEN DevCPL486.SetLabel(local) END;
					condition(n.right, x, false, true)
			| eql..geq:
					f := n.left.typ.form;
					IF f = Int64 THEN DevCPM.err(260)
					ELSIF f IN {String8, String16, Comp} THEN
						IF (n.left.class = Nmop) & (n.left.subcl = conv) THEN	(* converted must be source *)
							StringOp(n.right, n.left, x, y, FALSE); DevCPC486.CmpString(x, y, n.subcl, TRUE)
						ELSE
							StringOp(n.left, n.right, x, y, FALSE); DevCPC486.CmpString(x, y, n.subcl, FALSE)
						END
					ELSIF f IN {Real32, Real64} THEN FloatDOp(n, x)
					ELSE
						IF CountedPtr(n.left) OR CountedPtr(n.right) THEN DevCPM.err(270) END;
						DualExp(n.left, n.right, x, y, {}, {}, {stk}, {stk});
						IF (x.mode = Reg) OR (y.mode = Con) THEN DevCPC486.IntDOp(x, y, n.subcl, FALSE)
						ELSIF (y.mode = Reg) OR (x.mode = Con) THEN DevCPC486.IntDOp(y, x, n.subcl, TRUE); x := y
						ELSE DevCPC486.Load(x, {}, {}); DevCPC486.IntDOp(x, y, n.subcl, FALSE)
						END 
					END
			| in: DualExp(n.left, n.right, x, y, {}, {}, {short, mem, stk}, {con, stk});
					DevCPC486.In(x, y)
			| bit: Check(n.left, ux, sx);
					expr(n.right, x, {}, ux + {short});
					Mem(n.left, y, DevCPT.notyp, {}, {});
					DevCPC486.Load(x, {}, {short});
					DevCPC486.In(x, y)
			| queryfn:
					AdrExpr(n.right, x, {}, {CX, SI, DI});
					CompareIntTypes(n.left.typ, x, false, num);
					IF num > 0 THEN 
						Check(n.right.link, ux, sx); IPAssign(n.right.link, n.left, x, y, ux); DevCPC486.Assign(x, y);
						x.offset := 1	(* true *)
					ELSE x.offset := 0	(* false *)
					END;
					x.mode := Con; DevCPC486.MakeCond(x)
			END
		ELSIF n.class = Ncomp THEN
			CompStat(n.left); condition(n.right, x, false, true); CompRelease(n.left, x);
			IF x.mode = Stk THEN DevCPL486.GenCode(9DH); (* pop flags *) x.mode := Cond END
		ELSE expr(n, x, {}, {}); DevCPC486.MakeCond(x)	(* const, var, or call *)
		END
	END condition;
	
	PROCEDURE expr(n: DevCPT.Node; VAR x: DevCPL486.Item; hint, stop: SET);
		VAR y, z: DevCPL486.Item; f, g: BYTE; cval: DevCPT.Const; false, true: DevCPL486.Label;
			uy: SET; sy: INTEGER; r: REAL;
	BEGIN
		f := n.typ.form;
		IF (f = Bool) & (n.class IN {Ndop, Nmop}) THEN
			false := DevCPL486.NewLbl; true := DevCPL486.NewLbl;
			condition(n, y, false, true);
			DevCPC486.LoadCond(x, y, false, true, hint, stop + {mem})
		ELSE
			CASE n.class OF
			   Nconst:
					IF n.obj = NIL THEN cval := n.conval ELSE cval := n.obj.conval END;
					CASE f OF
					   Byte..Int32, NilTyp, Pointer, Char16: DevCPL486.MakeConst(x, cval.intval, f)
					| Int64:
						DevCPL486.MakeConst(x, cval.intval, f);
						DevCPE.GetLongWords(cval, x.scale, x.offset)
					| Set: DevCPL486.MakeConst(x, SYSTEM.VAL(INTEGER, cval.setval), Set)
					| String8, String16, Real32, Real64: DevCPL486.AllocConst(x, cval, f)
					| Comp: 
						ASSERT(n.typ = DevCPT.guidtyp);
						IF n.conval # NIL THEN DevCPC486.GuidFromString(n.conval.ext, x)
						ELSE DevCPC486.GuidFromString(n.obj.typ.ext, x)
						END
					END
			| Nupto:	(* n.typ = DevCPT.settyp *)
					Check(n.right, uy, sy);
					expr(n.left, x, {}, wreg - {CX} + {high, mem, stk});
					DevCPC486.MakeSet(x, TRUE, FALSE, hint + stop + uy, {});
					DevCPC486.Assert(x, {}, uy);
					expr(n.right, y, {}, wreg - {CX} + {high, mem, stk});
					DevCPC486.MakeSet(y, TRUE, TRUE, hint + stop, {});
					DevCPC486.Load(x, hint + stop, {});
					IF x.mode = Con THEN DevCPC486.IntDOp(y, x, msk, TRUE); x := y
					ELSE DevCPC486.IntDOp(x, y, msk, FALSE)
					END
			| Nmop:
					CASE n.subcl OF
					| bit:
							expr(n.left, x, {}, wreg - {CX} + {high, mem, stk});
							DevCPC486.MakeSet(x, FALSE, FALSE, hint + stop, {})
					| conv:
							IF f IN {String8, String16} THEN
								expr(n.left, x, hint, stop);
								IF f = String8 THEN x.form := VString16to8 END	(* SHORT *)
							ELSE
								IF n.left.class = Nconst THEN	(* largeint -> longreal *)
									ASSERT((n.left.typ.form = Int64) & (f = Real64));
									DevCPL486.AllocConst(x, n.left.conval, n.left.typ.form);
								ELSE
									expr(n.left, x, hint + stop, {high});
								END;
								DevCPC486.Convert(x, f, -1, hint + stop, {})	(* ??? *)
							END
					| val:
							expr(n.left, x, hint + stop, {high, con}); DevCPC486.Convert(x, f, n.typ.size, hint, stop)	(* ??? *)
					| adr:
							IF n.left.class = Ntype THEN
								x.mode := Con; x.offset := 0; x.obj := n.left.obj; x.form := Int32; x.typ := n.left.typ;
							ELSE
								AdrExpr(n.left, x, hint + stop, {});
							END;
							DevCPC486.GetAdr(x, hint + stop, {})
					| typfn:
							IF n.left.class = Ntype THEN
								x.mode := Con; x.offset := 0; x.obj := n.left.obj; x.form := Int32; x.typ := n.left.typ;
								IF x.obj.typ.untagged THEN DevCPM.err(111) END
							ELSE
								expr(n.left, x, hint + stop, {});
								DevCPC486.Tag(x, y); DevCPC486.Free(x); x := y
							END;
							DevCPC486.Load(x, hint + stop, {})
					| minus, abs, cap:
							expr(n.left, x, hint + stop, {mem, stk});
							IF f = Int64 THEN DevCPM.err(260)
							ELSIF f IN realSet THEN DevCPC486.FloatMOp(x, n.subcl)
							ELSE DevCPC486.IntMOp(x, n.subcl)
							END
					END
			| Ndop:
					IF (f IN realSet) & (n.subcl # lsh) & (n.subcl # rot) THEN
						IF (n.subcl = ash) & (n.right.class = Nconst) & (n.right.conval.realval >= 0) THEN
							expr(n.left, x, {}, {mem, stk});
							cval := n.right.conval; sy := SHORT(ENTIER(cval.realval)); cval.realval := 1;
							WHILE sy > 0 DO cval.realval := cval.realval * 2; DEC(sy) END;
							DevCPL486.AllocConst(y, cval, Real32);
							DevCPC486.FloatDOp(x, y, times, FALSE)
						ELSE FloatDOp(n, x)
						END
					ELSIF (f = Int64) OR (n.typ = DevCPT.intrealtyp) THEN DevCPM.err(260); expr(n.left, x, {}, {})
					ELSE
						CASE n.subcl OF
						   times:
								IF f = Int8 THEN
									DualExp(n.left, n.right, x, y, {}, {}, wreg - {AX} + {high, mem, stk, con}, {AX, con, stk});
									DevCPC486.IntDOp(x, y, times, FALSE)
								ELSE IntDOp(n, x, hint + stop)
								END
						| div, mod:
								DualExp(n.left, n.right, x, y, {}, {}, wreg - {AX} + {high, mem, stk, con}, {AX, DX, mem, stk});
								DevCPC486.DivMod(x, y, n.subcl = mod)
						| plus:
								IF n.typ.form IN {String8, String16} THEN DevCPM.err(265); expr(n.left, x, {}, {})
								ELSE IntDOp(n, x, hint + stop)
								END
						| slash, minus, msk, min, max:
								IntDOp(n, x, hint + stop)
						| ash, lsh, rot:
								uy := {}; IF n.right.class # Nconst THEN uy := {CX} END;
								DualExp(n^.right, n^.left, y, x, {}, hint + stop, wreg - {CX} + {high, mem, stk}, uy + {con, mem, stk});
								DevCPC486.Shift(x, y, n^.subcl)
						| len:
								IF n.left.typ.form IN {String8, String16} THEN
									expr(n.left, x, wreg - {DI} , {}); DevCPC486.GetAdr(x, {}, wreg - {DI} + {con});
									DevCPC486.StrLen(x, n.left.typ, FALSE)
								ELSE
									design(n.left, x, hint + stop, {}); expr(n.right, y, {}, {}); DevCPC486.Len(x, y)
								END
						END
					END
			| Ncall:
					Call(n, x)
			| Ncomp:
					CompStat(n.left); expr(n.right, x, hint, stop); CompRelease(n.left, x);
					IF x.mode = Stk THEN DevCPC486.Pop(x, x.form, hint, stop) END
			ELSE
				design(n, x, hint + stop, stop * {loaded}); DevCPC486.Prepare(x, hint + stop, {})	(* ??? *)
			END
		END;
		x.typ := n.typ;
		DevCPC486.Assert(x, hint, stop)
	END expr;
	
	PROCEDURE AddCopy (n: DevCPT.Node; VAR dest, dadr, len: DevCPL486.Item; last: BOOLEAN);
		VAR adr, src: DevCPL486.Item; u: SET; s: INTEGER;
	BEGIN
		Check(n, u, s);
		DevCPC486.Assert(dadr, wreg - {DI}, u + {SI, CX});
		IF len.mode # Con THEN DevCPC486.Assert(len, wreg - {CX}, u + {SI, DI}) END;
		expr(n, src, wreg - {SI}, {});
		adr := src; DevCPC486.GetAdr(adr, {}, wreg - {SI} + {con});
		IF len.mode # Con THEN DevCPC486.Load(len, {}, wreg - {CX} + {con}) END;
		DevCPC486.Load(dadr, {}, wreg - {DI} + {con});
		DevCPC486.AddCopy(dest, src, last)
	END AddCopy;
	
	PROCEDURE StringCopy (left, right: DevCPT.Node);
		VAR x, y, ax, ay, len: DevCPL486.Item;
	BEGIN
		IF IsAllocDynArr(left) THEN expr(left, x, wreg - {CX}, {DI})	(* keep len descriptor *)
		ELSE expr(left, x, wreg - {DI}, {})
		END;
		ax := x; DevCPC486.GetAdr(ax, {}, wreg - {DI});
		DevCPC486.Free(x); DevCPC486.ArrayLen(x, len, wreg - {CX}, {});
		WHILE right.class = Ndop DO
			ASSERT(right.subcl = plus);
			AddCopy(right.left, x, ax, len, FALSE);
			right := right.right
		END;
		AddCopy(right, x, ax, len, TRUE);
		DevCPC486.Free(len)
	END StringCopy;
	
	PROCEDURE Checkpc;
	BEGIN
		DevCPE.OutSourceRef(DevCPM.errpos)
	END Checkpc;

	PROCEDURE^ stat (n: DevCPT.Node; VAR end: DevCPL486.Label);
	
	PROCEDURE CondStat (if, last: DevCPT.Node; VAR hint: INTEGER; VAR else, end: DevCPL486.Label);
		VAR local: DevCPL486.Label; x: DevCPL486.Item; cond, lcond: DevCPT.Node;
	BEGIN
		local := DevCPL486.NewLbl;
		DevCPM.errpos := if.conval.intval; Checkpc; cond := if.left;
		IF (last # NIL) & (cond.class = Ndop) & (cond.subcl >= eql) & (cond.subcl <= geq)
				& (last.class = Ndop) & (last.subcl >= eql) & (last.subcl <= geq)
				& SameExp(cond.left, last.left) & SameExp(cond.right, last.right) THEN	(*  reuse comparison *)
			DevCPC486.setCC(x, cond.subcl, ODD(hint), hint >= 2)
		ELSIF (last # NIL) & (cond.class = Nmop) & (cond.subcl = is) & (last.class = Nmop) & (last.subcl = is)
				& SameExp(cond.left, last.left) THEN
			DevCPC486.ShortTypTest(x, cond.obj.typ)	(* !!! *)
		ELSE condition(cond, x, else, local)
		END;
		hint := x.reg;
		DevCPC486.JumpF(x, else);
		IF local # DevCPL486.NewLbl THEN DevCPL486.SetLabel(local) END;
		stat(if.right, end);
	END CondStat;

	PROCEDURE IfStat (n: DevCPT.Node; withtrap: BOOLEAN; VAR end: DevCPL486.Label);
		VAR else, local: DevCPL486.Label; if, last: DevCPT.Node; hint: INTEGER;
	BEGIN	(* n.class = Nifelse *)
		if := n.left; last := NIL;
		WHILE (if # NIL) & ((if.link # NIL) OR (n.right # NIL) OR withtrap) DO
			else := DevCPL486.NewLbl; 
			CondStat(if, last, hint, else, end);
			IF sequential THEN DevCPC486.Jump(end) END;
			DevCPL486.SetLabel(else); last := if.left; if := if.link
		END;
		IF n.right # NIL THEN stat(n.right, end)
		ELSIF withtrap THEN DevCPM.errpos := n.conval.intval; Checkpc; DevCPC486.Trap(withTrap); sequential := FALSE
		ELSE CondStat(if, last, hint, end, end)
		END
	END IfStat;
	
	PROCEDURE CasePart (n: DevCPT.Node; VAR x: DevCPL486.Item; VAR else: DevCPL486.Label; last: BOOLEAN);
		VAR this, higher: DevCPL486.Label; m: DevCPT.Node; low, high: INTEGER;
	BEGIN
		IF n # NIL THEN
			this := SHORT(ENTIER(n.conval.realval));
			IF useTree IN n.conval.setval THEN
				IF n.left # NIL THEN
					IF n.right # NIL THEN
						higher := DevCPL486.NewLbl;
						DevCPC486.CaseJump(x, n.conval.intval, n.conval.intval2, this, higher, TRUE, FALSE);
						CasePart(n.left, x, else, FALSE);
						DevCPL486.SetLabel(higher);
						CasePart(n.right, x, else, last)
					ELSE
						DevCPC486.CaseJump(x, n.conval.intval, n.conval.intval2, this, else, FALSE, FALSE);
						CasePart(n.left, x, else, last);
					END
				ELSE
					DevCPC486.CaseJump(x, n.conval.intval, n.conval.intval2, this, else, FALSE, TRUE);
					IF n.right # NIL THEN CasePart(n.right, x, else, last)
					ELSIF ~last THEN DevCPC486.Jump(else)
					END
				END
			ELSE
				IF useTable IN n.conval.setval THEN
					m := n; WHILE m.left # NIL DO m := m.left END; low := m.conval.intval;
					m := n; WHILE m.right # NIL DO m := m.right END; high := m.conval.intval2;
					DevCPC486.CaseTableJump(x, low, high, else);
					actual := low; last := TRUE
				END;
				CasePart(n.left, x, else, FALSE);
				WHILE actual < n.conval.intval DO
					DevCPL486.GenCaseEntry(else, FALSE); INC(actual)
				END;
				WHILE actual < n.conval.intval2 DO
					DevCPL486.GenCaseEntry(this, FALSE); INC(actual)
				END;
				DevCPL486.GenCaseEntry(this, last & (n.right = NIL)); INC(actual);
				CasePart(n.right, x, else, last)
			END;
			n.conval.realval := this
		END
	END CasePart;
	
	PROCEDURE CaseStat (n: DevCPT.Node; VAR end: DevCPL486.Label);
		VAR x: DevCPL486.Item; case, lab: DevCPT.Node; low, high, tab: INTEGER; else, this: DevCPL486.Label;
	BEGIN
		expr(n.left, x, {}, {mem, con, short, float, stk}); else := DevCPL486.NewLbl;
		IF (n.right.right # NIL) & (n.right.right.class = Ngoto) THEN	(* jump to goto optimization *)
			CasePart(n.right.link, x, else, FALSE); DevCPC486.Free(x);
			n.right.right.right.conval.intval2 := else; sequential := FALSE
		ELSE
			CasePart(n.right.link, x, else, TRUE); DevCPC486.Free(x);
			DevCPL486.SetLabel(else);
			IF n.right.conval.setval # {} THEN stat(n.right.right, end)
			ELSE DevCPC486.Trap(caseTrap); sequential := FALSE
			END
		END;
		case := n.right.left;
		WHILE case # NIL DO	(* case.class = Ncasedo *)
			IF sequential THEN DevCPC486.Jump(end) END;
			lab := case.left;
			IF (case.right # NIL) & (case.right.class = Ngoto) THEN	(* jump to goto optimization *)
				case.right.right.conval.intval2 := SHORT(ENTIER(lab.conval.realval));
				ASSERT(lab.link = NIL); sequential := FALSE
			ELSE
				WHILE lab # NIL DO
					this := SHORT(ENTIER(lab.conval.realval)); DevCPL486.SetLabel(this); lab := lab.link
				END;
				stat(case.right, end)
			END;
			case := case.link
		END
	END CaseStat;

	PROCEDURE Dim(n: DevCPT.Node; VAR x, nofel: DevCPL486.Item; VAR fact: INTEGER; dimtyp: DevCPT.Struct);
		VAR len: DevCPL486.Item; u: SET; s: INTEGER;
	BEGIN
		Check(n, u, s);
		IF (nofel.mode = Reg) & (nofel.reg IN u) THEN DevCPC486.Push(nofel) END;
		expr(n, len, {}, {mem, short});
		IF nofel.mode = Stk THEN DevCPC486.Pop(nofel, Int32, {}, {}) END;
		IF len.mode = Stk THEN DevCPC486.Pop(len, Int32, {}, {}) END;
		DevCPC486.MulDim(len, nofel, fact, dimtyp);
		IF n.link # NIL THEN
			Dim(n.link, x, nofel, fact, dimtyp.BaseTyp);
		ELSE
			DevCPC486.New(x, nofel, fact)
		END;
		DevCPC486.SetDim(x, len, dimtyp)
	END Dim;

	PROCEDURE CompStat (n: DevCPT.Node);
		VAR x, y, sp, old, len, nofel: DevCPL486.Item; fact: INTEGER; typ: DevCPT.Struct;
	BEGIN
		Checkpc;
		WHILE (n # NIL) & DevCPM.noerr DO
			ASSERT(n.class = Nassign);
			IF n.subcl = assign THEN
				IF n.right.typ.form IN {String8, String16} THEN
					StringCopy(n.left, n.right)
				ELSE
					IF (n.left.typ.sysflag = interface) & ~CountedPtr(n.right) THEN
						IPAssign(NIL, n.right, x, y, {});	(* no Release *)
					ELSE expr(n.right, y, {}, {})
					END;
					expr(n.left, x, {}, {});
					DevCPC486.Assign(x, y)
				END
			ELSE ASSERT(n.subcl = newfn);
				typ := n.left.typ.BaseTyp;
				ASSERT(typ.comp = DynArr);
				ASSERT(n.right.link = NIL);
				expr(n.right, y, {}, wreg - {CX} + {mem, stk});
				DevCPL486.MakeReg(sp, SP, Int32);
				DevCPC486.CopyReg(sp, old, {}, {CX});
				DevCPC486.CopyReg(y, len, {}, {CX});
				IF typ.BaseTyp.form = Char16 THEN
					DevCPL486.MakeConst(x, 2, Int32); DevCPL486.GenMul(x, y, FALSE)
				END;
				DevCPC486.StackAlloc;
				DevCPC486.Free(y);
				expr(n.left, x, {}, {}); DevCPC486.Assign(x, sp);
				DevCPC486.Push(len);
				DevCPC486.Push(old);
				typ.sysflag := stackArray
			END;
			n := n.link
		END
	END CompStat;
	
	PROCEDURE CompRelease (n: DevCPT.Node; VAR res: DevCPL486.Item);
		VAR x, y, sp: DevCPL486.Item;
	BEGIN
		IF n.link # NIL THEN CompRelease(n.link, res) END;
		ASSERT(n.class = Nassign);
		IF n.subcl = assign THEN
			IF (n.left.typ.form = Pointer) & (n.left.typ.sysflag = interface) THEN
				IF res.mode = Cond THEN
					DevCPL486.GenCode(9CH); (* push flags *)
					res.mode := Stk
				ELSIF res.mode = Reg THEN
					IF res.form < Int16 THEN DevCPC486.Push(res)
					ELSE DevCPC486.Assert(res, {}, {AX, CX, DX})
					END
				END;
				expr(n.left, x, wreg - {DI}, {loaded});
				DevCPC486.IPRelease(x, 0, TRUE, TRUE);
				n.left.obj.used := FALSE
			END
		ELSE ASSERT(n.subcl = newfn);
			DevCPL486.MakeReg(sp, SP, Int32); DevCPL486.GenPop(sp);
			DevCPL486.MakeConst(y, 0, Pointer);
			expr(n.left, x, {}, {}); DevCPC486.Assign(x, y)
		END
	END CompRelease;
	
	PROCEDURE Assign(n: DevCPT.Node; ux: SET);
		VAR r: DevCPT.Node; f: BYTE; false, true: DevCPL486.Label; x, y, z: DevCPL486.Item; uf, uy: SET; s: INTEGER;
	BEGIN
		r := n.right; f := r.typ.form; uf := {};
		IF (r.class IN {Nmop, Ndop}) THEN
			IF (r.subcl = conv) & (f # Set) &
(*
				(DevCPT.Includes(f, r.left.typ.form) OR DevCPT.Includes(f, n.left.typ.form)) THEN r := r.left;
				IF ~(f IN realSet) & (r.typ.form IN realSet) & (r.typ # DevCPT.intrealtyp) THEN uf := {AX} END (* entier *)
*)
				(DevCPT.Includes(f, r.left.typ.form) OR DevCPT.Includes(f, n.left.typ.form)) &
				((f IN realSet) OR ~(r.left.typ.form IN realSet)) THEN r := r.left
			ELSIF (f IN {Char8..Int32, Set, Char16, String8, String16}) & SameExp(n.left, r.left) THEN
				IF r.class = Ndop THEN
					IF (r.subcl IN {slash, plus, minus, msk}) OR (r.subcl = times) & (f = Set) THEN
						expr(r.right, y, {}, ux); expr(n.left, x, {}, {});
						DevCPC486.Load(y, {}, {}); DevCPC486.IntDOp(x, y, r.subcl, FALSE);
						RETURN
					ELSIF r.subcl IN {ash, lsh, rot} THEN
						expr(r.right, y, wreg - {CX} + {high, mem}, ux); expr(n.left, x, {}, {});
						DevCPC486.Load(y, {}, wreg - {CX} + {high}); DevCPC486.Shift(x, y, r.subcl);
						RETURN
					END
				ELSE
					IF r.subcl IN {minus, abs, cap} THEN
						expr(n.left, x, {}, {}); DevCPC486.IntMOp(x, r.subcl); RETURN
					END
				END
			ELSIF f = Bool THEN
				IF (r.subcl = not) & SameExp(n.left, r.left) THEN
					expr(n.left, x, {}, {}); DevCPC486.IntMOp(x, not); RETURN
				END
			END
		END;
		IF (n.left.typ.sysflag = interface) & (n.left.typ.form = Pointer) THEN IPAssign(n.left, r, x, y, ux)
		ELSE expr(r, y, {high}, ux); expr(n.left, x, {}, uf + {loaded});	(* high ??? *)
		END;
		DevCPC486.Assign(x, y)
	END Assign;
	
	PROCEDURE stat (n: DevCPT.Node; VAR end: DevCPL486.Label);
		VAR x, y, nofel: DevCPL486.Item; local, next, loop, prevExit: DevCPL486.Label; fact, sx, sz: INTEGER; ux, uz: SET;
	BEGIN
		sequential := TRUE; INC(nesting);
		WHILE (n # NIL) & DevCPM.noerr DO
			IF n.link = NIL THEN next := end ELSE next := DevCPL486.NewLbl END;
			DevCPM.errpos := n.conval.intval; DevCPL486.BegStat;
			CASE n.class OF
			| Ninittd:
					(* done at load-time *)
			| Nassign:
					Checkpc;
					Check(n.left, ux, sx);
					CASE n.subcl OF
					   assign:
							IF n.left.typ.form = Comp THEN
								IF (n.right.class = Ndop) & (n.right.typ.form IN {String8, String16}) THEN
									StringCopy(n.left, n.right)
								ELSE
									StringOp(n.left, n.right, x, y, TRUE);
									IF DevCPC486.ContainsIPtrs(n.left.typ) THEN IPStructAssign(n.left.typ) END;
									DevCPC486.Copy(x, y, FALSE)
								END
							ELSE Assign(n, ux)
							END
					| getfn:
							Mem(n.right, y, n.left.typ, {}, ux);
							expr(n.left, x, {}, {loaded});
							DevCPC486.Assign(x, y)
					| putfn:
							expr(n.right, y, {}, ux);
							Mem(n.left, x, n.right.typ, {}, {});
							DevCPC486.Assign(x, y)
					| incfn, decfn:
							expr(n.right, y, {}, ux); expr(n.left, x, {}, {});
							IF n.left.typ.form = Int64 THEN 
								DevCPC486.LargeInc(x, y, n.subcl = decfn)
							ELSE
								DevCPC486.Load(y, {}, {}); DevCPC486.IntDOp(x, y, SHORT(SHORT(plus - incfn + n.subcl)), FALSE)
							END
					| inclfn:
							expr(n.right, y, {}, wreg - {CX} + {high, mem, stk}); DevCPC486.MakeSet(y, FALSE, FALSE, ux, {});
							DevCPC486.Assert(y, {}, ux); expr(n.left, x, {}, {});
							DevCPC486.Load(y, {}, {}); DevCPC486.IntDOp(x, y, plus, FALSE)
					| exclfn:
							expr(n.right, y, {}, wreg - {CX} + {high, mem, stk}); DevCPC486.MakeSet(y, FALSE, TRUE, ux, {});
							DevCPC486.Assert(y, {}, ux); expr(n.left, x, {}, {});
							DevCPC486.Load(y, {}, {}); DevCPC486.IntDOp(x, y, times, FALSE)
					| getrfn:
							expr(n.right, y, {}, {});
							IF y.offset < 8 THEN	
								DevCPL486.MakeReg(y, y.offset, n.left.typ.form);	(* ??? *)
								expr(n.left, x, {}, {loaded}); DevCPC486.Assign(x, y)
							ELSE DevCPM.err(220)
							END
					| putrfn:
							expr(n.left, x, {}, {});
							IF x.offset < 8 THEN
								DevCPL486.MakeReg(x, x.offset, n.right.typ.form);	(* ??? *)
								expr(n.right, y, wreg - {x.reg}, {}); DevCPC486.Assign(x, y)
							ELSE DevCPM.err(220)
							END
					| newfn:
							y.typ := n.left.typ;
							IF n.right # NIL THEN
								IF y.typ.BaseTyp.comp = Record THEN
									expr(n.right, nofel, {}, {AX, CX, DX, mem, stk});
									DevCPC486.New(y, nofel, 1);
								ELSE (*open array*)
									nofel.mode := Con; nofel.form := Int32; fact := 1;
									Dim(n.right, y, nofel, fact, y.typ.BaseTyp)
								END
							ELSE
								DevCPL486.MakeConst(nofel, 0, Int32);
								DevCPC486.New(y, nofel, 1);
							END;
							DevCPC486.Assert(y, {}, ux); expr(n.left, x, {}, {loaded}); DevCPC486.Assign(x, y)
					| sysnewfn:
							expr(n.right, y, {}, {mem, short}); DevCPC486.SysNew(y);
							DevCPC486.Assert(y, {}, ux); expr(n.left, x, {}, {}); DevCPC486.Assign(x, y)
					| copyfn:
							StringOp(n.left, n.right, x, y, TRUE);
							DevCPC486.Copy(x, y, TRUE)
					| movefn:
							Check(n.right.link, uz, sz);
							expr(n.right, y, {}, wreg - {SI} + {short} + ux + uz);
							expr(n.left, x, {}, wreg - {DI} + {short} + uz);
							expr(n.right.link, nofel, {}, wreg - {CX} + {mem, stk, short});
							DevCPC486.Load(x, {}, wreg - {DI} + {con});
							DevCPC486.Load(y, {}, wreg - {SI} + {con}); 
							DevCPC486.SysMove(nofel)
					END;
					sequential := TRUE
			| Ncall:
					Checkpc;
					Call(n, x); sequential := TRUE
			| Nifelse:
					IF (n.subcl # assertfn) OR assert THEN IfStat(n, FALSE, next) END
			| Ncase:
					Checkpc;
					CaseStat(n, next)
			| Nwhile:
					local := DevCPL486.NewLbl;
					IF n.right # NIL THEN DevCPC486.Jump(local) END;
					loop := DevCPL486.NewLbl; DevCPL486.SetLabel(loop);
					stat(n.right, local); DevCPL486.SetLabel(local);
					DevCPM.errpos := n.conval.intval; Checkpc;
					condition(n.left, x, next, loop); DevCPC486.JumpT(x, loop); sequential := TRUE
			| Nrepeat:
					loop := DevCPL486.NewLbl; DevCPL486.SetLabel(loop);
					local := DevCPL486.NewLbl; stat(n.left, local); DevCPL486.SetLabel(local);
					DevCPM.errpos := n.conval.intval; Checkpc;
					condition(n.right, x, loop, next); DevCPC486.JumpF(x, loop); sequential := TRUE
			| Nloop:
					prevExit := Exit; Exit := next;
					loop := DevCPL486.NewLbl; DevCPL486.SetLabel(loop); stat(n.left, loop);
					IF sequential THEN DevCPC486.Jump(loop) END;
					next := Exit; Exit := prevExit; sequential := FALSE
			| Nexit:
					Checkpc;
					DevCPC486.Jump(Exit); sequential := FALSE
			| Nreturn:
					IF n.left # NIL THEN
						Checkpc;
						IF (n.obj.typ.sysflag = interface) & (n.obj.typ.form = Pointer)
							& (n.left.class # Nconst) & ~CountedPtr(n.left) THEN IPAssign(NIL, n.left, y, x, {})
						ELSE expr(n.left, x, wreg - {AX}, {})
						END;
						DevCPC486.Result(n.obj, x)
					END;
					IF (nesting > 1) OR (n.link # NIL) THEN DevCPC486.Jump(Return) END;
					sequential := FALSE
			| Nwith:
					IfStat(n, n.subcl = 0, next)
			| Ntrap:
					Checkpc;
					DevCPC486.Trap(n.right.conval.intval); sequential := TRUE
			| Ncomp:
					CompStat(n.left); stat(n.right, next); x.mode := 0; CompRelease(n.left, x)
			| Ndrop:
					Checkpc;
					expr(n.left, x, {}, {}); DevCPC486.Free(x)
			| Ngoto:
					IF n.left # NIL THEN
						Checkpc;
						condition(n.left, x, next, n.right.conval.intval2);
						DevCPC486.JumpT(x, n.right.conval.intval2)
					ELSE
						DevCPC486.Jump(n.right.conval.intval2);
						sequential := FALSE
					END
			| Njsr:
					DevCPL486.GenJump(-3, n.right.conval.intval2, FALSE)	(* call n.right *)
			| Nret:
					DevCPL486.GenReturn(0); sequential := FALSE	(* ret 0 *)
			| Nlabel:
					DevCPL486.SetLabel(n.conval.intval2)
			END;
			DevCPC486.CheckReg; DevCPL486.EndStat; n := n.link;
			IF n = NIL THEN end := next
			ELSIF next # DevCPL486.NewLbl THEN DevCPL486.SetLabel(next)
			END
		END;
		DEC(nesting)
	END stat;
	
	PROCEDURE CheckFpu (n: DevCPT.Node; VAR useFpu: BOOLEAN);
	BEGIN
		WHILE n # NIL DO
			IF n.typ.form IN {Real32, Real64} THEN useFpu := TRUE END;
			CASE n.class OF
			| Ncase:
				CheckFpu(n.left, useFpu); CheckFpu(n.right.left, useFpu); CheckFpu(n.right.right, useFpu)
			| Ncasedo:
				CheckFpu(n.right, useFpu)
			| Ngoto, Ndrop, Nloop, Nreturn, Nmop, Nfield, Nderef, Nguard:
				CheckFpu(n.left, useFpu)
			| Nassign, Ncall, Nifelse, Nif, Nwhile, Nrepeat, Nwith, Ncomp, Ndop, Nupto, Nindex:
				CheckFpu(n.left, useFpu); CheckFpu(n.right, useFpu)
			| Njsr, Nret, Nlabel, Ntrap, Nexit, Ninittd, Ntype, Nproc, Nconst, Nvar, Nvarpar:
			END;
			n := n.link
		END
	END CheckFpu;
	
	PROCEDURE procs(n: DevCPT.Node);
		VAR proc, obj: DevCPT.Object; i, j: INTEGER; end: DevCPL486.Label;
			ch: SHORTCHAR; name: DevCPT.Name; useFpu: BOOLEAN;
	BEGIN
		INC(DevCPL486.level); nesting := 0;
		WHILE (n # NIL) & DevCPM.noerr DO
			DevCPC486.imLevel[DevCPL486.level] := DevCPC486.imLevel[DevCPL486.level - 1]; proc := n.obj; 
			IF imVar IN proc.conval.setval THEN INC(DevCPC486.imLevel[DevCPL486.level]) END;
			procs(n.left);
			DevCPM.errpos := n.conval.intval;
			useFpu := FALSE; CheckFpu(n.right, useFpu);
			DevCPC486.Enter(proc, n.right = NIL, useFpu);
			InitializeIPVars(proc);
			end := DevCPL486.NewLbl; Return := DevCPL486.NewLbl; stat(n.right, end);
			DevCPM.errpos := n.conval.intval2; Checkpc;
			IF sequential OR (end # DevCPL486.NewLbl) THEN
				DevCPL486.SetLabel(end);
				IF (proc.typ # DevCPT.notyp) & (proc.sysflag # noframe) THEN DevCPC486.Trap(funcTrap) END
			END;
			DevCPL486.SetLabel(Return);
			ReleaseIPVars(proc);
			DevCPC486.Exit(proc, n.right = NIL);
			IF proc.mode = TProc THEN
				name := proc.link.typ.strobj.name^$; i := 0;
				WHILE name[i] # 0X DO INC(i) END;
				name[i] := "."; INC(i); j := 0; ch := proc.name[0];
				WHILE (ch # 0X) & (i < LEN(name)-1) DO name[i] := ch; INC(i); INC(j); ch := proc.name[j] END ;
				name[i] := 0X;
			ELSE name := proc.name^$
			END;
			DevCPE.OutRefName(name); DevCPE.OutRefs(proc.scope.right);
			n := n.link
		END;
		DEC(DevCPL486.level)
	END procs;
	
	PROCEDURE Module*(prog: DevCPT.Node);
		VAR end: DevCPL486.Label; name: DevCPT.Name; obj, p: DevCPT.Object; n: DevCPT.Node;
			aAd, rAd: INTEGER; typ: DevCPT.Struct; useFpu: BOOLEAN;
	BEGIN
		DevCPH.UseReals(prog, {DevCPH.longDop, DevCPH.longMop});
		DevCPM.NewObj(DevCPT.SelfName);
		IF DevCPM.noerr THEN
			DevCPE.OutHeader; n := prog.right;
			WHILE (n # NIL) & (n.class = Ninittd) DO n := n.link END;
			useFpu := FALSE; CheckFpu(n, useFpu);
			DevCPC486.Enter(NIL, n = NIL, useFpu);
			end := DevCPL486.NewLbl; stat(n, end); DevCPL486.SetLabel(end);
			DevCPM.errpos := prog.conval.intval2; Checkpc;
			DevCPC486.Exit(NIL, n = NIL);
			IF prog.link # NIL THEN	(* close section *)
				DevCPL486.SetLabel(DevCPE.closeLbl);
				useFpu := FALSE; CheckFpu(prog.link, useFpu);
				DevCPC486.Enter(NIL, FALSE, useFpu);
				end := DevCPL486.NewLbl; stat(prog.link, end); DevCPL486.SetLabel(end);
				DevCPM.errpos := SHORT(ENTIER(prog.conval.realval)); Checkpc;
				DevCPC486.Exit(NIL, FALSE)
			END;
			name := "$$"; DevCPE.OutRefName(name); DevCPE.OutRefs(DevCPT.topScope.right);
			DevCPM.errpos := prog.conval.intval;
			WHILE query # NIL DO
				typ := query.typ; query.typ := DevCPT.int32typ;
				query.conval.intval := 20;	(* parameters *)
				query.conval.intval2 := -8;	(* saved registers *)
				DevCPC486.Enter(query, FALSE, FALSE);
				InstallQueryInterface(typ, query);
				DevCPC486.Exit(query, FALSE);
				name := "QueryInterface"; DevCPE.OutRefName(name);
				query := query.nlink
			END;
			procs(prog.left);
			DevCPC486.InstallStackAlloc;
			addRef := NIL; release := NIL; release2 := NIL;
			DevCPC486.intHandler := NIL;
			IF DevCPM.noerr THEN DevCPE.OutCode END;
			IF ~DevCPM.noerr THEN DevCPM.DeleteObj END
		END
	END Module;

END DevCPV486.
