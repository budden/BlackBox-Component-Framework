MODULE HostUtf8;

	(*
		A. V. Shiryaev, 2012.10

		UTF-8 encoder and decoder
	*)

	CONST
		maxShortCharsPerChar* = 3;

	TYPE
		Decoder* = RECORD
			b: INTEGER;
			st: INTEGER (* 0 - no state, 1 - one char expected, 2 - two chars expected, -1 - error *)
		END;

	PROCEDURE Encode* (IN f: ARRAY OF CHAR; VAR fR, fLen: INTEGER; VAR t: ARRAY OF SHORTCHAR; VAR tW: INTEGER);
		VAR x: INTEGER;
	BEGIN
		WHILE fLen > 0 DO
			x := ORD(f[fR]);

(* XhtmlStdFileWriters.Char *)
(* http://www.lemoda.net/c/ucs2-to-utf8/index.html *)
(* http://tools.ietf.org/html/rfc3629 *)
			CASE x OF 00H..7FH:
				t[tW] := SHORT(CHR(x)); INC(tW)
			| 80H..7FFH:
				t[tW] := SHORT(CHR(x DIV 64 + 192));
				t[tW+1] := SHORT(CHR(x MOD 64 + 128));
				INC(tW, 2)
			ELSE (* | 800H..0FFFFH *)
				t[tW] := SHORT(CHR(x DIV 4096 + 224));
				t[tW+1] := SHORT(CHR((x DIV 64) MOD 64 + 128));
				t[tW+2] := SHORT(CHR(x MOD 64 + 128));
				INC(tW, 3)
			END;

			INC(fR); DEC(fLen)
		END
	END Encode;

	PROCEDURE ShortLen* (IN f: ARRAY OF CHAR; fLen: INTEGER): INTEGER;
		VAR i, j: INTEGER;
	BEGIN
		i := 0; j := 0;
		WHILE i < fLen DO
			CASE ORD(f[i]) OF 00H..7FH: INC(j)
			| 80H..7FFH: INC(j, 2)
			ELSE (* | 800H..0FFFFH *) INC(j, 3)
			END;
			INC(i)
		END;
		RETURN j
	END ShortLen;

	PROCEDURE ShortLenToLongLen* (IN s: ARRAY OF CHAR; VAR shortLen: INTEGER; OUT longLen: INTEGER);
	BEGIN
		longLen := 0;
		WHILE shortLen > 0 DO
			CASE ORD(s[longLen]) OF 00H..7FH: DEC(shortLen)
			| 80H..7FFH: DEC(shortLen, 2)
			ELSE (* | 800H..0FFFFH *) DEC(shortLen, 3)
			END;
			INC(longLen)
		END
	END ShortLenToLongLen;

	PROCEDURE Short* (IN s: ARRAY OF CHAR; OUT ss: ARRAY OF SHORTCHAR);
		VAR fR, fLen, tW: INTEGER;
	BEGIN
		fR := 0; fLen := LEN(s$); tW := 0;
		Encode(s, fR, fLen, ss, tW);
		IF fLen = 0 THEN ss[tW] := 0X
		ELSE ss[0] := 0X
		END
	END Short;

	PROCEDURE Decode* (VAR d: Decoder; IN f: ARRAY OF SHORTCHAR; VAR fR, fLen: INTEGER; VAR t: ARRAY OF CHAR; VAR tW: INTEGER; OUT state: BOOLEAN);
		VAR x: INTEGER;
	BEGIN
		WHILE fLen > 0 DO
			x := ORD(f[fR]);

(* http://www.lemoda.net/c/utf8-to-ucs2/index.html *)
(* http://tools.ietf.org/html/rfc3629 *)
			CASE d.st OF 0:
				CASE x OF 0..127:
					t[tW] := CHR(x); INC(tW)
				| 194..223:
					d.b := x MOD 32;
					INC(d.st) (* d.st := 1 *)
				| 224..239:
					d.b := x MOD 16;
					INC(d.st, 2) (* d.st := 2 *)
				ELSE
					DEC(d.st); (* d.st := -1 *)
					RETURN
				END
			| 1:
				CASE x OF 128..191:
					t[tW] := CHR(d.b * 64 + x MOD 64); INC(tW);
					DEC(d.st) (* d.st := 0 *)
				ELSE
					DEC(d.st, 2); (* d.st := -1 *)
					RETURN
				END
			| 2:
				CASE x OF 128..159:
					IF d.b = 0 THEN
						DEC(d.st, 3); (* d.st := -1 *)
						RETURN
					ELSE
						d.b := d.b * 64 + x MOD 64;
						DEC(d.st) (* d.st := 1 *)
					END
				| 160..191:
					d.b := d.b * 64 + x MOD 64;
					DEC(d.st) (* d.st := 1 *)
				ELSE
					DEC(d.st, 3); (* d.st := -1 *)
					RETURN
				END
			END;

			INC(fR); DEC(fLen)
		END;

		CASE d.st OF 0: state := FALSE
		| 1,2: state := TRUE
		END
	END Decode;

	PROCEDURE ResetDecoder* (VAR d: Decoder);
	BEGIN
		d.st := 0
	END ResetDecoder;

	PROCEDURE Long* (IN ss: ARRAY OF SHORTCHAR; OUT s: ARRAY OF CHAR);
		VAR d: Decoder;
			fR, fLen, tW: INTEGER; st: BOOLEAN;
	BEGIN
		ResetDecoder(d);
		fR := 0; fLen := LEN(ss$); tW := 0;
		Decode(d, ss, fR, fLen, s, tW, st);
		IF (fLen = 0) & ~st THEN s[tW] := 0X
		ELSE s[0] := 0X (* decode error or incomplete *)
		END
	END Long;

END HostUtf8.
