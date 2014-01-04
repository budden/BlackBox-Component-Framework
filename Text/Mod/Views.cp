MODULE TextViews;
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

(* could use +, $ in DrawLine cache implementation *)

	IMPORT
		Services, Fonts, Ports, Stores,
		Models, Views, Controllers, Properties, Dialog, Printing, Containers,
		TextModels, TextRulers, TextSetters;

	CONST
		(** v.DisplayMarks hide *)
		show* = FALSE; hide* = TRUE;

		(** v.ShowRange focusOnly **)
		any* = FALSE; focusOnly* = TRUE;

		parasign = 0B6X;	(* paragraph sign, to mark non-ruler paragraph breaks *)

		mm = Ports.mm; inch16 = Ports.inch DIV 16; point = Ports.point;
		maxScrollHeight = 16 * point; maxScrollSteps = 100; fuseScrollHeight = maxScrollHeight DIV 2;
		maxHeight = maxScrollHeight * maxScrollSteps;
		adjustMask = {TextRulers.leftAdjust, TextRulers.rightAdjust};

		(* SetOp.mode *)
		setMarks = 0; setSetter = 1; setDefs = 2;

		scrollingKey = "#System:Scrolling";
		viewSettingKey = "#System:ViewSetting";

		minVersion = 0; maxVersion = 0; maxStdVersion = 0;


	TYPE
		View* = POINTER TO ABSTRACT RECORD (Containers.View) END;

		Directory* = POINTER TO ABSTRACT RECORD
			defAttr-: TextModels.Attributes
		END;


		Location* = RECORD
			(** start of line and position of location **)
			start*, pos*: INTEGER;
			(** coordinates of location **)
			x*, y*: INTEGER;
			(** line dimensions at location **)
			asc*, dsc*: INTEGER;
			(** if view at location: **)
			view*: Views.View;
			l*, t*, r*, b*: INTEGER
		END;


		PositionMsg* = RECORD (Models.Message)
			focusOnly*: BOOLEAN;
			beg*, end*: INTEGER
		END;


		PageMsg* = RECORD (Properties.Message)
			current*: INTEGER
		END;


		Line = POINTER TO RECORD
			next: Line;
			start, asc, h: INTEGER;
			attr: TextRulers.Attributes;	(* attr = box.ruler.style.attr *)
			box: TextSetters.LineBox	(* caching of box.rpos not consistent *)
		END;

		StdView = POINTER TO RECORD (View)
			(* model *)
			text: TextModels.Model;
			org: INTEGER;
			dy: INTEGER;	(* 0 <= dy < Height(first line) *)
			defRuler: TextRulers.Ruler;
			defAttr: TextModels.Attributes;
			hideMarks: BOOLEAN;
			(* general state *)
			cachedRd: TextSetters.Reader;
			(* line grid cache *)
			trailer: Line;	(* trailer # NIL => trailer.eot, trailer.next # trailer *)
			bot: INTEGER;	(* max(f : f seen by Restore : f.b) *)
			(* setter *)
			setter, setter0: TextSetters.Setter	(* setter # setter0 lazily detects setter change *)
		END;

		StdDirectory = POINTER TO RECORD (Directory) END;

		ScrollOp = POINTER TO RECORD (Stores.Operation)
			v: StdView;
			org, dy: INTEGER;
			bunchOrg, bunchDy: INTEGER;
			bunch: BOOLEAN;	(* bunch => bunchOrg, bunchDy valid *)
			silent: BOOLEAN	(* original caller of Do(op) already handled situation *)
		END;

		SetOp = POINTER TO RECORD (Stores.Operation)
			mode: INTEGER;
			view: StdView;
			hideMarks: BOOLEAN;
			setter: TextSetters.Setter;
			defRuler: TextRulers.Ruler;
			defAttr: TextModels.Attributes
		END;

		FindAnyFrameMsg = RECORD (Views.Message)
			(* find frame with smallest height (frame.b - frame.t) that displays view; NIL if none found *)
			frame: Views.Frame	(* OUT, initially NIL *)
		END;

		FindFocusFrameMsg = RECORD (Controllers.Message)
			(* find outermost focus frame displaying view; NIL if none found *)
			view: Views.View;	(* IN *)
			frame: Views.Frame	(* OUT, initially NIL *)
		END;


	VAR
		ctrlDir-: Containers.Directory;
		dir-, stdDir-: Directory;


	(* forward used in GetStart, UpdateView, ShowRangeIn *)
	PROCEDURE ^ DoSetOrigin (v: StdView; org, dy: INTEGER; silent: BOOLEAN);


	(** View **)

	PROCEDURE (v: View) Internalize2- (VAR rd: Stores.Reader), EXTENSIBLE;
	(** pre: ~v.init **)
	(** post: v.init **)
		VAR thisVersion: INTEGER;
	BEGIN
		(*v.Internalize^(rd);*)
		IF rd.cancelled THEN RETURN END;
		rd.ReadVersion(minVersion, maxVersion, thisVersion)
	END Internalize2;

	PROCEDURE (v: View) Externalize2- (VAR wr: Stores.Writer), EXTENSIBLE;
	(** pre: v.init **)
	BEGIN
		(*v.Externalize^(wr);*)
		wr.WriteVersion(maxVersion)
	END Externalize2;

	PROCEDURE (v: View) ThisModel* (): TextModels.Model, EXTENSIBLE;
		VAR m: Containers.Model;
	BEGIN
		m := v.ThisModel^();
		IF m # NIL THEN
			RETURN m(TextModels.Model)
		ELSE
			RETURN NIL
		END
	END ThisModel;

	PROCEDURE (v: View) DisplayMarks* (hide: BOOLEAN), NEW, ABSTRACT;
	PROCEDURE (v: View) HidesMarks* (): BOOLEAN, NEW, ABSTRACT;
	PROCEDURE (v: View) SetSetter* (setter: TextSetters.Setter), NEW, ABSTRACT;
	PROCEDURE (v: View) ThisSetter* (): TextSetters.Setter, NEW, ABSTRACT;
	PROCEDURE (v: View) SetOrigin* (org, dy: INTEGER), NEW, ABSTRACT;
	(** post: org = ThisLine(org) => v.org = org, v.dy = dy; else v.org = ThisLine(org), v.dy = 0 **)

	PROCEDURE (v: View) PollOrigin* (OUT org, dy: INTEGER), NEW, ABSTRACT;
	PROCEDURE (v: View) SetDefaults* (r: TextRulers.Ruler; a: TextModels.Attributes),
		NEW, ABSTRACT;
	(** pre: r.init, a.init **)

	PROCEDURE (v: View) PollDefaults* (OUT r: TextRulers.Ruler; OUT a: TextModels.Attributes),
		NEW, ABSTRACT;
	PROCEDURE (v: View) GetThisLocation* (f: Views.Frame; pos: INTEGER; OUT loc: Location),
		NEW, ABSTRACT;

	PROCEDURE (v: View) GetRect* (f: Views.Frame; view: Views.View; OUT l, t, r, b: INTEGER);
		VAR con: Models.Context; loc: Location; pos: INTEGER;
	BEGIN
		con := view.context;
		ASSERT(con # NIL, 20); ASSERT(con.ThisModel() = v.ThisModel(), 21);
		pos := con(TextModels.Context).Pos();
		v.GetThisLocation(f, pos, loc);
		IF loc.view = view THEN
			l := loc.l; t := loc.t; r := loc.r; b := loc.b
		ELSE
			l := MAX(INTEGER); t := MAX(INTEGER); r := l; b := t
		END
	END GetRect;

	PROCEDURE (v: View) GetRange* (f: Views.Frame; OUT beg, end: INTEGER), NEW, ABSTRACT;
	(** post: beg = beg of first visible line, end = end of last visible line **)

	PROCEDURE (v: View) ThisPos* (f: Views.Frame; x, y: INTEGER): INTEGER, NEW, ABSTRACT;
	PROCEDURE (v: View) ShowRangeIn* (f: Views.Frame; beg, end: INTEGER), NEW, ABSTRACT;
	PROCEDURE (v: View) ShowRange* (beg, end: INTEGER; focusOnly: BOOLEAN), NEW, ABSTRACT;
	(** post: in all frames (resp. in front or otherwise target frame if focusOnly):
		if possible, first visible pos <= k <= last visible pos,
		with k = beg if beg = end and beg <= k < end otherwise **)


	(** Directory **)

	PROCEDURE (d: Directory) Set* (defAttr: TextModels.Attributes), NEW, EXTENSIBLE;
	BEGIN
		ASSERT(defAttr # NIL, 20); ASSERT(defAttr.init, 21);
		d.defAttr := defAttr
	END Set;

	PROCEDURE (d: Directory) New* (text: TextModels.Model): View, NEW, ABSTRACT;


	(** miscellaneous **)

	PROCEDURE SetCtrlDir* (d: Containers.Directory);
	BEGIN
		ASSERT(d # NIL, 20); ctrlDir := d
	END SetCtrlDir;

	PROCEDURE SetDir* (d: Directory);
	BEGIN
		ASSERT(d # NIL, 20); dir := d
	END SetDir;


	PROCEDURE Focus* (): View;
		VAR v: Views.View;
	BEGIN
		v := Controllers.FocusView();
		IF (v # NIL) & (v IS View) THEN RETURN v(View) ELSE RETURN NIL END
	END Focus;

	PROCEDURE FocusText* (): TextModels.Model;
		VAR v: View;
	BEGIN
		v := Focus();
		IF v # NIL THEN RETURN v.ThisModel() ELSE RETURN NIL END
	END FocusText;

	PROCEDURE Deposit*;
	BEGIN
		Views.Deposit(dir.New(NIL))
	END Deposit;


	PROCEDURE ShowRange* (text: TextModels.Model; beg, end: INTEGER; focusOnly: BOOLEAN);
	(** post: in all front or target frames displaying a view displaying t:
		if possible, first visible pos <= k <= last visible pos,
		with k = beg if beg = end and beg <= k < end otherwise **)
		VAR pm: PositionMsg;
	BEGIN
		ASSERT(text # NIL, 20);
		pm.beg := beg; pm.end := end; pm.focusOnly := focusOnly;
		Models.Broadcast(text, pm)
	END ShowRange;


	PROCEDURE ThisRuler* (v: View; pos: INTEGER): TextRulers.Ruler;
		VAR r: TextRulers.Ruler; a: TextModels.Attributes; rpos: INTEGER;
	BEGIN
		v.PollDefaults(r, a); rpos := -1; TextRulers.GetValidRuler(v.ThisModel(), pos, -1, r, rpos);
		RETURN r
	END ThisRuler;


	(* auxiliary procedures *)

	PROCEDURE GetReader (v: StdView; start: INTEGER; IN box: TextSetters.LineBox
	): TextSetters.Reader;
		VAR st: TextSetters.Setter; rd: TextSetters.Reader;
	BEGIN
		ASSERT(box.ruler # NIL, 100);
		st := v.ThisSetter();
		rd := v.cachedRd; v.cachedRd := NIL;	(* reader recycling *)
		rd := st.NewReader(rd);
		rd.Set(rd.r, v.text, box.left, start, box.ruler, box.rpos, st.vw, st.hideMarks);
		RETURN rd
	END GetReader;

	PROCEDURE CacheReader (v: StdView; rd: TextSetters.Reader);
	BEGIN
		ASSERT(v.cachedRd = NIL, 20);
		v.cachedRd := rd
	END CacheReader;


	(* line descriptors *)

	PROCEDURE SetLineAsc (st: TextSetters.Setter; t: Line; dsc: INTEGER);
	(* pre: dsc: descender of previous line (-1 if first line) *)
	BEGIN
		t.asc := t.box.asc + st.GridOffset(dsc, t.box);
		t.h := t.asc + t.box.dsc
	END SetLineAsc;

	PROCEDURE NewLine (st: TextSetters.Setter; start, dsc: INTEGER): Line;
	(* pre: start: start of line to measure; dsc: descender of previous line (-1 if first line) *)
		VAR t: Line;
	BEGIN
		NEW(t); st.GetLine(start, t.box);
		t.start := start; SetLineAsc(st, t, dsc);
		t.attr := t.box.ruler.style.attr;
		RETURN t
	END NewLine;

	PROCEDURE AddLine (st: TextSetters.Setter; VAR t: Line; VAR start, y: INTEGER);
	BEGIN
		t.next := NewLine(st, start, t.box.dsc); t := t.next;
		INC(start, t.box.len); INC(y, t.h)
	END AddLine;

	PROCEDURE InitLines (v: StdView);
		VAR asc, dsc, w: INTEGER; t0, t: Line; start, y: INTEGER;
	BEGIN
		v.defAttr.font.GetBounds(asc, dsc, w);
		NEW(t0); start := v.org; y := v.dy;
		t0.box.dsc := -1;	(* dsc = -1: trailer.next is first line *)
		t := t0; AddLine(v.ThisSetter(), t, start, y); t.next := t0;	(* at least one valid line desc *)
		t0.start := start; t0.asc := asc; t0.h := asc + dsc;	(* trailer.(asc, h) for caret display following last line *)
		t0.attr := NIL;
		t0.box.eot := TRUE; t0.box.len := 0;
		t0.box.ruler := NIL;
		t0.box.left := -1;	(* make trailer async to every other line *)
		v.trailer := t0; v.bot := 0
	END InitLines;

	PROCEDURE ExtendLines (v: StdView; bot: INTEGER);
		VAR st: TextSetters.Setter; t0, t: Line; start, y: INTEGER;
	BEGIN
		IF bot >= v.bot THEN
			t0 := v.trailer; start := t0.start;
			y := v.dy; t := t0; WHILE t.next # t0 DO t := t.next; INC(y, t.h) END;
			IF (y < bot) & ~t.box.eot THEN
				st := v.ThisSetter();
				REPEAT AddLine(st, t, start, y) UNTIL (y >= bot) OR t.box.eot;
				t.next := t0; t0.start := start
			END;
			v.bot := bot
		END
	END ExtendLines;

	PROCEDURE ReduceLines (v: StdView; bot: INTEGER);
		VAR t0, t: Line; y: INTEGER;
	BEGIN
		IF bot <= v.bot THEN
			t0 := v.trailer; y := v.dy;
			t := t0; WHILE (t.next # t0) & (y < bot) DO t := t.next; INC(y, t.h) END;
			t0.start := t.next.start; t.next := t0;
			v.bot := bot
		END
	END ReduceLines;

	PROCEDURE ValidateLines (v: StdView; bot: INTEGER);
		VAR st: TextSetters.Setter; w, h, len: INTEGER;
	BEGIN
		IF v.setter # NIL THEN
			v.context.GetSize(w, h);	(* possibly adapt to changed width *)
			IF v.setter.vw # w THEN v.setter0 := NIL; v.trailer := NIL END
		END;
		len := v.text.Length();
		IF (v.org > len) OR (v.trailer # NIL) & (v.trailer.start > len) THEN v.trailer := NIL END;
		IF v.trailer = NIL THEN
			IF v.org > len THEN v.org := len END;
			st := v.ThisSetter(); v.org := st.ThisLine(v.org);
			InitLines(v)
		END;
		ExtendLines(v, bot)
	END ValidateLines;

	PROCEDURE PrependLines (v: StdView);
		VAR st: TextSetters.Setter;  t0, t1, t: Line; start, y: INTEGER;
	BEGIN
		t0 := v.trailer; start := v.org; y := v.dy;
		IF t0.start # start THEN
			st := v.ThisSetter();
			t := t0; t1 := t0.next;
			WHILE (t1.start # start) & (y < v.bot) DO AddLine(st, t, start, y) END;
			IF y >= v.bot THEN
				t.next := t0; t0.start := start
			ELSE
				t.next := t1;
				IF t1 # v.trailer THEN SetLineAsc(st, t1, t.box.dsc) END
			END
		END
	END PrependLines;


	(* update frame after insert/delete/replace *)

	PROCEDURE ThisViewLine (v: StdView; y: INTEGER): Line;
	(* pre: 0 <= y < v.bot *)
		VAR t: Line; py: INTEGER;
	BEGIN
		t := v.trailer.next; py := v.dy;
		WHILE ~t.box.eot & (py + t.h < y) DO INC(py, t.h); t := t.next END;
		RETURN t
	END ThisViewLine;

	PROCEDURE LocateThisLine (v: StdView; start: INTEGER; OUT t: Line; OUT y: INTEGER);
		VAR t1: Line;
	BEGIN
		t := v.trailer.next; y := v.dy;
		t1 := v.trailer.next;
		WHILE t.start # start DO INC(y, t.h); t := t.next; ASSERT(t # t1, 100) END
	END LocateThisLine;

	PROCEDURE GetStart (st: TextSetters.Setter; v: StdView; beg: INTEGER; OUT start: INTEGER);
	(* find start of line containing beg after text change; tuned using valid line descs *)
		VAR s, t: Line;
	BEGIN
		s := v.trailer; t := s.next;
		WHILE (t # v.trailer) & (t.start + t.box.len < beg) DO s := t; t := s.next END;
		IF s # v.trailer THEN	(* at least first line desc possibly still valid *)
			start := st.NextLine(s.start);	(* NextLine can be much cheaper than ThisLine *)
			IF start # t.start THEN
				GetStart(st, v, s.start, start)
			ELSIF ~t.box.eot & (start + t.box.len = beg) & (st.NextLine(start) = beg) THEN
				start := beg
			END
		ELSE
			IF v.org <= v.text.Length() THEN
				start := st.ThisLine(v.org)
			ELSE
				start := st.ThisLine(v.text.Length())
			END;
			IF start < v.org THEN
				DoSetOrigin(v, start, 0, TRUE)
			ELSIF start > v.org THEN
				start := v.org
			END
		END
	END GetStart;

	PROCEDURE GetStringStart (v: StdView; t: Line; pos: INTEGER; OUT p1, x: INTEGER);
		VAR rd: TextSetters.Reader;
	BEGIN
		p1 := t.start; x := t.box.left;
		IF t.box.views THEN
			rd := GetReader(v, p1, t.box); rd.Read;
			WHILE ~rd.eot & (rd.pos <= pos) DO
				rd.AdjustWidth(t.start, p1, t.box, rd.w); INC(rd.x, rd.w);
				IF rd.view # NIL THEN p1 := rd.pos; x := rd.x END;
				rd.Read
			END;
			CacheReader(v, rd)
		END
	END GetStringStart;

	PROCEDURE InSynch (t0, t1: Line): BOOLEAN;
	BEGIN
		RETURN (t0.start = t1.start) & (t0.asc = t1.asc) & (t0.attr = t1.attr)
			& (t0.box.left = t1.box.left) & (t0.box.asc = t1.box.asc) & (t0.box.dsc = t1.box.dsc)
			& (t0.box.rbox = t1.box.rbox) & (t0.box.bop = t1.box.bop)
	END InSynch;

	PROCEDURE RebuildView (v: StdView);
	BEGIN
		v.setter0 := NIL;
		IF v.trailer # NIL THEN v.trailer := NIL; v.bot := 0; Views.Update(v, Views.rebuildFrames) END
	END RebuildView;

	PROCEDURE UpdateIn (v: StdView; l, t, b: INTEGER);
	BEGIN
		Views.UpdateIn(v, l, t, MAX(INTEGER), b, Views.rebuildFrames)
	END UpdateIn;

	PROCEDURE UpdateFrames (v: StdView; t0, t1, u: Line; beg, y0, yu: INTEGER);
		VAR t, te: Line; b, x, b0, b1, top, bot: INTEGER;
	BEGIN
		IF ((beg < t0.next.start) OR t0.box.eot) & ~t0.box.adj
		 & ((beg < t1.next.start) OR t1.box.eot) & ~t1.box.adj
		 & InSynch(t0, t1) THEN
			GetStringStart(v, t1, beg, beg, x)
		ELSE
			beg := t1.start
		END;
		b := y0; t := t0; WHILE t # u DO INC(b, t.h); t := t.next END;
		IF b = yu THEN
			te := u
		ELSE	(* t = u *)
			te := v.trailer;
			b0 := b; WHILE t # v.trailer DO INC(b0, t.h); t := t.next END;
			IF yu < b THEN ExtendLines(v, v.bot) ELSE ReduceLines(v, v.bot) END;
			b1 := y0; t := t1; WHILE t # v.trailer DO INC(b1, t.h); t := t.next END;
			IF b1 < b0 THEN UpdateIn(v, 0, b1, b0) END	(* erase trailer *)
		END;
		IF t1.start < beg THEN	(* conserve head of t1 *)
			UpdateIn(v, x, y0, y0 + t1.h);	(* redraw tail of t1 *)
			top := y0 + t1.h
		ELSE
			top := y0
		END;
		bot := y0; REPEAT INC(bot, t1.h); t1 := t1.next UNTIL t1 = te;
		IF top < bot THEN UpdateIn(v, 0, top, bot) END	(* redraw affected lines *)
	END UpdateFrames;

	PROCEDURE UpdateView (v: StdView; beg, end, delta: INTEGER);
		VAR st: TextSetters.Setter; r: TextRulers.Ruler; rpos: INTEGER;
			s0, t0, t, tn, u: Line; start, y, y0: INTEGER;
	BEGIN
		IF v.trailer # NIL THEN
			v.setter0 := NIL; st := v.ThisSetter();
			IF (beg <= v.trailer.start) & ((end >= v.org) OR (end - delta >= v.org)) THEN
				GetStart(st, v, beg, start);
				y0 := v.dy; s0 := v.trailer;
				WHILE s0.next.start < start DO s0 := s0.next; INC(y0, s0.h) END;
	
				t := s0.next; WHILE (t # v.trailer) & (t.start < end) DO t := t.next END;
				IF (t = v.trailer.next) & (t.start >= end) THEN
					REPEAT
						INC(t.start, delta);
						IF t.box.rpos >= end THEN INC(t.box.rpos, delta) END;
						t := t.next
					UNTIL t = v.trailer.next
				ELSE
					WHILE (t # v.trailer.next) & (t.start >= end) DO
						INC(t.start, delta);
						IF t.box.rpos >= end THEN INC(t.box.rpos, delta) END;
						t := t.next
					END
				END;
				tn := s0; y := y0; t0 := s0.next; u := t0;
				REPEAT
					t := tn; AddLine(st, tn, start, y); (* start = end(tn), y = bot(tn) *)
					WHILE (u # v.trailer) & (u.start < tn.start) DO u := u.next END
				UNTIL tn.box.eot OR (y > v.bot)
					OR (tn.start >= end) & (u.start = tn.start) & (u.box.len = tn.box.len)
						& (u.asc = tn.asc) & (u.attr = tn.attr) & (u.box.dsc = tn.box.dsc)
						& (u.box.rpos = tn.box.rpos);	(* can be expensive ... *)
				IF tn.box.eot OR (y > v.bot) THEN
					t := tn; u := v.trailer; v.trailer.start := start
				ELSE
					DEC(y, tn.h)
				END;
				t.next := u;
				IF (s0 # v.trailer) & (s0.next # v.trailer) THEN s0.box.eot := FALSE END;
				ASSERT(v.trailer.start <= v.text.Length(), 100);
				UpdateFrames(v, t0, s0.next, u, beg, y0, y)
			ELSIF end <= v.org THEN
				INC(v.org, delta);
(*
				IF end < v.org - delta - 500 THEN start := v.org ELSE start := st.ThisLine(v.org) END;
				(* this is not safe; even a change 500 characters away could force the view's origin to a
				new position in order to maintain the invariant that the origin always falls on a line start;
				however, ThisLine can be quite expensive -- can we rely on TextSetters cache ? *)
*)
				start := st.ThisLine(v.org);
				r := v.defRuler; rpos := -1; TextRulers.GetValidRuler(v.text, start, -1, r, rpos);
				IF (v.org = start) & (v.trailer.next.attr = r.style.attr) THEN
					t := v.trailer;
					REPEAT
						t := t.next; INC(t.start, delta);
						IF t.box.rpos < start THEN t.box.rpos := rpos ELSE INC(t.box.rpos, delta) END
					UNTIL t = v.trailer
				ELSE
					DoSetOrigin(v, start, 0, TRUE); RebuildView(v)
				END
			END
		END
	END UpdateView;

	PROCEDURE StyleUpdate (v: StdView; oldAttr: TextRulers.Attributes);
		VAR t: Line; beg: INTEGER; first: BOOLEAN;
	BEGIN
		IF v.trailer # NIL THEN
			t := v.trailer.next; first := TRUE;
			WHILE t # v.trailer DO
				WHILE (t # v.trailer) & (t.attr # oldAttr) DO t := t.next END;
				IF t # v.trailer THEN
					IF first THEN v.Neutralize; first := FALSE END;
					beg := t.start; t := t.next;
					WHILE (t # v.trailer) & (t.attr = oldAttr) DO t := t.next END;
					UpdateView(v, beg, t.start, 0)
				END
			END
		END
	END StyleUpdate;


	(* line drawing *)

	PROCEDURE DrawLine (v: StdView;
		start: INTEGER; IN box: TextSetters.LineBox;
		f: Views.Frame; l, r, y, t: INTEGER; pageF: BOOLEAN
	);
	(* pre: area cleared *)
	(* [l,r) for high-level clipping to tune update after small change *)
		CONST cacheLen = 128;
		VAR rd: TextSetters.Reader; ra: TextRulers.Attributes;
			v1: Views.View; c: Containers.Controller;
			py, end, skip: INTEGER;
			cache: RECORD	(* initially: long = TRUE, len = 0 *)
				x, y: INTEGER; color: Ports.Color; font: Fonts.Font;
				len: INTEGER;
				buf: ARRAY cacheLen OF CHAR
			END;

		PROCEDURE FlushCaches;
		BEGIN
			IF cache.len > 0 THEN
				cache.buf[cache.len] := 0X;
				f.DrawString(cache.x, cache.y, cache.color, cache.buf, cache.font)
			END;
			cache.len := 0
		END FlushCaches;

		PROCEDURE CacheString (x, y: INTEGER; c: INTEGER; IN s: ARRAY OF CHAR;
			f: Fonts.Font
		);
			VAR i, j, len: INTEGER;
		BEGIN
			len := 0; WHILE s[len] # 0X DO INC(len) END;
			IF (cache.len + len >= cacheLen) OR (cache.y # y) OR (cache.color # c) OR (cache.font # f) THEN
				FlushCaches
			END;
			ASSERT(cache.len + len < cacheLen, 100);
			IF cache.len = 0 THEN cache.x := x; cache.y := y; cache.color := c; cache.font := f END;
			i := 0; j := cache.len;
			WHILE i < len DO cache.buf[j] := s[i]; INC(i); INC(j) END;
			cache.len := j
		END CacheString;

(*
		PROCEDURE CacheString (x, y: INTEGER; c: INTEGER; IN s: ARRAY OF CHAR;
			f: Fonts.Font
		);
			VAR i, j, len: INTEGER;
		BEGIN
			(* flush first, then promote *)
			len := 0; WHILE s[len] # 0X DO INC(len) END;
			IF (cache.len + len >= cacheLen) OR (cache.y # y) OR (cache.color # c) OR (cache.font # f) THEN
				FlushCaches
			END;
			IF (cache.len > 0) & cache.short THEN	(* promote short chars to chars *)
				i := 0; WHILE i < cache.len DO cache.buf[i] := cache.sbuf[i]; INC(i) END
			END;
			cache.short := FALSE;
			ASSERT(cache.len + len < cacheLen, 100);
			IF cache.len = 0 THEN cache.x := x; cache.y := y; cache.color := c; cache.font := f END;
			i := 0; j := cache.len;
			WHILE i < len DO cache.buf[j] := s[i]; INC(i); INC(j) END;
			cache.len := j
		END CacheString;
*)

	BEGIN
		IF box.len > 0 THEN
			cache.len := 0;
			end := start + box.len; skip := start + box.skipOff;
			rd := GetReader(v, start, box); rd.Read;
			WHILE ~rd.eot & (rd.pos <= end) & (rd.x < r) DO
				IF rd.pos > skip THEN rd.w := rd.endW END;
				rd.AdjustWidth(start, rd.pos, box, rd.w);
				IF rd.x + rd.w > l THEN
					v1 := rd.view;
					IF v1 # NIL THEN
						FlushCaches;
						IF ~((TextModels.hideable IN rd.textOpts) & v.hideMarks) THEN
							c := v.ThisController();
							Views.InstallFrame(f, v1,
								rd.x, y - rd.attr.offset + rd.dsc - rd.h, 0,
								(c # NIL) & (v1 = c.ThisFocus()) )
						END
					ELSIF (rd.h > 0) & (rd.w > 0) THEN
						IF box.rbox & ~v.hideMarks THEN rd.string[0] := parasign END;	(* ¶ sign *)
						py := y - rd.attr.offset;
						IF rd.string[0] > " " THEN
							CacheString(rd.x, py, rd.attr.color, rd.string, rd.attr.font);
							IF ~v.hideMarks & (TextModels.hideable IN rd.textOpts) THEN
								f.DrawRect(rd.x, py - box.asc + f.dot,
									MIN(rd.x + rd.w, f.r), py + box.dsc - f.dot, 0, Ports.grey25)
							END
						ELSIF rd.string[0] # 0X THEN
							FlushCaches;
							IF ~v.hideMarks & (TextModels.hideable IN rd.textOpts) THEN
								f.DrawRect(rd.x, py - box.asc + f.dot, rd.x + rd.w, py + box.dsc - f.dot, 0, Ports.grey25)
							ELSIF ((rd.string[0] = ' ') OR (rd.string[0] = TextModels.tab))  & 
									  ({Fonts.underline, Fonts.strikeout} * rd.attr.font.style # {})
							THEN
								f.DrawSpace(rd.x, py, rd.w, rd.attr.color, rd.attr.font)
							END
						ELSIF rd.string[0] # 0X THEN
							CacheString(rd.x, py, rd.attr.color, rd.string, rd.attr.font)
						ELSE FlushCaches
						END
					END
				END;
				INC(rd.x, rd.w); rd.Read
			END;
			FlushCaches;
			CacheReader(v, rd)
		END;
		IF v.hideMarks & ~pageF THEN
			ra := box.ruler.style.attr;
			IF TextRulers.pageBreak IN ra.opts THEN
				IF (box.rpos = start) & (ra.lead >= f.dot) THEN
					f.DrawLine(l, t, r - f.dot, t, 0, Ports.grey50)
				ELSIF (box.rpos = start - 1) & (ra.lead < f.dot) THEN
					f.DrawLine(l, t, r - f.dot, t, 0, Ports.grey50)
				END
			END
		END
	END DrawLine;

	PROCEDURE DrawDecorations (v: StdView; u: Line; f: Views.Frame; l, t, r, b: INTEGER);
		VAR a: TextRulers.Attributes; i,  x: INTEGER; col: Ports.Color;
			st: TextSetters.Setter; srd: TextSetters.Reader; rd: TextModels.Reader;
	BEGIN
		IF t < b THEN
			i := 0; a := u.attr; srd := NIL;
			WHILE i < a.tabs.len DO
				IF TextRulers.barTab IN a.tabs.tab[i].type THEN
					x := a.tabs.tab[i].stop;
					IF (l <= x) & (x < r) THEN
						IF u.box.rpos = -1 THEN col := v.defAttr.color
						ELSIF srd = NIL THEN
							st := v.ThisSetter();
							srd := v.cachedRd; v.cachedRd := NIL;
							srd := st.NewReader(srd);
							srd.Set(srd.r, v.text, 0, 0, v.defRuler, 0, st.vw, st.hideMarks); rd := srd.r;
							rd.SetPos(u.box.rpos); rd.Read; col := rd.attr.color
						END;
						f.DrawLine(x, t, x, b - f.dot, 0, col)
					END
				END;
				INC(i)
			END;
			IF srd # NIL THEN CacheReader(v, srd) END
		END
	END DrawDecorations;


	(* focus-message handling *)

	PROCEDURE PollSection (v: StdView; f: Views.Frame; VAR msg: Controllers.PollSectionMsg);
		CONST ms = maxScrollSteps; mh = maxScrollHeight;
		VAR t: Line; steps, step: INTEGER;
	BEGIN
		IF msg.vertical THEN
			ValidateLines(v, f.b); t := v.trailer.next;
			IF t.h > 0 THEN
				steps := -((-t.h) DIV mh); step := -(v.dy DIV mh)
			ELSE steps := 1; step := 0
			END;
			msg.wholeSize := v.text.Length() * ms;
			msg.partPos := v.org * ms + t.box.len * ms * step DIV steps;
			msg.partSize := 0;
			msg.valid := (v.org > 0) OR (t.h > mh) OR (t.next # v.trailer);
			msg.done := TRUE
		END
	END PollSection;

	PROCEDURE Scroll (v: StdView; f: Views.Frame; VAR msg: Controllers.ScrollMsg);
		VAR st: TextSetters.Setter; box, box0: TextSetters.LineBox;
			t, t1, trailer: Line; org, len, dy, h, h1, sh, steps, step: INTEGER;
			poll: Controllers.PollSectionMsg;
	BEGIN
		IF msg.vertical THEN
			poll.vertical := TRUE;
			PollSection(v, f, poll)
		END;
		IF msg.vertical & poll.valid THEN
			org := v.org; dy := v.dy; st := v.ThisSetter(); trailer := v.trailer;
			CASE msg.op OF
			  Controllers.decLine:
				IF dy <= -(maxScrollHeight + fuseScrollHeight) THEN
					INC(dy, maxScrollHeight)
				ELSIF dy < 0 THEN
					dy := 0
				ELSIF org > 0 THEN
					org := st.PreviousLine(org); st.GetLine(org, box);
					h1 := box.asc + box.dsc + st.GridOffset(-1, box);
					IF h1 > maxScrollHeight + fuseScrollHeight THEN
						sh := h1 - h1 MOD maxScrollHeight;
						IF h1 - sh < fuseScrollHeight THEN DEC(sh, maxScrollHeight) END;
						dy := -sh
					ELSE dy := 0
					END
				END
			| Controllers.incLine:
				t := trailer.next;
				IF t.h + dy > maxScrollHeight + fuseScrollHeight THEN
					DEC(dy, maxScrollHeight)
				ELSIF ~t.box.eot THEN
					org := t.next.start; dy := 0
				END
			| Controllers.decPage:
				sh := f.b; DEC(sh, maxScrollHeight + sh MOD maxScrollHeight);
				IF dy <= -(sh + fuseScrollHeight) THEN
					INC(dy, sh)
				ELSE
					t := trailer.next;
					h := maxScrollHeight - dy;
					IF t.h < h THEN h := t.h END;
					box0 := t.box; h1:= h - st.GridOffset(-1, box0);
					WHILE (org > 0) & (h + fuseScrollHeight < f.b) DO
						org := st.PreviousLine(org); st.GetLine(org, box);
						h1 := box.asc + box.dsc;
						INC(h, h1 + st.GridOffset(box.dsc, box0));
						box0 := box
					END;
					h1 := h1 + st.GridOffset(-1, box0);
					sh := h1 - (h - f.b); DEC(sh, sh MOD maxScrollHeight);
					IF h1 - sh >= fuseScrollHeight THEN dy := -sh ELSE dy := 0 END
				END;
				IF (org > v.org) OR (org = v.org) & (dy <= v.dy) THEN	(* guarantee progress *)
					org := st.PreviousLine(org); st.GetLine(org, box);
					h1 := box.asc + box.dsc + st.GridOffset(-1, box);
					IF h1 > maxScrollHeight + fuseScrollHeight THEN
						dy := - (h1 DIV maxScrollHeight * maxScrollHeight)
					ELSE
						dy := 0
					END
				END
			| Controllers.incPage:
				t := trailer.next;
				sh := f.b; DEC(sh, maxScrollHeight + sh MOD maxScrollHeight);
				IF t.h + dy > sh + fuseScrollHeight THEN
					DEC(dy, sh)
				ELSE
					t := ThisViewLine(v, f.b); LocateThisLine(v, t.start, t1, h);
					IF (h + t.h >= f.b) & (t.h <= maxScrollHeight) THEN
						org := st.PreviousLine(t.start)
					ELSE org := t.start
					END;
					IF h + t.h - f.b > maxScrollHeight THEN
						sh := f.b - h; DEC(sh, maxScrollHeight + sh MOD maxScrollHeight);
						IF sh >= fuseScrollHeight THEN dy := -sh ELSE dy := 0 END
					ELSE
						dy := 0
					END
				END;
				IF (org < v.org) OR (org = v.org) & (dy >= v.dy) THEN	(* guarantee progress *)
					IF t.h + dy > maxScrollHeight + fuseScrollHeight THEN
						DEC(dy, maxScrollHeight)
					ELSE
						org := t.next.start; dy := 0
					END
				END
			| Controllers.gotoPos:
				org := st.ThisLine(msg.pos DIV maxScrollSteps); st.GetLine(org, box);
				sh := box.asc + box.dsc + st.GridOffset(-1, box);
				steps := -((-sh) DIV maxScrollHeight);
				IF (steps > 0) & (box.len > 0) THEN
					step := steps * (msg.pos - org * maxScrollSteps) DIV (maxScrollSteps * box.len);
(*
					step := steps * (msg.pos MOD maxScrollSteps) DIV maxScrollSteps;
*)
					dy := -(step * maxScrollHeight)
				ELSE
					dy := 0
				END
			ELSE
			END;
			len := v.text.Length();
			IF org > len THEN org := len; dy := 0 END;
			v.SetOrigin(org, dy);
			msg.done := TRUE
		END
	END Scroll;

	PROCEDURE NotifyViewsOnPage (v: StdView; beg, end, pageNo: INTEGER);
		VAR st: TextSetters.Setter; rd: TextSetters.Reader; r: TextModels.Reader;
			view: Views.View; current: INTEGER;
			page: PageMsg;
	BEGIN
		IF pageNo >= 0 THEN current := pageNo
		ELSIF Printing.par # NIL THEN current := Printing.Current() (* Printing.par.page.current *) + 1
		ELSE current := -1
		END;
		IF current >= 0 THEN
			st := v.ThisSetter();
			rd := v.cachedRd; v.cachedRd := NIL;	(* reader recycling *)
			rd := st.NewReader(rd);
			rd.Set(rd.r, v.text, 0, 0, v.defRuler, 0, st.vw, st.hideMarks);
			r := rd.r; r.SetPos(beg); r.ReadView(view);
			WHILE (r.Pos() <= end) & ~r.eot DO
				page.current := current; Views.HandlePropMsg(view, page); r.ReadView(view)
			END;
			CacheReader(v, rd)
		END
	END NotifyViewsOnPage;

	PROCEDURE Page (v: StdView; pageH: INTEGER; op, pageY: INTEGER; OUT done, eoy: BOOLEAN);
		VAR st: TextSetters.Setter; org, prev, page: INTEGER;
	BEGIN
		IF ~v.hideMarks & ((v.context = NIL) OR v.context.Normalize()) THEN
			v.DisplayMarks(hide)
		END;
		st := v.ThisSetter();
		IF op = Controllers.nextPageY THEN
			done := TRUE; org := st.NextPage(pageH, v.org); eoy := (org = v.text.Length());
			IF ~eoy THEN NotifyViewsOnPage(v, org, st.NextPage(pageH, org), -1) END
		ELSIF op =  Controllers.gotoPageY THEN
			ASSERT(pageY >= 0, 20);
			done := TRUE; org := 0; eoy := FALSE; page := 0;
			WHILE (page < pageY) & ~eoy DO
				prev := org; org := st.NextPage(pageH, org); eoy := org = prev;
				IF ~eoy THEN NotifyViewsOnPage(v, prev, org, page) END;
				INC(page)
			END;
			IF ~eoy THEN NotifyViewsOnPage(v, org, st.NextPage(pageH, org), page) END
		ELSE
			done := FALSE
		END;
		IF done & ~eoy THEN v.org := org; v.dy := 0; v.trailer := NIL; v.bot := 0 END
	END Page;


	PROCEDURE ShowAdjusted (v: StdView; shift: INTEGER; rebuild: BOOLEAN);
	BEGIN
		IF shift # 0 THEN Views.Scroll(v, 0, shift)
		ELSIF rebuild THEN UpdateIn(v, 0, 0, MAX(INTEGER))
		END;
		Views.RestoreDomain(v.Domain())
	END ShowAdjusted;

	PROCEDURE AdjustLines (v: StdView; org, dy: INTEGER;
		OUT shift: INTEGER; OUT rebuild: BOOLEAN
	);
	(* post: shift = 0  OR  ~rebuild *)
		VAR d: Stores.Domain; c: Containers.Controller; t, t0, t1: Line; org0, dy0, y: INTEGER;
	BEGIN
		d := v.Domain(); t0 := v.trailer; org0 := v.org; rebuild := FALSE; shift := 0;
		IF (d # NIL) & ((org # org0) OR (dy # v.dy)) THEN
			Views.RestoreDomain(d);	(* make sure that pixels are up-to-date before scrolling *)
			c := v.ThisController();
			IF c # NIL THEN
				Containers.FadeMarks(c, Containers.hide)	(* fade marks with overhang *)
			END
		END;
		IF (t0 # NIL) & (org = org0) & (dy # v.dy) THEN	(* sub-line shift *)
			shift := dy - v.dy;
		ELSIF (t0 # NIL) & (org > org0) & (org < t0.start) THEN	(* shift up *)
			LocateThisLine(v, org, t, y); t0.next := t;
			shift := dy - y
		ELSIF (t0 # NIL) & (org < org0) THEN	(* shift down *)
			t1 := t0.next; dy0 := v.dy + t1.asc; v.org := org; v.dy := dy;
			IF t1.start = org0 THEN	(* new lines need to be prepended *)
				PrependLines(v)	(* may change t1.asc *)
			END;
			ASSERT(t0.next.start = org, 100);
			IF org0 < t0.start THEN	(* former top still visible -> shift down *)
				LocateThisLine(v, org0, t, y); shift := y - (dy0 - t1.asc)
			ELSE	(* rebuild all *)
				rebuild := TRUE
			END
		ELSIF (t0 = NIL) OR (org # org0) OR (dy # v.dy) THEN	(* rebuild all *)
			rebuild := TRUE
		END;
		v.org := org; v.dy := dy;
		IF rebuild THEN	(* rebuild all *)
			v.trailer := NIL; ValidateLines(v, v.bot)
		ELSIF shift < 0 THEN	(* shift up *)
			INC(v.bot, shift); ExtendLines(v, v.bot - shift)
		ELSIF shift > 0 THEN	(* shift down *)
			INC(v.bot, shift); ReduceLines(v, v.bot - shift)
		END
	END AdjustLines;

	PROCEDURE Limit (v: StdView; bot: INTEGER; allLines: BOOLEAN): INTEGER;
		CONST minH = 12 * point;
		VAR s, t: Line; pos, y: INTEGER;
	BEGIN
		s := v.trailer.next; t := s; y := v.dy;
		WHILE ~t.box.eot & (y + t.h <= bot) DO INC(y, t.h); s := t; t := t.next END;
		IF ~allLines & (bot - y < t.h) & (bot - y < minH) THEN t := s END;
		pos := t.start + t.box.len;
(*
		IF t.box.eot THEN INC(pos) END;
*)
		RETURN pos
	END Limit;


	(* ScrollOp *)

	PROCEDURE (op: ScrollOp) Do;
		VAR org0, dy0, org, dy, shift: INTEGER; rebuild: BOOLEAN;
	BEGIN
		IF op.bunch THEN org := op.bunchOrg; dy := op.bunchDy
		ELSE org := op.org; dy := op.dy
		END;
		org0 := op.v.org; dy0 := op.v.dy;
		IF op.silent THEN
			op.v.org := org; op.v.dy := dy; op.silent := FALSE
		ELSE
			AdjustLines(op.v, org, dy, shift, rebuild); ShowAdjusted(op.v, shift, rebuild)
		END;
		IF op.bunch THEN op.bunch := FALSE ELSE op.org := org0; op.dy := dy0 END
	END Do;

	PROCEDURE DoSetOrigin (v: StdView; org, dy: INTEGER; silent: BOOLEAN);
	(* pre: org = v.ThisSetter().ThisLine(org) *)
		VAR con: Models.Context; last: Stores.Operation; op: ScrollOp;
			shift: INTEGER; rebuild: BOOLEAN;
	BEGIN
		IF (org # v.org) OR (dy # v.dy) THEN
			con := v.context;
			IF con # NIL THEN
				IF (v.Domain() = NIL) OR con.Normalize() THEN
					IF silent THEN
						v.org := org; v.dy := dy
					ELSE
						AdjustLines(v, org, dy, shift, rebuild); ShowAdjusted(v, shift, rebuild)
					END
				ELSE
					last := Views.LastOp(v);
					IF (last # NIL) & (last IS ScrollOp) THEN
						op := last(ScrollOp);
						op.bunch := TRUE; op.bunchOrg := org; op.bunchDy := dy;
						op.silent := silent;
						Views.Bunch(v)
					ELSE
						NEW(op); op.v := v; op.org := org; op.dy := dy;
						op.bunch := FALSE;
						op.silent := silent;
						Views.Do(v, scrollingKey, op)
					END
				END
			ELSE
				v.org := org; v.dy := dy
			END
		END
	END DoSetOrigin;


	(* SetOp *)

	PROCEDURE (op: SetOp) Do;
		VAR v: StdView; m: BOOLEAN;
			a: TextModels.Attributes; r: TextRulers.Ruler; s: TextSetters.Setter;
	BEGIN
		v := op.view;
		CASE op.mode OF
		  setMarks:
			m := v.hideMarks; v.hideMarks := op.hideMarks; op.hideMarks := m
		| setSetter:
			s := v.setter;
			IF s # NIL THEN s.ConnectTo(NIL, NIL, 0, FALSE) END;
			v.setter := op.setter; op.setter := s
		| setDefs:
			r := v.defRuler; a := v.defAttr;
			v.defRuler := op.defRuler; v.defAttr := op.defAttr;
			op.defRuler := r; op.defAttr := a;
(*
			IF (v.defAttr.Domain() # NIL) & (v.defAttr.Domain() # v.Domain()) THEN
				v.defAttr := Stores.CopyOf(v.defAttr)(TextModels.Attributes)
			END;
			Stores.Join(v, v.defAttr);
*)
			IF v.defAttr # NIL THEN (* could be for undo operations *)
				IF ~Stores.Joined(v, v.defAttr) THEN
					IF ~Stores.Unattached(v.defAttr) THEN
						v.defAttr := Stores.CopyOf(v.defAttr)(TextModels.Attributes)
					END;
					Stores.Join(v, v.defAttr)
				END;
			END;
			
			IF v.defRuler # NIL THEN Stores.Join(v, v.defRuler) END;
		END;
		RebuildView(v)
	END Do;

	PROCEDURE DoSet (op: SetOp; mode: INTEGER; v: StdView);
	BEGIN
		op.mode := mode; op.view := v; Views.Do(v, viewSettingKey, op)
	END DoSet;


	(* StdView *)

	PROCEDURE (v: StdView) Internalize2 (VAR rd: Stores.Reader);
		VAR st: Stores.Store; r: TextRulers.Ruler; a: TextModels.Attributes;
			org, dy: INTEGER; thisVersion: INTEGER; hideMarks: BOOLEAN;
	BEGIN
		v.Internalize2^(rd);
		IF rd.cancelled THEN RETURN END;
		rd.ReadVersion(minVersion, maxStdVersion, thisVersion);
		IF rd.cancelled THEN RETURN END;
		rd.ReadBool(hideMarks);
		rd.ReadStore(st); ASSERT(st # NIL, 100);
		IF ~(st IS TextRulers.Ruler) THEN
			rd.TurnIntoAlien(Stores.alienComponent);
			Stores.Report("#Text:AlienDefaultRuler", "", "", "");
			RETURN
		END;
		r := st(TextRulers.Ruler);
		TextModels.ReadAttr(rd, a);
		rd.ReadInt(org); rd.ReadInt(dy);
		v.DisplayMarks(hideMarks);
		v.setter := TextSetters.dir.New(); v.setter0 := NIL;
		v.SetDefaults(r, a); v.SetOrigin(org, dy);
		v.trailer := NIL; v.bot := 0
	END Internalize2;

	PROCEDURE (v: StdView) Externalize2 (VAR wr: Stores.Writer);
		VAR org, dy: INTEGER; hideMarks: BOOLEAN;
			a: Stores.Store;
	BEGIN
		v.Externalize2^(wr);
		IF (v.context = NIL) OR v.context.Normalize() THEN
			org := 0; dy := 0; hideMarks := TRUE
		ELSE
			org := v.org; dy := v.dy; hideMarks := v.hideMarks
		END;
		wr.WriteVersion(maxStdVersion);
		wr.WriteBool(hideMarks);
		a := Stores.CopyOf(v.defAttr); (*Stores.InitDomain(a, v.Domain());*) Stores.Join(v, a);
			(* bkwd-comp hack: avoid link => so that pre release 1.3 Internalize can still read texts *)
		wr.WriteStore(v.defRuler);
		wr.WriteStore(a);
		wr.WriteInt(org); wr.WriteInt(dy)
	END Externalize2;

	PROCEDURE (v: StdView) CopyFromModelView2 (source: Views.View; model: Models.Model);
		VAR s: TextSetters.Setter; r: TextRulers.Ruler;
	BEGIN
		(* v.CopyFromModelView^(source, model); *)
		WITH source: StdView DO
			s := Stores.CopyOf(source.setter)(TextSetters.Setter);
			v.setter := s; v.setter0 := NIL;
			r := TextRulers.CopyOf(source.defRuler, Views.deep);
			v.DisplayMarks(source.HidesMarks());
			v.SetDefaults(r, source.defAttr);
			v.trailer := NIL; v.bot := 0;
			IF v.text = source.text THEN
				v.org := source.org; v.dy := source.dy
			END
		END
	END CopyFromModelView2;

	PROCEDURE (v: StdView) Restore (f: Views.Frame; l, t, r, b: INTEGER);
		VAR st: TextSetters.Setter; u0, u: Line;
			y0, y, w, h: INTEGER; end: INTEGER; pageF: BOOLEAN;
	BEGIN
		ASSERT(v.context # NIL, 20);
		IF v.setter # NIL THEN v.context.GetSize(w, h) END;
		IF (v.setter = NIL) OR (v.setter.vw # w) THEN
			Views.RemoveFrames(f, l, t, r, b)
		END;
		ValidateLines(v, b);
		u := v.trailer.next; y := v.dy;
		pageF := Views.IsPrinterFrame(f) & v.context.Normalize();
		IF pageF THEN	(* on page-formatted frames do not display truncated lines at bottom *)
			st := v.ThisSetter(); end := st.NextPage(f.b - f.t, v.org)
		END;
		WHILE (u # v.trailer) & (y + u.h <= t) DO INC(y, u.h); u := u.next END;
		y0 := y; u0 := u;
		IF (u = v.trailer.next) & (y < b) THEN	(* at least one line per page *)
			ASSERT((u.box.len > 0) OR u.box.eot OR (u.next = v.trailer), 100);
			DrawLine(v, u.start, u.box, f, l, r, y + u.asc, y + u.h - u.box.dsc - u.box.asc, pageF);
			INC(y, u.h); u := u.next
		END;
		WHILE (u # v.trailer) & (y < b) & (~pageF OR (u.start < end)) DO
			ASSERT((u.box.len > 0) OR u.box.eot OR (u.next = v.trailer), 101);
			IF u.box.ruler # u0.box.ruler THEN
				DrawDecorations(v, u0, f, l, y0, r, y); u0 := u; y0 := y
			END;
			DrawLine(v, u.start, u.box, f, l, r, y + u.asc, y + u.h - u.box.dsc - u.box.asc, pageF);
			INC(y, u.h); u := u.next
		END;
		IF y0 # y THEN DrawDecorations(v, u0, f, l, y0, r, y) END
	END Restore;

	PROCEDURE (v: StdView) DisplayMarks (hide: BOOLEAN);
		VAR op: SetOp; c: Containers.Controller;
	BEGIN
		IF v.hideMarks # hide THEN
			c := v.ThisController();
			IF c # NIL THEN Containers.FadeMarks(c, Containers.hide) END;
			IF (v.context # NIL) & ~v.context.Normalize() THEN
				NEW(op); op.hideMarks := hide; DoSet(op, setMarks, v)
			ELSE
				v.hideMarks := hide; RebuildView(v)
			END
		END
	END DisplayMarks;

	PROCEDURE (v: StdView) HidesMarks (): BOOLEAN;
	BEGIN
		RETURN v.hideMarks
	END HidesMarks;

	PROCEDURE (v: StdView) SetSetter (setter: TextSetters.Setter);
		VAR op: SetOp;
	BEGIN
		ASSERT(setter # NIL, 20);
		IF v.setter # setter THEN
			IF v.setter # NIL THEN
				NEW(op); op.setter := setter; DoSet(op, setSetter, v)
			ELSE v.setter := setter
			END
		END
	END SetSetter;

	PROCEDURE (v: StdView) ThisSetter (): TextSetters.Setter;
		VAR st: TextSetters.Setter; w, h: INTEGER;
	BEGIN
		st := v.setter; ASSERT(st # NIL, 20);
		IF st # v.setter0 THEN
			IF v.context # NIL THEN
				v.context.GetSize(w, h)
			ELSE
				IF Dialog.metricSystem THEN
					w := 165*mm
				ELSE
					w := 104*inch16
				END
			END;
			st.ConnectTo(v.text, v.defRuler, w, v.hideMarks);
			v.setter0 := st
		END;
		RETURN st
	END ThisSetter;

	PROCEDURE (d: StdView) AcceptableModel (m: Containers.Model): BOOLEAN;
	BEGIN
		RETURN m IS TextModels.Model
	END AcceptableModel;
	
	PROCEDURE (v: StdView) InitModel2 (m: Containers.Model);
	BEGIN
		ASSERT(m IS TextModels.Model, 23);
		v.text := m(TextModels.Model)
	END InitModel2;

	PROCEDURE (v: StdView) SetOrigin (org, dy: INTEGER);
		VAR st: TextSetters.Setter; start: INTEGER;
	BEGIN
		ASSERT(v.text # NIL, 20);
		st := v.ThisSetter(); start := st.ThisLine(org);
		IF start # org THEN org := start; dy := 0 END;
		DoSetOrigin(v, org, dy, FALSE)
	END SetOrigin;

	PROCEDURE (v: StdView) PollOrigin (OUT org, dy: INTEGER);
	BEGIN
		org := v.org; dy := v.dy
	END PollOrigin;

	PROCEDURE (v: StdView) SetDefaults (r: TextRulers.Ruler; a: TextModels.Attributes);
		VAR op: SetOp;
	BEGIN
		ASSERT(r # NIL, 20); ASSERT(r.style.attr.init, 21);
		ASSERT(a # NIL, 22); ASSERT(a.init, 23);
		IF (v.defRuler # r) OR (v.defAttr # a) THEN
(*
			(*IF (v.context # NIL) & (r # v.defRuler) THEN*)
			IF (v.Domain() # NIL) & (r # v.defRuler) THEN
				Stores.InitDomain(r, v.Domain())
			END;
*)
			IF r # v.defRuler THEN Stores.Join(v, r) END;
			NEW(op); op.defRuler := r; op.defAttr := a; DoSet(op, setDefs, v)
		END
	END SetDefaults;

	PROCEDURE (v: StdView) PollDefaults (OUT r: TextRulers.Ruler; OUT a: TextModels.Attributes);
	BEGIN
		r := v.defRuler; a := v.defAttr
	END PollDefaults;

(*
	PROCEDURE (v: StdView) PropagateDomain;
		VAR m: Models.Model;
	BEGIN
		ASSERT(v.setter # NIL, 20); ASSERT(v.text # NIL, 21);
		ASSERT(v.defRuler # NIL, 22); ASSERT(v.defAttr # NIL, 23);
		v.PropagateDomain^;
		m := v.ThisModel();
		IF m # NIL THEN Stores.InitDomain(m, v.Domain()) END;
		Stores.InitDomain(v.defRuler, v.Domain())
	END PropagateDomain;
	*)
(*
	PROCEDURE (v: StdView) Flush, NEW;
	BEGIN
		v.trailer := NIL; v.bot := 0; v.setter0 := NIL
	END Flush;
*)
	PROCEDURE (v: StdView) HandleModelMsg2 (VAR msg: Models.Message);
	BEGIN
		IF msg.model = v.text THEN
			WITH msg: Models.UpdateMsg DO
				WITH msg: TextModels.UpdateMsg DO
					IF msg.op IN {TextModels.insert, TextModels.delete, TextModels.replace} THEN
						UpdateView(v, msg.beg, msg.end, msg.delta)
					ELSE	(* unknown text op happened *)
						RebuildView(v)
					END
				ELSE	(* unknown text update happened *)
					RebuildView(v)
				END
			| msg: PositionMsg DO
				v.ShowRange(msg.beg, msg.end, msg.focusOnly)
			ELSE
			END
		ELSE	(* domaincast received *)
			WITH msg: TextRulers.UpdateMsg DO
				StyleUpdate(v, msg.oldAttr)
			| msg: Models.UpdateMsg DO	(* forced rebuild *)
				RebuildView(v)
			ELSE
			END
		END
	END HandleModelMsg2;

	PROCEDURE (v: StdView) HandleViewMsg2 (f: Views.Frame; VAR msg: Views.Message);
	BEGIN
		IF msg.view = v THEN
			WITH msg: FindAnyFrameMsg DO
				IF (msg.frame = NIL) OR (msg.frame.b - msg.frame.t > f.b - f.t) THEN msg.frame := f END
			ELSE
			END
		ELSE
			WITH msg: Views.UpdateCachesMsg DO	(* display view in new frame *)
				IF Views.Era(v) # Models.Era(v.text) THEN
					(* view/setter caches outdated - possible if v previous to this notification had no frame open *)
					v.setter0 := NIL; v.trailer := NIL; v.bot := 0
				END
			ELSE
			END
		END
	END HandleViewMsg2;

	PROCEDURE (v: StdView) HandleCtrlMsg2 (f: Views.Frame;
		VAR msg: Controllers.Message; VAR focus: Views.View
	);
	BEGIN
		WITH msg: Controllers.PollSectionMsg DO
			IF (focus = NIL) OR ~msg.focus THEN
				PollSection(v, f, msg);
				focus := NIL
			END
		| msg: FindFocusFrameMsg DO
			IF (msg.view = v) & (msg.frame = NIL) THEN msg.frame := f END
		| msg: Controllers.ScrollMsg DO
			IF (focus = NIL) OR ~msg.focus THEN
				Scroll(v, f, msg);
				focus := NIL
			END
		| msg: Controllers.PageMsg DO
			Page(v, f.b - f.t, msg.op, msg.pageY, msg.done, msg.eoy);
			focus := NIL
		ELSE
		END
	END HandleCtrlMsg2;

	PROCEDURE (v: StdView) HandlePropMsg2 (VAR p: Properties.Message);
		CONST minW = 5 * point; maxW = maxHeight; minH = 5 * point; maxH = maxHeight;
		VAR st: TextSetters.Setter;
	BEGIN
		WITH p: Properties.SizePref DO
			IF p.w = Views.undefined THEN p.w := v.defRuler.style.attr.right END;
			IF p.h = Views.undefined THEN p.h := MAX(INTEGER) END
		| p: Properties.BoundsPref DO
			st := v.ThisSetter();
			st.GetBox(0, v.text.Length(), maxW, maxH, p.w, p.h);
			IF p.w < minW THEN p.w := minW END;
			IF p.h < minH THEN p.h := minH END
		| p: Properties.ResizePref DO
			p.fixed := FALSE;
			p.horFitToPage := ~(TextRulers.rightFixed IN v.defRuler.style.attr.opts);
			p.verFitToWin := TRUE
		| p: Properties.TypePref DO
			IF Services.Is(v, p.type) THEN p.view := v END
		| p: Containers.DropPref DO
			p.okToDrop := TRUE
		ELSE
		END
	END HandlePropMsg2;


	PROCEDURE (v: StdView) GetThisLocation (f: Views.Frame; pos: INTEGER; OUT loc: Location);
	(* pre: f must be displayed *)
	(* if position lies outside view, the next best location inside will be taken *)
		VAR rd: TextSetters.Reader; t: Line; p1, y, w, h: INTEGER;
	BEGIN
		ValidateLines(v, f.b);
		y := v.dy;
		IF pos < v.org THEN
			t := v.trailer.next;
			loc.start := t.start; loc.pos := t.start;
			loc.x := 0; loc.y := y; loc.asc := t.asc; loc.dsc := t.h - t.asc; loc.view := NIL;
			RETURN
		ELSIF pos < v.trailer.start THEN
			t := v.trailer.next;
			WHILE ~t.box.eot & ~((t.start <= pos) & (pos < t.next.start)) DO
				INC(y, t.h); t := t.next
			END
		ELSE	(* pos >= v.trailer.start *)
			t := v.trailer.next; WHILE ~t.box.eot DO INC(y, t.h); t := t.next END;
			IF t = v.trailer THEN
				loc.start := t.start; loc.pos := t.start;
				loc.x := 0; loc.y := y; loc.asc := t.asc; loc.dsc := t.h - t.asc; loc.view := NIL;
				RETURN
			END
		END;
		p1 := t.start;
		rd := GetReader(v, p1, t.box); rd.Read;
		WHILE rd.pos < pos DO
			p1 := rd.pos; rd.AdjustWidth(t.start, p1, t.box, rd.w); INC(rd.x, rd.w); rd.Read
		END;
		IF LEN(rd.string$) > 1 THEN	(* collated subsequence *)
			rd.x := f.CharPos(rd.x, pos - p1, rd.string, rd.attr.font);
			IF rd.pos = pos THEN rd.Read END
		ELSIF rd.pos = pos THEN
			rd.AdjustWidth(t.start, pos, t.box, rd.w); INC(rd.x, rd.w); rd.Read
		ELSE
			ASSERT(p1 = pos, 100)
		END;
		loc.view := rd.view;
		loc.start := t.start; loc.pos := pos;
		loc.x := rd.x; loc.y := y; loc.asc := t.asc; loc.dsc := t.h - t.asc;
		IF loc.view # NIL THEN
			v.context.GetSize(w, h);
			IF rd.x + rd.w > w THEN rd.w := w - rd.x END;
			loc.l := rd.x; loc.t := y - rd.attr.offset + t.asc + rd.dsc - rd.h;
			loc.r := loc.l + rd.w; loc.b := loc.t + rd.h
		END;
		CacheReader(v, rd)
	END GetThisLocation;

	PROCEDURE (v: StdView) GetRange (f: Views.Frame; OUT beg, end: INTEGER);
		VAR t: Line;
	BEGIN
		ValidateLines(v, f.b);
		t := ThisViewLine(v, f.t); beg := t.start; end := Limit(v, f.b, TRUE)
	END GetRange;

	PROCEDURE (v: StdView) ThisPos (f: Views.Frame; x, y: INTEGER): INTEGER;
	(* pre: f must be displayed *)
	(* post: f.org <= result <= v.text.Length() *)
		VAR rd: TextSetters.Reader; t: Line; p1, end, py: INTEGER;
	BEGIN
		ValidateLines(v, f.b);
		t := v.trailer.next; py := v.dy;
		WHILE ~t.box.eot & (py + t.h <= y) DO INC(py, t.h); t := t.next END;
		p1 := t.start; end := p1 + t.box.len;
		IF py + t.h > y THEN
			IF (end > p1) & (y >= v.dy) THEN
				IF t.box.eot THEN INC(end) END;
				rd := GetReader(v, p1, t.box);
				rd.Read; rd.AdjustWidth(t.start, rd.pos, t.box, rd.w);
				WHILE (rd.x + rd.SplitWidth(rd.w) < x) & (rd.pos < end) DO
					p1 := rd.pos; INC(rd.x, rd.w);
					rd.Read; rd.AdjustWidth(t.start, rd.pos, t.box, rd.w)
				END;
				IF LEN(rd.string$) > 1 THEN	(* collated subsequence *)
					INC(p1, f.CharIndex(rd.x, x, rd.string, rd.attr.font))
				END;
				CacheReader(v, rd)
			END
		ELSE p1 := end
		END;
		RETURN p1
	END ThisPos;

	PROCEDURE (v: StdView) ShowRangeIn (f: Views.Frame; beg, end: INTEGER);
		CONST minH = 12 * point;
		VAR c: Models.Context; st: TextSetters.Setter; t, t1: Line;
			org0, last, len,  org, dy,  p, q: INTEGER; y, h, mh: INTEGER;
			box, box0: TextSetters.LineBox; loc, loc1: Location;
			focus: BOOLEAN;
	BEGIN
		focus := f = Controllers.FocusFrame();
		c := v.context;
		st := v.ThisSetter(); ValidateLines(v, f.b); org0 := v.org;
		last := Limit(v, f.b, FALSE); len := v.text.Length();
		IF last = len THEN p := st.ThisLine(last); LocateThisLine(v, p, t1, y); h := f.b - y END;
		IF (beg > last)
		OR (beg = last) & ((last < len) OR (len > 0) & (h < t1.h) & (h < minH))
		OR (end < org0)
		OR (beg < end) & (end = org0) THEN
			org := -1; dy := 0;
			IF beg <= org0 THEN	(* try to adjust by scrolling up *)
				p := st.PreviousLine(v.org);
				IF p <= beg THEN	(* reveal one line at top *)
					org := p; st.GetLine(org, box);
					h := box.asc + box.dsc + st.GridOffset(-1, box);
					IF h > maxScrollHeight + fuseScrollHeight THEN
						dy := -(h - h MOD maxScrollHeight);
						IF h + dy < fuseScrollHeight THEN INC(dy, maxScrollHeight) END
					END
				END
			END;
			IF (org = -1) & (beg >= last) THEN	(* try to adjust by scrolling down *)
				p := st.ThisLine(last); q := st.NextLine(p); st.GetLine(q, box);
				IF (beg < q + box.len) OR (p = q) THEN	(* reveal one line at bottom *)
					LocateThisLine(v, p, t1, y);
					h := box.asc + box.dsc + st.GridOffset(t1.box.dsc, box);
					IF h > maxScrollHeight + fuseScrollHeight THEN h := maxScrollHeight END;
					mh := y + t1.h - f.b + h;
					t := v.trailer.next; h := v.dy;
					WHILE (t # v.trailer) & (h < mh) DO INC(h, t.h); t := t.next END;
					IF t.start > v.org THEN org := t.start END
				END
			END;
			IF org = -1 THEN	(* adjust by moving into "nice" position *)
				mh := f.b DIV 3;
				org := st.ThisLine(beg); st.GetLine(org, box0);
				h := box0.asc + box0.dsc + st.GridOffset(-1, box0); p := org;
				WHILE (p > 0) & (h < mh) DO
					DEC(h, st.GridOffset(-1, box0)); org := p;
					p := st.PreviousLine(org); st.GetLine(p, box);
					INC(h, box.asc + box.dsc + st.GridOffset(box.dsc, box0));
					box0 := box
				END;
				IF (org = len) & (len > 0) THEN org := st.PreviousLine(org) END
			END;
			DoSetOrigin(v, org, dy, FALSE)
		END;
		IF focus THEN
			f := Controllers.FocusFrame();
			IF (f # NIL) & (f.view = v) THEN
			
				v.GetThisLocation(f, beg, loc);
				v.GetThisLocation(f, end, loc1);
				IF (loc.y = loc1.y) & (loc.x <= loc1.x) THEN
					c.MakeVisible(loc.x, loc.y, loc1.x, loc1.y)
				END
			ELSE
				HALT(100); (* this should not happen *)
			END
		END;
(*
		IF c IS Documents.Context THEN
			v.GetThisLocation(f, beg, loc);
			v.GetThisLocation(f, end, loc1);
			IF (loc.y = loc1.y) & (loc.x <= loc1.x) THEN
				Documents.MakeVisible(c(Documents.Context).ThisDoc(), f, loc.x, loc.y, loc1.x, loc1.y)
			END
		END
*)
	END ShowRangeIn;

	PROCEDURE (v: StdView) ShowRange (beg, end: INTEGER; focusOnly: BOOLEAN);
		VAR fmsg: FindFocusFrameMsg; amsg: FindAnyFrameMsg; f: Views.Frame;
	BEGIN
		IF focusOnly THEN
			fmsg.view := v; fmsg.frame := NIL; Controllers.Forward(fmsg); f := fmsg.frame
		ELSE
			amsg.frame := NIL; Views.Broadcast(v, amsg); f := amsg.frame
		END;
		IF f # NIL THEN v.ShowRangeIn(f, beg, end) END
	END ShowRange;


	(* StdDirectory *)

	PROCEDURE (d: StdDirectory) New (text: TextModels.Model): View;
		VAR v: StdView; c: Controllers.Controller; r: TextRulers.Ruler;
	BEGIN
		r := TextRulers.dir.New(NIL);
		IF text = NIL THEN text := TextModels.dir.New() END;
		(* IF text.Domain() # NIL THEN Stores.InitDomain(r, text.Domain()) END; *)
		Stores.Join(text, r);
		NEW(v); v.hideMarks := FALSE; v.bot := 0; v.org := 0; v.dy := 0;
		v.InitModel(text);
		v.SetDefaults(r, d.defAttr);
		v.SetSetter(TextSetters.dir.New());
		v.DisplayMarks(hide);
		IF ctrlDir # NIL THEN v.SetController(ctrlDir.New()) END;
		(* Stores.InitDomain(v, text.Domain()); *)
		Stores.Join(v, text);
		RETURN v
	END New;


	PROCEDURE Init;
		VAR d: StdDirectory; a: TextModels.Attributes; res: INTEGER;
	BEGIN
		Dialog.Call("TextControllers.Install", "#Text:CntrlInstallFailed", res);
		NEW(a); a.InitFromProp(NIL);	(* use defaults *)
		NEW(d); d.defAttr := a;
		stdDir := d; dir := d
	END Init;

BEGIN
	Init
END TextViews.
