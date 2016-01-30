MODULE Meta;
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

	IMPORT SYSTEM, Kernel;

	CONST
		(** result codes for object classes, type classes, visibility classes **)
		undef* = 0;

		(** object classes **)
		typObj* = 2; varObj* = 3; procObj* = 4; fieldObj* = 5; modObj* = 6; parObj* = 7;

		(** type classes **)
		boolTyp* = 1; sCharTyp* = 2; charTyp* = 3;
		byteTyp* = 4; sIntTyp* = 5; intTyp* = 6;
		sRealTyp* = 7; realTyp* = 8; setTyp* = 9;
		longTyp* = 10; anyRecTyp* = 11; anyPtrTyp* = 12;
		sysPtrTyp = 13; 
		procTyp* = 16; recTyp* = 17; arrTyp* = 18; ptrTyp* = 19;
		
		(** record attributes **)
		final* = 0; extensible* = 1; limited* = 2; abstract* = 3;
		
		(** visibility **)
		hidden* = 1; readOnly* = 2; private = 3; exported* = 4;
		value* = 10; in* = 11; out* = 12; var* = 13;

		(* scanner modes *)
		modScan = 1; globScan = 2; recVarScan = 3; recTypeScan = 4;

	TYPE
		Name* = ARRAY 256 OF CHAR;

		Value* = ABSTRACT RECORD END;	(* to be extended once with a single field of any type *)
		
		ArrayPtr = POINTER TO Array;

		Item* = RECORD (Value)
			obj-: INTEGER;			(* typObj, varObj, procObj, fieldObj, modObj, parObj *)
			typ-: INTEGER;			(* typObj, varObj, fieldObj, parObj: type;	else: 0 *)
			vis-: INTEGER;			(* varObj, procObj, fieldObj, parObj: vis;	else: 0 *)
			adr-: INTEGER;			(* varObj, procObj: adr;	fieldObj: offs;	parObj: num;	else: 0 *)
			mod: Kernel.Module;	(* static varObj, procObj, modObj: mod;	else: NIL *)
			desc: Kernel.Type;		(* typObj, varObj, fieldObj, parObj: struct;	procObj: sig;	else: NIL *)
			ptr: ArrayPtr;				  (* # NIL => item valid;	dynamic varObj: ptr;	else: dummy *)
			ext: Kernel.ItemExt		(* all method calls forwarded if # NIL *)
		END;

		Scanner* = RECORD
			this-: Item;
			eos-: BOOLEAN;	(* end of scan *)
			mode: INTEGER;	(* modScan, globScan, recVarScan, recTypeScan *)
			base: INTEGER;	(* recVarScan, recTypeScan: base level index *)
			vis: INTEGER;		(* recVarScan: record vis *)
			adr: INTEGER;		(* recVarScan: record adr *)
			idx: INTEGER;		(* globScan, recVarScan, recTypeScan: object index *)
			desc: Kernel.Type;	(* recVarScan, recTypeScan: record desc *)
			mod: Kernel.Module;	(* modScan: next mod;	globScan, recVarScan: source mod *)
			obj: Kernel.Object	(* globScan, recVarScan, recTypeScan: actual object *)
		END;
		
		LookupFilter* = PROCEDURE (IN path: ARRAY OF CHAR; OUT i: Item; OUT done: BOOLEAN);
	
		FilterHook = POINTER TO RECORD
			next: FilterHook;
			filter: LookupFilter
		END;

		Array = EXTENSIBLE RECORD
			w0, w1, w2: INTEGER;	(* gc header *)
			len: ARRAY 16 OF INTEGER	(* dynamic array length table *)
		END;
		
		SStringPtr = POINTER TO ARRAY [1] OF SHORTCHAR;
		StringPtr = POINTER TO ARRAY [1] OF CHAR;
	
	VAR
		dummy: ArrayPtr;	(* dummy object for item.ptr *)
		filterHook: FilterHook;


	(* preconditions:
		ASSERT(i.ptr # NIL, 20);	(* invalid item *)
		ASSERT(i.typ >= recTyp, 21);	(* wrong type *)
		ASSERT(i.obj = varObj, 22);	(* wrong object class *)
		ASSERT((i.mod = NIL) OR (i.mod.refcnt >= 0), 23);	(* unloaded object module *)
		ASSERT(i.desc.mod.refcnt >= 0, 24);	(* unloaded type module *)
		ASSERT(d.id DIV 16 MOD 16 = 1, 25);	(* value not extended once *)
		ASSERT(d.fields.num = 1, 26);	(* not a single value field *)
		ASSERT(i.vis = exported, 27);	(* write protected destination *)
		ASSERT(type.desc.base[t.id DIV 16 MOD 16] = t, 28);	(* wrong pointer type *)
		ASSERT((i < n) & (d.obj[i].id DIV 16 MOD 16 = exported), 29);	(* unexported type *)
		ASSERT(type.desc.id DIV 4 MOD 4 < limited, 30);	(* limited or abstract type *)
		ASSERT(i.ext = NIL, 31);	(* unsupported extension *)
	*)


	PROCEDURE DescOf (IN x: ANYREC): Kernel.Type;
	BEGIN
		RETURN SYSTEM.VAL(Kernel.Type, SYSTEM.TYP(x))
	END DescOf;

	PROCEDURE TypOf (struct: Kernel.Type): INTEGER;
	BEGIN
		IF SYSTEM.VAL(INTEGER, struct) DIV 256 = 0 THEN
			RETURN SYSTEM.VAL(INTEGER, struct)
		ELSE
			RETURN 16 + struct.id MOD 4
		END
	END TypOf;
	
	PROCEDURE LenOf (IN i: Item): INTEGER;
	BEGIN
		IF i.desc.size # 0 THEN RETURN i.desc.size
		ELSIF i.ptr = dummy THEN RETURN 0
		ELSE RETURN i.ptr.len[i.desc.id DIV 16 MOD 16 - 1]
		END
	END LenOf;
	
	PROCEDURE SizeOf (IN i: Item): INTEGER;
		VAR el: Item;
	BEGIN
		CASE i.typ OF
		| anyRecTyp: RETURN 0
		| boolTyp, sCharTyp, byteTyp: RETURN 1
		| charTyp, sIntTyp: RETURN 2
		| longTyp, realTyp: RETURN 8
		| recTyp: RETURN i.desc.size
		| arrTyp:
			el.desc := i.desc.base[0]; el.typ := TypOf(el.desc); el.ptr := i.ptr; 
			RETURN LenOf(i) * SizeOf(el)
		ELSE RETURN 4
		END
	END SizeOf;
	
	PROCEDURE SignatureOf (IN i: Item): Kernel.Signature;
	BEGIN
		IF i.obj = procObj THEN
			RETURN SYSTEM.VAL(Kernel.Signature, i.desc)
		ELSE
			RETURN SYSTEM.VAL(Kernel.Signature, i.desc.base[0])
		END
	END SignatureOf;
	
	
	PROCEDURE GetName (IN path: ARRAY OF CHAR; OUT name: ARRAY OF CHAR; VAR i: INTEGER);
		VAR j: INTEGER; ch: CHAR;
	BEGIN
		j := 0; ch := path[i];
		WHILE (j < LEN(name) - 1) & ((ch >= "0") & (ch <= "9") OR (CAP(ch) >= "A") & (CAP(ch) <= "Z")
											OR (ch >= 0C0X) & (ch # "×") & (ch # "÷") & (ch <= 0FFX) OR (ch = "_")) DO
			name[j] := ch; INC(i); INC(j); ch := path[i]
		END;
		IF (ch = 0X) OR (ch = ".") OR (ch = "[") OR (ch = "^") THEN name[j] := 0X
		ELSE name[0] := 0X
		END
	END GetName;
	
	PROCEDURE LegalName (IN name: ARRAY OF CHAR): BOOLEAN;
		VAR i: INTEGER; ch: CHAR;
	BEGIN
		i := 0; ch := name[0];
		WHILE (i < LEN(name) - 1) & ((ch >= "0") & (ch <= "9") OR (CAP(ch) >= "A") & (CAP(ch) <= "Z")
											OR (ch >= 0C0X) & (ch # "×") & (ch # "÷") & (ch <= 0FFX) OR (ch = "_")) DO
			INC(i); ch := name[i]
		END;
		RETURN (i > 0) & (ch = 0X)
	END LegalName;
	

	(* ---------- Item properties ---------- *)
	
	PROCEDURE (VAR i: Item) Valid* (): BOOLEAN, NEW;
	BEGIN
		IF i.ext # NIL THEN RETURN i.ext.Valid() END;
		RETURN (i.ptr # NIL) & ((i.mod = NIL) OR (i.mod.refcnt >= 0)) & ((i.typ < recTyp) OR (i.desc.mod.refcnt >= 0))
	END Valid;

	PROCEDURE (VAR i: Item) GetTypeName* (OUT mod, type: Name), NEW;
		VAR n: Kernel.Name;
	BEGIN
		ASSERT(i.ext = NIL, 31);
		ASSERT(i.ptr # NIL, 20);
		ASSERT(i.typ >= recTyp, 21);
		ASSERT(i.desc.mod.refcnt >= 0, 24);
		mod := i.desc.mod.name$;
		Kernel.GetTypeName(i.desc, n);
		type := n$
	END GetTypeName;

	PROCEDURE (VAR i: Item) BaseTyp* (): INTEGER, NEW;
	BEGIN
		IF i.ext # NIL THEN RETURN i.ext.BaseTyp() END;
		ASSERT(i.ptr # NIL, 20);
		ASSERT(i.typ IN {arrTyp, recTyp, ptrTyp}, 21);
		RETURN TypOf(i.desc.base[0])
	END BaseTyp;

	PROCEDURE (VAR i: Item) Level* (): INTEGER, NEW;
	BEGIN
		ASSERT(i.ext = NIL, 31);
		ASSERT(i.ptr # NIL, 20);
		ASSERT(i.typ IN {recTyp, arrTyp}, 21);
		RETURN i.desc.id DIV 16 MOD 16
	END Level;

	PROCEDURE (VAR i: Item) Attribute* (): INTEGER, NEW;
	BEGIN
		ASSERT(i.ext = NIL, 31);
		ASSERT(i.ptr # NIL, 20);
		ASSERT(i.typ = recTyp, 21);
		RETURN i.desc.id DIV 4 MOD 4
	END Attribute;

	PROCEDURE (VAR i: Item) Size* (): INTEGER, NEW;
	BEGIN
		IF i.ext # NIL THEN RETURN i.ext.Size() END;
		ASSERT(i.ptr # NIL, 20);
		ASSERT(i.typ # undef, 21);
		RETURN SizeOf(i)
	END Size;
	
	PROCEDURE (VAR arr: Item) Len* (): INTEGER, NEW;
	BEGIN
		IF arr.ext # NIL THEN RETURN arr.ext.Len() END;
		ASSERT(arr.ptr # NIL, 20);
		ASSERT(arr.typ = arrTyp, 21);
		RETURN LenOf(arr)
	END Len;
	
	(* ---------- Item generation ---------- *)

	PROCEDURE SetUndef (VAR i: Item);
	BEGIN
		i.typ := undef; i.obj := undef; i.vis := undef;
		i.adr := undef; i.mod := NIL; i.desc := NIL; i.ptr := NIL; i.ext := NIL;
	END SetUndef;
	
	PROCEDURE SetItem (VAR i: Item; obj: Kernel.Object; mod: Kernel.Module);
		VAR t: Kernel.Type;
	BEGIN
		i.obj := obj.id MOD 16;
		i.vis := obj.id DIV 16 MOD 16;
		IF i.obj = procObj THEN
			i.typ := undef; i.desc := SYSTEM.VAL(Kernel.Type, obj.struct);
			i.adr := mod.procBase + obj.offs; i.mod := mod
		ELSE
			i.typ := TypOf(obj.struct); i.desc := obj.struct;
			IF i.obj = varObj THEN i.adr := mod.varBase + obj.offs; i.mod := mod
			ELSIF i.obj = fieldObj THEN i.adr := obj.offs; i.mod := NIL
			ELSE i.adr := undef; i.mod := NIL
			END
		END;
		i.ext := NIL
	END SetItem;
	
	PROCEDURE SetMod (VAR i: Item; mod: Kernel.Module);
	BEGIN
		i.obj := modObj; i.typ := undef; i.vis := undef;
		i.adr := undef; i.mod := mod; i.desc := NIL; i.ptr := dummy; i.ext := NIL
	END SetMod;


	PROCEDURE GetItem* (obj: ANYPTR; OUT i: Item);
	BEGIN
		ASSERT(obj # NIL, 28);
		i.obj := varObj; i.typ := recTyp; i.vis := exported;
		i.adr := SYSTEM.ADR(obj^); i.ptr := SYSTEM.VAL(ArrayPtr, obj);
		i.mod := NIL; i.desc := Kernel.TypeOf(obj); i.ext := NIL
	END GetItem;

	PROCEDURE Lookup* (IN name: ARRAY OF CHAR; OUT mod: Item);
		VAR m: Kernel.Module; done: BOOLEAN; filter: FilterHook;
	BEGIN
		done := FALSE; filter := filterHook;
		WHILE ~done & (filter # NIL) DO filter.filter(name, mod, done); filter := filter.next END;
		IF ~done & LegalName(name) THEN
			m := Kernel.ThisMod(name);
			IF m # NIL THEN SetMod(mod, m)
			ELSE SetUndef(mod)
			END
		ELSE SetUndef(mod)
		END
	END Lookup;

	PROCEDURE (VAR in: Item) Lookup* (IN name: ARRAY OF CHAR; VAR i: Item), NEW;
		VAR obj: Kernel.Object; o, v, lev, j, a: INTEGER; m: Kernel.Module; n: Kernel.Name;
	BEGIN
		IF in.ext # NIL THEN in.ext.Lookup(name, i); RETURN END;
		ASSERT(in.ptr # NIL, 20);
		IF LegalName(name) THEN
			IF in.obj = modObj THEN
				n := SHORT(name$);
				obj := Kernel.ThisObject(in.mod, n);
				IF obj # NIL THEN
					SetItem(i, obj, in.mod); i.ptr := dummy;
					IF (i.vis = hidden) OR (i.obj < typObj) THEN SetUndef(i) END
				ELSE SetUndef(i)
				END	
			ELSIF in.typ = recTyp THEN
				ASSERT(in.desc.mod.refcnt >= 0, 24);
				lev := in.desc.id DIV 16 MOD 16; j := 0;
				n := SHORT(name$);
				REPEAT
					obj := Kernel.ThisField(in.desc.base[j], n); INC(j)
				UNTIL (obj # NIL) OR (j > lev);
				IF obj # NIL THEN
					o := in.obj; a := in.adr; v := in.vis; m := in.mod;
					SetItem(i, obj, m); i.ptr := in.ptr;
					IF i.vis # hidden THEN
						IF o = varObj THEN
							i.obj := varObj; INC(i.adr, a); i.mod := m;
							IF v < i.vis THEN i.vis := v END
						END
					ELSE SetUndef(i)
					END
				ELSE SetUndef(i)
				END
			ELSE HALT(21)
			END
		ELSE SetUndef(i)
		END
	END Lookup;
	
	PROCEDURE (VAR i: Item) GetBaseType* (VAR base: Item), NEW;
		VAR n: INTEGER;
	BEGIN
		ASSERT(i.ext = NIL, 31);
		ASSERT(i.ptr # NIL, 20);
		ASSERT(i.typ IN {arrTyp, recTyp, ptrTyp}, 21); n := 0;
		IF i.typ = recTyp THEN n := i.desc.id DIV 16 MOD 16 - 1 END;
		IF n >= 0 THEN
			base.obj := typObj; base.vis := undef; base.adr := undef;
			base.mod := NIL; base.ptr := dummy; base.ext := NIL;
			base.desc := i.desc.base[n];
			base.typ := TypOf(base.desc)
		ELSE
			SetUndef(base)
		END
	END GetBaseType;

	PROCEDURE (VAR rec: Item) GetThisBaseType* (level: INTEGER; VAR base: Item), NEW;
	BEGIN
		ASSERT(rec.ext = NIL, 31);
		ASSERT(rec.ptr # NIL, 20);
		ASSERT(rec.typ = recTyp, 21);
		ASSERT((level >= 0) & (level < 16), 28);
		IF level <= rec.desc.id DIV 16 MOD 16 THEN
			base.obj := typObj; base.vis := undef; base.adr := undef;
			base.mod := NIL; base.ptr := dummy; base.ext := NIL;
			base.desc := rec.desc.base[level];
			base.typ := TypOf(base.desc)
		ELSE
			SetUndef(base)
		END
	END GetThisBaseType;
	
	PROCEDURE (VAR proc: Item) NumParam* (): INTEGER, NEW;
		VAR sig: Kernel.Signature;
	BEGIN
		ASSERT(proc.ext = NIL, 31);
		ASSERT(proc.ptr # NIL, 20);
		ASSERT((proc.obj = procObj) OR (proc.typ = procTyp), 21);
		sig := SignatureOf(proc);
		IF sig # NIL THEN RETURN sig.num ELSE RETURN -1 END
	END NumParam;

	PROCEDURE (VAR proc: Item) GetParam* (n: INTEGER; VAR par: Item), NEW;
		VAR sig: Kernel.Signature;
	BEGIN
		ASSERT(proc.ext = NIL, 31);
		ASSERT(proc.ptr # NIL, 20);
		ASSERT((proc.obj = procObj) OR (proc.typ = procTyp), 21);
		sig := SignatureOf(proc);
		IF (sig # NIL) & (n >= 0) & (n < sig.num) THEN
			par.obj := parObj; par.adr := n;
			par.vis := sig.par[n].id MOD 16; 
			par.mod := NIL; par.ptr := dummy; par.ext := NIL;
			par.desc := sig.par[n].struct; par.typ := TypOf(par.desc)
		ELSE
			SetUndef(par)
		END
	END GetParam;

	PROCEDURE (VAR proc: Item) GetParamName* (n: INTEGER; OUT name: Name), NEW;
		VAR sig: Kernel.Signature; mod: Kernel.Module; nm: Kernel.Name;
	BEGIN
		ASSERT(proc.ext = NIL, 31);
		ASSERT(proc.ptr # NIL, 20);
		IF proc.obj = procObj THEN mod := proc.mod
		ELSE ASSERT(proc.typ = procTyp, 21); mod := proc.desc.mod
		END;
		ASSERT(mod.refcnt >= 0, 23);
		sig := SignatureOf(proc);
		IF (sig # NIL) & (n >= 0) & (n < sig.num) THEN
			Kernel.GetObjName(mod, SYSTEM.VAL(Kernel.Object, SYSTEM.ADR(sig.par[n]) - 8), nm);
			name := nm$
		ELSE
			name := ""
		END
	END GetParamName;

	PROCEDURE (VAR proc: Item) GetReturnType* (VAR type: Item), NEW;
		VAR sig: Kernel.Signature;
	BEGIN
		ASSERT(proc.ext = NIL, 31);
		ASSERT(proc.ptr # NIL, 20);
		ASSERT((proc.obj = procObj) OR (proc.typ = procTyp), 21);
		sig := SignatureOf(proc);
		IF sig # NIL THEN
			type.obj := typObj; type.vis := undef; type.adr := undef;
			type.mod := NIL; type.ptr := dummy; type.ext := NIL;
			type.desc := sig.retStruct; type.typ := TypOf(type.desc)
		ELSE
			SetUndef(type)
		END
	END GetReturnType;

	PROCEDURE (VAR rec: Item) Is* (IN type: Value): BOOLEAN, NEW;
		VAR d: Kernel.Type;
	BEGIN
		ASSERT(rec.ext = NIL, 31);
		ASSERT(rec.ptr # NIL, 20);
		ASSERT(rec.typ = recTyp, 21);
		WITH type: Item DO
			ASSERT(type.ptr # NIL, 20);
			ASSERT(type.typ = recTyp, 21);
			d := type.desc
		ELSE
			d := DescOf(type);	(* type of value rec *)
			ASSERT(d.id DIV 16 MOD 16 = 1, 25);	(* level of type = 1*)
			ASSERT(d.fields.num = 1, 26);	(* one field in type *)
			d := d.fields.obj[0].struct;	(* type of field *)
			ASSERT(SYSTEM.VAL(INTEGER, d) DIV 256 # 0, 21);	(* type is structured *)
			IF d.id MOD 4 = 3 THEN d := d.base[0] END	(* deref ptr *)
		END;
		RETURN rec.desc.base[d.id DIV 16 MOD 16] = d	(* rec IS d *)
	END Is;

	PROCEDURE (VAR ptr: Item) Deref* (VAR ref: Item), NEW;
	BEGIN
		IF ptr.ext # NIL THEN ptr.ext.Deref(ref); RETURN END;
		ASSERT(ptr.ptr # NIL, 20);
		ASSERT(ptr.typ IN {sysPtrTyp, anyPtrTyp, ptrTyp}, 21);
		ASSERT(ptr.obj = varObj, 22);
		ASSERT((ptr.mod = NIL) OR (ptr.mod.refcnt >= 0), 23);
		SYSTEM.GET(ptr.adr, ref.adr);
		IF ref.adr # 0 THEN
			IF ptr.typ # ptrTyp THEN ref.typ := recTyp
			ELSE ref.desc := ptr.desc.base[0]; ref.typ := TypOf(ref.desc)
			END;
			ref.obj := varObj; ref.mod := NIL; ref.vis := exported;	(* !!! *)
			ref.ptr := SYSTEM.VAL(ArrayPtr, ref.adr);
			IF ref.typ = recTyp THEN
				ref.desc := DescOf(ref.ptr^);	(* dynamic type *)
			ELSIF ref.typ = arrTyp THEN
				ref.adr := SYSTEM.ADR(ref.ptr.len[ref.desc.id DIV 16 MOD 16]);	(* descriptor offset *)
			ELSE HALT(100)
			END
		ELSE SetUndef(ref)
		END
	END Deref;
	
	PROCEDURE (VAR arr: Item) Index* (index: INTEGER; VAR elem: Item), NEW;
	BEGIN
		IF arr.ext # NIL THEN arr.ext.Index(index, elem); RETURN END;
		ASSERT(arr.ptr # NIL, 20);
		ASSERT(arr.typ = arrTyp, 21);
		ASSERT(arr.obj = varObj, 22);
		IF (index >= 0) & (index < LenOf(arr)) THEN
			elem.obj := varObj; elem.vis := arr.vis;
			elem.mod := arr.mod; elem.ptr := arr.ptr; elem.ext := NIL;
			elem.desc := arr.desc.base[0]; elem.typ := TypOf(elem.desc);
			elem.adr := arr.adr + index * SizeOf(elem)
		ELSE
			SetUndef(elem)
		END
	END Index;
	
	PROCEDURE LookupPath* (IN path: ARRAY OF CHAR; OUT i: Item);
		VAR j, n: INTEGER; name: Name; ch: CHAR; done: BOOLEAN; filter: FilterHook;
	BEGIN
		done := FALSE; filter := filterHook;
		WHILE ~done & (filter # NIL) DO filter.filter(path, i, done); filter := filter.next END;
		IF ~done THEN
			j := 0;
			GetName(path, name, j);
			Lookup(name, i);
			IF (i.obj = modObj) & (path[j] = ".") THEN
				INC(j); GetName(path, name, j);
				i.Lookup(name, i); ch := path[j]; INC(j);
				WHILE (i.obj = varObj) & (ch # 0X) DO
					IF i.typ = ptrTyp THEN i.Deref(i) END;
					IF ch = "." THEN
						GetName(path, name, j);
						IF i.typ = recTyp THEN i.Lookup(name, i) ELSE SetUndef(i) END
					ELSIF ch = "[" THEN
						n := 0; ch := path[j]; INC(j);
						WHILE (ch >= "0") & (ch <= "9") DO n := 10 * n + ORD(ch) - ORD("0"); ch := path[j]; INC(j) END;
						IF (ch = "]") & (i.typ = arrTyp) THEN i.Index(n, i) ELSE SetUndef(i) END
					END;
					ch := path[j]; INC(j)
				END
			END
		END
	END LookupPath;

	(* ---------- Scanner ---------- *)

	PROCEDURE (VAR s: Scanner) ConnectToMods*, NEW;
	BEGIN
		SetUndef(s.this);
		s.this.ptr := dummy;
		s.mod := Kernel.modList;
		s.mode := modScan;
		s.eos := FALSE
	END ConnectToMods;

	PROCEDURE (VAR s: Scanner) ConnectTo* (IN obj: Item), NEW;
	BEGIN
		ASSERT(obj.ptr # NIL, 20);
		SetUndef(s.this); s.vis := obj.vis;
		s.this.ptr := obj.ptr; s.mod := obj.mod; s.idx := 0;
		IF obj.obj = modObj THEN
			ASSERT(s.mod.refcnt >= 0, 23);
			s.mode := globScan
		ELSIF obj.typ = recTyp THEN
			ASSERT(obj.desc.mod.refcnt >= 0, 24);
			s.desc := obj.desc; s.base := 0;
			IF obj.obj = varObj THEN s.mode := recVarScan; s.adr := obj.adr
			ELSE s.mode := recTypeScan
			END
		ELSE HALT(21)
		END;
		s.eos := FALSE
	END ConnectTo;

	PROCEDURE (VAR s: Scanner) Scan*, NEW;
		VAR desc: Kernel.Type;
	BEGIN
		ASSERT(s.this.ptr # NIL, 20);
		IF s.mode = modScan THEN
			IF s.mod # NIL THEN SetMod(s.this, s.mod); s.mod := s.mod.next
			ELSE SetUndef(s.this); s.eos := TRUE
			END
		ELSIF s.mode = globScan THEN
			ASSERT(s.mod.refcnt >= 0, 23);
			REPEAT
				IF s.idx >= s.mod.export.num THEN SetUndef(s.this); s.eos := TRUE; RETURN END;
				s.obj := SYSTEM.VAL(Kernel.Object, SYSTEM.ADR(s.mod.export.obj[s.idx]));
				SetItem(s.this, s.obj, s.mod); INC(s.idx)
			UNTIL (s.this.obj IN {procObj, varObj, typObj}) & (s.this.vis # hidden)
		ELSE
			ASSERT(s.desc.mod.refcnt >= 0, 24);
			desc := s.desc.base[s.base];
			REPEAT
				WHILE s.idx >= desc.fields.num DO
					IF desc = s.desc THEN SetUndef(s.this); s.eos := TRUE; RETURN END;
					INC(s.base); desc := s.desc.base[s.base]; s.idx := 0
				END;
				s.obj := SYSTEM.VAL(Kernel.Object, SYSTEM.ADR(desc.fields.obj[s.idx]));
				SetItem(s.this, s.obj, s.mod); INC(s.idx)
			UNTIL s.this.vis # hidden;
			IF s.mode = recVarScan THEN
				s.this.obj := varObj; INC(s.this.adr, s.adr); s.this.mod := s.mod; 
				IF s.vis < s.this.vis THEN s.this.vis := s.vis END
			END
		END
	END Scan;

	PROCEDURE (VAR s: Scanner) GetObjName* (OUT name: Name), NEW;
		VAR mod: Kernel.Module; n: Kernel.Name;
	BEGIN
		ASSERT(s.this.ptr # NIL, 20);
		IF s.mode = modScan THEN
			name := s.this.mod.name$	(* mf 24.08.2004 *)
		ELSE
			IF s.mode = globScan THEN mod := s.mod
			ELSE mod := s.desc.base[s.base].mod
			END;
			ASSERT(mod.refcnt >= 0, 23);
			Kernel.GetObjName(mod, s.obj, n);
			name := n$;
		END
	END GetObjName;
	
	PROCEDURE (VAR s: Scanner) Level* (): INTEGER, NEW;
	BEGIN
		ASSERT(s.this.ptr # NIL, 20);
		ASSERT(s.mode >= recVarScan, 22);
		RETURN s.base
	END Level;

	(* ---------- access to item values ---------- *)

	PROCEDURE ValToItem (IN x: Value; VAR i: Item);
		VAR desc: Kernel.Type;
	BEGIN
		desc := DescOf(x);
		ASSERT(desc.id DIV 16 MOD 16 = 1, 25);	(* level of x = 1*)
		ASSERT(desc.fields.num = 1, 26);	(* one field in x *)
		i.desc := desc.fields.obj[0].struct;
		i.typ := TypOf(i.desc); i.obj := varObj; i.ext := NIL; i.vis := exported;
		i.ptr := dummy; i.adr := SYSTEM.ADR(x)
	END ValToItem;
	
	PROCEDURE^ EqualSignature (a, b: Kernel.Signature): BOOLEAN;
	
	PROCEDURE EqualType (a, b: Kernel.Type): BOOLEAN;
	BEGIN
		LOOP
			IF a = b THEN RETURN TRUE END;
			IF (SYSTEM.VAL(INTEGER, a) DIV 256 = 0)
				OR (SYSTEM.VAL(INTEGER, b) DIV 256 = 0)
				OR (a.id MOD 4 # b.id MOD 4) THEN RETURN FALSE END;
			CASE a.id MOD 4 OF
			| recTyp - 16: RETURN FALSE
			| arrTyp - 16: IF (a.size # 0) OR (b.size # 0) THEN RETURN FALSE END
			| procTyp - 16: RETURN EqualSignature(SYSTEM.VAL(Kernel.Signature, a.base[0]),
																	 SYSTEM.VAL(Kernel.Signature, b.base[0]))
			ELSE (* ptrTyp *)
			END;
			a := a.base[0]; b := b.base[0]
		END
	END EqualType;
	
	PROCEDURE EqualSignature (a, b: Kernel.Signature): BOOLEAN;
		VAR i: INTEGER;
	BEGIN
		IF (a.num # b.num) OR ~EqualType(a.retStruct, b.retStruct) THEN RETURN FALSE END;
		i := 0;
		WHILE i < a.num DO
			IF (a.par[i].id MOD 256 # b.par[i].id MOD 256)
				OR ~EqualType(a.par[i].struct, b.par[i].struct) THEN RETURN FALSE END;
			INC(i)
		END;
		RETURN TRUE
	END EqualSignature;
	
	PROCEDURE Copy (IN a, b: Item; OUT ok: BOOLEAN);	(* b := a *)
		VAR n: INTEGER; at, bt: Item;
	BEGIN
		ok := FALSE;
		IF a.obj = procObj THEN
			IF (b.typ # procTyp)
				OR ~EqualSignature(SignatureOf(a), SignatureOf(b)) THEN RETURN END;
			SYSTEM.PUT(b.adr, a.adr); 
		ELSE	(* a.obj = varObj *)
			IF a.typ # b.typ THEN RETURN END;
			IF a.typ >= recTyp THEN
				IF a.typ = ptrTyp THEN
					at.desc := a.desc.base[0]; at.typ := TypOf(at.desc); at.ptr := dummy; at.ext := NIL;
					bt.desc := b.desc.base[0]; bt.typ := TypOf(bt.desc); bt.ptr := dummy; bt.ext := NIL;
					SYSTEM.GET(a.adr, n);
					IF (at.typ = recTyp) & (n # 0) THEN
						SYSTEM.GET(SYSTEM.VAL(INTEGER, n) - 4, at.desc);	(* dynamic type *)
						at.desc :=  at.desc.base[bt.desc.id DIV 16 MOD 16]	(* projection to b *)
					END
				ELSE at := a; bt := b
				END;
				WHILE (at.typ = arrTyp) & (bt.typ = arrTyp) DO
					IF LenOf(at) # LenOf(bt) THEN RETURN END;
					at.desc := at.desc.base[0]; at.typ := TypOf(at.desc);
					bt.desc := bt.desc.base[0]; bt.typ := TypOf(bt.desc)
				END;
				IF (at.desc # bt.desc) &
					~((at.typ = procTyp) & (bt.typ = procTyp)
						& EqualSignature(SignatureOf(at), SignatureOf(bt))) THEN RETURN END
			END;
			SYSTEM.MOVE(a.adr, b.adr, SizeOf(b))
		END;
		ok := TRUE
	END Copy;
	
	PROCEDURE (VAR proc: Item) Call* (OUT ok: BOOLEAN), NEW;
		VAR p: Kernel.Command; sig: Kernel.Signature;
	BEGIN
		IF proc.ext # NIL THEN proc.ext.Call(ok); RETURN END;
		ASSERT(proc.ptr # NIL, 20);
		IF proc.obj = procObj THEN
			p := SYSTEM.VAL(Kernel.Command, proc.adr)
		ELSE ASSERT((proc.obj = varObj) & (proc.typ = procTyp), 22);
			SYSTEM.GET(proc.adr, p)
		END;
		ASSERT((proc.mod = NIL) OR (proc.mod.refcnt >= 0), 23);
		sig := SignatureOf(proc);
		IF (sig.retStruct = NIL) & (sig.num = 0) & (p # NIL) THEN p(); ok := TRUE
		ELSE ok := FALSE
		END
	END Call;
	
	PROCEDURE PutParam (IN par: Item; sig: Kernel.Signature; p: INTEGER;
										VAR data: ARRAY OF INTEGER; VAR n: INTEGER;
										OUT ok: BOOLEAN);	(* check & assign a parameter *)
		VAR mode, fTyp, aTyp, padr, i: INTEGER; fDesc, aDesc: Kernel.Type;
			l: LONGINT; s: SHORTINT; b: BYTE;
	BEGIN
		ok := FALSE;
		ASSERT(par.ext = NIL, 31);
		ASSERT(par.ptr # NIL, 20);
		ASSERT(par.obj = varObj, 22);
		ASSERT((par.mod = NIL) OR (par.mod.refcnt >= 0), 23);
		mode := sig.par[p].id MOD 16;
		IF mode >= out THEN ASSERT(par.vis = exported, 27) END;
		fDesc := sig.par[p].struct;
		fTyp := TypOf(fDesc);
		aDesc := par.desc;
		aTyp := TypOf(aDesc);
		padr := par.adr;
		IF (fTyp = recTyp) OR (fTyp = anyRecTyp) THEN
			IF (aTyp # recTyp)
				OR (mode = value) & (aDesc # fDesc)
				OR (fTyp = recTyp) & (aDesc.base[fDesc.id DIV 16 MOD 16] # fDesc) THEN RETURN END;
			data[n] := padr; INC(n);
			data[n] := SYSTEM.VAL(INTEGER, aDesc); INC(n)
		ELSIF fTyp = arrTyp THEN
			data[n] := padr; INC(n);
			IF fDesc.size # 0 THEN data[n] := SizeOf(par); INC(n) END;
			WHILE (TypOf(fDesc) = arrTyp) & (TypOf(aDesc) = arrTyp) DO
				IF aDesc.size # 0 THEN i := aDesc.size	(* actual static size *)
				ELSE i := par.ptr.len[aDesc.id DIV 16 MOD 16 - 1]	(* actual dynamic size *)
				END;
				IF fDesc.size = 0 THEN data[n] := i; INC(n)
				ELSIF fDesc.size # i THEN RETURN
				END;
				fDesc := fDesc.base[0]; aDesc := aDesc.base[0]
			END;
			IF fDesc # aDesc THEN RETURN END
		ELSIF fTyp >= anyPtrTyp THEN	(* pointer *)
			IF fTyp = ptrTyp THEN
				fDesc := fDesc.base[0];	(* formal base type *)
				IF (mode = value) & (TypOf(fDesc) = recTyp) THEN
					IF (aTyp # ptrTyp) & (aTyp # anyPtrTyp) THEN RETURN END;
					SYSTEM.GET(padr, i); SYSTEM.GET(i - 4, aDesc);	(* dynamic record type *)
					aDesc := aDesc.base[fDesc.id DIV 16 MOD 16]	(* projection *)
				ELSE
					IF aTyp # ptrTyp THEN RETURN END;
					aDesc := aDesc.base[0];	(* actual base type *)
					WHILE (TypOf(fDesc) = arrTyp) & (TypOf(aDesc) = arrTyp) DO
						IF fDesc.size # aDesc.size THEN RETURN END;
						fDesc := fDesc.base[0]; aDesc := aDesc.base[0]
					END
				END;
				IF fDesc # aDesc THEN RETURN END
			ELSIF fTyp = anyPtrTyp THEN
				IF (aTyp # anyPtrTyp) & ((aTyp # ptrTyp) OR (TypOf(aDesc.base[0]) # recTyp)) THEN RETURN END
			ELSIF fTyp = procTyp THEN
				IF (aTyp # procTyp) OR (fDesc.size # aDesc.size) THEN RETURN END	(* same fingerprint *)
			END;
			IF mode = value THEN SYSTEM.GET(padr, data[n]); INC(n)
			ELSE data[n] := padr; INC(n)
			END
		ELSE	(* basic type *)
			IF fTyp # aTyp THEN RETURN END;
			IF mode = value THEN
				CASE SizeOf(par) OF
				| 1: SYSTEM.GET(padr, b); data[n] := b; INC(n)
				| 2: SYSTEM.GET(padr, s); data[n] := s; INC(n)
				| 4: SYSTEM.GET(padr, i); data[n] := i; INC(n)
				| 8: SYSTEM.GET(padr, l); data[n] := SHORT(l); INC(n); data[n] := SHORT(l DIV 100000000L); INC(n)
				END
			ELSE	(* var par *)
				data[n] := padr; INC(n)
			END
		END;
		ok := TRUE
	END PutParam;
	
	PROCEDURE GetResult (ret: LONGINT; VAR dest: Item; sig: Kernel.Signature;
											OUT ok: BOOLEAN);	(* assign return value *)
		VAR x: Item; i: INTEGER; s: SHORTINT; b: BYTE;
	BEGIN
		ASSERT(dest.ext = NIL, 31);
		ASSERT(dest.ptr # NIL, 20);
		ASSERT(dest.obj = varObj, 22);
		ASSERT((dest.mod = NIL) OR (dest.mod.refcnt >= 0), 23);
		ASSERT(dest.vis = exported, 27);
		x.desc := sig.retStruct; x.typ := TypOf(x.desc);
		x.obj := varObj; x.ptr := dummy;
		CASE TypOf(sig.retStruct) OF
		| boolTyp, sCharTyp, byteTyp: b := SHORT(SHORT(SHORT(ret))); x.adr := SYSTEM.ADR(b);
		| charTyp, sIntTyp: s := SHORT(SHORT(ret)); x.adr := SYSTEM.ADR(s);
		| longTyp, realTyp: x.adr := SYSTEM.ADR(ret);
		| intTyp, sRealTyp, setTyp, anyPtrTyp, procTyp, ptrTyp: i := SHORT(ret); x.adr := SYSTEM.ADR(i);
		END;
		Copy(x, dest, ok)
	END GetResult;
	
	PROCEDURE (VAR proc: Item) ParamCall* (IN par: ARRAY OF Item; VAR dest: Item;
																			OUT ok: BOOLEAN), NEW;
		VAR n, p, adr, padr: INTEGER; ret: LONGINT;
			data: ARRAY 256 OF INTEGER; sig: Kernel.Signature;
	BEGIN
		ok := TRUE;
		ASSERT(proc.ext = NIL, 31);
		ASSERT(proc.ptr # NIL, 20);
		IF proc.obj = procObj THEN adr := proc.adr
		ELSE ASSERT((proc.obj = varObj) & (proc.typ = procTyp), 22);
			SYSTEM.GET(proc.adr, adr);
			IF adr = 0 THEN ok := FALSE; RETURN END
		END;
		ASSERT((proc.mod = NIL) OR (proc.mod.refcnt >= 0), 23);
		sig := SignatureOf(proc);
		ASSERT((sig # NIL) & (LEN(par) >= sig.num), 32);
		n := 0; p := 0;
		WHILE ok & (p < sig.num) DO	(* check & assign parameters *)
			PutParam(par[p], sig, p, data, n, ok);
			INC(p)
		END;
		IF ok THEN
			ret := Kernel.Call(adr, sig, data, n);
			IF sig.retStruct # NIL THEN GetResult(ret, dest, sig, ok) END
		END
	END ParamCall;

	PROCEDURE (VAR proc: Item) ParamCallVal* (IN par: ARRAY OF POINTER TO Value; VAR dest: Value;
																			OUT ok: BOOLEAN), NEW;
		TYPE IP = POINTER TO Item;
		VAR n, p, adr, padr: INTEGER; ret: LONGINT; x: Item;
			data: ARRAY 256 OF INTEGER; sig: Kernel.Signature;
	BEGIN
		ok := TRUE;
		ASSERT(proc.ext = NIL, 31);
		ASSERT(proc.ptr # NIL, 20);
		IF proc.obj = procObj THEN adr := proc.adr
		ELSE ASSERT((proc.obj = varObj) & (proc.typ = procTyp), 22);
			SYSTEM.GET(proc.adr, adr);
			IF adr = 0 THEN ok := FALSE; RETURN END
		END;
		ASSERT((proc.mod = NIL) OR (proc.mod.refcnt >= 0), 23);
		sig := SignatureOf(proc);
		ASSERT((sig # NIL) & (LEN(par) >= sig.num), 32);
		n := 0; p := 0;
		WHILE ok & (p < sig.num) DO	(* check & assign parameters *)
			IF par[p] IS IP THEN
				PutParam(par[p](IP)^, sig, p, data, n, ok)
			ELSE
				ValToItem(par[p]^, x);
				PutParam(x, sig, p, data, n, ok)
			END;
			INC(p)
		END;
		IF ok THEN
			ret := Kernel.Call(adr, sig, data, n);
			IF sig.retStruct # NIL THEN
				WITH dest: Item DO
					GetResult(ret, dest, sig, ok)
				ELSE
					ValToItem(dest, x);
					GetResult(ret, x, sig, ok)
				END
			END
		END
	END ParamCallVal;

	PROCEDURE (VAR var: Item) GetVal* (VAR x: Value; OUT ok: BOOLEAN), NEW;
		VAR xi: Item;
	BEGIN
		ASSERT(var.ext = NIL, 31);
		ASSERT(var.ptr # NIL, 20);
		ASSERT(var.obj IN {varObj, procObj}, 22);
		ASSERT((var.mod = NIL) OR (var.mod.refcnt >= 0), 23);
		WITH x: Item DO
			ASSERT(x.ptr # NIL, 20);
			ASSERT(x.obj = varObj, 22);
			ASSERT((x.mod = NIL) OR (x.mod.refcnt >= 0), 23);
			ASSERT(x.vis = exported, 27);
			Copy(var, x, ok)
		ELSE
			ValToItem(x, xi); Copy(var, xi, ok)
		END
	END GetVal;

	PROCEDURE (VAR var: Item) PutVal* (IN x: Value; OUT ok: BOOLEAN), NEW;
		VAR xi: Item;
	BEGIN
		ASSERT(var.ext = NIL, 31);
		ASSERT(var.ptr # NIL, 20);
		ASSERT(var.obj = varObj, 22);
		ASSERT((var.mod = NIL) OR (var.mod.refcnt >= 0), 23);
		ASSERT(var.vis = exported, 27);
		WITH x: Item DO
			ASSERT(x.ptr # NIL, 20);
			ASSERT(x.obj IN {varObj, procObj}, 22);
			ASSERT((x.mod = NIL) OR (x.mod.refcnt >= 0), 23);
			Copy(x, var, ok)
		ELSE
			ValToItem(x, xi); Copy(xi, var, ok)
		END
	END PutVal;

	PROCEDURE (VAR var: Item) GetStringVal* (OUT x: ARRAY OF CHAR; OUT ok: BOOLEAN), NEW;
		VAR i, n: INTEGER; p: StringPtr;
	BEGIN
		IF var.ext # NIL THEN var.ext.GetStringVal(x, ok); RETURN END;
		ASSERT(var.ptr # NIL, 20);
		ASSERT((var.typ = arrTyp) & (SYSTEM.VAL(INTEGER, var.desc.base[0]) = charTyp), 21);
		ASSERT(var.obj = varObj, 22);
		ASSERT((var.mod = NIL) OR (var.mod.refcnt >= 0), 23);
		p := SYSTEM.VAL(StringPtr, var.adr); i := 0; n := LenOf(var);
		WHILE (i < n) & (p[i] # 0X) DO INC(i) END;
		IF (i < n) & (i < LEN(x)) THEN x := p^$; ok := TRUE
		ELSE x := ""; ok := FALSE
		END
	END GetStringVal;

	PROCEDURE (VAR var: Item) GetSStringVal* (OUT x: ARRAY OF SHORTCHAR; OUT ok: BOOLEAN), NEW;
		VAR i, n: INTEGER; p: SStringPtr;
	BEGIN
		IF var.ext # NIL THEN var.ext.GetSStringVal(x, ok); RETURN END;
		ASSERT(var.ptr # NIL, 20);
		ASSERT((var.typ = arrTyp) & (SYSTEM.VAL(INTEGER, var.desc.base[0]) = sCharTyp), 21);
		ASSERT(var.obj = varObj, 22);
		ASSERT((var.mod = NIL) OR (var.mod.refcnt >= 0), 23);
		p := SYSTEM.VAL(SStringPtr, var.adr); i := 0; n := LenOf(var);
		WHILE (i < n) & (p[i] # 0X) DO INC(i) END;
		IF (i < n) & (i < LEN(x)) THEN x := p^$; ok := TRUE
		ELSE x := ""; ok := FALSE
		END
	END GetSStringVal;

	PROCEDURE (VAR var: Item) PutStringVal* (IN x: ARRAY OF CHAR; OUT ok: BOOLEAN), NEW;
		VAR i: INTEGER; p: StringPtr;
	BEGIN
		IF var.ext # NIL THEN var.ext.PutStringVal(x, ok); RETURN END;
		ASSERT(var.ptr # NIL, 20);
		ASSERT((var.typ = arrTyp) & (SYSTEM.VAL(INTEGER, var.desc.base[0]) = charTyp), 21);
		ASSERT(var.obj = varObj, 22);
		ASSERT((var.mod = NIL) OR (var.mod.refcnt >= 0), 23);
		ASSERT(var.vis = exported, 27);
		p := SYSTEM.VAL(StringPtr, var.adr); i := 0;
		WHILE (i < LEN(x)) & (x[i] # 0X) DO INC(i) END;
		IF (i < LEN(x)) & (i < LenOf(var)) THEN p^ := x$; ok := TRUE
		ELSE ok := FALSE
		END
	END PutStringVal;

	PROCEDURE (VAR var: Item) PutSStringVal* (IN x: ARRAY OF SHORTCHAR; OUT ok: BOOLEAN), NEW;
		VAR i: INTEGER; p: SStringPtr;
	BEGIN
		IF var.ext # NIL THEN var.ext.PutSStringVal(x, ok); RETURN END;
		ASSERT(var.ptr # NIL, 20);
		ASSERT((var.typ = arrTyp) & (SYSTEM.VAL(INTEGER, var.desc.base[0]) = sCharTyp), 21);
		ASSERT(var.obj = varObj, 22);
		ASSERT((var.mod = NIL) OR (var.mod.refcnt >= 0), 23);
		ASSERT(var.vis = exported, 27);
		p := SYSTEM.VAL(SStringPtr, var.adr); i := 0;
		WHILE (i < LEN(x)) & (x[i] # 0X) DO INC(i) END;
		IF (i < LEN(x)) & (i < LenOf(var)) THEN p^ := x$; ok := TRUE
		ELSE ok := FALSE
		END
	END PutSStringVal;

	PROCEDURE  (VAR var: Item) PtrVal* (): ANYPTR, NEW;
		VAR p: ANYPTR;
	BEGIN
		IF var.ext # NIL THEN RETURN var.ext.PtrVal() END;
		ASSERT(var.ptr # NIL, 20);
		ASSERT(var.typ IN {anyPtrTyp, ptrTyp}, 21);
		ASSERT(var.obj = varObj, 22);
		ASSERT((var.mod = NIL) OR (var.mod.refcnt >= 0), 23);
		SYSTEM.GET(var.adr, p);
		RETURN p
	END PtrVal;

	PROCEDURE (VAR var: Item) PutPtrVal* (x: ANYPTR), NEW;
		VAR vt, xt: Kernel.Type;
	BEGIN
		IF var.ext # NIL THEN var.ext.PutPtrVal(x); RETURN END;
		ASSERT(var.ptr # NIL, 20);
		ASSERT(var.typ IN {anyPtrTyp, ptrTyp}, 21);
		ASSERT(var.obj = varObj, 22);
		ASSERT((var.mod = NIL) OR (var.mod.refcnt >= 0), 23);
		ASSERT(var.vis = exported, 27);
		IF (x # NIL) & (var.typ = ptrTyp) THEN
			vt := var.desc.base[0]; xt := Kernel.TypeOf(x);
			ASSERT(xt.base[vt.id DIV 16 MOD 16] = vt, 28);	(* xt IS vt *)
		END;
		SYSTEM.PUT(var.adr, x)
	END PutPtrVal;

	PROCEDURE (VAR var: Item) IntVal* (): INTEGER, NEW;
		VAR sc: SHORTCHAR; ch: CHAR; s: BYTE; i: SHORTINT; x: INTEGER;
	BEGIN
		IF var.ext # NIL THEN RETURN var.ext.IntVal() END;
		ASSERT(var.ptr # NIL, 20);
		ASSERT(var.obj = varObj, 22);
		ASSERT((var.mod = NIL) OR (var.mod.refcnt >= 0), 23);
		IF var.typ = sCharTyp THEN SYSTEM.GET(var.adr, sc); x := ORD(sc)
		ELSIF var.typ = charTyp THEN SYSTEM.GET(var.adr, ch); x := ORD(ch)
		ELSIF var.typ = byteTyp THEN SYSTEM.GET(var.adr, s); x := s
		ELSIF var.typ = sIntTyp THEN SYSTEM.GET(var.adr, i); x := i
		ELSIF var.typ = intTyp THEN SYSTEM.GET(var.adr, x)
		ELSE HALT(21)
		END;
		RETURN x
	END IntVal;

	PROCEDURE (VAR var: Item) PutIntVal* (x: INTEGER), NEW;
	BEGIN
		IF var.ext # NIL THEN var.ext.PutIntVal(x); RETURN END;
		ASSERT(var.ptr # NIL, 20);
		ASSERT(var.obj = varObj, 22);
		ASSERT((var.mod = NIL) OR (var.mod.refcnt >= 0), 23);
		ASSERT(var.vis = exported, 27);
		IF var.typ = sCharTyp THEN SYSTEM.PUT(var.adr, SHORT(CHR(x)))
		ELSIF var.typ = charTyp THEN SYSTEM.PUT(var.adr, CHR(x))
		ELSIF var.typ = byteTyp THEN SYSTEM.PUT(var.adr, SHORT(SHORT(x)))
		ELSIF var.typ = sIntTyp THEN SYSTEM.PUT(var.adr, SHORT(x))
		ELSIF var.typ = intTyp THEN SYSTEM.PUT(var.adr, x)
		ELSE HALT(21)
		END
	END PutIntVal;

	PROCEDURE (VAR var: Item) RealVal* (): REAL, NEW;
		VAR r: SHORTREAL; x: REAL;
	BEGIN
		IF var.ext # NIL THEN RETURN var.ext.RealVal() END;
		ASSERT(var.ptr # NIL, 20);
		ASSERT(var.obj = varObj, 22);
		ASSERT((var.mod = NIL) OR (var.mod.refcnt >= 0), 23);
		IF var.typ = sRealTyp THEN SYSTEM.GET(var.adr, r); x := r
		ELSIF var.typ = realTyp THEN SYSTEM.GET(var.adr, x)
		ELSE HALT(21)
		END;
		RETURN x
	END RealVal;

	PROCEDURE (VAR var: Item) PutRealVal* (x: REAL), NEW;
	BEGIN
		IF var.ext # NIL THEN var.ext.PutRealVal(x); RETURN END;
		ASSERT(var.ptr # NIL, 20);
		ASSERT(var.obj = varObj, 22);
		ASSERT((var.mod = NIL) OR (var.mod.refcnt >= 0), 23);
		ASSERT(var.vis = exported, 27);
		IF var.typ = sRealTyp THEN SYSTEM.PUT(var.adr, SHORT(x))
		ELSIF var.typ = realTyp THEN SYSTEM.PUT(var.adr, x)
		ELSE HALT(21)
		END
	END PutRealVal;

	PROCEDURE (VAR var: Item) LongVal* (): LONGINT, NEW;
		VAR x: LONGINT;
	BEGIN
		IF var.ext # NIL THEN RETURN var.ext.LongVal() END;
		ASSERT(var.ptr # NIL, 20);
		ASSERT(var.typ = longTyp, 21);
		ASSERT(var.obj = varObj, 22);
		ASSERT((var.mod = NIL) OR (var.mod.refcnt >= 0), 23);
		SYSTEM.GET(var.adr, x);
		RETURN x
	END LongVal;

	PROCEDURE (VAR var: Item) PutLongVal* (x: LONGINT), NEW;
	BEGIN
		IF var.ext # NIL THEN var.ext.PutLongVal(x); RETURN END;
		ASSERT(var.ptr # NIL, 20);
		ASSERT(var.typ = longTyp, 21);
		ASSERT(var.obj = varObj, 22);
		ASSERT((var.mod = NIL) OR (var.mod.refcnt >= 0), 23);
		ASSERT(var.vis = exported, 27);
		SYSTEM.PUT(var.adr, x)
	END PutLongVal;

	PROCEDURE (VAR var: Item) CharVal* (): CHAR, NEW;
		VAR x: CHAR; s: SHORTCHAR;
	BEGIN
		IF var.ext # NIL THEN RETURN var.ext.CharVal() END;
		ASSERT(var.ptr # NIL, 20);
		ASSERT(var.obj = varObj, 22);
		ASSERT((var.mod = NIL) OR (var.mod.refcnt >= 0), 23);
		IF var.typ = sCharTyp THEN SYSTEM.GET(var.adr, s); x := s
		ELSIF var.typ = charTyp THEN SYSTEM.GET(var.adr, x)
		ELSE HALT(21)
		END;
		RETURN x
	END CharVal;

	PROCEDURE (VAR var: Item) PutCharVal* (x: CHAR), NEW;
	BEGIN
		IF var.ext # NIL THEN var.ext.PutCharVal(x); RETURN END;
		ASSERT(var.ptr # NIL, 20);
		ASSERT(var.obj = varObj, 22);
		ASSERT((var.mod = NIL) OR (var.mod.refcnt >= 0), 23);
		ASSERT(var.vis = exported, 27);
		IF var.typ = sCharTyp THEN SYSTEM.PUT(var.adr, SHORT(x))
		ELSIF var.typ = charTyp THEN SYSTEM.PUT(var.adr, x)
		ELSE HALT(21)
		END
	END PutCharVal;

	PROCEDURE (VAR var: Item) BoolVal* (): BOOLEAN, NEW;
		VAR x: BOOLEAN;
	BEGIN
		IF var.ext # NIL THEN RETURN var.ext.BoolVal() END;
		ASSERT(var.ptr # NIL, 20);
		ASSERT(var.typ = boolTyp, 21);
		ASSERT(var.obj = varObj, 22);
		ASSERT((var.mod = NIL) OR (var.mod.refcnt >= 0), 23);
		SYSTEM.GET(var.adr, x);
		RETURN x
	END BoolVal;

	PROCEDURE (VAR var: Item) PutBoolVal* (x: BOOLEAN), NEW;
	BEGIN
		IF var.ext # NIL THEN var.ext.PutBoolVal(x); RETURN END;
		ASSERT(var.ptr # NIL, 20);
		ASSERT(var.typ = boolTyp, 21);
		ASSERT(var.obj = varObj, 22);
		ASSERT((var.mod = NIL) OR (var.mod.refcnt >= 0), 23);
		ASSERT(var.vis = exported, 27);
		SYSTEM.PUT(var.adr, x)
	END PutBoolVal;

	PROCEDURE (VAR var: Item) SetVal* (): SET, NEW;
		VAR x: SET;
	BEGIN
		IF var.ext # NIL THEN RETURN var.ext.SetVal() END;
		ASSERT(var.ptr # NIL, 20);
		ASSERT(var.typ = setTyp, 21);
		ASSERT(var.obj = varObj, 22);
		ASSERT((var.mod = NIL) OR (var.mod.refcnt >= 0), 23);
		SYSTEM.GET(var.adr, x);
		RETURN x
	END SetVal;

	PROCEDURE (VAR var: Item) PutSetVal* (x: SET), NEW;
	BEGIN
		IF var.ext # NIL THEN var.ext.PutSetVal(x); RETURN END;
		ASSERT(var.ptr # NIL, 20);
		ASSERT(var.typ = setTyp, 21);
		ASSERT(var.obj = varObj, 22);
		ASSERT((var.mod = NIL) OR (var.mod.refcnt >= 0), 23);
		ASSERT(var.vis = exported, 27);
		SYSTEM.PUT(var.adr, x)
	END PutSetVal;

	PROCEDURE (VAR  type: Item) New* (): ANYPTR, NEW;
		VAR p: ANYPTR; i, n, id: INTEGER; d: Kernel.Directory; desc: Kernel.Type;
	BEGIN
		ASSERT(type.ext = NIL, 31);
		ASSERT(type.ptr # NIL, 20);
		desc := type.desc;
		IF type.typ = ptrTyp THEN desc := desc.base[0] END;
		ASSERT(TypOf(desc) = recTyp, 21);
		ASSERT(desc.mod.refcnt >= 0, 24);
		i := 0; d := type.desc.mod.export; n := d.num; id := type.desc.id DIV 256;
		WHILE (i < n) & (d.obj[i].id DIV 256 # id) DO INC(i) END;
		ASSERT((i < n) & (d.obj[i].id DIV 16 MOD 16 = exported), 29);
		ASSERT(desc.id DIV 4 MOD 4 < limited, 30);
		Kernel.NewObj(p, desc);
		RETURN p
	END New;

	PROCEDURE (VAR  val: Item) Copy* (): ANYPTR, NEW;
		VAR p: ANYPTR; i, n, id: INTEGER; d: Kernel.Directory;
	BEGIN
		ASSERT(val.ext = NIL, 31);
		ASSERT(val.ptr # NIL, 20);
		ASSERT(val.typ = recTyp, 21);
		ASSERT(val.obj = varObj, 22);
		ASSERT(val.desc.mod.refcnt >= 0, 24);
		i := 0; d := val.desc.mod.export; n := d.num; id := val.desc.id DIV 256;
		WHILE (i < n) & (d.obj[i].id DIV 256 # id) DO INC(i) END;
		ASSERT((i < n) & (d.obj[i].id DIV 16 MOD 16 = exported), 29);
		ASSERT(val.desc.id DIV 4 MOD 4 < limited, 30);
		Kernel.NewObj(p, val.desc);
		SYSTEM.MOVE(val.adr, p, val.desc.size);
		RETURN p
	END Copy;

	PROCEDURE (VAR rec: Item) CallWith* (proc: PROCEDURE(VAR rec, par: ANYREC); VAR par: ANYREC), NEW;
	BEGIN
		ASSERT(rec.ext = NIL, 31);
		ASSERT(rec.ptr # NIL, 20);
		ASSERT(rec.typ = recTyp, 21);
		ASSERT(rec.obj = varObj, 22);
		ASSERT((rec.mod = NIL) OR (rec.mod.refcnt >= 0), 23);
		proc(SYSTEM.THISRECORD(rec.adr, SYSTEM.VAL(INTEGER, rec.desc)), par)
	END CallWith;


	PROCEDURE InstallFilter* (filter: LookupFilter);
		VAR h: FilterHook;
	BEGIN
		ASSERT(filter # NIL, 20);
		NEW(h); h.filter := filter; h.next := filterHook; filterHook := h
	END InstallFilter;

	PROCEDURE UninstallFilter* (filter: LookupFilter);
		VAR h, a: FilterHook;
	BEGIN
		ASSERT(filter # NIL, 20);
		h := filterHook; a := NIL;
		WHILE (h # NIL) & (h.filter # filter) DO a := h; h := h.next END;
		IF h # NIL THEN
			IF a = NIL THEN filterHook := h.next ELSE a.next := h.next END
		END
	END UninstallFilter;

	PROCEDURE GetThisItem* (IN attr: ANYREC; OUT i: Item);
	BEGIN
		WITH attr: Kernel.ItemAttr DO
			i.obj := attr.obj; i.vis := attr.vis; i.typ := attr.typ; i.adr := attr.adr;
			i.mod := attr.mod; i.desc := attr.desc; i.ptr := attr.ptr; i.ext := attr.ext;
			IF i.ptr = NIL THEN i.ptr := dummy END
		END
	END GetThisItem;

BEGIN
	NEW(dummy)
END Meta.
