MODULE ConvUtf8;

	(*
		Converter for Import / Export filter for 8-bit Unicode Transformation Format
	*)

	IMPORT
		Files, Stores, TextModels, TextViews;

	CONST
		CR = 0DX; LF = 0AX;

		font1 = "Arial Unicode MS";
		font2 = "Lucida Sans Unicode";
		unicodefont = font1;

	PROCEDURE SetFont (md: TextModels.Model);
		VAR at: TextModels.Attributes; beg, end: INTEGER;
	BEGIN
		beg := 0; end := md.Length();
		NEW(at); at.InitFromProp(md.Prop(beg, end));
		at := TextModels.NewTypeface(at, unicodefont);
		md.SetAttr(beg, end, at);
	END SetFont;

	PROCEDURE ReadChar (IN rd: Stores.Reader; VAR ch: CHAR);
		VAR
			c1, c2, c3: BYTE;
	BEGIN	(* UTF-8 format *)
		rd.ReadByte(c1);
		ch := CHR(c1);
		IF c1 < 0 THEN (* c1 < 0 &  c1 > -64 = C0 = 110x xxxx *)
			rd.ReadByte(c2);
			ch := CHR(64 * (c1 MOD 32) + (c2 MOD 64));
			IF c1 >=  - 32 THEN (* c1 < 0 & c1 >= -32 = E0 = 1110 xxxxx *)
				rd.ReadByte(c3);
				ch := CHR(4096 * (c1 MOD 16) + 64 * (c2 MOD 64) + (c3 MOD 64));
			END;
		END;
	END ReadChar;

	PROCEDURE WriteChar (IN wr: Stores.Writer; ch: CHAR);
	BEGIN	(* UTF-8 format *)
		IF ch <= 7FX THEN
			wr.WriteByte(SHORT(SHORT(ORD(ch))))
		ELSIF ch <= 7FFX THEN
			wr.WriteByte(SHORT(SHORT( - 64 + ORD(ch) DIV 64)));
			wr.WriteByte(SHORT(SHORT( - 128 + ORD(ch) MOD 64)))
		ELSE
			wr.WriteByte(SHORT(SHORT( - 32 + ORD(ch) DIV 4096)));
			wr.WriteByte(SHORT(SHORT( - 128 + ORD(ch) DIV 64 MOD 64)));
			wr.WriteByte(SHORT(SHORT( - 128 + ORD(ch) MOD 64)))
		END
	END WriteChar;

	PROCEDURE ImportUtf8* (f: Files.File; OUT s: Stores.Store);
		VAR
			rd: Stores.Reader; md: TextModels.Model; wr: TextModels.Writer; ch, nch: CHAR;
	BEGIN
		ASSERT(f # NIL, 20);
		rd.ConnectTo(f); rd.SetPos(0);
		md := TextModels.dir.New(); wr := md.NewWriter(NIL);
		ReadChar(rd, ch);
		WHILE ~rd.rider.eof DO
			ReadChar(rd, nch);
			IF (ch = CR) & (nch = LF) THEN ReadChar(rd, nch)
			ELSIF ch = LF THEN ch := CR
			END;
			wr.WriteChar(ch);
			ch := nch;
		END;
		SetFont(md);
		s := TextViews.dir.New(md)
	END ImportUtf8;

	PROCEDURE ExportUtf8* (s: Stores.Store; f: Files.File);
		VAR wr: Stores.Writer; md: TextModels.Model; rd: TextModels.Reader; ch: CHAR;
	BEGIN
		ASSERT(s # NIL, 20); ASSERT(f # NIL, 21);
		wr.ConnectTo(f); wr.SetPos(0);
		md := s(TextViews.View).ThisModel();
		IF md # NIL THEN
			rd := md.NewReader(NIL);
			rd.ReadChar(ch);
			WHILE ~rd.eot DO
				IF ch = CR THEN WriteChar(wr, LF) ELSE WriteChar(wr, ch) END;
				rd.ReadChar(ch)
			END
		END
	END ExportUtf8;

END ConvUtf8.
