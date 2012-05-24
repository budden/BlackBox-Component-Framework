MODULE Terminals;

	TYPE
		Terminal* = POINTER TO ABSTRACT RECORD END;
		Directory* = POINTER TO ABSTRACT RECORD END;

	VAR dir-, stdDir-: Directory;

	PROCEDURE (terminal: Terminal) WriteLn*, NEW, ABSTRACT;
	PROCEDURE (terminal: Terminal) WriteString* (IN string: ARRAY OF CHAR), NEW, ABSTRACT;
	PROCEDURE (terminal: Terminal) Clear*, NEW, ABSTRACT;
	PROCEDURE (terminal: Terminal) Show*, NEW, ABSTRACT;

	PROCEDURE (d: Directory) New* (IN title: ARRAY OF CHAR; width, height: INTEGER): Terminal, NEW, ABSTRACT;
	
	PROCEDURE SetDir* (d: Directory);
	BEGIN
		ASSERT(d # NIL, 20);
		dir := d;
		IF stdDir = NIL THEN stdDir := d END
	END SetDir;

END Terminals.
