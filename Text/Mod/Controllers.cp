MODULE TextControllers;
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

	IMPORT
		Services, Stores, Ports, Models, Views, Dialog, Controllers, Properties, Containers,
		TextModels, TextRulers, TextSetters, TextViews;

	CONST
		noAutoScroll* = 16; noAutoIndent* = 17;

		(** Controller.SetCaret pos; Controller.SetSelection beg, end **)
		none* = -1;

		(* Track mode *)
		chars = 0; words = 1; lines = 2;	(* plus "none", defined above *)

		enter = 3X; rdel = 7X; ldel = 8X;
		aL = 1CX; aR = 1DX; aU = 1EX; aD = 1FX;
		pL = 10X; pR = 11X; pU = 12X; pD = 13X;
		dL = 14X; dR = 15X; dU = 16X; dD = 17X;

		viewcode = TextModels.viewcode;
		tab = TextModels.tab; line = TextModels.line; para = TextModels.para;

		point = Ports.point; mm = Ports.mm; inch16 = Ports.inch DIV 16;

		boundCaret = TRUE;
		lenCutoff = 2000;	(* max run length inspected to fetch properties *)

		attrChangeKey = "#Text:AttributeChange";
		resizingKey = "#System:Resizing";
		insertingKey = "#System:Inserting";
		deletingKey = "#System:Deleting";
		movingKey = "#System:Moving";
		copyingKey = "#System:Copying";
		linkingKey = "#System:Linking";
		replacingKey = "#System:Replacing";

		minVersion = 0; maxVersion = 0; maxStdVersion = 0;


	TYPE
		Controller* = POINTER TO ABSTRACT RECORD (Containers.Controller)
			view-: TextViews.View;
			text-: TextModels.Model	(** view # NIL => text = view.ThisText() **)
		END;

		Directory* = POINTER TO ABSTRACT RECORD (Containers.Directory) END;


		FilterPref* = RECORD (Properties.Preference)
			controller*: Controller;	(** IN, set to text controller asking for filter **)
			frame*: Views.Frame;	(** IN, set to frame of controlled text view **)
			x*, y*: INTEGER;	(** IN, set to coordinates of cursor in frame space **)
			filter*: BOOLEAN	(** preset to FALSE **)
		END;

		FilterPollCursorMsg* = RECORD (Controllers.Message)
			controller*: Controller;	(** IN, set to text controller asking for filter **)
			x*, y*: INTEGER;
			cursor*: INTEGER;	(** as for Controllers.PollCursorMsg **)
			done*: BOOLEAN	(** OUT; initialized to FALSE **)
		END;

		FilterTrackMsg* = RECORD (Controllers.Message)
			controller*: Controller;	(** IN, set to text controller asking for filter **)
			x*, y*: INTEGER;
			modifiers*: SET;	(** as for Controllers.TrackMsg **)
			done*: BOOLEAN	(** OUT; initialized to FALSE **)
		END;


		StdCtrl = POINTER TO RECORD (Controller)
			(* general state *)
			cachedRd: TextModels.Reader;
			cachedWr: TextModels.Writer;
			insAttr: TextModels.Attributes;  (* preset attrs for next typed char *)
			autoBeg, autoEnd: INTEGER;	(* lazy auto-scrolling;
																invalid if (-1, .); initially (MAX(LONGINT), 0) *)
			(* caret *)
			carPos: INTEGER;	(* HasCaret()  iff  0 <= carPos <= text.Length() *)
			carLast: INTEGER;	(* used to recover caret at meaningful position *)
			carX, lastX: INTEGER;	(* arrow up/down anti-aliasing *)
			carTick: LONGINT;	(* next tick to invert flashing caret mark *)
			carVisible: BOOLEAN;	(* caret currently visible - used for flashing caret *)
			(* selection *)
			selBeg, selEnd: INTEGER;	(* HasSel()  iff  0 <= selBeg < selEnd <= text.Length() *)
			aliasSelBeg, aliasSelEnd: INTEGER;	(* need lazy synchronization? *)
			selPin0, selPin1: INTEGER;	(* anchor points of selection *)
			(* most recent scroll-while-tracking step *)
			lastStep: LONGINT
		END;

		StdDirectory = POINTER TO RECORD (Directory) END;


		(* messages *)

		ModelMessage* = ABSTRACT RECORD (Models.Message) END;
			(** messages to control virtual model extensions, such as marks **)

		SetCaretMsg* = EXTENSIBLE RECORD (ModelMessage)
			pos*: INTEGER
		END;

		SetSelectionMsg* = EXTENSIBLE RECORD (ModelMessage)
			beg*, end*: INTEGER
		END;


		ViewMessage = ABSTRACT RECORD (Views.Message) END;

		CaretMsg = RECORD (ViewMessage)
			show: BOOLEAN
		END;

		SelectionMsg = RECORD (ViewMessage)
			beg, end: INTEGER;
			show: BOOLEAN
		END;


		(* miscellaneous *)

		TrackState = RECORD
			x, y: INTEGER;
			toggle: BOOLEAN
		END;


	VAR
		dir-, stdDir-: Directory;


	PROCEDURE CachedReader (c: StdCtrl): TextModels.Reader;
		VAR rd: TextModels.Reader;
	BEGIN
		rd := c.text.NewReader(c.cachedRd); c.cachedRd := NIL; RETURN rd
	END CachedReader;

	PROCEDURE CacheReader (c: StdCtrl; rd: TextModels.Reader);
	BEGIN
		c.cachedRd := rd
	END CacheReader;


	PROCEDURE CachedWriter (c: StdCtrl; attr: TextModels.Attributes): TextModels.Writer;
		VAR wr: TextModels.Writer;
	BEGIN
		wr := c.text.NewWriter(c.cachedWr); wr.SetAttr(attr);
		c.cachedRd := NIL; RETURN wr
	END CachedWriter;

	PROCEDURE CacheWriter (c: StdCtrl; wr: TextModels.Writer);
	BEGIN
		c.cachedWr := wr
	END CacheWriter;


	(** Controller **)

	PROCEDURE (c: Controller) Internalize2- (VAR rd: Stores.Reader), EXTENSIBLE;
		VAR v: INTEGER;
	BEGIN
		(* c.Internalize^(rd); *)
		rd.ReadVersion(minVersion, maxVersion, v)
	END Internalize2;

	PROCEDURE (c: Controller) Externalize2- (VAR wr: Stores.Writer), EXTENSIBLE;
	BEGIN
		(* c.Externalize^(wr); *)
		wr.WriteVersion(maxVersion)
	END Externalize2;

	PROCEDURE (c: Controller) InitView2* (v: Views.View), EXTENSIBLE;
	BEGIN
		ASSERT((v = NIL) # (c.view = NIL), 21);
		IF c.view = NIL THEN ASSERT(v IS TextViews.View, 22) END;
		(* c.InitView^(v); *)
		IF v # NIL THEN c.view := v(TextViews.View); c.text := c.view.ThisModel()
		ELSE c.view := NIL; c.text := NIL
		END
	END InitView2;

	PROCEDURE (c: Controller) ThisView* (): TextViews.View, EXTENSIBLE;
	BEGIN
		RETURN c.view
	END ThisView;


	(** caret **)

	PROCEDURE (c: Controller) CaretPos* (): INTEGER, NEW, ABSTRACT;
	PROCEDURE (c: Controller) SetCaret* (pos: INTEGER), NEW, ABSTRACT;
	(** pre: pos = none  OR  0 <= pos <= c.text.Length() **)
	(** post: c.carPos = pos **)


	(** selection **)

	PROCEDURE (c: Controller) GetSelection* (OUT beg, end: INTEGER), NEW, ABSTRACT;
	(** post: beg = end  OR  0 <= beg <= end <= c.text.Length() **)

	PROCEDURE (c: Controller) SetSelection* (beg, end: INTEGER), NEW, ABSTRACT;
	(** pre: beg = end  OR  0 <= beg < end <= c.text.Length() **)
	(** post: c.selBeg = beg, c.selEnd = end **)


	(** Directory **)

	PROCEDURE (d: Directory) NewController* (opts: SET): Controller, ABSTRACT;

	PROCEDURE (d: Directory) New* (): Controller, EXTENSIBLE;
	BEGIN
		RETURN d.NewController({})
	END New;


	(** miscellaneous **)

	PROCEDURE SetDir* (d: Directory);
	BEGIN
		ASSERT(d # NIL, 20); dir := d
	END SetDir;

	PROCEDURE Install*;
	BEGIN
		TextViews.SetCtrlDir(dir)
	END Install;


	PROCEDURE Focus* (): Controller;
		VAR v: Views.View; c: Containers.Controller;
	BEGIN
		v := Controllers.FocusView();
		IF (v # NIL) & (v IS TextViews.View) THEN
			c := v(TextViews.View).ThisController();
			IF (c # NIL) & (c IS Controller) THEN RETURN c(Controller)
			ELSE RETURN NIL
			END
		ELSE RETURN NIL
		END
	END Focus;


	PROCEDURE SetCaret* (text: TextModels.Model; pos: INTEGER);
	(** pre: text # NIL,  pos = none  OR  0 <= pos <= text.Length() **)
		VAR cm: SetCaretMsg;
	BEGIN
		ASSERT(text # NIL, 20); ASSERT(none <= pos, 21); ASSERT(pos <= text.Length(), 22);
		cm.pos := pos; Models.Broadcast(text, cm)
	END SetCaret;

	PROCEDURE SetSelection* (text: TextModels.Model; beg, end: INTEGER);
	(** pre: text # NIL,  beg = end  OR  0 <= beg < end <= text.Length() **)
		VAR sm: SetSelectionMsg;
	BEGIN
		ASSERT(text # NIL, 20);
		IF beg # end THEN
			ASSERT(0 <= beg, 21); ASSERT(beg < end, 22); ASSERT(end <= text.Length(), 23)
		END;
		sm.beg := beg; sm.end := end; Models.Broadcast(text, sm)
	END SetSelection;


	(* support for cursor/selection/focus marking *)

	PROCEDURE BlinkCaret (c: StdCtrl; f: Views.Frame; tick: LONGINT);
		VAR vis: BOOLEAN;
	BEGIN
		IF (c.carPos # none) & f.front & (tick >= c.carTick) THEN
			IF c.carVisible THEN
				c.MarkCaret(f, Containers.hide); c.carVisible := FALSE
			ELSE
				c.carVisible := TRUE; c.MarkCaret(f, Containers.show)
			END;
			c.carTick := tick + Dialog.caretPeriod
		END
	END BlinkCaret;

	PROCEDURE FlipCaret (c: StdCtrl; show: BOOLEAN);
		VAR msg: CaretMsg;
	BEGIN
		msg.show := show;
		Views.Broadcast(c.view, msg)
	END FlipCaret;

	PROCEDURE CheckCaret (c: StdCtrl);
		VAR text: TextModels.Model; len, pos: INTEGER;
	BEGIN
		IF ~(Containers.noCaret IN c.opts) THEN
			IF (c.carPos = none) & ~(boundCaret & (c.selBeg # c.selEnd)) & (c.ThisFocus() = NIL) THEN
				text := c.text; len := text.Length(); pos := c.carLast; 
				IF pos < 0 THEN pos := 0 ELSIF pos > len THEN pos := len END;
				(* c.carVisible := FALSE; c.carTick := 0;	(* force visible mark *) *)
				SetCaret(text, pos)
			END
		ELSE c.carPos := none
		END
	END CheckCaret;



	PROCEDURE HiliteRect (f: Views.Frame; l, t, r, b, s: INTEGER; show: BOOLEAN);
	BEGIN
		IF s = Ports.fill THEN
			f.MarkRect(l, t, r, b, Ports.fill, Ports.hilite, show)
		ELSE
			f.MarkRect(l, t, r - s, t + s, s, Ports.hilite, show);
			f.MarkRect(l, t + s, l + s, b - s, s, Ports.hilite, show);
			f.MarkRect(l + s, b - s, r, b, s, Ports.hilite, show);
			f.MarkRect(r - s, t + s, r, b - s, s, Ports.hilite, show)
		END
	END HiliteRect;

	PROCEDURE MarkSelRange (c: StdCtrl; f: Views.Frame; b, e: TextViews.Location;
		front, show: BOOLEAN
	);
		VAR fw, ff, r, t: INTEGER;
	BEGIN
		IF front THEN fw := 0; ff := Ports.fill ELSE fw := f.dot; ff := fw END;
		IF b.start # e.start THEN
			r := f.r; t := b.y + b.asc + b.dsc;
			HiliteRect(f, b.x, b.y, r + fw, t + fw, ff, show);
			IF t < e.y THEN HiliteRect(f, 0, t, r + fw, e.y + fw, ff, show) END;
			b.x := f.l; b.y := e.y
		END;
		HiliteRect(f, b.x, b.y, e.x + fw, e.y + e.asc + e.dsc + fw, ff, show)
	END MarkSelRange;

	PROCEDURE MarkSelection (c: StdCtrl; f: Views.Frame; beg, end: INTEGER; show: BOOLEAN);
		VAR b, e: TextViews.Location; s: Views.View; 
	BEGIN
		IF (beg # end) & f.mark THEN
			ASSERT(beg < end, 20);
			s := c.Singleton();
			IF s # NIL THEN
				IF beg + 1 = end THEN Containers.MarkSingleton(c, f, show) END
			ELSE
				c.view.GetThisLocation(f, beg, b); c.view.GetThisLocation(f, end, e);
				IF (b.pos < e.pos) OR (b.pos = e.pos) & (b.x < e.x) THEN
					MarkSelRange(c, f, b, e, f.front, show)
				END
			END
		END
	END MarkSelection;

	PROCEDURE FlipSelection (c: StdCtrl; beg, end: INTEGER; show: BOOLEAN);
		VAR msg: SelectionMsg;
	BEGIN
		msg.beg := beg; msg.end := end; msg.show := show;
		Views.Broadcast(c.view, msg)
	END FlipSelection;


	PROCEDURE InitMarks (c: StdCtrl);
	BEGIN
		c.autoBeg := MAX(INTEGER); c.autoEnd := 0;
		c.carPos := none; c.carVisible := FALSE; c.carLast := none; c.carTick := 0; c.carX := -1;
		c.selBeg := none; c.selEnd := none;
		c.lastStep := 0
	END InitMarks;

	PROCEDURE AutoShowRange (c: StdCtrl; beg, end: INTEGER);
	BEGIN
		IF (beg <= c.autoBeg) & (c.autoEnd <= end) THEN
			c.autoBeg := beg; c.autoEnd := end	(* new range includes old range: expand *)
		ELSE
			c.autoBeg := -1	(* schizopheric scroll request -> don't scroll at all *)
		END
	END AutoShowRange;

	PROCEDURE UpdateMarks (c: StdCtrl; op: INTEGER; beg, end, delta: INTEGER);
	(* ensure that marks are valid after updates *)
	BEGIN
		CASE op OF
		  TextModels.insert:
			c.carLast := end; c.selBeg := end; c.selEnd := end; beg := end
		| TextModels.delete:
			c.carLast := beg; c.selBeg := beg; c.selEnd := beg; end := beg
		| TextModels.replace:
		ELSE
			HALT(100)
		END;
		AutoShowRange(c, beg, end)
	END UpdateMarks;


	(* support for smart cut/copy/paste and attributing *)

	PROCEDURE LegalChar (ch: CHAR): BOOLEAN;
	BEGIN
		IF ch < 100X THEN
			CASE ORD(ch) OF
				ORD(viewcode),
				ORD(tab), ORD(line), ORD(para),
				ORD(" ") .. 7EH, 80H .. 0FFH: RETURN TRUE
			ELSE RETURN FALSE
			END
		ELSE RETURN TRUE
		END
	END LegalChar;

	PROCEDURE LeftTerminator (ch: CHAR): BOOLEAN;
	BEGIN
		IF ch < 100X THEN
			CASE ch OF
				viewcode, tab, line, para, '"', "'", "(", "[", "{": RETURN TRUE
			ELSE RETURN FALSE
			END
		ELSE RETURN TRUE
		END
	END LeftTerminator;

	PROCEDURE RightTerminator (ch, ch1: CHAR): BOOLEAN;
	BEGIN
		IF ch < 100X THEN
			CASE ch OF
			  0X, viewcode, tab, line, para,
			  "!", '"', "'", "(", ")", ",", ";", "?", "[", "]", "{", "}": RETURN TRUE
			| ".", ":":
				CASE ch1 OF
				  0X, viewcode, tab, line, para, " ": RETURN TRUE
				ELSE RETURN FALSE
				END
			ELSE RETURN FALSE
			END
		ELSE RETURN TRUE
		END
	END RightTerminator;

	PROCEDURE ReadLeft (rd: TextModels.Reader; pos: INTEGER; OUT ch: CHAR);
	BEGIN
		IF pos > 0 THEN rd.SetPos(pos - 1); rd.ReadChar(ch)
		ELSE rd.SetPos(pos); ch := " "
		END
	END ReadLeft;

	PROCEDURE SmartRange (c: StdCtrl; VAR beg, end: INTEGER);
	(* if possible and whole words are covered,
	extend [beg, end) to encompass either a leading or a trailing blank *)
		VAR rd: TextModels.Reader; we, be: INTEGER; ch, ch0, ch1: CHAR; rightTerm: BOOLEAN;
	BEGIN
(*
disable intelligent delete/cut/move for now
		rd := CachedReader(c); ReadLeft(rd, beg, ch0); rd.ReadChar(ch);
		IF ((ch0 <= " ") OR LeftTerminator(ch0)) & (ch # " ") THEN
			(* range covers beg of word *)
			we := beg; be := beg;
			WHILE (ch # 0X) & (be <= end) DO
				ch1 := ch; rd.ReadChar(ch); INC(be);
				IF (ch1 # " ") & ((be <= end) OR ~RightTerminator(ch1, ch)) THEN we := be END
			END;
			rightTerm := RightTerminator(ch1, ch);
			IF (beg < we) & (we = end) & ((we < be) OR rightTerm) THEN
				(* range covers end of word *)
				IF (we < be) & (ch1 = " ") THEN
					INC(end)	(* include trailing blank *)
				ELSIF (beg > 0) & rightTerm & (ch0 = " ") THEN
					DEC(beg)	(* include leading blank *)
				END
			END
		END;
		CacheReader(c, rd)
*)
	END SmartRange;

	PROCEDURE OnlyWords (c: StdCtrl; beg, end: INTEGER): BOOLEAN;
		VAR rd: TextModels.Reader; we, be: INTEGER; ch, ch0, ch1: CHAR;
			rightTerm, words: BOOLEAN;
	BEGIN
		words := FALSE;
		rd := CachedReader(c); ReadLeft(rd, beg, ch0); rd.ReadChar(ch);
		IF ((ch0 <= " ") OR LeftTerminator(ch0)) & (ch # " ") THEN	(* range covers beg of word *)
			we := beg; be := beg;
			WHILE (ch # 0X) & (be <= end) DO
				ch1 := ch; rd.ReadChar(ch); INC(be);
				IF (ch1 # " ") & ((be <= end) OR ~RightTerminator(ch1, ch)) THEN
					we := be
				END
			END;
			rightTerm := RightTerminator(ch1, ch);
			IF (beg < we) & (we = end) & ((we < be) OR rightTerm) THEN	(* range covers end of word *)
				words := TRUE
			END
		END;
		CacheReader(c, rd);
		RETURN words
	END OnlyWords;

	PROCEDURE GetTargetField (t: TextModels.Model; pos: INTEGER;
		VAR touchL, touchM, touchR: BOOLEAN
	);
		VAR rd: TextModels.Reader; ch0, ch1: CHAR; leftTerm, rightTerm: BOOLEAN;
	BEGIN
		rd := t.NewReader(NIL); ReadLeft(rd, pos, ch0); rd.ReadChar(ch1);
		leftTerm := (ch0 <= " ") OR LeftTerminator(ch0);
		rightTerm := (ch1 <= " ") OR RightTerminator(ch1, 0X);
		touchL := ~leftTerm & rightTerm;
		touchM := ~leftTerm & ~rightTerm;
		touchR := leftTerm & ~rightTerm
	END GetTargetField;

	PROCEDURE LeftExtend (t: TextModels.Model; attr: TextModels.Attributes);
		VAR wr: TextModels.Writer;
	BEGIN
		wr := t.NewWriter(NIL); wr.SetAttr(attr); wr.SetPos(0); wr.WriteChar(" ")
	END LeftExtend;

	PROCEDURE RightExtend (t: TextModels.Model; attr: TextModels.Attributes);
		VAR wr: TextModels.Writer;
	BEGIN
		wr := t.NewWriter(NIL); wr.SetPos(t.Length()); wr.SetAttr(attr); wr.WriteChar(" ")
	END RightExtend;

	PROCEDURE MergeAdjust (target, inset: TextModels.Model; pos: INTEGER; OUT start: INTEGER);
		VAR rd: TextModels.Reader; a: TextModels.Attributes; ch, ch1: CHAR;
			touchL, touchM, touchR: BOOLEAN;
	BEGIN
		start := pos;
(*
disable intelligent paste for now
		GetTargetField(target, pos, touchL, touchM, touchR);
		IF touchL THEN
			rd := inset.NewReader(NIL); rd.SetPos(0);
			rd.ReadChar(ch); a := rd.attr; rd.ReadChar(ch1);
			IF (ch > " ") & ~RightTerminator(ch, ch1) THEN LeftExtend(inset, a); INC(start) END
		END;
		IF touchR & (inset.Length() > 0) THEN
			rd := inset.NewReader(rd); rd.SetPos(inset.Length() - 1); rd.ReadChar(ch);
			IF (ch > " ") & ~LeftTerminator(ch) THEN RightExtend(inset, rd.attr) END
		END
*)
	END MergeAdjust;


	PROCEDURE InsertionAttr (c: StdCtrl): TextModels.Attributes;
		VAR rd: TextModels.Reader; r: TextRulers.Ruler; a: TextModels.Attributes; ch: CHAR;
	BEGIN
		a := c.insAttr;
		IF a = NIL THEN
			rd := CachedReader(c); a := NIL;
			IF c.carPos # none THEN
				ReadLeft(rd, c.carPos, ch); a := rd.attr;
				IF ((ch <= " ") OR (ch = TextModels.nbspace)) & (c.carPos < c.text.Length()) THEN
					rd.ReadChar(ch);
					IF ch > " " THEN a := rd.attr END
				END
			ELSIF boundCaret & (c.selBeg # c.selEnd) THEN
				rd.SetPos(c.selBeg); rd.ReadChar(ch); a := rd.attr;
				c.insAttr := a
			END;
			IF a = NIL THEN c.view.PollDefaults(r, a) END;
			CacheReader(c, rd)
		END;
		RETURN a
	END InsertionAttr;


	PROCEDURE GetTargetRange (c: StdCtrl; OUT beg, end: INTEGER);
	BEGIN
		IF boundCaret & (c.selBeg # c.selEnd) THEN
			beg := c.selBeg; end := c.selEnd
		ELSE
			beg := c.carPos; end := beg
		END
	END GetTargetRange;


	PROCEDURE DoEdit (name: Stores.OpName;
		c: StdCtrl; beg, end: INTEGER;
		attr: TextModels.Attributes; ch: CHAR; view: Views.View; w, h: INTEGER;
		buf: TextModels.Model; bufbeg, bufend: INTEGER;	(* buf # NIL & bufend < 0: bufend = buf.Length() *)
		pos: INTEGER
	);
		VAR script: Stores.Operation; wr: TextModels.Writer; cluster: BOOLEAN;
	BEGIN
		IF (beg < end)	(* something to delete *)
		OR (attr # NIL)	(* something new to write *)
		OR (buf # NIL)	(* something new to insert *)
		THEN
			cluster := (beg < end) OR (attr = NIL) OR (view # NIL);
			(* don't script when typing a single character -> TextModels will bunch if possible *)
			(* ~cluster => name is reverted to #System.Inserting by TextModels *)
			IF cluster THEN Models.BeginScript(c.text, name, script) END;
			IF beg < end THEN
				c.text.Delete(beg, end);
				IF pos > beg THEN DEC(pos, end - beg) END
			END;
			IF attr # NIL THEN
				ASSERT(buf = NIL, 20);
				wr := CachedWriter(c, attr); wr.SetPos(pos);
				IF view # NIL THEN wr.WriteView(view, w, h) ELSE wr.WriteChar(ch) END;
				CacheWriter(c, wr)
			ELSIF buf # NIL THEN
				IF bufend < 0 THEN bufend := buf.Length() END;
				c.text.Insert(pos, buf, bufbeg, bufend)
			END;
			IF cluster THEN Models.EndScript(c.text, script) END;
			CheckCaret(c)
		END
	END DoEdit;


	(* editing *)

	PROCEDURE ThisPos (v: TextViews.View; f: Views.Frame; x, y: INTEGER): INTEGER;
		VAR loc: TextViews.Location; pos: INTEGER;
	BEGIN
		pos := v.ThisPos(f, x, y); v.GetThisLocation(f, pos, loc);
		IF (loc.view # NIL) & (x > (loc.l + loc.r) DIV 2) THEN INC(pos) END;
		RETURN pos
	END ThisPos;

	PROCEDURE ShowPos (c: StdCtrl; beg, end: INTEGER);
	BEGIN
		IF ~(noAutoScroll IN c.opts) THEN
			c.view.ShowRange(beg, end, TextViews.focusOnly)
		END
	END ShowPos;


	PROCEDURE Indentation (c: StdCtrl; pos: INTEGER): TextModels.Model;
	(* pre: c.carPos # none *)
		VAR st: TextSetters.Setter; buf: TextModels.Model; rd: TextModels.Reader;
			wr: TextModels.Writer; ch: CHAR; spos: INTEGER;
	BEGIN
		buf := NIL;
		rd := CachedReader(c);
		st := c.view.ThisSetter(); spos := st.ThisSequence(pos); rd.SetPos(spos); rd.ReadChar(ch);
		IF (ch = tab) & (spos < pos) THEN
			buf := TextModels.CloneOf(c.text); wr := buf.NewWriter(NIL); wr.SetPos(buf.Length());
			wr.SetAttr(InsertionAttr(c));
			wr.WriteChar(line);
			REPEAT wr.WriteChar(tab); rd.ReadChar(ch) UNTIL (ch # tab) OR (rd.Pos() > pos)
		END;
		CacheReader(c, rd);
		RETURN buf
	END Indentation;

	PROCEDURE InsertChar (c: StdCtrl; ch: CHAR);
		VAR buf: TextModels.Model; attr: TextModels.Attributes;
			beg, end: INTEGER; legal: BOOLEAN; name: Stores.OpName;
	BEGIN
		attr := NIL; buf := NIL;
		IF ch < 100X THEN legal := LegalChar(ch) ELSE legal := TRUE END;	(* should check Unicode *)
		IF (ch = ldel) OR (ch = rdel) THEN name := deletingKey ELSE name := replacingKey END;
		IF boundCaret & (c.selBeg # c.selEnd) & (legal OR (ch = ldel) OR (ch = rdel) OR (ch = enter)) THEN
			beg := c.selBeg; end := c.selEnd;
			IF (ch = ldel) OR (ch = rdel) THEN SmartRange(c, beg, end); ch := 0X END
		ELSE
			beg := c.carPos; end := beg
		END;
		IF (c.carPos # none) OR boundCaret & (c.selBeg # c.selEnd) THEN
			IF (ch = line) OR (ch = enter) THEN
				IF noAutoIndent IN c.opts THEN buf := NIL ELSE buf := Indentation(c, beg) END;
				IF buf = NIL THEN ch := line; legal := TRUE ELSE ch := 0X; legal := FALSE END
			END;
			IF legal THEN
				attr := InsertionAttr(c)
			ELSIF (ch = ldel) & (c.carPos > 0) THEN
				beg := c.carPos - 1; end := c.carPos
			ELSIF (ch = rdel) & (c.carPos < c.text.Length()) THEN
				beg := c.carPos; end := c.carPos + 1
			END
		END;
		DoEdit(name, c, beg, end, attr, ch, NIL, 0, 0, buf, 0, -1, beg)
	END InsertChar;

	PROCEDURE InsertText (c: StdCtrl; beg, end: INTEGER; text: TextModels.Model; OUT start: INTEGER);
		VAR buf: TextModels.Model;
	BEGIN
		buf := TextModels.CloneOf(text); buf.InsertCopy(0, text, 0, text.Length());
		IF beg = end THEN MergeAdjust(c.text, buf, beg, start) ELSE start := beg END;
		DoEdit(insertingKey, c, beg, end, NIL, 0X, NIL, 0, 0, buf, 0, -1, beg)
	END InsertText;

	PROCEDURE InsertView (c: StdCtrl; beg, end: INTEGER; v: Views.View; w, h: INTEGER);
	BEGIN
		DoEdit(insertingKey, c, beg, end, InsertionAttr(c), 0X, v, w, h, NIL, 0, 0, beg)
	END InsertView;


	PROCEDURE InSubFrame (f, f1: Views.Frame; x, y: INTEGER): BOOLEAN;
	BEGIN
		INC(x, f.gx - f1.gx); INC(y, f.gy - f1.gy);
		RETURN (f1.l <= x) & (x < f1.r) & (f1.t <= y) & (y < f1.b)
	END InSubFrame;

	PROCEDURE InFrame (f: Views.Frame; x, y: INTEGER): BOOLEAN;
	BEGIN
		RETURN (f.l <= x) & (x < f.r) & (f.t <= y) & (y < f.b)
	END InFrame;


	(* filtered tracking *)

	PROCEDURE IsFilter (v: Views.View; c: StdCtrl; f: Views.Frame; x, y: INTEGER): BOOLEAN;
		VAR pref: FilterPref;
	BEGIN
		pref.controller := c; pref.frame := f; pref.x := x; pref.y := y;
		pref.filter := FALSE;
		Views.HandlePropMsg(v, pref);
		RETURN pref.filter
	END IsFilter;

	PROCEDURE FindFilter (c: StdCtrl; f: Views.Frame; x, y: INTEGER; OUT filter: Views.View);
		CONST catchRange = 1000;
		VAR rd: TextModels.Reader; pos, beg, end: INTEGER; isF: BOOLEAN;
	BEGIN
		c.view.GetRange(f, beg, end); DEC(beg, catchRange);
		pos := c.view.ThisPos(f, x, y);
		IF pos < c.text.Length() THEN INC(pos) END;	(* let filter handle itself *)
		rd := CachedReader(c); rd.SetPos(pos);
		REPEAT
			rd.ReadPrevView(filter);
			isF := (filter # NIL) & IsFilter(filter, c, f, x, y);
		UNTIL isF OR rd.eot OR (rd.Pos() < beg);
		IF ~isF THEN filter := NIL END;
		CacheReader(c, rd)
	END FindFilter;

	PROCEDURE FilteredPollCursor (c: StdCtrl; f: Views.Frame;
		VAR msg: Controllers.PollCursorMsg; VAR done: BOOLEAN
	);
		VAR filter, focus: Views.View; x, y: INTEGER; modifiers: SET; isDown: BOOLEAN; fmsg: FilterPollCursorMsg;
	BEGIN
		FindFilter(c, f, msg.x, msg.y, filter);
		IF filter # NIL THEN
			(* f.Input(x, y, modifiers, isDown); *)
			fmsg.x := msg.x; fmsg.y := msg.y; fmsg.cursor := msg.cursor;
			fmsg.controller := c; fmsg.done := FALSE;
			(*Views.ForwardCtrlMsg(f, fmsg) - does not work f.view # filter !!*)
			focus := NIL;
			filter.HandleCtrlMsg(f, fmsg, focus);
			IF fmsg.done THEN msg.cursor := fmsg.cursor END;
			done := fmsg.done
		END
	END FilteredPollCursor;

	PROCEDURE FilteredTrack (c: StdCtrl; f: Views.Frame;
		VAR msg: Controllers.TrackMsg; VAR done: BOOLEAN
	);
		VAR filter, focus: Views.View; fmsg: FilterTrackMsg;
	BEGIN
		FindFilter(c, f, msg.x, msg.y, filter);
		IF filter # NIL THEN
			fmsg.x := msg.x; fmsg.y := msg.y; fmsg.modifiers := msg.modifiers;
			fmsg.controller := c; fmsg.done := FALSE;
			(*Views.ForwardCtrlMsg(f, fmsg) - does not work f.view # filter !!*)
			focus := NIL; filter.HandleCtrlMsg(f, fmsg, focus);
			done := fmsg.done
		END
	END FilteredTrack;


	(* StdCtrl *)

	PROCEDURE (c: StdCtrl) Internalize2 (VAR rd: Stores.Reader);
		VAR thisVersion: INTEGER;
	BEGIN
		c.Internalize2^(rd);
		IF rd.cancelled THEN RETURN END;
		rd.ReadVersion(minVersion, maxStdVersion, thisVersion);
		IF rd.cancelled THEN RETURN END;
		InitMarks(c)
	END Internalize2;

	PROCEDURE (c: StdCtrl) Externalize2 (VAR wr: Stores.Writer);
	BEGIN
		c.Externalize2^(wr);
		wr.WriteVersion(maxStdVersion)
	END Externalize2;

	PROCEDURE (c: StdCtrl) CopyFrom (source: Stores.Store);
	BEGIN
		c.CopyFrom^(source); InitMarks(c)
	END CopyFrom;

	PROCEDURE (c: StdCtrl) Neutralize2;
	BEGIN
		(* c.Neutralize^; *)
		c.SetCaret(none)
	END Neutralize2;

	PROCEDURE (c: StdCtrl) GetContextType (OUT type: Stores.TypeName);
	BEGIN
		type := "TextViews.View"
	END GetContextType;

	PROCEDURE (c: StdCtrl) GetValidOps (OUT valid: SET);
	BEGIN
		valid := {};
		IF (c.carPos # none) OR (boundCaret & (c.selBeg # c.selEnd)) THEN
			valid := valid + {Controllers.pasteChar, Controllers.paste}
		END;
		IF c.selBeg # c.selEnd THEN
			valid := valid + {Controllers.cut, Controllers.copy}
		END
	END GetValidOps;

	PROCEDURE (c: StdCtrl) NativeModel (m: Models.Model): BOOLEAN;
	BEGIN
		ASSERT(m # NIL, 20);
		RETURN m IS TextModels.Model
	END NativeModel;

	PROCEDURE (c: StdCtrl) NativeView (v: Views.View): BOOLEAN;
	BEGIN
		ASSERT(v # NIL, 20);
		RETURN v IS TextViews.View
	END NativeView;

	PROCEDURE (c: StdCtrl) NativeCursorAt (f: Views.Frame; x, y: INTEGER): INTEGER;
	BEGIN
		RETURN Ports.textCursor
	END NativeCursorAt;

	PROCEDURE (c: StdCtrl) PollNativeProp (selection: BOOLEAN;
		VAR p: Properties.Property; VAR truncated: BOOLEAN
	);
		VAR beg, end: INTEGER;
	BEGIN
		IF selection & (c.selBeg = c.selEnd) THEN
			p := InsertionAttr(c).Prop(); truncated := FALSE
		ELSE
			IF selection THEN beg := c.selBeg; end := c.selEnd
			ELSE beg := 0; end := c.text.Length()
			END;
(*
			truncated := (end - beg > lenCutoff);
			IF truncated THEN end := beg + lenCutoff END;
*)
			p := c.text.Prop(beg, end)
		END
	END PollNativeProp;

	PROCEDURE (c: StdCtrl) SetNativeProp (selection: BOOLEAN; old, p: Properties.Property);
		VAR t: TextModels.Model; beg, end: INTEGER;
	BEGIN
		t := c.text;
		IF selection THEN beg := c.selBeg; end := c.selEnd ELSE beg := 0; end := t.Length() END;
		IF beg < end THEN
			t.Modify(beg, end, old, p);
			IF selection THEN c.SetSelection(beg, end) END
		ELSIF selection THEN
			c.insAttr := TextModels.ModifiedAttr(InsertionAttr(c), p)
		END
	END SetNativeProp;

	PROCEDURE (c: StdCtrl) MakeViewVisible (v: Views.View);
		VAR pos: INTEGER;
	BEGIN
		ASSERT(v # NIL, 20);
		ASSERT(v.context # NIL, 21);
		ASSERT(v.context.ThisModel() = c.text, 22);
		pos := v.context(TextModels.Context).Pos();
		ShowPos(c, pos, pos + 1)
	END MakeViewVisible;

	PROCEDURE (c: StdCtrl) GetFirstView (selection: BOOLEAN; OUT v: Views.View);
		VAR rd: TextModels.Reader; beg, end: INTEGER;
	BEGIN
		IF selection THEN beg := c.selBeg; end := c.selEnd
		ELSE beg := 0; end := c.text.Length()
		END;
		IF beg < end THEN
			rd := CachedReader(c); rd.SetPos(beg); rd.ReadView(v);
			IF rd.Pos() > end THEN v := NIL END;
			CacheReader(c, rd)
		ELSE v := NIL
		END
	END GetFirstView;

	PROCEDURE (c: StdCtrl) GetNextView (selection: BOOLEAN; VAR v: Views.View);
		VAR con: Models.Context; rd: TextModels.Reader; beg, end, pos: INTEGER;
	BEGIN
		ASSERT(v # NIL, 20); con := v.context;
		ASSERT(con # NIL, 21); ASSERT(con.ThisModel() = c.text, 22);
		IF selection THEN beg := c.selBeg; end := c.selEnd
		ELSE beg := 0; end := c.text.Length()
		END;
		pos := con(TextModels.Context).Pos();
		IF (beg <= pos) & (pos < end) THEN
			rd := CachedReader(c); rd.SetPos(pos + 1); rd.ReadView(v);
			IF rd.Pos() > end THEN v := NIL END;
			CacheReader(c, rd)
		ELSE v := NIL
		END
	END GetNextView;

	PROCEDURE (c: StdCtrl) GetPrevView (selection: BOOLEAN; VAR v: Views.View);
		VAR con: Models.Context; rd: TextModels.Reader; beg, end, pos: INTEGER;
	BEGIN
		ASSERT(v # NIL, 20); con := v.context;
		ASSERT(con # NIL, 21); ASSERT(con.ThisModel() = c.text, 22);
		IF selection THEN beg := c.selBeg; end := c.selEnd
		ELSE beg := 0; end := c.text.Length()
		END;
		pos := con(TextModels.Context).Pos();
		IF (beg < pos) & (pos <= end) THEN
			rd := CachedReader(c); rd.SetPos(pos); rd.ReadPrevView(v);
			IF rd.Pos() < beg THEN v := NIL END;
			CacheReader(c, rd)
		ELSE v := NIL
		END
	END GetPrevView;

	PROCEDURE (c: StdCtrl) GetSelectionBounds (f: Views.Frame; OUT x, y, w, h: INTEGER);
		VAR b, e: TextViews.Location;
	BEGIN
		c.GetSelectionBounds^(f, x, y, w, h);
		IF w = Views.undefined THEN
			c.view.GetThisLocation(f, c.selBeg, b);
			c.view.GetThisLocation(f, c.selEnd, e);
			IF b.start = e.start THEN x := b.x; w := e.x - b.x;
			ELSE x := f.l; w := f.r - f.l;
			END;
			y := b.y; h := e.y + e.asc + e.dsc - b.y
		END
	END GetSelectionBounds;

	PROCEDURE (c: StdCtrl) MarkPickTarget (source, f: Views.Frame;
		sx, sy, x, y: INTEGER; show: BOOLEAN
	);
		VAR b, e: TextViews.Location; pos: INTEGER;
	BEGIN
		pos := c.view.ThisPos(f, x, y);
		IF pos < c.text.Length() THEN
			c.view.GetThisLocation(f, pos, b);
			c.view.GetThisLocation(f, pos + 1, e);
			IF (b.pos < e.pos) OR (b.pos = e.pos) & (b.x < e.x) THEN
				MarkSelRange(c, f, b, e, TRUE, show)
			END
		END
	END MarkPickTarget;

	PROCEDURE (c: StdCtrl) MarkDropTarget (source, f: Views.Frame;
		sx, sy, dx, dy, w, h, rx, ry: INTEGER; type: Stores.TypeName; isSingle, show: BOOLEAN
	);
		VAR loc: TextViews.Location; pos: INTEGER;
	BEGIN
		pos := c.view.ThisPos(f, dx, dy);
		IF (source # NIL) & ((source.view = f.view) OR (source.view.ThisModel() = f.view.ThisModel()))
		& (c.selBeg < pos) & (pos < c.selEnd) THEN
			pos := c.selBeg
		END;
		c.view.GetThisLocation(f, pos, loc);
		f.MarkRect(loc.x, loc.y, loc.x + f.unit, loc.y + loc.asc + loc.dsc, Ports.fill, Ports.invert, show);
		IF (isSingle OR ~Services.Extends(type, "TextViews.View")) & (w > 0) & (h > 0) THEN
			DEC(dx, rx); DEC(dy, ry);
			f.MarkRect(dx, dy, dx + w, dy + h, 0, Ports.dim25, show)
		END
	END MarkDropTarget;


	PROCEDURE GetThisLine (c: StdCtrl; pos: INTEGER; OUT beg, end: INTEGER);
		VAR st: TextSetters.Setter;
	BEGIN
		st := c.view.ThisSetter();
		beg := st.ThisLine(pos); end := st.NextLine(beg);
		IF end = beg THEN end := c.text.Length() END;
	END GetThisLine;

	PROCEDURE GetThisChunk (c: StdCtrl; f: Views.Frame;
		VAR s: TrackState; OUT beg, end: INTEGER; OUT mode: INTEGER
	);
		VAR v: TextViews.View; b, e: TextViews.Location;
			st: TextSetters.Setter; ruler: TextRulers.Ruler; ra: TextRulers.Attributes;
			pos, r: INTEGER;
	BEGIN
		v := c.view; st := v.ThisSetter(); pos := ThisPos(v, f, s.x, s.y);
		ruler := TextViews.ThisRuler(v, pos); ra := ruler.style.attr;
		r := ra.right; IF ~(TextRulers.rightFixed IN ra.opts) OR (r > f.r) THEN r := f.r END;
		st.GetWord(pos, beg, end);
		v.GetThisLocation(f, beg, b); v.GetThisLocation(f, end, e);
		IF (s.x < f.l) OR (s.x >= r) THEN	(* outside of line box: whole line *)
			GetThisLine(c, pos, beg, end);
			mode := lines
		ELSIF (s.y < b.y) OR (s.y < b.y + b.asc + b.dsc) & (s.x < b.x)
		OR (s.y >= e.y) & (s.x >= e.x) OR (s.y >= e.y + e.asc + e.dsc) THEN
			(* outside of word: single char *)
			beg := ThisPos(v, f, s.x, s.y); v.GetThisLocation(f, beg, b);
			IF (b.x > s.x) & (beg > 0) THEN DEC(beg) END;
			IF beg < c.text.Length() THEN end := beg + 1 ELSE end := beg END;
			mode := words
		ELSE	(* whole word *)
			mode := words
		END
	END GetThisChunk;

	PROCEDURE SetSel (c: StdCtrl; beg, end: INTEGER);
	(* pre: ~(Containers.noSelection IN c.opts) *)
	BEGIN
		IF beg >= end THEN c.SetCaret(beg) ELSE c.SetSelection(beg, end) END
	END SetSel;

	PROCEDURE PrepareToTrack (c: StdCtrl; f: Views.Frame;
		VAR s: TrackState; mode: INTEGER;
		VAR pin0, pin1, pos: INTEGER
	);
		VAR loc: TextViews.Location; beg, end: INTEGER; m: INTEGER;
	BEGIN
		pos := ThisPos(c.view, f, s.x, s.y);
		IF mode IN {chars, words, lines} THEN
			GetThisChunk(c, f, s, pin0, pin1, m)
		ELSE pin0 := pos; pin1 := pos
		END;
		IF s.toggle & ((c.selBeg # c.selEnd) OR boundCaret & (c.carPos # none))
		& ~(Containers.noSelection IN c.opts) THEN	(* modify existing selection *)
			IF c.selBeg # c.selEnd THEN
				beg := c.selBeg; end := c.selEnd
			ELSE
				beg := c.carPos; end := beg; c.selPin0 := beg; c.selPin1 := beg
			END;
			IF pin1 > c.selPin0 THEN
				end := pin1; pin0 := beg
			ELSIF pin0 < c.selPin1 THEN
				beg := pin0; pin0 := end
			END;
			SetSel(c, beg, end);
			pin1 := pin0
		ELSIF mode IN {chars, words, lines} THEN
			SetSel(c, pin0, pin1);
			pos := pin1
		ELSE
			SetCaret(c.text, pos)
		END;
		c.lastStep := Services.Ticks()
	END PrepareToTrack;

	PROCEDURE ScrollDelay (d: INTEGER): INTEGER;
		VAR second, delay: INTEGER;
	BEGIN
		second := Services.resolution;
		IF d < 2 * mm THEN delay := second DIV 2
		ELSIF d < 4 * mm THEN delay := second DIV 3
		ELSIF d < 6 * mm THEN delay := second DIV 5
		ELSIF d < 8 * mm THEN delay := second DIV 10
		ELSE delay := second DIV 20
		END;
		RETURN delay
	END ScrollDelay;

	PROCEDURE ScrollWhileTracking (c: StdCtrl; f: Views.Frame; VAR x0, y0, x, y: INTEGER);
	(* currently, there are no provisions to scroll while tracking inside an embedded view *)
		VAR now: LONGINT; (* normalize: BOOLEAN; *) scr: Controllers.ScrollMsg;
	BEGIN
		(* normalize := c.view.context.Normalize(); *)
		now := Services.Ticks();
		IF x < f.l THEN x0 := x; x := f.l ELSIF x > f.r THEN x0 := x; x := f.r END;
		IF (y < f.t) (* & normalize*) THEN
			IF c.lastStep + ScrollDelay(f.t - y) <= now THEN
				c.lastStep := now;
				scr.focus := TRUE; scr.vertical := TRUE; scr.op := Controllers.decLine;
				scr.done := FALSE;
				Controllers.ForwardVia(Controllers.frontPath, scr)
			END
		ELSIF (y > f.b) (* & normalize *) THEN
			IF c.lastStep + ScrollDelay(y - f.b) <= now THEN
				c.lastStep := now;
				scr.focus := TRUE; scr.vertical := TRUE; scr.op := Controllers.incLine;
				scr.done := FALSE;
				Controllers.ForwardVia(Controllers.frontPath, scr)
			END
		ELSE
			y0 := y
		END
	END ScrollWhileTracking;

	PROCEDURE (c: StdCtrl) TrackMarks (f: Views.Frame; x, y: INTEGER; units, extend, add: BOOLEAN);
		VAR s: TrackState; pos, beg, end, pin0, pin1, p, p1: INTEGER;
			modifiers: SET; mode, m: INTEGER; isDown, noSel: BOOLEAN;
	BEGIN
		IF c.opts * Containers.mask # Containers.mask THEN	(* track caret or selection *)
			s.x := x; s.y := y; s.toggle := extend;
			noSel := Containers.noSelection IN c.opts;
			IF units & ~noSel THEN	(* select units, i.e. words or lines *)
				GetThisChunk(c, f, s, beg, end, mode)
			ELSE	(* set caret or selection *)
				mode := none
			END;
			PrepareToTrack(c, f, s, mode, pin0, pin1, p); x := s.x; y := s.y;
			beg := pin0; end := pin1;
			IF p < pin0 THEN beg := p ELSIF p > pin1 THEN end := p END;
			p := -1;
			f.Input(s.x, s.y, modifiers, isDown);
			WHILE isDown DO
(*
			REPEAT
				f.Input(s.x, s.y, modifiers, isDown);
*)
				IF (s.x # x) OR (s.y # y) THEN
					ScrollWhileTracking(c, f, x, y, s.x, s.y);
					p1 := ThisPos(c.view, f, s.x, s.y);
					IF p1 # p THEN
						p := p1;
						IF mode IN {words, lines} THEN
							IF mode = words THEN
								GetThisChunk(c, f, s, beg, end, m)
							ELSE
								GetThisLine(c, p, beg, end)
							END;
							IF p > pin0 THEN pos := end ELSE pos := beg END
						ELSE pos := p
						END;
						beg := pin0; end := pin1;
						IF noSel THEN
							c.SetCaret(pos)
						ELSE
							IF pos < pin0 THEN beg := pos ELSIF pos > pin1 THEN end := pos END;
							SetSel(c, beg, end);
							IF c.selPin0 = c.selPin1 THEN
								IF pos < pin0 THEN c.selPin0 := pos; c.selPin1 := pin1
								ELSIF pos > pin1 THEN c.selPin0 := pin0; c.selPin1 := pos
								END
							END
						END
					END
				END;
				f.Input(s.x, s.y, modifiers, isDown)
			END
(*
			UNTIL ~isDown
*)
		END
	END TrackMarks;

	PROCEDURE (c: StdCtrl) Resize (v: Views.View; l, t, r, b: INTEGER);
		VAR con: Models.Context;
	BEGIN
		ASSERT(v # NIL, 20); con := v.context;
		ASSERT(con # NIL, 21); ASSERT(con.ThisModel() = c.text, 22);
		con.SetSize(r - l, b - t)
	END Resize;

	PROCEDURE (c: StdCtrl) DeleteSelection;
		VAR beg, end: INTEGER;
	BEGIN
		beg := c.selBeg; end := c.selEnd;
		IF beg # end THEN
			SmartRange(c, beg, end);
			DoEdit(deletingKey, c, beg, end, NIL, 0X, NIL, 0, 0, NIL, 0, 0, 0)
		END
	END DeleteSelection;

	PROCEDURE (c: StdCtrl) MoveLocalSelection (f, dest: Views.Frame; x, y, dx, dy: INTEGER);
		VAR buf: TextModels.Model; pos, beg0, end0, beg, end, start, len: INTEGER;
	BEGIN
		pos := dest.view(TextViews.View).ThisPos(dest, dx, dy);
(* smart move disabled for now --> use true move instead of copy
		beg0 := c.selBeg; end0 := c.selEnd; beg := beg0; end := end0;
		SmartRange(c, beg, end);
		IF (beg < pos) & (pos < end) THEN pos := beg END;
		buf := TextModels.CloneOf(c.text); buf.CopyFrom(0, c.text, beg0, end0);
		IF OnlyWords(c, beg0, end0) THEN MergeAdjust(c.text, buf, pos, start) ELSE start := pos END;
		len := end0 - beg0;
		IF start >= end THEN DEC(start, end - beg) END;
		IF pos # beg THEN
			DoEdit(movingKey, c, beg, end, NIL, 0X, NIL, 0, 0, buf, pos);
			SetSelection(c.text, start, start + len);
			AutoShowRange(c, start, start + len)
		END
*)
		beg := c.selBeg; end := c.selEnd;
		IF (pos < beg) OR (pos > end) THEN
			len := end - beg; start := pos;
			IF start >= end THEN DEC(start, len) END;
			DoEdit(movingKey, c, 0, 0, NIL, 0X, NIL, 0, 0, c.text, beg, end, pos);
			SetSelection(c.text, start, start + len);
			AutoShowRange(c, start, start + len)
		END
	END MoveLocalSelection;

	PROCEDURE (c: StdCtrl) CopyLocalSelection (f, dest: Views.Frame; x, y, dx, dy: INTEGER);
		VAR buf: TextModels.Model; pos, beg, end, start, len: INTEGER;
	BEGIN
		pos := dest.view(TextViews.View).ThisPos(dest, dx, dy);
		beg := c.selBeg; end := c.selEnd;
		IF (beg < pos) & (pos < end) THEN pos := beg END;
		buf := TextModels.CloneOf(c.text); buf.InsertCopy(0, c.text, beg, end);
		IF OnlyWords(c, beg, end) THEN MergeAdjust(c.text, buf, pos, start) ELSE start := pos END;
		len := end - beg;
		DoEdit(copyingKey, c, 0, 0, NIL, 0X, NIL, 0, 0, buf, 0, -1, pos);
		SetSelection(c.text, start, start + len);
		AutoShowRange(c, start, start + len)
	END CopyLocalSelection;

	PROCEDURE (c: StdCtrl) SelectionCopy (): Containers.Model;
		VAR t: TextModels.Model;
	BEGIN
		IF c.selBeg # c.selEnd THEN
			t := TextModels.CloneOf(c.text); t.InsertCopy(0, c.text, c.selBeg, c.selEnd);
		ELSE t := NIL
		END;
		RETURN t
	END SelectionCopy;

	PROCEDURE (c: StdCtrl) NativePaste (m: Models.Model; f: Views.Frame);
		VAR beg, end, start: INTEGER;
	BEGIN
		WITH m: TextModels.Model DO
			GetTargetRange(c, beg, end);
			IF beg # none THEN InsertText(c, beg, end, m, start) END
		END
	END NativePaste;

	PROCEDURE (c: StdCtrl) ArrowChar (f: Views.Frame; ch: CHAR; units, select: BOOLEAN);
		VAR st: TextSetters.Setter; v: TextViews.View; loc: TextViews.Location;
			org, len,  p, pos,  b, e,  beg, end,  d, d0, edge,  x, dy: INTEGER;
			change, rightEdge, rightDir: BOOLEAN;
			scroll: Controllers.ScrollMsg;
	BEGIN
		c.insAttr := NIL;
		Models.StopBunching(c.text);
		v := c.view; st := v.ThisSetter();
		change := select OR (c.selBeg = c.selEnd);
		IF c.selBeg # c.selEnd THEN beg := c.selBeg; end := c.selEnd
		ELSE beg := c.carPos; end := beg; c.carLast := beg
		END;
		len := c.text.Length();
		rightDir := (ch = aR) OR (ch = pR) OR (ch = dR) OR (ch = aD) OR (ch = pD) OR (ch = dD);
		rightEdge := 	change & (c.carLast < end)
							OR rightDir & (~change OR (beg = end) & (c.carLast = end));
		IF rightEdge THEN edge := end ELSE edge := beg END;
		ShowPos(c, edge, edge);
		b := beg; e := end; d := edge; d0 := edge;
		CASE ch OF
		| aL:
			IF units THEN
				p := d; e := d;
				WHILE (p > 0) & ((edge = d) OR (edge = e)) DO DEC(p); st.GetWord(p, edge, e) END;
			ELSIF change THEN DEC(edge)
			END
		| pL, dL:
			v.GetThisLocation(f, edge, loc); edge := loc.start
		| aR:
			IF units THEN
				p := d; e := edge;
				WHILE (p < len) & ((edge <= d) OR (edge = e)) DO INC(p); st.GetWord(p, edge, e) END
			ELSIF change THEN INC(edge)
			END
		| pR, dR:
			v.GetThisLocation(f, edge, loc); p := st.NextLine(loc.start);
			IF p = loc.start THEN p := len ELSE DEC(p) END;
			IF p > edge THEN edge := p END
		| aU:
			IF units THEN
				p := st.ThisSequence(edge);
				IF p < edge THEN edge := p ELSE edge := st.PreviousSequence(edge) END
			ELSE
				v.PollOrigin(org, dy); v.GetThisLocation(f, edge, loc);
				IF c.lastX >= 0 THEN x := c.lastX ELSE x := loc.x END;
				c.carX := x;
				IF loc.start > 0 THEN
					edge := v.ThisPos(f, x, loc.y - 1);
					IF (edge >= loc.start) & (org > 0) THEN
						v.SetOrigin(org - 1, 0);
						v.GetThisLocation(f, edge, loc);
						edge := v.ThisPos(f, x, loc.y - 1)
					END
				END
			END
		| pU:
			v.PollOrigin(org, dy);
			IF edge > org THEN edge := org
			ELSIF org > 0 THEN
				scroll.focus := TRUE; scroll.vertical := TRUE; scroll.op := Controllers.decPage;
				scroll.done := FALSE;
				Views.ForwardCtrlMsg(f, scroll);
				v.PollOrigin(edge, dy)
			END
		| dU:
			edge := 0
		| aD:
			IF units THEN
				p := st.NextSequence(st.ThisSequence(edge));
				IF p > edge THEN edge := p ELSE edge := st.NextSequence(p) END
			ELSE
				v.GetThisLocation(f, edge, loc);
				IF c.lastX >= 0 THEN x := c.lastX ELSE x := loc.x END;
				c.carX := x;
				edge := v.ThisPos(f, x, loc.y + loc.asc + loc.dsc + 1)
			END
		| pD:
			v.GetRange(f, b, e);
			IF e < len THEN
				scroll.focus := TRUE; scroll.vertical := TRUE; scroll.op := Controllers.incPage;
				scroll.done := FALSE;
				Views.ForwardCtrlMsg(f, scroll);
				v.GetRange(f, edge, e)
			ELSE edge := len
			END
		| dD:
			edge := len
		END;
		IF rightEdge THEN end := edge ELSE beg := edge END;
		IF ~select THEN
			IF rightDir THEN beg := edge ELSE end := edge END
		END;
		IF beg < 0 THEN beg := 0 ELSIF beg > len THEN beg := len END;
		IF end < beg THEN end := beg ELSIF end > len THEN end := len END;
		IF beg = end THEN
			ShowPos(c, beg, end)
		ELSE
			IF rightEdge THEN ShowPos(c, end - 1, end) ELSE ShowPos(c, beg, beg + 1) END
		END;
		SetSel(c, beg, end)
	END ArrowChar;

	PROCEDURE (c: StdCtrl) ControlChar (f: Views.Frame; ch: CHAR);
	BEGIN
		InsertChar(c, ch)
	END ControlChar;

	PROCEDURE (c: StdCtrl) PasteChar (ch: CHAR);
	BEGIN
		InsertChar(c, ch)
	END PasteChar;

	PROCEDURE (c: StdCtrl) PasteView (f: Views.Frame; v: Views.View; w, h: INTEGER);
		VAR t: TextModels.Model; pos, start, beg, end, len: INTEGER;
	BEGIN
		GetTargetRange(c, beg, end);
		IF beg # none THEN InsertView(c, beg, end, v, w, h) END
	END PasteView;

	PROCEDURE (c: StdCtrl) Drop (src, f: Views.Frame; sx, sy, x, y, w, h, rx, ry: INTEGER;
		v: Views.View; isSingle: BOOLEAN
	);
		VAR t: TextModels.Model; pos, start, beg, end, len: INTEGER;
	BEGIN
		pos := ThisPos(c.view, f, x, y);
		WITH v: TextViews.View DO t := v.ThisModel() ELSE t := NIL END;
		IF (t # NIL) & ~isSingle THEN
			InsertText(c, pos, pos, t, start); len := t.Length()
		ELSE
			InsertView(c, pos, pos, v, w, h); start := pos; len := 1
		END;
		SetSelection(c.text, start, start + len);
		AutoShowRange(c, start, start + len)
	END Drop;

	PROCEDURE (c: StdCtrl) PickNativeProp (f: Views.Frame; x, y: INTEGER; VAR p: Properties.Property);
		VAR rd: TextModels.Reader;
	BEGIN
		rd := CachedReader(c); rd.SetPos(ThisPos(c.view, f, x, y)); rd.Read;
		IF ~rd.eot THEN p := rd.attr.Prop() ELSE p := NIL END;
		CacheReader(c, rd)
	END PickNativeProp;

	PROCEDURE (c: StdCtrl) HandleModelMsg (VAR msg: Models.Message);
		VAR done: BOOLEAN;
	BEGIN
		c.HandleModelMsg^(msg);
		IF msg.model = c.text THEN
			WITH msg: Models.UpdateMsg DO
				WITH msg: TextModels.UpdateMsg DO
					CASE msg.op OF
					  TextModels.insert, TextModels.delete, TextModels.replace:
						UpdateMarks(c, msg.op, msg.beg, msg.end, msg.delta)
					ELSE	(* unknown text op happened *)
						c.view.Neutralize
					END
				ELSE	(* unknown text update happened *)
					c.view.Neutralize
				END
			| msg: ModelMessage DO
				WITH msg: SetCaretMsg DO
					c.SetCaret(msg.pos)
				| msg: SetSelectionMsg DO
					c.SetSelection(msg.beg, msg.end)
				ELSE
				END
			ELSE
			END
		END
	END HandleModelMsg;

	PROCEDURE (c: StdCtrl) HandleViewMsg (f: Views.Frame; VAR msg: Views.Message);
	BEGIN
		c.HandleViewMsg^(f, msg);
		IF msg.view = c.view THEN
			WITH msg: ViewMessage DO
				WITH msg: CaretMsg DO
					c.MarkCaret(f, msg.show)
				| msg: SelectionMsg DO
					MarkSelection(c, f, msg.beg, msg.end, msg.show)
				END
			ELSE
			END
		END
	END HandleViewMsg;

	PROCEDURE (c: StdCtrl) HandleCtrlMsg (f: Views.Frame;
		VAR msg: Controllers.Message; VAR focus: Views.View
	);
		VAR g: Views.Frame; beg, end: INTEGER; done: BOOLEAN;
	BEGIN
		IF (msg IS Controllers.MarkMsg) OR (msg IS Controllers.TickMsg) THEN
			beg := c.autoBeg; end := c.autoEnd;
			c.autoBeg := MAX(INTEGER); c.autoEnd := 0
		END;
		WITH msg: Controllers.TickMsg DO
			IF ~(noAutoScroll IN c.opts)
				& (0 <= beg) & (beg <= end) & (end <= c.text.Length())
				& c.view.context.Normalize()
			THEN
				c.view.ShowRange(beg, end, TextViews.focusOnly)
			END;
			IF focus = NIL THEN
				CheckCaret(c); BlinkCaret(c, f, msg.tick);
				IF (c.selBeg # c.aliasSelBeg) OR (c.selEnd # c.aliasSelEnd) THEN
					(* lazy update of text-synchronous alias marks *)
					c.aliasSelBeg := c.selBeg; c.aliasSelEnd := c.selEnd;
					SetSelection(c.text, c.selBeg, c.selEnd)
				END
			END
		| msg: Controllers.MarkMsg DO
			c.carX := -1;
			IF msg.show THEN c.carVisible := TRUE; c.carTick := 0 END
		| msg: Controllers.TrackMsg DO
			c.insAttr := NIL; c.carX := -1; Models.StopBunching(c.text)
		| msg: Controllers.EditMsg DO
			c.lastX := c.carX; c.carX := -1;
			IF focus = NIL THEN CheckCaret(c) END
		| msg: Controllers.ReplaceViewMsg DO
			c.carX := -1
		| msg: Controllers.TransferMessage DO
			c.carX := -1
		| msg: Properties.EmitMsg DO
			c.carX := -1
		ELSE
		END;
		done := FALSE;
		WITH msg: Controllers.CursorMessage DO
			IF TRUE (* Containers.noCaret IN c.opts *) THEN	(* mask or browser mode *)
				g := Views.FrameAt(f, msg.x, msg.y);
				IF (g = NIL) OR IsFilter(g.view, c, f, msg.x, msg.y) THEN
					WITH msg: Controllers.PollCursorMsg DO
						FilteredPollCursor(c, f, msg, done)
					| msg: Controllers.TrackMsg DO
						FilteredTrack(c, f, msg, done)
					ELSE
					END
				END
			END
		ELSE
		END;
		IF ~done THEN c.HandleCtrlMsg^(f, msg, focus) END
	END HandleCtrlMsg;


	(* caret *)

	PROCEDURE (c: StdCtrl) HasCaret (): BOOLEAN;
	BEGIN
		RETURN c.carPos # none
	END HasCaret;

	PROCEDURE (c: StdCtrl) MarkCaret (f: Views.Frame; show: BOOLEAN);
		CONST carW = 1; carMinH = 7;	(* in frame dots *)
		VAR loc: TextViews.Location; pos, beg, end,  u, x, y, w, h: INTEGER; fm: INTEGER;
	BEGIN
		pos := c.carPos;
		IF (pos # none) & f.mark & (f.front & c.carVisible OR ~f.front) THEN
			c.view.GetRange(f, beg, end);
			IF (beg <= pos) & (pos <= end) THEN
				u := f.dot;
				c.view.GetThisLocation(f, pos, loc);
				IF f.front THEN fm := Ports.invert ELSE fm := Ports.dim50 END;
				x := loc.x; y := loc.y; h := loc.asc + loc.dsc;
				IF Dialog.thickCaret THEN w := 2 * carW * u ELSE w := carW * u END;
				IF x >= f.r - w THEN DEC(x, w) END;
				IF h < carMinH * u THEN h := carMinH * u END;	(* special caret in lines of (almost) zero height *)
				f.MarkRect(x, y, x + w, y + h, Ports.fill, fm, show)
			END
		END
	END MarkCaret;

	PROCEDURE (c: StdCtrl) CaretPos (): INTEGER;
	BEGIN
		RETURN c.carPos
	END CaretPos;

	PROCEDURE (c: StdCtrl) SetCaret (pos: INTEGER);
	BEGIN
		ASSERT(none <= pos, 20); ASSERT(pos <= c.text.Length(), 21);
		c.insAttr := NIL;
		IF pos # c.carPos THEN
			IF (pos # none) & (c.carPos = none) THEN
				IF boundCaret THEN c.SetSelection(none, none) END;
				c.SetFocus(NIL)
			END;

			IF Containers.noCaret IN c.opts THEN pos := none END;
			IF c.carPos # none THEN
				c.carLast := c.carPos; FlipCaret(c, Containers.hide)
			END;
			c.carPos := pos;
			IF pos # none THEN
				c.carVisible := TRUE; c.carTick := Services.Ticks() + Dialog.caretPeriod; FlipCaret(c, Containers.show)
			END
		END
	END SetCaret;


	(* selection *)

	PROCEDURE (c: StdCtrl) HasSelection (): BOOLEAN;
	BEGIN
		RETURN c.selBeg # c.selEnd
	END HasSelection;

	PROCEDURE (c: StdCtrl) Selectable (): BOOLEAN;
	BEGIN
		RETURN c.text.Length() > 0
	END Selectable;

	PROCEDURE (c: StdCtrl) SetSingleton (s: Views.View);
		VAR s0: Views.View;
	BEGIN
		s0 := c.Singleton();
		c.SetSingleton^(s);
		s := c.Singleton();
		IF s # s0 THEN
			c.insAttr := NIL;
			IF s # NIL THEN
				c.selBeg := s.context(TextModels.Context).Pos(); c.selEnd := c.selBeg + 1;
				c.selPin0 := c.selBeg; c.selPin1 := c.selEnd
			ELSE c.selBeg := none; c.selEnd := none
			END
		END
	END SetSingleton;

	PROCEDURE (c: StdCtrl) SelectAll (select: BOOLEAN);
	(** extended by subclass to include intrinsic selections **)
	BEGIN
		IF select THEN c.SetSelection(0, c.text.Length()) ELSE c.SetSelection(none, none) END
	END SelectAll;

	PROCEDURE (c: StdCtrl) InSelection (f: Views.Frame; x, y: INTEGER): BOOLEAN;
	(* pre: c.selBeg # c.selEnd *)
	(* post: (x, y) in c.selection *)
		VAR b, e: TextViews.Location; y0, y1, y2, y3: INTEGER;
	BEGIN
		c.view.GetThisLocation(f, c.selBeg, b); y0 := b.y; y1 := y0 + b.asc + b.dsc;
		c.view.GetThisLocation(f, c.selEnd, e); y2 := e.y; y3 := y2 + e.asc + e.dsc;
		RETURN ((y >= y0) & (x >= b.x) OR (y >= y1)) & ((y < y2) OR (y < y3) & (x < e.x))
	END InSelection;

	PROCEDURE (c: StdCtrl) MarkSelection (f: Views.Frame; show: BOOLEAN);
	BEGIN
		MarkSelection(c, f, c.selBeg, c.selEnd, show)
	END MarkSelection;

	PROCEDURE (c: StdCtrl) GetSelection (OUT beg, end: INTEGER);
	BEGIN
		beg := c.selBeg; end := c.selEnd
	END GetSelection;

	PROCEDURE (c: StdCtrl) SetSelection (beg, end: INTEGER);
		VAR t: TextModels.Model; rd: TextModels.Reader;
			beg0, end0, p: INTEGER; singleton: BOOLEAN;
	BEGIN
		t := c.text; ASSERT(t # NIL, 20);
		IF Containers.noSelection IN c.opts THEN end := beg
		ELSIF beg # end THEN
			ASSERT(0 <= beg, 21); ASSERT(beg < end, 22); ASSERT(end <= t.Length(), 23)
		END;
		beg0 := c.selBeg; end0 := c.selEnd;
		c.insAttr := NIL;
		IF (beg # beg0) OR (end # end0) THEN
			IF (beg # end) & (c.selBeg = c.selEnd) THEN
				IF boundCaret THEN
					IF c.carPos = end THEN p := c.carPos ELSE p := beg END;
					c.SetCaret(none); c.carLast := p
				END;
				c.SetFocus(NIL);
				c.selPin0 := beg; c.selPin1 := end
			ELSIF boundCaret & (beg = end) THEN
				c.selPin1 := c.selPin0	(* clear selection anchors *)
			END;
			IF beg + 1 = end THEN
				rd := CachedReader(c);
				rd.SetPos(beg); rd.Read; singleton := rd.view # NIL;
				CacheReader(c, rd)
			ELSE singleton := FALSE
			END;
			IF singleton THEN	(* native or singleton -> singleton *)
				IF rd.view # c.Singleton() THEN c.SetSingleton(rd.view) END
			ELSIF c.Singleton() # NIL THEN	(* singleton -> native *)
				c.SetSingleton(NIL);
				c.selBeg := beg; c.selEnd := end;
				FlipSelection(c, beg, end, Containers.show)
			ELSE	(* native -> native *)
				c.selBeg := beg; c.selEnd := end;
				IF (beg0 <= beg) & (end <= end0) THEN	(* reduce *)
					p := end0; end0 := beg; beg := end; end := p
				ELSIF (beg <= beg0) & (end0 <= end) THEN	(* extend *) 
					p := end; end := beg0; beg0 := end0; end0 := p
				ELSIF (beg <= beg0) & (beg0 <= end) THEN	(* shift left *)
					p := end; end := beg0; beg0 := p
				ELSIF (end >= end0) & (beg <= end0) THEN	(* shift right *)
					p := end0; end0 := beg; beg := p
				END;
				IF beg0 < end0 THEN FlipSelection(c, beg0, end0, Containers.show) END;
				IF beg < end THEN FlipSelection(c, beg, end, Containers.show) END
			END
		END
	END SetSelection;


	(* StdDirectory *)

	PROCEDURE (d: StdDirectory) NewController (opts: SET): Controller;
		VAR c: StdCtrl;
	BEGIN
		NEW(c); c.SetOpts(opts); InitMarks(c); RETURN c
	END NewController;


	PROCEDURE Init;
		VAR d: StdDirectory;
	BEGIN
		NEW(d); dir := d; stdDir := d
	END Init;

BEGIN
	Init
END TextControllers.
