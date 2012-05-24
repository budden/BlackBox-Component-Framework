MODULE Stderr;

(*
	History:
	Date:	Author:	Change:
	??-Jan-2007	Rainer Neubauer	Start of development.
	26-May-2008	Rainer Neubauer	Split in two different versions (BB standalone and for coding to LabView)
	.		Init is not exported any more
	.		SetWindowTitle added
	04-Sep-2008	Rainer Neubauer	SetWindowTitle -> WindowSetTitle
	05-Jul-2009	Rainer Neubauer	Module LvTime replaced by BasicsTime
	30-Oct-2009	Rainer Neubauer	discarded: RedirectOutput, WindowSetTitle
*)


	IMPORT Strings, Dates, Terminals;

	CONST
		maxLengthOfIntString = 10; (* 32 Bits Integer value to string *)
		maxLengthOfRealString = 30; (* arbitrary *)

	VAR
		terminal: Terminals.Terminal; 


	PROCEDURE Int* (inValue: INTEGER);
		VAR string: ARRAY maxLengthOfIntString + 1 OF CHAR;
	BEGIN
		Strings.IntToString (inValue, string);
		terminal.WriteString (string); terminal.Show
	END Int;


	PROCEDURE Ln*;
	BEGIN
		terminal.WriteLn; terminal.Show
	END Ln;

	PROCEDURE Real* (inValue: REAL);
		VAR string: ARRAY maxLengthOfRealString + 1 OF CHAR;
	BEGIN
		Strings.RealToString (inValue, string);
		terminal.WriteString (string); terminal.Show
	END Real;

	PROCEDURE String* (IN string: ARRAY OF CHAR);
	BEGIN
		terminal.WriteString (string); terminal.Show
	END String;


	PROCEDURE TimeLn*;
		VAR
			date: Dates.Date;
			dateString: ARRAY 31 OF CHAR;
			time: Dates.Time;
			timeString: ARRAY 9 OF CHAR;
	BEGIN
		Dates.GetDate (date);
		Dates.DateToString (date, Dates.plainAbbreviated, dateString);
		Dates.GetTime (time);
		Dates.TimeToString (time, timeString);

		terminal.WriteString (dateString + "  " + timeString);
		terminal.WriteLn; terminal.WriteLn;
		terminal.Show
	END TimeLn;

	PROCEDURE Init;
	BEGIN
		ASSERT(Terminals.dir # NIL, 20);
		IF terminal = NIL THEN
			terminal := Terminals.dir.New ("Stderr", 80, 60);
			ASSERT(terminal # NIL, 21)
		END
	END Init;

BEGIN
	Init
END Stderr.
