﻿MODULE DevElfLinker;
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

(*
	DevElfLinker version compatible with BlackBox Component Builder release 1.6.
	This module will replace DevElfLinker, once the final version of BlackBox 1.6 will be released.
*)

	IMPORT
		Kernel, Files, Dialog,
		TextMappers,
		StdLog, DevCommanders;

	CONST
		NewRecFP = 4E27A847H;
		NewArrFP = 76068C78H;

		OFdir = "Code";
		SYSdir = "System";

		(* meta interface consts *)
		mConst = 1; mTyp = 2; mVar = 3; mProc = 4;
		mInternal = 1; mExported = 4;

		(* mod desc fields *)
		modOpts = 4; modRefcnt = 8; modTerm = 40; modNames = 84; modImports = 92; modExports = 96;

		(* .dynsym entries *)
		stbLocal = 0; stbGlobal = 1;
		sttNotype = 0; sttObject = 1; sttFunc = 2; sttSection = 3;
		shnUnd = 0; shnAbs = 0FFF1H;

		fixup = 0;
		noSymbol = MIN(INTEGER);
		noAddr = MIN(INTEGER);
		firstDllSymbolVal = 12;

		(* distinguished section header indexes. *)
		textIndexVal = 1;	(* index of the .text section header in the section header table *)
		rodataIndexVal = 3;	(* index of the .rodata section header in the section header table *)
		dynsymIndexVal = 5;	(* index of the .dynsym section header in the section header table *)
		dynstrIndexVal = 6;	(* index of the .dynstr section header in the section header table *)

		(* fixed elements dimensions *)
		elfHeaderSizeVal = 52;	(* size of the ELF file header *)
		shEntrySizeVal = 40;	(* size of an entry in the section header table *)
		dynsymEntrySizeVal = 16; (* size of a symbol table entry *)
		dynamicEntrySizeVal = 8; (* size of an entry in the dynamic section *)
		gotEntrySizeVal = 4; (* size of an entry in the got section *)
		relEntrySizeVal = 8; (* size of an entry in a relocation section *)
		phEntrySizeVal = 32; (* size of an entry in the program header *)

		shNumVal = 12; (* number of entries in the section header table. See WriteSectionHeaderTable *)
		shStrndxVal = shNumVal - 1; (* index of the string table for section names. See WriteSectionHeaderTable *)
		phNumVal = 3; (* number of entries in the program header table *)

		(* sections alignments (in bytes) *)
		textAlign = 4H;
		dynsymAlign = 4H;
		dynstrAlign = 1H;
		hashAlign = 4H;
		gotAlign = 4H;
		dynamicAlign = 4H;
		shstrtabAlign = 1H;
		bssAlign = 4H;
		rodataAlign = 8H;
		relAlign = 4H;

		pageSize = 1000H; (* I386 page size *)

		r38632 = 1; r386pc32 = 2; r386Relative = 8; (* ELF relocation types *)

	TYPE
		Name = ARRAY 40 OF SHORTCHAR;

		Export = POINTER TO RECORD
			next: Export;
			name: Name;
			adr: INTEGER
		END;

		Module = POINTER TO RECORD
			next: Module;
			name: Name;
			fileName: Files.Name;
			file: Files.File;
			hs, ms, ds, cs, vs, ni, ma, ca, va: INTEGER;
			dll, intf: BOOLEAN;
			exp: Export;
			imp: POINTER TO ARRAY OF Module;
			data: POINTER TO ARRAY OF BYTE
		END;

		Strtab = RECORD
			tab: ARRAY 4096 OF SHORTCHAR;
			cur: INTEGER
		END;

		Relocation = RECORD
			offset, type: INTEGER
		END;

		RelTab = RECORD
			tab: ARRAY 65536 OF Relocation;
			cur: INTEGER
		END;

		Section = RECORD
			fileOffset,
			memOffset,
			size: INTEGER
		END;

	VAR
		W: TextMappers.Formatter;
		Out: Files.File;
		R: Files.Reader;
		Ro: Files.Writer;
		error, isDll, isStatic: BOOLEAN;
		modList, kernel, main, last, impg, impd: Module;
		numMod, lastTerm: INTEGER;
		firstExp, lastExp: Export;
		CodeSize, DataSize, ConSize: INTEGER;
		maxCode, numExp: INTEGER;
		newRec, newArr: Name;
		code: POINTER TO ARRAY OF BYTE;

		(* fixup positions *)
		entryPos,
		expPos,
		shstrtabPos,
		finiPos: INTEGER;

		(* sections *)
		text, reltext, relrodata, rodata, dynstr, shstrtab, hash, got, dynsym, dynamic, bss: Section;

		(* distinguished file and memory offsets *)
		shOffsetVal,	(* section header table file offset *)
		phOffsetVal,	(* program header table file offset *)
		finiMemOffsetVal: INTEGER;	(* memory offset (aka virtual address) of the finalization code (CLOSE sections) *)

		dynsymInfoVal,	(* value of the info field for the .dynsym section *)
		sonameStrIndexVal: INTEGER;	(* string table index of the name of hte library *)

		(* segment dimensions *)
		textSegmentSizeVal,
		dataSegmentSizeVal,
		dynamicSegmentSizeVal: INTEGER;

		headerstrtab, dynstrtab: Strtab;
		hashtab: ARRAY 256 OF Name;

		neededIdx: ARRAY 256 OF INTEGER;

		relTextTab, relRodataTab: RelTab;

		soName: Name;

		doWrite: BOOLEAN;

	PROCEDURE (VAR t: Strtab) AddName (IN s: ARRAY OF SHORTCHAR; OUT idx: INTEGER), NEW;
		VAR i: INTEGER;
	BEGIN
		ASSERT((t.cur + LEN(s$)) <= LEN(t.tab), 20); (* table buffer not large enough: TODO enlarge? *)
		idx := t.cur;
		i := 0;
		WHILE s[i] # 0X DO
			t.tab[t.cur] := s[i];
			INC(i); INC(t.cur)
		END;
		t.tab[t.cur] := s[i]; (* copy the 0X *)
		INC(t.cur)
	END AddName;

	PROCEDURE (VAR t: RelTab) Add (offset, type: INTEGER), NEW;
	BEGIN
		ASSERT(t.cur < LEN(t.tab), 20); (* table buffer not large enough: TODO enlarge? *)
		t.tab[t.cur].offset := offset;		
		t.tab[t.cur].type := type;
		INC(t.cur)
	END Add;

	PROCEDURE AddNeededIdx (idx: INTEGER);
		VAR i, len: INTEGER;
	BEGIN
		ASSERT(idx > 0, 20);	(* index must be positive *)
		len := LEN(neededIdx);
		i := 0;
		WHILE (i # len) & (neededIdx[i] # 0) DO INC(i) END;
		IF i # len THEN
			neededIdx[i] := idx
		ELSE
			HALT(21)	(* no more space for indexes *)
		END
	END AddNeededIdx;

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
		IF doWrite THEN
		Ro.WriteByte(SHORT(ORD(ch)))
		END
	END WriteCh;

	PROCEDURE Write2 (x: INTEGER);
	BEGIN
		IF doWrite THEN
			Ro.WriteByte(SHORT(SHORT(x MOD 256))); x := x DIV 256;
			Ro.WriteByte(SHORT(SHORT(x MOD 256)))
		END
	END Write2;

	PROCEDURE Write4 (x: INTEGER);
	BEGIN
		IF doWrite THEN
			Ro.WriteByte(SHORT(SHORT(x MOD 256))); x := x DIV 256;
			Ro.WriteByte(SHORT(SHORT(x MOD 256))); x := x DIV 256;
			Ro.WriteByte(SHORT(SHORT(x MOD 256))); x := x DIV 256;
			Ro.WriteByte(SHORT(SHORT(x MOD 256)))
		END
	END Write4;

	PROCEDURE WriteBytes (IN x: ARRAY OF BYTE; beg, len: INTEGER);
	BEGIN
		IF doWrite THEN
			Ro.WriteBytes(x, beg, len)
		END
	END WriteBytes;

	PROCEDURE Align (alignment: INTEGER);
	BEGIN
		WHILE Ro.Pos() MOD alignment # 0 DO WriteCh(0X) END
	END Align;
	
	PROCEDURE Aligned (pos, alignment: INTEGER): INTEGER;
	BEGIN
		RETURN (pos + (alignment - 1)) DIV alignment * alignment
	END Aligned;
	
	PROCEDURE Put (mod: Module; a, x: INTEGER);
	BEGIN
		ASSERT((mod.data # NIL) & ((a >= 0) & (a <= LEN(mod.data))), 20);
		mod.data[a] := SHORT(SHORT(x)); INC(a); x := x DIV 256;
		mod.data[a] := SHORT(SHORT(x)); INC(a); x := x DIV 256;
		mod.data[a] := SHORT(SHORT(x)); INC(a); x := x DIV 256;
		mod.data[a] := SHORT(SHORT(x))
	END Put;

	PROCEDURE Get (mod: Module; a: INTEGER; VAR x: INTEGER);
	BEGIN
		ASSERT((mod.data # NIL) & ((a >= 0) & (a + 3 <= LEN(mod.data))), 20);
		x := ((mod.data[a + 3] * 256 +
			(mod.data[a + 2] MOD 256)) * 256 +
			(mod.data[a + 1] MOD 256)) * 256 +
			(mod.data[a] MOD 256)
	END Get;

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
		SkipLink; SkipLink; SkipLink; SkipLink; SkipLink; SkipLink;
		i := 0;
		WHILE i < mod.ni DO
			imp := mod.imp[i];
			IF imp # NIL THEN
				RNum(x);
				WHILE x # 0 DO
					ReadName(name); RNum(y);
					IF x = mVar THEN
						SkipLink;
						IF imp.dll THEN
							exp := imp.exp;
							WHILE (exp # NIL) & (exp.name # name) DO exp := exp.next END;
							IF exp = NIL THEN
								NEW(exp); exp.name := name$;
								exp.next := imp.exp; imp.exp := exp
							 END
						END
					ELSIF x = mTyp THEN RNum(y);
						IF imp.dll THEN
							RNum(y);
							IF y # 0 THEN
								W.WriteString("type descriptor (");
								W.WriteString(imp.name$); W.WriteChar(".");
								W.WriteSString(name);
								W.WriteString(") imported from DLL in ");
								W.WriteString(mod.name$);
								W.WriteLn; StdLog.text.Append(StdLog.buf); error := TRUE;
								RETURN
							END
						ELSE SkipLink
						END
					ELSIF x = mProc THEN
						IF imp.dll THEN
							SkipLink;
							exp := imp.exp;
							WHILE (exp # NIL) & (exp.name # name) DO exp := exp.next END;
							IF exp = NIL THEN
								NEW(exp); exp.name := name$;
								exp.next := imp.exp; imp.exp := exp
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
		VAR mod, im, t: Module; x, i: INTEGER; impdll: BOOLEAN; name: Name;
	BEGIN
		ASSERT(isDll, 126);
		mod := modList; modList := NIL; numMod := 0;
		WHILE mod # NIL DO	(* reverse mod list & count modules *)
			IF ~mod.dll THEN INC(numMod) END;
			t := mod; mod := t.next; t.next := modList; modList := t
		END;
		IF isStatic THEN
			CodeSize :=
				6 + 5 * numMod + 2	(* _init() *)
				+ 1 + 5 * numMod + 2	(* _fini() *)
		ELSE
			CodeSize :=
				6 + 5 + 2	(* _init() *)
				+ 1 + 5 + 2	(* _fini() *)
		END;
		DataSize := 0; ConSize := 0;
		maxCode := 0; numExp := 0;
		mod := modList;
		WHILE mod # NIL DO
			IF ~mod.dll THEN
				mod.file := ThisFile(mod.fileName);
				IF mod.file # NIL THEN
					R := mod.file.NewReader(R); R.SetPos(0);
					Read4(x);
					IF x = 6F4F4346H THEN
						Read4(x);
						Read4(mod.hs); Read4(mod.ms); Read4(mod.ds); Read4(mod.cs);
						Read4(mod.vs); RNum(mod.ni); ReadName(mod.name); impdll := FALSE;
						IF mod.ni > 0 THEN
							NEW(mod.imp, mod.ni);
							x := 0;
							WHILE x < mod.ni DO
								ReadName(name);
								IF name = "$$" THEN
									IF (mod # kernel) & (kernel # NIL) THEN
										mod.imp[x] := kernel
									ELSE
										W.WriteSString("no kernel"); W.WriteLn;
										StdLog.text.Append(StdLog.buf); error := TRUE
									END
								ELSIF name[0] = "$" THEN
									i := 1;
									WHILE name[i] # 0X DO name[i-1] := name[i]; INC(i) END;
									name[i-1] := 0X; impdll := TRUE; im := modList;
									WHILE (im # mod) & (im.name # name) DO im := im.next END;
									IF (im = NIL) OR ~im.dll THEN
										NEW(im); im.next := modList; modList := im;
										im.dll := TRUE;
										im.name := name$;
										dynstrtab.AddName(name, i);
										AddNeededIdx(i)
									END;
									mod.imp[x] := im
								ELSE
									im := modList;
									WHILE (im # mod) & (im.name # name) DO im := im.next END;
									IF im # mod THEN
										mod.imp[x] := im
									ELSE
										W.WriteSString(name);
										W.WriteString(" not present (imported in ");
										W.WriteString(mod.name$); W.WriteChar(")");
										W.WriteLn; StdLog.text.Append(StdLog.buf); error := TRUE
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
						W.WriteString(mod.name$); W.WriteString(": wrong file type"); 
						W.WriteLn; StdLog.text.Append(StdLog.buf); error := TRUE
					END;
					mod.file.Close; mod.file := NIL
				ELSE
					W.WriteString(mod.name$); W.WriteString(" not found"); 
					W.WriteLn; StdLog.text.Append(StdLog.buf); error := TRUE
				END;
				last := mod
			END;
			mod := mod.next
		END;
		IF ~isStatic & (main = NIL) THEN
			W.WriteSString("no main module specified"); W.WriteLn;
			StdLog.text.Append(StdLog.buf); error := TRUE
		END;
		IF DataSize = 0 THEN DataSize := 1 END
	END ReadHeaders;

	PROCEDURE WriteElfHeader;
	BEGIN
		ASSERT(Ro.Pos() = 0, 100);
		dynstrtab.AddName(soName$, sonameStrIndexVal);
		Write4(464C457FH); Write4(00010101H); Write4(0); Write4(0); (* Magic *)
		Write2(3); (* ET_DYN e_type Object file type *)
		Write2(3); (* EM_386 e_machine Architecture *)
		Write4(1); (* EV_CURRENT e_version Object file version *)
		Write4(text.memOffset); (* e_entry Entry point virtual address *)
		entryPos := Ro.Pos();
		Write4(fixup); (* e_phoff Program header table file offset *)
		Write4(fixup); (* e_shoff: Section header table file offset *)
		Write4(0); (* e_flags Processor-specific flags *)
		Write2(elfHeaderSizeVal); (* e_ehsize ELF header size in bytes *)
		Write2(phEntrySizeVal); (* e_phentsize Program header table entry size *)
		Write2(phNumVal); (* e_phnum Program header table entry count *)
		Write2(shEntrySizeVal); (* e_shentsize Section header table entry size *)
		Write2(shNumVal); (* e_shnum Section header table entry count *)
		Write2(shStrndxVal); (* e_shstrndx Section header string table index *)
		ASSERT(Ro.Pos() = elfHeaderSizeVal, 101)
	END WriteElfHeader;

	PROCEDURE FixupElfHeader;
	BEGIN
		Ro.SetPos(entryPos);
		Write4(phOffsetVal);
		Write4(shOffsetVal)
	END FixupElfHeader;

	PROCEDURE WriteNullSectionHeader;
	BEGIN
		Write4(0); (* sh_name Section name (string tbl index) *)
		Write4(0); (* SHT_NULL sh_type Section type *)
		Write4(0); (* sh_flags Section flags *)
		Write4(0); (* ELF header + program header table; sh_addr Section virtual addr at execution *)
		Write4(0); (* sh_offset Section file offset *)
		Write4(0); (* sh_size Section size in bytes *)
		Write4(0); (* SHN_UNDEF sh_link Link to another section *)
		Write4(0); (* sh_info Additional section information *)
		Write4(0); (* sh_addralign Section alignment *)
		Write4(0) (* sh_entsize Entry size if section holds table *)
	END WriteNullSectionHeader;

	PROCEDURE WriteTextSectionHeader;
		VAR i: INTEGER;
	BEGIN
		headerstrtab.AddName(".text", i);
		Write4(i); (* sh_name Section name (string tbl index) *)
		Write4(1); (* SHT_PROGBITS sh_type Section type *)
		Write4(2H + 4H); (* SHF_ALLOC + SHF_EXECINSTR sh_flags Section flags *)
		Write4(text.memOffset); (* sh_addr Section virtual addr at execution *)
		Write4(text.fileOffset); (* sh_offset Section file offset *)
		Write4(text.size); (* sh_size Section size in bytes *)
		Write4(0); (* SHN_UNDEF sh_link Link to another section *)
		Write4(0); (* sh_info Additional section information *)
		Write4(textAlign); (* sh_addralign Section alignment *)
		Write4(0) (* sh_entsize Entry size if section holds table *)
	END WriteTextSectionHeader;

	PROCEDURE WriteRelTextSectionHeader;
		VAR i: INTEGER;
	BEGIN
		headerstrtab.AddName(".rel.text", i);
		Write4(i); (* sh_name Section name (string tbl index) *)
		Write4(9); (* SHT_REL sh_type Section type *)
		Write4(2H); (* SHF_ALLOC sh_flags Section flags *)
		Write4(reltext.memOffset); (* sh_addr Section virtual addr at execution *)
		Write4(reltext.fileOffset); (* sh_offset Section file offset *)
		Write4(reltext.size); (* sh_size Section size in bytes *)
		Write4(dynsymIndexVal); (* sh_link Link to another section -> index of the associated symbol table *)
		Write4(textIndexVal); (* sh_info Additional section information -> index of the relocated section *)
		Write4(relAlign); (* sh_addralign Section alignment *)
		Write4(relEntrySizeVal) (* sh_entsize Entry size if section holds table *)
	END WriteRelTextSectionHeader;

	PROCEDURE WriteRelRodataSectionHeader;
		VAR i: INTEGER;
	BEGIN
		headerstrtab.AddName(".rel.rodata", i);
		Write4(i); (* sh_name Section name (string tbl index) *)
		Write4(9); (* SHT_REL sh_type Section type *)
		Write4(2H); (* SHF_ALLOC sh_flags Section flags *)
		Write4(relrodata.memOffset); (* sh_addr Section virtual addr at execution *)
		Write4(relrodata.fileOffset); (* sh_offset Section file offset *)
		Write4(relrodata.size); (* sh_size Section size in bytes *)
		Write4(dynsymIndexVal); (* sh_link Link to another section -> index of the associated symbol table *)
		Write4(rodataIndexVal); (* sh_info Additional section information -> index of the relocated section *)
		Write4(relAlign); (* sh_addralign Section alignment *)
		Write4(relEntrySizeVal) (* sh_entsize Entry size if section holds table *)
	END WriteRelRodataSectionHeader;

	PROCEDURE WriteRodataSectionHeader;
		VAR i: INTEGER;
	BEGIN
		headerstrtab.AddName(".rodata", i);
		Write4(i); (* sh_name Section name (string tbl index) *)
		Write4(1); (* SHT_PROGBITS sh_type Section type *)
		Write4(2H); (* SHF_ALLOC sh_flags Section flags *)
		Write4(rodata.memOffset); (* sh_addr Section virtual addr at execution *)
		Write4(rodata.fileOffset); (* sh_offset Section file offset *)
		Write4(rodata.size); (* sh_size Section size in bytes *)
		Write4(0); (* sh_link Link to another section *)
		Write4(0); (* sh_info Additional section information *)
		Write4(rodataAlign); (* sh_addralign Section alignment *)
		Write4(0) (* sh_entsize Entry size if section holds table *)
	END WriteRodataSectionHeader;

	PROCEDURE WriteDynsymSectionHeader;
		VAR i: INTEGER;
	BEGIN
		headerstrtab.AddName(".dynsym", i);
		Write4(i); (* sh_name Section name (string tbl index) *)
		Write4(11); (* SHT_DYNSYM sh_type Section type *)
		Write4(2H); (* SHF_ALLOC sh_flags Section flags *)
		Write4(dynsym.memOffset); (* sh_addr Section virtual addr at execution *)
		Write4(dynsym.fileOffset); (* sh_offset Section file offset *)
		Write4(dynsym.size); (* sh_size Section size in bytes *)
		Write4(dynstrIndexVal); (* sh_link Link to another section -> index of the associated string table *)
		expPos := Ro.Pos();
		Write4(fixup); (* sh_info Additional section information -> see docu 4-17 *)
		Write4(dynsymAlign); (* sh_addralign Section alignment *)
		Write4(dynsymEntrySizeVal) (* sh_entsize Entry size if section holds table *)
	END WriteDynsymSectionHeader;

	PROCEDURE FixupDynsymSectionHeader;
	BEGIN
		Ro.SetPos(expPos);
		Write4(dynsymInfoVal)
	END FixupDynsymSectionHeader;
	
	PROCEDURE WriteDynstrSectionHeader;
		VAR i: INTEGER;
	BEGIN
		headerstrtab.AddName(".dynstr", i);
		Write4(i); (* sh_name Section name (string tbl index) *)
		Write4(3); (* SHT_STRTAB sh_type Section type *)
		Write4(2H); (* SHF_ALLOC sh_flags Section flags *)
		Write4(dynstr.memOffset); (* sh_addr Section virtual addr at execution *)
		Write4(dynstr.fileOffset); (* sh_offset Section file offset *)
		Write4(dynstr.size); (* sh_size Section size in bytes *)
		Write4(0); (* SHN_UNDEF sh_link Link to another section *)
		Write4(0); (* sh_info Additional section information *)
		Write4(dynstrAlign); (* sh_addralign Section alignment *)
		Write4(0) (* sh_entsize Entry size if section holds table *)
	END WriteDynstrSectionHeader;
	
	PROCEDURE WriteHashSectionHeader;
		VAR i: INTEGER;
	BEGIN
		headerstrtab.AddName(".hash", i);
		Write4(i); (* sh_name Section name (string tbl index) *)
		Write4(5); (* SHT_HASH sh_type Section type *)
		Write4(2H); (* SHF_ALLOC sh_flags Section flags *)
		Write4(hash.memOffset); (* sh_addr Section virtual addr at execution *)
		Write4(hash.fileOffset); (* sh_offset Section file offset *)
		Write4(hash.size); (* sh_size Section size in bytes *)
		Write4(dynsymIndexVal); (* sh_link Link to another section *)
		Write4(0); (* sh_info Additional section information *)
		Write4(hashAlign); (* sh_addralign Section alignment *)
		Write4(4H) (* sh_entsize Entry size if section holds table *)
	END WriteHashSectionHeader;

	PROCEDURE WriteGotSectionHeader;
		VAR i: INTEGER;
	BEGIN
		headerstrtab.AddName(".got", i);
		Write4(i); (* sh_name Section name (string tbl index) *)
		Write4(1); (* SHT_PROGBITS sh_type Section type *)
		Write4(2H + 1H); (* SHF_ALLOC + SHF_WRITE sh_flags Section flags *)
		Write4(got.memOffset); (* sh_addr Section virtual addr at execution *)
		Write4(got.fileOffset); (* sh_offset Section file offset *)
		Write4(got.size); (* sh_size Section size in bytes *)
		Write4(0); (* SHN_UNDEF sh_link Link to another section *)
		Write4(0); (* sh_info Additional section information *)
		Write4(gotAlign); (* sh_addralign Section alignment *)
		Write4(gotEntrySizeVal) (* sh_entsize Entry size if section holds table *)
	END WriteGotSectionHeader;
	
	PROCEDURE WriteBssSectionHeader;
		VAR i: INTEGER;
	BEGIN
		headerstrtab.AddName(".bss", i);
		Write4(i); (* sh_name Section name (string tbl index) *)
		Write4(8); (* SHT_NOBITS sh_type Section type *)
		Write4(2H + 1H); (* SHF_ALLOC + SHF_WRITE sh_flags Section flags *)
		Write4(bss.memOffset); (* sh_addr Section virtual addr at execution *)
		Write4(bss.fileOffset); (* sh_offset Section file offset *)
		Write4(bss.size); (* sh_size Section size in bytes *)
		Write4(0); (* SHN_UNDEF sh_link Link to another section *)
		Write4(0); (* sh_info Additional section information *)
		Write4(bssAlign); (* sh_addralign Section alignment *)
		Write4(0) (* sh_entsize Entry size if section holds table *)
	END WriteBssSectionHeader;
	
	PROCEDURE WriteDynamicSectionHeader;
		VAR i: INTEGER;
	BEGIN
		headerstrtab.AddName(".dynamic", i);
		Write4(i); (* sh_name Section name (string tbl index) *)
		Write4(6); (* SHT_DYNAMIC sh_type Section type *)
		Write4(2H); (* SHF_ALLOC sh_flags Section flags *)
		Write4(dynamic.memOffset); (* sh_addr Section virtual addr at execution *)
		Write4(dynamic.fileOffset); (* sh_offset Section file offset *)
		Write4(dynamic.size); (* sh_size Section size in bytes *)
		Write4(dynstrIndexVal); (* sh_link Link to another section -> index of the associated symbol table *)
		Write4(0); (* sh_info Additional section information *)
		Write4(dynamicAlign); (* sh_addralign Section alignment *)
		Write4(dynamicEntrySizeVal) (* sh_entsize Entry size if section holds table *)
	END WriteDynamicSectionHeader;
	
	PROCEDURE WriteShstrtabSectionHeader;
		VAR i: INTEGER;
	BEGIN
		headerstrtab.AddName(".shstrtab", i);
		Write4(i); (* sh_name Section name (string tbl index) *)
		Write4(3); (* SHT_STRTAB sh_type Section type *)
		Write4(0); (* sh_flags Section flags *)
		Write4(0); (* sh_addr Section virtual addr at execution *)
		Write4(shstrtab.fileOffset); (* sh_offset Section file offset *)
		shstrtabPos := Ro.Pos();
		Write4(fixup); (* sh_size Section size in bytes *)
		Write4(0); (* SHN_UNDEF sh_link Link to another section *)
		Write4(0); (* sh_info Additional section information *)
		Write4(shstrtabAlign); (* sh_addralign Section alignment *)
		Write4(0) (* sh_entsize Entry size if section holds table *)
	END WriteShstrtabSectionHeader;
	
	PROCEDURE FixupShstrtabSectionHeader;
	BEGIN
		Ro.SetPos(shstrtabPos);
		Write4(shstrtab.size)
	END FixupShstrtabSectionHeader;

	PROCEDURE WriteRelSectionHeaders;
	BEGIN
		WriteRelTextSectionHeader;
		WriteRelRodataSectionHeader
	END WriteRelSectionHeaders;
	
	PROCEDURE WriteSectionHeaderTable;
	BEGIN
		shOffsetVal := Ro.Pos();
		WriteNullSectionHeader;
		WriteTextSectionHeader;
		WriteRodataSectionHeader;
		WriteRelSectionHeaders;
		WriteDynsymSectionHeader;
		WriteDynstrSectionHeader;
		WriteHashSectionHeader;
		WriteGotSectionHeader;
		WriteDynamicSectionHeader;
		WriteBssSectionHeader;
		WriteShstrtabSectionHeader	(* see shStrndxVal *)
		(* see shNumVal *)
	END WriteSectionHeaderTable;

	PROCEDURE FixupSectionHeaderTable;
	BEGIN
		FixupDynsymSectionHeader;
		FixupShstrtabSectionHeader
	END FixupSectionHeaderTable;

	PROCEDURE WriteTextSegment;
	BEGIN
		Write4(1); (* PT_LOAD *)
		Write4(0); (* offset *)
		Write4(0); (* vaddr *)
		Write4(0); (* paddr *)
		Write4(textSegmentSizeVal); (* file size *)
		Write4(textSegmentSizeVal); (* mem size *)
		Write4(4H + 1H + 2H); (* flags: R+E+W *)
		Write4(pageSize) (* I386 page size *)
	END WriteTextSegment;
	
	PROCEDURE WriteDataSegment;
	BEGIN
		Write4(1); (* PT_LOAD *)
		Write4(got.fileOffset); (* offset text segment size *)
		Write4(got.memOffset); (* vaddr: offset + alignment * nof pages of text segment *)
		Write4(got.memOffset); (* paddr: offset + alignment * nof pages of text segment *)
		Write4(dataSegmentSizeVal); (* file size *)
		Write4(dataSegmentSizeVal + bss.size); (* mem size -> dataSegmentSizeVal + NOBITS sections *)
		Write4(4H + 2H); (* flags: R+W *)
		Write4(pageSize) (* I386 page size *)
	END WriteDataSegment;
	
	PROCEDURE WriteDynamicSegment;
	BEGIN
		Write4(2); (* PT_DYNAMIC *)
		Write4(dynamic.fileOffset); (* offset text segment size *)
		Write4(dynamic.memOffset); (* vaddr: offset of .dynamic section *)
		Write4(dynamic.memOffset); (* paddr: vaddr + alignment * nof pages of text segment *)
		Write4(dynamicSegmentSizeVal); (* file size *)
		Write4(dynamicSegmentSizeVal); (* mem size *)
		Write4(4H + 2H); (* flags: R+W *)
		Write4(dynamicAlign) (* dynamic section alignement*)
	END WriteDynamicSegment;
	
	PROCEDURE WriteProgramHeaderTable;
	BEGIN
		phOffsetVal := Ro.Pos();
		WriteTextSegment; (* .text .rel.text .rodata .dynsym .dynstr .hash *)
		WriteDataSegment; (* .got .dynamic .bss *)
		WriteDynamicSegment (* .dynamic *)
	END WriteProgramHeaderTable;
	
	PROCEDURE SearchObj (mod: Module; VAR name: ARRAY OF SHORTCHAR; m, fp, opt: INTEGER; VAR adr: INTEGER);
		VAR dir, len, ntab, f, id, l, r, p, n, i, j: INTEGER; nch, och: SHORTCHAR;
	BEGIN
		Get(mod, mod.ms + modExports, dir); DEC(dir, rodata.memOffset + mod.ma); Get(mod, dir, len); INC(dir, 4);
		Get(mod, mod.ms + modNames, ntab); DEC(ntab, rodata.memOffset + mod.ma);
		IF name # "" THEN
			l := 0; r := len;
			WHILE l < r DO	(* binary search *)
				n := (l + r) DIV 2; p := dir + n * 16;
				Get(mod, p + 8, id);
				i := 0; j := ntab + id DIV 256; nch := name[0]; och := SHORT(CHR(mod.data[j]));
				WHILE (nch = och) & (nch # 0X) DO INC(i); INC(j); nch := name[i]; och := SHORT(CHR(mod.data[j])) END;
				IF och = nch THEN
					IF id MOD 16 = m THEN
						Get(mod, p, f);
						IF m = mTyp THEN
							IF ODD(opt) THEN Get(mod, p + 4, f) END;
							IF (opt > 1) & (id DIV 16 MOD 16 # mExported) THEN
								W.WriteString(mod.name$); W.WriteChar("."); W.WriteSString(name);
								W.WriteString(" imported from "); W.WriteString(impg.name$);
								W.WriteString(" has wrong visibility"); W.WriteLn; error := TRUE
							END;
							Get(mod, p + 12, adr)
						ELSIF m = mVar THEN
							Get(mod, p + 4, adr); INC(adr, bss.memOffset + mod.va)
						ELSIF m = mProc THEN
							Get(mod, p + 4, adr); INC(adr, text.memOffset + mod.ca)
						END;
						IF f # fp THEN
							W.WriteString(mod.name$); W.WriteChar("."); W.WriteSString(name);
							W.WriteString(" imported from "); W.WriteString(impg.name$);
							W.WriteString(" has wrong fprint"); W.WriteLn; error := TRUE
						END
					ELSE
						W.WriteString(mod.name$); W.WriteChar("."); W.WriteSString(name);
						W.WriteString(" imported from "); W.WriteString(impg.name$);
						W.WriteString(" has wrong class"); W.WriteLn; error := TRUE
					END;
					RETURN
				END;
				IF och < nch THEN l := n + 1 ELSE r := n END
			END;
			W.WriteString(mod.name$); W.WriteChar("."); W.WriteSString(name);
			W.WriteString(" not found (imported from "); W.WriteString(impg.name$);
			W.WriteChar(")"); W.WriteLn; error := TRUE
		ELSE (* anonymous type *)
			WHILE len > 0 DO
				Get(mod, dir + 4, f); Get(mod, dir + 8, id);
				IF (f = fp) & (id MOD 16 = mTyp) & (id DIV 256 = 0) THEN
					Get(mod, dir + 12, adr); RETURN
				END;
				DEC(len); INC(dir, 16)
			END;
			W.WriteString("anonymous type in "); W.WriteString(mod.name$);
			W.WriteString(" not found"); W.WriteLn; error := TRUE
		END
	END SearchObj;
	
	PROCEDURE CollectExports (mod: Module);
		VAR dir, len, ntab, id, i, j, n: INTEGER; e, exp: Export;
	BEGIN
		ASSERT(mod.intf & ~mod.dll, 20);
		Get(mod, mod.ms + modExports, dir);
		DEC(dir, rodata.memOffset + mod.ma); Get(mod, dir, len); INC(dir, 4);
		Get(mod, mod.ms + modNames, ntab); DEC(ntab, rodata.memOffset + mod.ma); n := 0;
		WHILE n < len DO
			Get(mod, dir + 8, id);
			IF (id DIV 16 MOD 16 # mInternal) & (id MOD 16 = mProc) THEN	(* exported procedure *)
				NEW(exp);
				i := 0; j := ntab + id DIV 256;
				WHILE mod.data[j] # 0 DO exp.name[i] := SHORT(CHR(mod.data[j])); INC(i); INC(j) END;
				exp.name[i] := 0X;
				Get(mod, dir + 4, exp.adr);
				IF id MOD 16 = mProc THEN
					INC(exp.adr, text.memOffset + mod.ca)
				ELSE
					HALT(126);
					ASSERT(id MOD 16 = mVar); INC(exp.adr, bss.memOffset + mod.va)
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
				INC(numExp)
			END;
			INC(n); INC(dir, 16)
		END
	END CollectExports;

	PROCEDURE Relocate0 (link, adr, sym: INTEGER);
		CONST
			absolute = 100; relative = 101; copy = 102; table = 103; tableend = 104; (* BB fixup types *)
			noElfType = MIN(INTEGER);
		VAR
			offset, linkadr, bbType, elfType, n, x: INTEGER; relText: BOOLEAN;
	BEGIN
		WHILE link # 0 DO
			RNum(offset);
			WHILE link # 0 DO
				IF link > 0 THEN
					n := (code[link] MOD 256) + (code[link+1] MOD 256) * 256 + code[link+2] * 65536;
					bbType := code[link+3];
					linkadr := text.memOffset + impg.ca + link
				ELSE
					n := (impg.data[-link] MOD 256) + (impg.data[-link+1] MOD 256) * 256 + impg.data[-link+2] * 65536;
					bbType := impg.data[-link+3];
					linkadr := rodata.memOffset + impg.ma - link
				END;
				elfType := noElfType;
				IF bbType = absolute THEN
					IF sym = noSymbol THEN
						x := adr + offset;
						elfType := r386Relative
					ELSE
						x := 0H;
						elfType := r38632 + sym * 256
					END
				ELSIF bbType = relative THEN
					IF sym = noSymbol THEN
						x := adr + offset - linkadr - 4
					ELSE
						x := 0FFFFFFFCH;
						elfType := r386pc32 + sym * 256
					END
				ELSIF bbType = copy THEN
					Get(impd, adr + offset - rodata.memOffset - impd.ma, x);
					IF x # 0 THEN elfType := r386Relative END
				ELSIF bbType = table THEN
					x := adr + n; n := link + 4;
					elfType := r386Relative
				ELSIF bbType = tableend THEN
					x := adr + n; n := 0;
					elfType := r386Relative
				ELSE HALT(99)
				END;
				relText := link > 0; 
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
				IF elfType # noElfType THEN
					IF relText THEN
						relTextTab.Add(linkadr, elfType)
					ELSE
						relRodataTab.Add(linkadr, elfType)
					END
				END;
				link := n
			END;
			RNum(link)
		END
	END Relocate0;
	
	PROCEDURE Relocate (adr: INTEGER);
		VAR link: INTEGER;
	BEGIN
		RNum(link); Relocate0(link, adr, noSymbol)
	END Relocate;

	PROCEDURE RelocateSymbol (adr, sym: INTEGER);
		VAR link: INTEGER;
	BEGIN
		RNum(link); Relocate0(link, adr, sym)
	END RelocateSymbol;
	
	PROCEDURE SymbolIndex (IN name: Name): INTEGER;
		VAR n: INTEGER; exp: Export; m: Module;
	BEGIN
		n := 0; exp := NIL;
		m := modList;
		WHILE (m # NIL) & (exp = NIL) DO
			IF m.dll THEN
				exp := m.exp;
				WHILE (exp # NIL) & (exp.name$ # name$) DO
					INC(n);
					exp := exp.next
				END
			END;
			m := m.next
		END;
		ASSERT((exp # NIL) & (exp.name$ = name$), 60);
		RETURN firstDllSymbolVal + n
	END SymbolIndex;

	PROCEDURE WriteTextSection;
		VAR mod, m: Module; i, x, a, sym, fp, opt: INTEGER; exp: Export; name: Name;
	BEGIN
		ASSERT(isDll, 126);
		ASSERT(~doWrite OR (Ro.Pos() = text.fileOffset), 100);
		WriteCh(053X);	(* push ebx *)	(* _init() *)
		a := 1;
		WriteCh(0BBX); Write4(rodata.memOffset  + last.ma + last.ms);	(* mov bx, modlist *)
		relTextTab.Add(text.memOffset + a + 1, r386Relative);
		INC(a, 5);
		IF isStatic THEN
			m := modList;
			WHILE m # NIL DO
				IF ~m.dll THEN
					WriteCh(0E8X); INC(a, 5); Write4(m.ca - a)	(* call body *)
				END;
				m := m.next
			END
		ELSE
			WriteCh(0E8X); INC(a, 5); Write4(main.ca - a)	(* call main *)
		END;
		WriteCh(05BX); 	(* pop ebx *)
		WriteCh(0C3X);	(* ret *)
		INC(a, 2);
		finiMemOffsetVal := text.memOffset + a;
		WriteCh(053X);	(* push ebx *)	(* _fini() *)
		INC(a);
		finiPos := text.memOffset + a;
		IF isStatic THEN
			i := 0;
			WHILE i < numMod DO	(* nop for call terminator *)
				WriteCh(02DX); Write4(0);	(* sub EAX, 0 *)
				INC(i); INC(a, 5)
			END
		ELSE
			WriteCh(02DX); Write4(0);	(* sub EAX, 0 *)
			INC(a, 5)
		END;
		lastTerm := a;
		WriteCh(05BX); 	(* pop ebx *)
		WriteCh(0C3X);	(* ret *)	
		IF ~doWrite THEN NEW(code, maxCode) END;
		mod := modList;
		WHILE mod # NIL DO
			impg := mod;
			impd := mod;
			IF ~mod.dll THEN
				mod.file := ThisFile(mod.fileName);
				R := mod.file.NewReader(R);
				R.SetPos(mod.hs);
				IF ~doWrite THEN NEW(mod.data, mod.ms + mod.ds) END;
				R.ReadBytes(mod.data^, 0, mod.ms + mod.ds);
				R.ReadBytes(code^, 0, mod.cs);
				RNum(x);
				IF x # 0 THEN
					IF (mod # kernel) & (kernel # NIL) THEN
						SearchObj(kernel, newRec, mProc, NewRecFP, 0, a);
						IF error THEN RETURN END;
						Relocate0(x, a, noSymbol)
					ELSE
						W.WriteSString("no kernel"); W.WriteLn;
						StdLog.text.Append(StdLog.buf);
						error := TRUE;
						RETURN
					END
				END;
				RNum(x);
				IF x # 0 THEN
					IF (mod # kernel) & (kernel # NIL) THEN
						SearchObj(kernel, newArr, mProc, NewArrFP, 0, a);
						IF error THEN RETURN END;
						Relocate0(x, a, noSymbol)
					ELSE
						W.WriteSString("no kernel"); W.WriteLn;
						StdLog.text.Append(StdLog.buf); error := TRUE;
						RETURN
					END
				END;
				Relocate(rodata.memOffset + mod.ma); (* metalink *)
				Relocate(rodata.memOffset + mod.ma + mod.ms); (* desclink *)
				Relocate(text.memOffset + mod.ca); (* codelink *)
				Relocate(bss.memOffset + mod.va); (* datalink *)
				i := 0;
				WHILE i < mod.ni DO
					m := mod.imp[i]; impd := m; RNum(x);
					WHILE x # 0 DO
						ReadName(name); RNum(fp); opt := 0;
						IF x = mTyp THEN RNum(opt) END;
						sym := noSymbol;
						IF m.dll THEN
							IF (x = mProc) OR (x = mVar) THEN
								exp := m.exp;
								WHILE exp.name # name DO exp := exp.next END;
								a := noAddr;
								sym := SymbolIndex(name)
							END
						ELSE
							SearchObj(m, name, x, fp, opt, a);
							IF error THEN RETURN END
						END;
						IF x # mConst THEN
							RelocateSymbol(a, sym)
						END;
						RNum(x)
					END;
					IF ~m.dll THEN
						Get(mod, mod.ms + modImports, x); DEC(x, rodata.memOffset + mod.ma); INC(x, 4 * i);
						Put(mod, x, rodata.memOffset + m.ma + m.ms);	(* imp ref *)
						relRodataTab.Add(rodata.memOffset + mod.ma + x, r386Relative);
						Get(m, m.ms + modRefcnt, x); Put(m, m.ms + modRefcnt, x + 1)	(* inc ref count *)
					END;
					INC(i)
				END;
				WriteBytes(code^, 0, mod.cs);
				IF mod.intf THEN CollectExports(mod) END;
				mod.file.Close; mod.file := NIL
			END;
			mod := mod.next
		END;
		ASSERT(~doWrite OR (text.size = Ro.Pos() - text.fileOffset), 101)
	END WriteTextSection;

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
					WriteCh(0E8X); Write4(x - lastTerm + 5 * i - text.memOffset)	(* call term *)
				END
			END
		END
	END WriteTermCode;

	PROCEDURE FixupTextSection;
	BEGIN
		ASSERT(isDll, 126);
		Ro.SetPos(finiPos);
		IF isStatic THEN
			WriteTermCode(modList, 0)
		ELSE
			WriteTermCode(main, 0)
		END
	END FixupTextSection;

	PROCEDURE WriteRelSection (IN s: Section; IN t: RelTab);
		VAR i: INTEGER;
	BEGIN
		ASSERT(s.fileOffset = Ro.Pos(), 100);
		i := 0;
		WHILE i # t.cur DO
			Write4(t.tab[i].offset);
			Write4(t.tab[i].type);
			INC(i)
		END;
		ASSERT(s.size = Ro.Pos() - s.fileOffset, 101)
	END WriteRelSection;

	PROCEDURE WriteRelSections;
	BEGIN
		WriteRelSection(reltext, relTextTab);
		WriteRelSection(relrodata, relRodataTab)
	END WriteRelSections;
	
	PROCEDURE WriteRodataSection;
		VAR mod, lastMod: Module; x: INTEGER;
	BEGIN
		ASSERT(~doWrite OR (rodata.fileOffset = Ro.Pos()), 100);
		mod := modList; lastMod := NIL;
		WHILE mod # NIL DO
			IF ~mod.dll THEN
				IF lastMod # NIL THEN
					Put(mod, mod.ms, rodata.memOffset + lastMod.ma + lastMod.ms);	(* mod list *)
					relRodataTab.Add(rodata.memOffset + mod.ma + mod.ms, r386Relative)
				END;
				Get(mod, mod.ms + modOpts, x);
				IF isStatic THEN INC(x, 10000H) END;	(* set init bit (16) *)
				IF isDll THEN INC(x, 1000000H) END;	(* set dll bit (24) *)
				Put(mod, mod.ms + modOpts, x);
				WriteBytes(mod.data^, 0, mod.ms + mod.ds);
				lastMod := mod
			END;
			mod := mod.next
		END;		
		ASSERT(~doWrite OR (rodata.size = Ro.Pos() - rodata.fileOffset), 101)
	END WriteRodataSection;
		
	PROCEDURE WriteSymbolTableEntry (IN name: ARRAY OF SHORTCHAR; val, size: INTEGER; bind, type: BYTE; shndx: INTEGER);
		VAR i: INTEGER; info: SHORTCHAR;
	BEGIN
		IF name # "" THEN dynstrtab.AddName(name, i)
		ELSE i := 0
		END;
		Write4(i);
		Write4(val);
		Write4(size);
		info := SHORT(CHR(bind * 16 + type));
		WriteCh(info);
		WriteCh(0X); (* Symbol visibility *)
		Write2(shndx)
	END WriteSymbolTableEntry;
	
	PROCEDURE FixupSymbolTableEntry (val, size: INTEGER; bind, type: BYTE; shndx: INTEGER);
		VAR info: SHORTCHAR;
	BEGIN
		Ro.SetPos(Ro.Pos() + 4); (* skip name *)
		Write4(val);
		Write4(size);
		info := SHORT(CHR(bind * 16 + type));
		WriteCh(info);
		WriteCh(0X); (* Symbol visibility *)
		Write2(shndx)
	END FixupSymbolTableEntry;
	
	PROCEDURE WriteDynsymSection;
		VAR e: Export; m: Module; i: INTEGER;
	BEGIN
		ASSERT(Ro.Pos() = dynsym.fileOffset, 100);
		WriteSymbolTableEntry("", 0, 0, 0, 0, 0);
		WriteSymbolTableEntry("", text.memOffset, 0, stbLocal, sttSection, 1); (* .text section *)
		WriteSymbolTableEntry("", rodata.memOffset, 0, stbLocal, sttSection, 2); (* .rodata section *)
		WriteSymbolTableEntry("", reltext.memOffset, 0, stbLocal, sttSection, 3); (* .rel.text.section *)
		WriteSymbolTableEntry("", relrodata.memOffset, 0, stbLocal, sttSection, 4); (* .rel.rodata section *)
		WriteSymbolTableEntry("", dynsym.memOffset, 0, stbLocal, sttSection, 5); (* .dynsym section *)
		WriteSymbolTableEntry("", dynstr.memOffset, 0, stbLocal, sttSection, 6); (* .dynstr section *)
		WriteSymbolTableEntry("", hash.memOffset, 0, stbLocal, sttSection, 7); (* .hash section *)
		WriteSymbolTableEntry("", got.memOffset, 0, stbLocal, sttSection, 8); (* .got section *)
		WriteSymbolTableEntry("", dynamic.memOffset, 0, stbLocal, sttSection, 9); (* .dynamic section *)
		WriteSymbolTableEntry("", bss.memOffset, 0, stbLocal, sttSection, 10); (* .bss section *)
		dynsymInfoVal := 11;
		i := dynsymInfoVal;
		WriteSymbolTableEntry("_DYNAMIC", dynamic.memOffset, 0, stbGlobal, sttObject, shnAbs);
		hashtab[i] := "_DYNAMIC";
		INC(i);
		ASSERT(i = firstDllSymbolVal);
		m := modList;
		WHILE m # NIL DO
			IF m.dll THEN
				e := m.exp;
				WHILE e # NIL DO
					WriteSymbolTableEntry(e.name, 0, 0, stbGlobal, sttNotype, shnUnd);
					hashtab[i] := e.name$;
					INC(i);
					e := e.next
				END
			END;
			m := m.next
		END;
		e := firstExp;
		WHILE e # NIL DO
			WriteSymbolTableEntry(e.name, fixup, 0, stbGlobal, sttFunc, textIndexVal);
			hashtab[i] := e.name$; INC(i);
			e := e.next
		END;
		WriteSymbolTableEntry("_GLOBAL_OFFSET_TABLE_", got.memOffset, 0, stbGlobal, sttObject, shnAbs);
		hashtab[i] := "_GLOBAL_OFFSET_TABLE_";
		ASSERT(dynsym.size = Ro.Pos() - dynsym.fileOffset, 101)
	END WriteDynsymSection;
	
	PROCEDURE FixupDynsymSection;
		VAR e: Export; m: Module;
	BEGIN
		Ro.SetPos(dynsym.fileOffset + dynsymEntrySizeVal * firstDllSymbolVal);
		m := modList;
		WHILE m # NIL DO
			IF m.dll THEN
				e := m.exp;
				WHILE e # NIL DO
					Ro.SetPos(Ro.Pos() + dynsymEntrySizeVal);
					e := e.next
				END
			END;
			m := m.next
		END;
		Ro.SetPos(Ro.Pos() + 4);
		e := firstExp;
		WHILE e # NIL DO
			Write4(e.adr);
			Ro.SetPos(Ro.Pos() + 12);
			e := e.next
		END
	END FixupDynsymSection;

	PROCEDURE WriteStringTable (IN t: Strtab);
		VAR i: INTEGER;
	BEGIN
		i := 0;
		WHILE i # t.cur DO
			WriteCh(t.tab[i]);
			INC(i)
		END
	END WriteStringTable;

	PROCEDURE WriteDynstrSection;
	BEGIN
		ASSERT(Ro.Pos() = dynstr.fileOffset, 100);
		WriteStringTable(dynstrtab);
		ASSERT(dynstr.size = Ro.Pos() - dynstr.fileOffset, 101)
	END WriteDynstrSection;

	PROCEDURE Hash (name: ARRAY OF SHORTCHAR): INTEGER;
		VAR i, h, g: INTEGER;
	BEGIN
		h := 0; i := 0;
		WHILE name[i] # 0X DO
			h := ASH(h, 4) + ORD(name[i]);
			g := ORD(BITS(h) * BITS(0F0000000H));
			IF g # 0 THEN
				h := ORD(BITS(h) / BITS(SHORT((g MOD 100000000L) DIV 1000000H)))
			END;
			h := ORD(BITS(h) * (-BITS(g)));
			INC(i)
		END;
		RETURN h
	END Hash;

	PROCEDURE AddToChain (VAR c: ARRAY OF INTEGER; i, idx: INTEGER);
		VAR k: INTEGER;
	BEGIN
		IF c[i] # 0 THEN
			k := i;
			WHILE c[k] # 0 DO k := c[k] END;
			c[k] := idx
		ELSE
			c[i] := idx
		END
	END AddToChain;

	PROCEDURE WriteHashSection;
		VAR n, i, hi: INTEGER; b, c: POINTER TO ARRAY OF INTEGER;
	BEGIN
		ASSERT(hash.fileOffset = Ro.Pos(), 100);
		n := dynsym.size DIV dynsymEntrySizeVal; (* number of enties in the symbol table *)
		NEW(b, n);
		NEW(c, n);
		i := 0;
		WHILE i # n DO
			c[i] := 0; (* STN_UNDEF *)
			IF hashtab[i] # "" THEN
				hi := Hash(hashtab[i]) MOD n;
				IF b[hi] # 0 THEN (* another word has the same index *)
					AddToChain(c, i, b[hi])  (*c[i] := b[hi]*)
				END;
				b[hi] := i
			END;
			INC(i)
		END;
		Write4(n); (* nbucket *)
		Write4(n); (* nchain *)
		i := 0;
		WHILE i # n DO
			Write4(b[i]);
			INC(i)
		END;
		i := 0;
		WHILE i # n DO
			Write4(c[i]);
			INC(i)
		END;
		ASSERT(hash.size = Ro.Pos() - hash.fileOffset, 101)
	END WriteHashSection;
	
	PROCEDURE WriteGotSection;
	BEGIN
		ASSERT(got.fileOffset = Ro.Pos(), 100);
		Write4(dynamic.memOffset); (* addr of .dynamic section *)
		Write4(0); (* reserved for ? *)
		Write4(0); (* reserved for ? *)
		ASSERT(got.size = Ro.Pos() - got.fileOffset, 101)
	END WriteGotSection;
	
	PROCEDURE WriteDynamicSectionEntry (tag, val: INTEGER);
	BEGIN
		Write4(tag);
		Write4(val)
	END WriteDynamicSectionEntry;
	
	PROCEDURE WriteDynamicSection;
		CONST dtNull = 0; dtNeeded = 1; dtHash = 4; dtStrtab = 5; dtSymtab = 6;
			dtStrsz = 10; dtSyment = 11; dtInit = 12; dtFini = 13; dtSoname = 14; dtRel = 17; dtRelsz = 18; dtRelent = 19;
			dtTextrel = 22;
		VAR i: INTEGER;
	BEGIN
		ASSERT(dynamic.fileOffset = Ro.Pos(), 100);
		WriteDynamicSectionEntry(dtSoname, fixup);
		WriteDynamicSectionEntry(dtFini, fixup);
		WriteDynamicSectionEntry(dtInit, text.memOffset);
		WriteDynamicSectionEntry(dtHash, hash.memOffset);
		WriteDynamicSectionEntry(dtStrtab, dynstr.memOffset);
		WriteDynamicSectionEntry(dtSymtab, dynsym.memOffset);
		WriteDynamicSectionEntry(dtStrsz, dynstr.size);
		WriteDynamicSectionEntry(dtSyment, dynsymEntrySizeVal);
		WriteDynamicSectionEntry(dtRel, reltext.memOffset);
		WriteDynamicSectionEntry(dtRelsz, reltext.size + relrodata.size);
		WriteDynamicSectionEntry(dtRelent, relEntrySizeVal);
		i := 0;
		WHILE neededIdx[i] # 0 DO
			WriteDynamicSectionEntry(dtNeeded, neededIdx[i]);
			INC(i)
		END;
		WriteDynamicSectionEntry(dtTextrel, 0);
		WriteDynamicSectionEntry(dtNull, 0); (* DT_NULL: marks the end *)
		ASSERT(dynamic.size = Ro.Pos() - dynamic.fileOffset, 101)
	END WriteDynamicSection;
	
	PROCEDURE FixupDynamicSection;
		VAR i: INTEGER;
	BEGIN
		Ro.SetPos(dynamic.fileOffset + 4);
		Write4(sonameStrIndexVal);
		Ro.SetPos(Ro.Pos() + 4);
		Write4(finiMemOffsetVal)
	END FixupDynamicSection;
	
	PROCEDURE WriteBssSection;
	BEGIN
(*
		The .bss section does not take space in the file.
		This procedure serves consistency-check purposes.
*)
		ASSERT(bss.fileOffset = Ro.Pos(), 100)
	END WriteBssSection;

	PROCEDURE WriteShstrtabSection;
	BEGIN
		ASSERT(shstrtab.fileOffset = Ro.Pos(), 100);
		WriteStringTable(headerstrtab);
		shstrtab.size := Ro.Pos() - shstrtab.fileOffset
	END WriteShstrtabSection;

	PROCEDURE GetImpListSize (OUT len: INTEGER; OUT count: INTEGER);
		VAR m: Module; e: Export;
	BEGIN
		len := 0; count := 0;
		m := modList;
		WHILE m # NIL DO
			IF m.dll THEN
				e := m.exp;
				WHILE e # NIL DO
					INC(len, LEN(e.name$) + 1);
					INC(count);
					e := e.next
				END
			END;
			m := m.next
		END
	END GetImpListSize;
	
	PROCEDURE GetExpListSize (OUT len: INTEGER; OUT count: INTEGER);
		VAR e: Export;
	BEGIN
		count := 0; len := 0;
		e := firstExp;
		WHILE e # NIL DO
			INC(len, LEN(e.name$) + 1);
			INC(count);
			e := e.next
		END
	END GetExpListSize;
	
	PROCEDURE DynsymSize (init: INTEGER): INTEGER;
		VAR size: INTEGER;
	BEGIN
		size := init;
		INC(size, dynsymEntrySizeVal * 11); (* sections entries *)
		INC(size, dynsymEntrySizeVal); (* _DYNAMIC symbol *)
		INC(size, dynsymEntrySizeVal); (* _GLOBAL_OFFSET_TABLE_ symbol *)
		RETURN size
	END DynsymSize;
	
	PROCEDURE DynstrSize (init: INTEGER): INTEGER;
		VAR size: INTEGER;
	BEGIN
		size := init + 1;
		INC(size, dynstrtab.cur - 1);
		INC(size, LEN(soName$) + 1); (* library name *)
		INC(size, 9); (* "_DYNAMIC" symbol + 0X *)
		INC(size, 21 + 1); (* "_GLOBAL_OFFSET_TABLE_" symbol + trailing 0X *)
		RETURN size
	END DynstrSize;
	
	PROCEDURE DynamicSize (init: INTEGER): INTEGER;
		VAR i, size: INTEGER;
	BEGIN
		size := init;
		i := 0;
		WHILE neededIdx[i] # 0 DO
			INC(size, dynamicEntrySizeVal);
			INC(i)
		END;
		RETURN size
	END DynamicSize;
	
	PROCEDURE CalculateLayout;
		VAR headerSize, impCount, expCount, impLen, expLen: INTEGER;
	BEGIN
		ASSERT(~error, 20);
		headerSize := elfHeaderSizeVal + shEntrySizeVal * shNumVal + phEntrySizeVal * phNumVal;
		text.fileOffset := Aligned(headerSize, textAlign);
		text.memOffset := text.fileOffset;
		text.size := CodeSize;
		rodata.fileOffset := Aligned(text.fileOffset + text.size, rodataAlign);
		rodata.memOffset := rodata.fileOffset;
		rodata.size := ConSize;
		reltext.fileOffset := Aligned(rodata.fileOffset + rodata.size, relAlign);
		reltext.memOffset := reltext.fileOffset;
		doWrite := FALSE;
		WriteTextSection;	(* this only calculates the number of text relocations *)
		IF error THEN RETURN END;
		reltext.size := relEntrySizeVal * relTextTab.cur;
		relrodata.fileOffset := reltext.fileOffset + reltext.size;
		relrodata.memOffset := relrodata.fileOffset;
		IF ~error THEN
			WriteRodataSection	(* this only calculates the number of data relocations *)
		ELSE
			RETURN
		END;
		relrodata.size := relEntrySizeVal * relRodataTab.cur;
		dynsym.fileOffset := Aligned(relrodata.fileOffset + relrodata.size, dynsymAlign);
		dynsym.memOffset := dynsym.fileOffset;
		GetImpListSize(impLen, impCount);
		GetExpListSize(expLen, expCount);
		dynsym.size := DynsymSize((impCount + expCount) * dynsymEntrySizeVal);
		dynstr.fileOffset := Aligned(dynsym.fileOffset + dynsym.size, dynstrAlign);
		dynstr.memOffset := dynstr.fileOffset;
		dynstr.size := DynstrSize(impLen + expLen);
		hash.fileOffset := Aligned(dynstr.fileOffset + dynstr.size, hashAlign);
		hash.memOffset := hash.fileOffset;
		hash.size := 8 + dynsym.size DIV dynsymEntrySizeVal * 4 * 2;
		got.fileOffset := Aligned(hash.fileOffset + hash.size, gotAlign);
		got.memOffset := Aligned(got.fileOffset, pageSize) + got.fileOffset MOD pageSize;
		got.size := 3 * gotEntrySizeVal;
		dynamic.fileOffset := Aligned(got.fileOffset + got.size, dynamicAlign);
		dynamic.memOffset := got.memOffset + dynamic.fileOffset - got.fileOffset;
		dynamic.size := DynamicSize(13 * dynamicEntrySizeVal);
		bss.fileOffset := Aligned(dynamic.fileOffset + dynamic.size, bssAlign);
		bss.memOffset := dynamic.memOffset + bss.fileOffset - dynamic.fileOffset;		
		bss.size := DataSize;
		shstrtab.fileOffset := Aligned(bss.fileOffset, shstrtabAlign);
		shstrtab.size := fixup;
		textSegmentSizeVal := got.fileOffset;
		dataSegmentSizeVal := shstrtab.fileOffset - got.fileOffset;
		dynamicSegmentSizeVal := shstrtab.fileOffset - dynamic.fileOffset;
		relTextTab.cur := 0;
		relRodataTab.cur := 0;
		firstExp := NIL; lastExp := NIL;
		doWrite := TRUE
	END CalculateLayout;

	PROCEDURE WriteOut;
		VAR res: INTEGER;
	BEGIN
		ASSERT(~error, 20);
		Out := Files.dir.New(Files.dir.This(""), Files.ask);
		IF Out # NIL THEN
			Ro := Out.NewWriter(Ro); Ro.SetPos(0);
			CalculateLayout;
			IF ~error THEN WriteElfHeader END;
			IF ~error THEN WriteSectionHeaderTable END;
			IF ~error THEN WriteProgramHeaderTable END;
			IF ~error THEN Align(textAlign); WriteTextSection END;
			IF ~error THEN Align(rodataAlign); WriteRodataSection END;
			IF ~error THEN Align(relAlign); WriteRelSections END;
			IF ~error THEN Align(dynsymAlign); WriteDynsymSection END;
			IF ~error THEN Align(dynstrAlign); WriteDynstrSection END;
			IF ~error THEN Align(hashAlign); WriteHashSection END;
			IF ~error THEN Align(gotAlign); WriteGotSection END;
			IF ~error THEN Align(dynamicAlign); WriteDynamicSection END;
			IF ~error THEN Align(bssAlign); WriteBssSection END;
			IF ~error THEN Align(shstrtabAlign); WriteShstrtabSection END;

			IF ~error THEN FixupElfHeader END;
			IF ~error THEN FixupSectionHeaderTable END;
			IF ~error THEN FixupTextSection END;
			IF ~error THEN FixupDynsymSection END;
			IF ~error THEN FixupDynamicSection END;
			Out.Register(soName$, "so", Files.ask, res);
			IF res # 0 THEN error := TRUE END
		ELSE
			error := TRUE
		END
	END WriteOut;
	
	PROCEDURE ResetHashtab;
		VAR i: INTEGER;
	BEGIN
		i := 0;
		WHILE i # LEN(hashtab) DO
			hashtab[i] := "";
			INC(i)
		END
	END ResetHashtab;

	PROCEDURE ResetNeededIdx;
		VAR i: INTEGER;
	BEGIN
		i := 0;
		WHILE i # LEN(neededIdx) DO
			neededIdx[i] := 0;
			INC(i)
		END
	END ResetNeededIdx;

	PROCEDURE MakeSoName (VAR name: ARRAY OF CHAR; type: ARRAY OF CHAR);
		VAR i, j: INTEGER; ext: Files.Name; ch: CHAR;
	BEGIN
		ASSERT((type = "") OR (type[0] = "."), 20);
		i := 0;
		WHILE (name[i] # 0X) & (name[i] # ".") DO INC(i) END;
		IF name[i] = "." THEN
			IF name[i + 1] = 0X THEN name[i] := 0X END
		ELSIF i < LEN(name) - (LEN(type$) + 1) THEN
			IF type = "" THEN ext := ".so" ELSE ext := type$ END;
			j := 0; ch := ext[0];
			WHILE ch # 0X DO
				IF (ch >= "A") & (ch <= "Z") THEN
					ch := CHR(ORD(ch) + ORD("a") - ORD("A"))
				END;
				name[i] := ch; INC(i); INC(j); ch := ext[j]
			END;
			name[i] := 0X
		END
	END MakeSoName;
	
	PROCEDURE ParseExt (IN S: TextMappers.Scanner; OUT ext: Files.Name);
		VAR ch: CHAR; i: INTEGER;
	BEGIN
		ext := "";
		S.rider.ReadPrevChar(ch);
		IF ch = "." THEN
			S.rider.ReadChar(ch);
			i := 0;
			WHILE (ch # 20X) & (ch # 9X) DO
				ext[i] := ch;
				INC(i);
				S.rider.ReadChar(ch)
			END;
			ext[i] := 0X
		ELSIF (ch # 20X) & (ch # 9X) THEN
			W.WriteSString("Invalid character '");W.WriteChar(ch); W.WriteSString("' for file name.");
			W.WriteLn; StdLog.text.Append(StdLog.buf); error := TRUE
		END;
		S.SetPos(S.rider.Pos())
	END ParseExt;

	PROCEDURE ParseModList (S: TextMappers.Scanner; end: INTEGER);
		VAR mod: Module;
	BEGIN
		WHILE (S.start < end) & (S.type = TextMappers.string) DO
			NEW(mod); mod.fileName := S.string$;
			mod.next := modList; modList := mod;
			S.Scan;
			WHILE (S.start < end) & (S.type = TextMappers.char) &
				((S.char = "*") OR (S.char = "+") OR (S.char = "$") OR (S.char = "#")) DO
				IF S.char = "*" THEN mod.dll := TRUE
				ELSIF S.char = "+" THEN kernel := mod
				ELSIF S.char = "$" THEN main := mod
				ELSE mod.intf := TRUE;
					ASSERT(isDll, 126);
					IF ~isDll THEN
						W.WriteSString("Exports from Exe not possible. Use LinkDll or LinkDynDll.");
						W.WriteLn; StdLog.text.Append(StdLog.buf); error := TRUE
					END
				END;
				S.Scan
			END
		END
	END ParseModList;

	PROCEDURE LinkIt;
		VAR S: TextMappers.Scanner; name, ext: Files.Name; end: INTEGER;
	BEGIN
		doWrite := TRUE;
		headerstrtab.tab[0] := 0X;
		headerstrtab.cur := 1;
		dynstrtab.tab[0] := 0X;
		dynstrtab.cur := 1;
		relTextTab.cur := 0;
		relRodataTab.cur := 0;
		ResetHashtab;
		ResetNeededIdx;
		modList := NIL; kernel := NIL; main := NIL;
		last := NIL; impg := NIL; impd := NIL;
		firstExp := NIL; lastExp := NIL;
		Dialog.ShowStatus("linking");
		error := FALSE; modList := NIL;
		IF DevCommanders.par = NIL THEN RETURN END;
		S.ConnectTo(DevCommanders.par.text);
		S.SetPos(DevCommanders.par.beg);
		end := DevCommanders.par.end;
		DevCommanders.par := NIL;
		W.ConnectTo(StdLog.buf); S.Scan;
		IF S.type = TextMappers.string THEN
			name := S.string$;
			ext := "";
			ParseExt(S, ext); S.Scan;
			IF ~error THEN
				MakeSoName(name, ext);
				IF (S.type = TextMappers.char) & (S.char = ":") THEN S.Scan;
					IF (S.type = TextMappers.char) & (S.char = "=") THEN S.Scan;
						ParseModList(S, end);
						ReadHeaders;
						soName := SHORT(name$);
						IF ~error THEN
							WriteOut
						END;
						IF ~error THEN
							W.WriteString("Library " + name + " written: ");
							W.WriteInt(Out.Length()); W.WriteString("    "); W.WriteInt(text.size)
						END
					ELSE
						error := TRUE;
						W.WriteString(" := missing")
					END
				ELSE
					error := TRUE;
					W.WriteString(" := missing")
				END;
				W.WriteLn; StdLog.text.Append(StdLog.buf)
			END
		END;
		IF error THEN Dialog.ShowStatus("Failed to write library") ELSE Dialog.ShowStatus("Ok") END;
		W.ConnectTo(NIL); S.ConnectTo(NIL);
		modList := NIL; kernel := NIL; main := NIL; firstExp := NIL; lastExp := NIL;
		last := NIL; impg := NIL; impd := NIL; code := NIL
	END LinkIt;

(*
	exes are not supported

	PROCEDURE Link*;
	BEGIN
		HALT(126);
		isDll := FALSE; isStatic := FALSE;
		LinkIt
	END Link;
	
	PROCEDURE LinkExe*;
	BEGIN
		HALT(126);
		isDll := FALSE; isStatic := TRUE;
		LinkIt
	END LinkExe;
*)
	
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
		
BEGIN
	newRec := "NewRec"; newArr := "NewArr"
END DevElfLinker.