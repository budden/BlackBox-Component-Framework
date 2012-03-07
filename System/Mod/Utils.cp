MODULE Utils;

	IMPORT (*Kernel,*) Files;
	
	CONST
		OFdir* = "Code";
		SYSdir* = "System";
		
		objType* = "ocf";
		symType* = "osf";
		
		codeType* = "cp";
		docType* = "odc";
	
	PROCEDURE SplitName* (name: ARRAY OF CHAR; VAR head, tail: ARRAY OF CHAR);
		(* portable *)
		VAR i, j: INTEGER; ch, lch: CHAR;
	BEGIN
		i := 0; ch := name[0];
		IF ch # 0X THEN
			REPEAT
				head[i] := ch; lch := ch; INC(i); ch := name[i]
			UNTIL (ch = 0X)
				OR ((ch >= "A") & (ch <= "Z") OR (ch >= "À") & (ch # "×") & (ch <= "Þ"))
					& ((lch < "A") OR (lch > "Z") & (lch < "À") OR (lch = "×") OR (lch > "Þ"));
			head[i] := 0X; j := 0;
			WHILE ch # 0X DO tail[j] := ch; INC(i); INC(j); ch := name[i] END;
			tail[j] := 0X;
			IF tail = "" THEN tail := head$; head := "" END
		ELSE head := ""; tail := ""
		END
	END SplitName;

	PROCEDURE MakeFileName* (VAR name: ARRAY OF CHAR; type: ARRAY OF CHAR);
		VAR i, j: INTEGER; ext: ARRAY 8 OF CHAR; ch: CHAR;
	BEGIN
		i := 0;
		WHILE (name[i] # 0X) & (name[i] # ".") DO INC(i) END;
		IF name[i] = "." THEN
			IF name[i + 1] = 0X THEN name[i] := 0X END
		ELSIF i < LEN(name) - 4 THEN
			IF type = "" THEN ext := docType ELSE ext := type$ END;
			name[i] := "."; INC(i); j := 0; ch := ext[0];
			WHILE ch # 0X DO
				IF (ch >= "A") & (ch <= "Z") THEN
					ch := CHR(ORD(ch) + ORD("a") - ORD("A"))
				END;
				name[i] := ch; INC(i); INC(j); ch := ext[j]
			END;
			name[i] := 0X
		END
	END MakeFileName;

	PROCEDURE GetSubLoc* (mod: ARRAY OF CHAR; cat: Files.Name;
											OUT loc: Files.Locator; OUT name: Files.Name);
		VAR sub: Files.Name; file: Files.File; type: Files.Type;
	BEGIN
		IF cat$ = "Sym" THEN type := symType
		ELSIF cat$ = "Code" THEN type := objType
		ELSIF cat$ = "Mod" THEN type := codeType
		ELSE type := ""
		END;
		SplitName(mod, sub, name); MakeFileName(name, type);
		loc := Files.dir.This(sub); file := NIL;
		IF loc # NIL THEN
			loc := loc.This(cat);
			IF sub = "" THEN
				IF loc # NIL THEN
					file := Files.dir.Old(loc, name, Files.shared);
					IF file = NIL THEN loc := NIL END
				END;
				IF loc = NIL THEN
					loc := Files.dir.This(SYSdir);
					IF loc # NIL THEN loc := loc.This(cat) END
				END
			END
		END
	END GetSubLoc;

	PROCEDURE ThisObjFile* (VAR name: ARRAY OF CHAR): Files.File;
		VAR f: Files.File; loc: Files.Locator; dir, fname: Files.Name;
	BEGIN
		SplitName(name, dir, fname);
		MakeFileName(fname, objType);
		loc := Files.dir.This(dir); loc := loc.This(OFdir);
		f := Files.dir.Old(loc, fname, TRUE);
		IF (f = NIL) & (dir = "") THEN
			loc := Files.dir.This(SYSdir); loc := loc.This(OFdir);
			f := Files.dir.Old(loc, fname, TRUE)
		END;
		RETURN f
	END ThisObjFile;
	
	PROCEDURE GenName* (VAR from, to: ARRAY OF SHORTCHAR; ext: ARRAY OF SHORTCHAR);
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
	
END Utils.
