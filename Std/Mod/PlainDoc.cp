MODULE StdPlainDoc;

	(*
		Purpose: Defines a UTF-8 encoded format as default format for storage of documents instead of ODC
		Initial version: 26 Aug 2013
		Author: Peter Cushnir
		Contributors: Romiras
	*)

	IMPORT
		SYSTEM, Kernel, Services, Views, Files, Converters, Windows, Stores, Dialog, StdDialog;

	CONST
		plainTyp = 'cp';

	TYPE
		GuardAction = POINTER TO RECORD (Services.Action)
			adr: INTEGER;
			next: GuardAction;
			t: LONGINT;
			name: Files.Name;
			conv: Converters.Converter;
		END;

		ViewHook = POINTER TO RECORD (Views.ViewHook)
			guards: GuardAction;
		END;

	VAR
		utf8conv: Converters.Converter;
		thisHook: ViewHook;
		defaultHook: Views.ViewHook;

	PROCEDURE Remove (VAR root: GuardAction; old: GuardAction);
		VAR dummy, g: GuardAction;
	BEGIN
		ASSERT(old # NIL, 20);
		Services.RemoveAction(old);
		NEW(dummy); dummy.next:=root;
		g:=dummy;
		WHILE (g # NIL) & (old # NIL) DO
			IF g.next=old THEN
				g.next:=old.next; old.next:=NIL; old:=NIL;
			END;
			g:=g.next
		END;
		root:=dummy.next
	END Remove;

	PROCEDURE Add (VAR root: GuardAction; new: GuardAction);
	BEGIN
		ASSERT(new # NIL, 20);
		new.next:=root;
		root:=new;
		Services.DoLater(new, Services.now)
	END Add;

	PROCEDURE This (root: GuardAction; adr: INTEGER): GuardAction;
		VAR g: GuardAction;
	BEGIN
		g:=root;
		WHILE (g # NIL) & (g.adr # adr) DO g:=g.next END;
	RETURN g
	END This;

	PROCEDURE Clean (root: GuardAction);
		VAR g: GuardAction;
	BEGIN
		g:=root;
		WHILE (g # NIL) DO Services.RemoveAction(g); g:=g.next END
	END Clean;

	PROCEDURE (a: GuardAction) Do;
		VAR g: GuardAction;
	BEGIN
		IF thisHook # NIL THEN
			g:=thisHook.guards;
			WHILE (g # NIL) & (g # a) DO g:=g.next END;
			IF (g=a) & ((Services.Ticks()-a.t)<1000) THEN Services.DoLater(a, Services.now)
			ELSE
				Remove(thisHook.guards, a)
			END
		END
	END Do;

	PROCEDURE (h: ViewHook) Open (v: Views.View; title: ARRAY OF CHAR; loc: Files.Locator; name: Files.Name; conv: Converters.Converter; asTool, asAux, noResize, allowDuplicates, neverDirty: BOOLEAN);
		VAR this: GuardAction;
	BEGIN
		this:=This(h.guards, Services.AdrOf(v^));
		IF this # NIL THEN
			Dialog.ShowStatus('Open '+this.name$);
			name:=this.name; conv:=this.conv;
			Remove(h.guards, this);
		END;
		StdDialog.Open(v, title, loc, name, conv, asTool, asAux, noResize, allowDuplicates, neverDirty)
	END Open;

	PROCEDURE (h: ViewHook) OldView (loc: Files.Locator; name: Files.Name; VAR conv: Converters.Converter): Views.View;
		(* Based on StdDialog.OldView *)
		VAR w: Windows.Window; s: Stores.Store; converter: Converters.Converter; a: GuardAction;
			fi: Files.FileInfo; vs: Dialog.String; pos: INTEGER;

		PROCEDURE FixName (VAR name: ARRAY OF CHAR);
			VAR i: INTEGER;
		BEGIN
			i:=0;
			WHILE (name[i] # 0X) & (name[i] # ".") DO INC(i) END;
			IF name[i] = "." THEN name[i]:=0X END;
			Kernel.MakeFileName(name, plainTyp);
		END FixName;

	BEGIN
		ASSERT(loc # NIL, 20); ASSERT(name # "", 21);

		vs:=name$;
		FixName(vs);
		fi:=Files.dir.FileList(loc);
		WHILE (fi # NIL) & (fi.name$ # vs) DO fi:=fi.next END;

		IF fi # NIL THEN	(* found plainTyp *)
			FixName(name);
			Dialog.ShowStatus('Found '+name$+' UTF-8 document');
			converter := utf8conv;
			NEW(a)
		ELSE
			Kernel.MakeFileName(name, "");
			converter := conv
		END;
		s := NIL;
		IF loc.res # 77 THEN
			w := Windows.dir.First();
			WHILE (w # NIL) & ((w.loc = NIL) OR (w.name = "") OR (w.loc.res = 77) OR ~Files.dir.SameFile(loc, name, w.loc, w.name) OR (w.conv # converter)) DO
				w := Windows.dir.Next(w)
			END;
			IF w # NIL THEN s := w.doc.ThisView() END
		END;
		IF s = NIL THEN
			Converters.Import(loc, name, converter, s);
			IF s # NIL THEN
				IF a # NIL THEN
					a.adr:=Services.AdrOf(s^);
					a.t:=Services.Ticks();
					a.name:=name;
					a.conv:=converter;
					Add(h.guards, a)
				END;
				StdDialog.RecalcView(s(Views.View))
			END
		END;
		IF s # NIL THEN RETURN s(Views.View) ELSE RETURN NIL END
	END OldView;

	PROCEDURE (h: ViewHook) RegisterView (v: Views.View; loc: Files.Locator; name: Files.Name; conv: Converters.Converter);
	BEGIN
		ASSERT(v # NIL, 20); ASSERT(loc # NIL, 21); ASSERT(name # "", 22);
		Kernel.MakeFileName(name, "");
		Converters.Export(loc, name, conv, v)
	END RegisterView;

	PROCEDURE GetDefaultHook;
		VAR m: Kernel.Module; i: INTEGER; p: ANYPTR;
	BEGIN
		m:=Kernel.ThisMod('Views');
		i := 0;
		WHILE (i < m.nofptrs) & (defaultHook = NIL) DO
			SYSTEM.GET(m.varBase + m.ptrs[i], p); INC(i);
			IF (p # NIL) & (p IS Views.ViewHook) THEN
				defaultHook:=p (Views.ViewHook)
			END;
			INC(i)
		END;
	END GetDefaultHook;

	PROCEDURE FindUtf8Converter;
	BEGIN
		utf8conv := Converters.list;
		WHILE (utf8conv # NIL) & (utf8conv.fileType$ # plainTyp) DO utf8conv := utf8conv.next END;
		IF utf8conv = NIL THEN utf8conv := Converters.list END	(* use default document converter *)
	END FindUtf8Converter;

	PROCEDURE Init*;
	BEGIN END Init;

BEGIN
	GetDefaultHook;
	NEW(thisHook); Views.SetViewHook(thisHook);
	Converters.Register ("ConvUtf8.ImportUtf8", "ConvUtf8.ExportUtf8", "TextViews.View", plainTyp, {});
	FindUtf8Converter
CLOSE
	IF thisHook # NIL THEN
		Clean(thisHook.guards);
		Views.SetViewHook(defaultHook);
		thisHook:=NIL
	END;
END StdPlainDoc.
