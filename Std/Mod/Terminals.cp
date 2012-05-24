MODULE StdTerminals;

(* History:
	Date:	Author:	Change:
	26-Aug-2009	Rainer Neubauer	Start of development.
	08-May-2012	Romiras	Module was separated to abstract and implementation parts: Terminals, StdTerminals
*)

	IMPORT Ports, Views, Containers, Documents, StdDialog, TextControllers, TextMappers, TextModels, TextViews, Terminals;

	TYPE
		StdTerminal = POINTER TO RECORD (Terminals.Terminal)
			width, height: INTEGER;
			title: POINTER TO ARRAY OF CHAR;
			f: TextMappers.Formatter;
			m: TextModels.Model
		END;

		Directory = POINTER TO RECORD (Terminals.Directory) END;
	
	VAR
		dir: Directory;

	PROCEDURE^ Flush (terminal: StdTerminal);

	PROCEDURE (terminal: StdTerminal) Clear;
	BEGIN
		terminal.m.Delete (0, terminal.m.Length ());
		terminal.f.SetPos (0);
		Flush (terminal)
	END Clear;


	PROCEDURE Flush (terminal: StdTerminal);
		VAR pos: INTEGER;
	BEGIN
		Views.RestoreDomain (terminal.m.Domain ()); (* update display right now *)
		pos := terminal.m.Length();
		TextViews.ShowRange (terminal.m, pos, pos, TextViews.any); (* show end of text *)
		TextControllers.SetCaret (terminal.m, pos) (* position cursor at end of text *)
	END Flush;


	PROCEDURE (terminal: StdTerminal) Show;
		VAR
			controller: Containers.Controller;
			document: Documents.Document;
			view: TextViews.View;
	BEGIN
		view	 := TextViews.dir.New (terminal.m);
		document	 := Documents.dir.New (view, terminal.width * Ports.mm, terminal.height * Ports.mm);
		controller	 := document.ThisController ();
		controller.SetOpts (controller.opts + {Documents.winHeight} - {Documents.pageHeight} + {Documents.winWidth} - {Documents.pageWidth});
		(*Views.OpenAux (document, terminal.title$);*) StdDialog.Open (document, terminal.title$, NIL, "", NIL, FALSE, TRUE, FALSE, FALSE, TRUE);
		Flush (terminal)
	END Show;


	PROCEDURE (terminal: StdTerminal) WriteLn;
		VAR pos: INTEGER;
	BEGIN
		pos := terminal.m.Length ();
		terminal.f.SetPos (pos); (* manual input may have been done to the window *)
		terminal.f.WriteLn;
		Flush (terminal)
	END WriteLn;


	PROCEDURE (terminal: StdTerminal) WriteString (IN string: ARRAY OF CHAR);
		VAR pos: INTEGER;
	BEGIN
		pos := terminal.m.Length ();
		terminal.f.SetPos (pos); (* manual input may have been done to the window *)
		terminal.f.WriteString (string);
		Flush (terminal)
	END WriteString;

	PROCEDURE (d: Directory) New (IN title: ARRAY OF CHAR; width, height: INTEGER): Terminals.Terminal;
		VAR t: StdTerminal;
	BEGIN
		NEW(t);
		t.width := width;
		t.height := height;
		NEW (t.title, LEN (title$) + 1);
		t.title^ := title$;
		t.m := TextModels.dir.New ();
		t.f.ConnectTo (t.m);
		RETURN t
	END New;
	
	PROCEDURE Init;
	BEGIN
		NEW(dir);
		Terminals.SetDir(dir)
	END Init;

BEGIN
	Init
END StdTerminals.
