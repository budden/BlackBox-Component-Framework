MODULE TextRulers;
(**
	project	= "BlackBox"
	organization	= "www.oberon.ch"
	contributors	= "Oberon microsystems"
	version	= "System/Rsrc/About"
	copyright	= "System/Rsrc/About"
	license	= "Docu/BB-License"
	changes	= "
	- 20080904, et, Precondition check 21 in SetDir corrected
	"
	issues	= ""

**)

	(* re-check alien attributes: consider projection semantics *)

	IMPORT
		Kernel, Strings, Services, Fonts, Ports, Stores,
		Models, Views, Controllers, Properties, Dialog,
		TextModels;

	CONST
		(** Attributes.valid, Prop.known/valid **)	(* Mark.kind *)
		first* = 0; left* = 1; right* = 2; lead* = 3; asc* = 4; dsc* = 5; grid* = 6;
		opts* = 7; tabs* = 8;
		(* additional values for icons held by Mark.kind *)
		invalid = -1;
		firstIcon = 10; lastIcon = 25;
		rightToggle = 10;
		gridDec = 12; gridVal = 13; gridInc = 14;
		leftFlush = 16; centered = 17; rightFlush = 18; justified = 19;
		leadDec = 21; leadVal = 22; leadInc = 23;
		pageBrk = 25;

		modeIcons = {leftFlush .. justified};
		validIcons = {rightToggle, gridDec .. gridInc, leftFlush .. justified, leadDec .. leadInc, pageBrk};
		fieldIcons = {gridVal, leadVal};

		(** Attributes.opts **)
		leftAdjust* = 0; rightAdjust* = 1;
				(** both: fully justified; none: centered **)
		noBreakInside* = 2; pageBreak* = 3; parJoin* = 4;
				(** pageBreak of this ruler overrides parJoin request of previous ruler **)
		rightFixed* = 5;	(** has fixed right border **)

		options = {leftAdjust .. rightFixed};	(* options mask *)
		adjMask = {leftAdjust, rightAdjust};

		(** Attributes.tabType[i] **)
		maxTabs* = 32;
		centerTab* = 0; rightTab* = 1;
			(** both: (reserved); none: leftTab **)
		barTab* = 2;

		tabOptions = {centerTab .. barTab};	(* mask for presently valid options *)

		mm = Ports.mm; inch16 = Ports.inch DIV 16; point = Ports.point;
		tabBarHeight = 11 * point; scaleHeight = 10 * point; iconBarHeight = 14 * point;
		rulerHeight = tabBarHeight + scaleHeight + iconBarHeight;
		iconHeight = 10 * point; iconWidth = 12 * point; iconGap = 2 * point;
		iconPin = rulerHeight - (iconBarHeight - iconHeight) DIV 2;

		rulerChangeKey = "#Text:RulerChange";

		minVersion = 0;
		maxAttrVersion = 2; maxStyleVersion = 0; maxStdStyleVersion = 0;
		maxRulerVersion = 0; maxStdRulerVersion = 0;


	TYPE
		Tab* = RECORD
			stop*: INTEGER;
			type*: SET
		END;

		TabArray* = RECORD	(* should be POINTER TO ARRAY OF Tab -- but cannot protect *)
			len*: INTEGER;
			tab*: ARRAY maxTabs OF Tab
		END;

		Attributes* = POINTER TO EXTENSIBLE RECORD (Stores.Store)
			init-: BOOLEAN;	(* immutable once init holds *)
			first-, left-, right-, lead-, asc-, dsc-, grid-: INTEGER;
			opts-: SET;
			tabs-: TabArray
		END;

		AlienAttributes* = POINTER TO RECORD (Attributes)
			store-: Stores.Alien
		END;

		Style* = POINTER TO ABSTRACT RECORD (Models.Model)
			attr-: Attributes
		END;

		Ruler* = POINTER TO ABSTRACT RECORD (Views.View)
			style-: Style
		END;


		Prop* = POINTER TO RECORD (Properties.Property)
			first*, left*, right*, lead*, asc*, dsc*, grid*: INTEGER;
			opts*: RECORD val*, mask*: SET END;
			tabs*: TabArray
		END;


		UpdateMsg* = RECORD (Models.UpdateMsg)
			(** domaincast upon style update **)
			style*: Style;
			oldAttr*: Attributes
		END;


		Directory* = POINTER TO ABSTRACT RECORD
			attr-: Attributes
		END;


		StdStyle = POINTER TO RECORD (Style) END;

		StdRuler = POINTER TO RECORD (Ruler)
			sel: INTEGER;	(* sel # invalid => sel = kind of selected mark *)
			px, py: INTEGER	(* sel # invalid => px, py of selected mark *)
		END;

		StdDirectory = POINTER TO RECORD (Directory) END;

		Mark = RECORD
			ruler: StdRuler;
			l, r, t, b: INTEGER;
			px, py, px0, py0, x, y: INTEGER;
			kind, index: INTEGER;
			type: SET;	(* valid if kind = tabs *)
			tabs: TabArray;	(* if valid: tabs[index].type = type *)
			dirty: BOOLEAN
		END;

		SetAttrOp = POINTER TO RECORD (Stores.Operation)
			style: Style;
			attr: Attributes
		END;

		NeutralizeMsg = RECORD (Views.Message) END;


	VAR
		dir-, stdDir-: Directory;

		def: Attributes;
		prop: Prop;	(* recycled *)
		globRd: TextModels.Reader;	(* cache for temp reader; beware of reentrance *)
		font: Fonts.Font;

		marginGrid, minTabWidth, tabGrid: INTEGER;


	PROCEDURE ^ DoSetAttrOp (s: Style; attr: Attributes);

	PROCEDURE CopyTabs (IN src: TabArray; OUT dst: TabArray);
	(* a TabArray is a 256 byte structure - copying of used parts is much faster than ":= all" *)
		VAR i, n: INTEGER;
	BEGIN
		n := src.len; dst.len := n;
		i := 0; WHILE i < n DO dst.tab[i] := src.tab[i]; INC(i) END
	END CopyTabs;


	(** Attributes **)

	PROCEDURE (a: Attributes) CopyFrom- (source: Stores.Store), EXTENSIBLE;
	BEGIN
		WITH source: Attributes DO
			ASSERT(~a.init, 20); ASSERT(source.init, 21);
			a.init := TRUE;
			a.first := source.first; a.left := source.left; a.right := source.right;
			a.lead := source.lead; a.asc := source.asc; a.dsc := source.dsc; a.grid := source.grid;
			a.opts := source.opts;
			CopyTabs(source.tabs, a.tabs)
		END
	END CopyFrom;

	PROCEDURE (a: Attributes) Externalize- (VAR wr: Stores.Writer), EXTENSIBLE;
	(** pre: a.init **)
		VAR i: INTEGER; typedTabs: BOOLEAN;
	BEGIN
		ASSERT(a.init, 20);
		a.Externalize^(wr);
		i := 0; WHILE (i < a.tabs.len) & (a.tabs.tab[i].type = {}) DO INC(i) END;
		typedTabs := i < a.tabs.len;
		IF typedTabs THEN
			wr.WriteVersion(maxAttrVersion)
		ELSE
			wr.WriteVersion(1)	(* versions before 2 had only leftTabs *)
		END;
		wr.WriteInt(a.first); wr.WriteInt(a.left); wr.WriteInt(a.right);
		wr.WriteInt(a.lead); wr.WriteInt(a.asc); wr.WriteInt(a.dsc); wr.WriteInt(a.grid);
		wr.WriteSet(a.opts);
		wr.WriteXInt(a.tabs.len);
		i := 0; WHILE i < a.tabs.len DO wr.WriteInt(a.tabs.tab[i].stop); INC(i) END;
		IF typedTabs THEN
			i := 0; WHILE i < a.tabs.len DO wr.WriteSet(a.tabs.tab[i].type); INC(i) END
		END
	END Externalize;

	PROCEDURE (a: Attributes) Internalize- (VAR rd: Stores.Reader), EXTENSIBLE;
	(** pre: ~a.init **)
	(** post: a.init **)
		VAR thisVersion, i, n, trash: INTEGER; trashSet: SET;
	BEGIN
		ASSERT(~a.init, 20); a.init := TRUE;
		a.Internalize^(rd);
		IF rd.cancelled THEN RETURN END;
		rd.ReadVersion(minVersion, maxAttrVersion, thisVersion);
		IF rd.cancelled THEN RETURN END;
		rd.ReadInt(a.first); rd.ReadInt(a.left); rd.ReadInt(a.right);
		rd.ReadInt(a.lead); rd.ReadInt(a.asc); rd.ReadInt(a.dsc); rd.ReadInt(a.grid);
		rd.ReadSet(a.opts);
		rd.ReadXInt(n); a.tabs.len := MIN(n, maxTabs);
		i := 0; WHILE i < a.tabs.len DO rd.ReadInt(a.tabs.tab[i].stop); INC(i) END;
		WHILE i < n DO rd.ReadInt(trash); INC(i) END;
		IF thisVersion = 0 THEN	(* convert from v0 rightFixed to v1 ~rightFixed default *)
			INCL(a.opts, rightFixed)
		END;
		IF thisVersion >= 2 THEN
			i := 0; WHILE i < a.tabs.len DO rd.ReadSet(a.tabs.tab[i].type); INC(i) END;
			WHILE i < n DO rd.ReadSet(trashSet); INC(i) END
		ELSE
			i := 0; WHILE i < a.tabs.len DO a.tabs.tab[i].type := {}; INC(i) END
		END
	END Internalize;

	PROCEDURE Set (p: Prop; opt: INTEGER; VAR x: INTEGER; min, max, new: INTEGER);
	BEGIN
		IF opt IN p.valid THEN x := MAX(min, MIN(max, new)) END
	END Set;

	PROCEDURE ModifyFromProp (a: Attributes; p: Properties.Property);
		CONST maxW = 10000*mm; maxH = 32767 * point;
		VAR i: INTEGER; type, mask: SET;
	BEGIN
		WHILE p # NIL DO
			WITH p: Prop DO
				Set(p, first, a.first, 0, maxW, p.first);
				Set(p, left, a.left, 0, maxW, p.left);
				Set(p, right, a.right, MAX(a.left, a.first), maxW, p.right);
				Set(p, lead, a.lead, 0, maxH, p.lead);
				Set(p, asc, a.asc, 0, maxH, p.asc);
				Set(p, dsc, a.dsc, 0, maxH - a.asc, p.dsc);
				Set(p, grid, a.grid, 1, maxH, p.grid);
				IF opts IN p.valid THEN
					a.opts := a.opts * (-p.opts.mask) + p.opts.val * p.opts.mask
				END;
				IF (tabs IN p.valid) & (p.tabs.len >= 0) THEN
					IF (p.tabs.len > 0) & (p.tabs.tab[0].stop >= 0) THEN
						i := 0; a.tabs.len := MIN(p.tabs.len, maxTabs);
						REPEAT
							a.tabs.tab[i].stop := p.tabs.tab[i].stop;
							type := p.tabs.tab[i].type; mask := tabOptions;
							IF type * {centerTab, rightTab} = {centerTab, rightTab} THEN
								mask := mask - {centerTab, rightTab}
							END;
							a.tabs.tab[i].type := a.tabs.tab[i].type * (-mask) + type * mask;
							INC(i)
						UNTIL (i = a.tabs.len) OR (p.tabs.tab[i].stop < p.tabs.tab[i - 1].stop);
						a.tabs.len := i
					ELSE a.tabs.len := 0
					END
				END
			ELSE
			END;
			p := p.next
		END
	END ModifyFromProp;

	PROCEDURE (a: Attributes) ModifyFromProp- (p: Properties.Property), NEW, EXTENSIBLE;
	BEGIN
		ModifyFromProp(a, p)
	END ModifyFromProp;

	PROCEDURE (a: Attributes) InitFromProp* (p: Properties.Property), NEW, EXTENSIBLE;
	(** pre: ~a.init **)
	(** post: (a.init, p # NIL & x IN p.valid) => x set in a, else x defaults in a **)
	BEGIN
		ASSERT(~a.init, 20);
		a.init := TRUE;
		a.first := def.first; a.left := def.left; a.right := def.right;
		a.lead := def.lead; a.asc := def.asc; a.dsc := def.dsc; a.grid := def.grid;
		a.opts := def.opts;
		CopyTabs(def.tabs, a.tabs);
		ModifyFromProp(a, p)
	END InitFromProp;

	PROCEDURE (a: Attributes) Equals* (b: Attributes): BOOLEAN, NEW, EXTENSIBLE;
	(** pre: a.init, b.init **)
		VAR i: INTEGER;
	BEGIN
		ASSERT(a.init, 20); ASSERT(b.init, 21);
		IF a # b THEN
			i := 0;
			WHILE (i < a.tabs.len)
			 & (a.tabs.tab[i].stop = b.tabs.tab[i].stop)
			 & (a.tabs.tab[i].type = b.tabs.tab[i].type) DO
				INC(i)
			END;
			RETURN (Services.SameType(a, b))
				& (a.first = b.first) & (a.left = b.left) & (a.right = b.right)
				& (a.lead = b.lead) & (a.asc = b.asc) & (a.dsc = b.dsc) & (a.grid = b.grid)
				& (a.opts = b.opts) & (a.tabs.len = b.tabs.len) & (i = a.tabs.len)
		ELSE RETURN TRUE
		END
	END Equals;

	PROCEDURE (a: Attributes) Prop* (): Properties.Property, NEW, EXTENSIBLE;
	(** pre: a.init **)
	(** post: x attr in a => x IN p.valid, m set to value of attr in a **)
		VAR p: Prop;
	BEGIN
		ASSERT(a.init, 20);
		NEW(p);
		p.known := {first .. tabs}; p.valid := p.known;
		p.first := a.first; p.left := a.left; p.right := a.right;
		p.lead := a.lead; p.asc := a.asc; p.dsc := a.dsc; p.grid := a.grid;
		p.opts.val := a.opts; p.opts.mask := options;
		CopyTabs(a.tabs, p.tabs);
		RETURN p
	END Prop;


	PROCEDURE ReadAttr* (VAR rd: Stores.Reader; OUT a: Attributes);
		VAR st: Stores.Store; alien: AlienAttributes;
	BEGIN
		rd.ReadStore(st);
		ASSERT(st # NIL, 100);
		IF st IS Stores.Alien THEN
			NEW(alien); alien.store := st(Stores.Alien); Stores.Join(alien, alien.store);
			alien.InitFromProp(NIL); a := alien
		ELSE a := st(Attributes)
		END
	END ReadAttr;

	PROCEDURE WriteAttr* (VAR wr: Stores.Writer; a: Attributes);
	BEGIN
		ASSERT(a # NIL, 20); ASSERT(a.init, 21);
		WITH a: AlienAttributes DO wr.WriteStore(a.store) ELSE wr.WriteStore(a) END
	END WriteAttr;

	PROCEDURE ModifiedAttr* (a: Attributes; p: Properties.Property): Attributes;
	(** pre: a.init **)
	(** post: x IN p.valid => x in new attr set to value in p, else set to value in a **)
		VAR h: Attributes;
	BEGIN
		ASSERT(a.init, 20);
		h := Stores.CopyOf(a)(Attributes); h.ModifyFromProp(p);
		RETURN h
	END ModifiedAttr;


	(** AlienAttributes **)

	PROCEDURE (a: AlienAttributes) Externalize- (VAR wr: Stores.Writer);
	BEGIN
		HALT(100)
	END Externalize;

	PROCEDURE (a: AlienAttributes) Internalize- (VAR rd: Stores.Reader);
	BEGIN
		HALT(100)
	END Internalize;

	PROCEDURE (a: AlienAttributes) InitFromProp* (p: Properties.Property);
	BEGIN
		a.InitFromProp^(NIL)
	END InitFromProp;

	PROCEDURE (a: AlienAttributes) ModifyFromProp- (p: Properties.Property);
	BEGIN
		(* a.InitFromProp^(NIL) *)
		a.InitFromProp(NIL)
	END ModifyFromProp;


	(** Style **)

(*
	PROCEDURE (s: Style) PropagateDomain-, EXTENSIBLE;
		VAR dom: Stores.Domain;
	BEGIN
		ASSERT(s.attr # NIL, 20);
		dom := s.attr.Domain();
		IF (dom # NIL) & (dom # s.Domain()) THEN s.attr := Stores.CopyOf(s.attr)(Attributes) END;
		Stores.InitDomain(s.attr, s.Domain())
	END PropagateDomain;
*)

	PROCEDURE (s: Style) Externalize- (VAR wr: Stores.Writer), EXTENSIBLE;
	BEGIN
		s.Externalize^(wr);
		wr.WriteVersion(maxStyleVersion);
		WriteAttr(wr, s.attr)
	END Externalize;

	PROCEDURE (s: Style) Internalize- (VAR rd: Stores.Reader), EXTENSIBLE;
		VAR thisVersion: INTEGER;
	BEGIN
		s.Internalize^(rd);
		IF rd.cancelled THEN RETURN END;
		rd.ReadVersion(minVersion, maxStyleVersion, thisVersion);
		IF rd.cancelled THEN RETURN END;
		ReadAttr(rd, s.attr); Stores.Join(s, s.attr)
	END Internalize;

	PROCEDURE (s: Style) SetAttr* (attr: Attributes), NEW, EXTENSIBLE;
	(** pre: attr.init **)
	(** post: s.attr = attr OR s.attr.Equals(attr) **)
	BEGIN
		ASSERT(attr.init, 20);
		DoSetAttrOp(s, attr)
	END SetAttr;

	PROCEDURE (s: Style) CopyFrom- (source: Stores.Store), EXTENSIBLE;
	BEGIN
		WITH source: Style DO
			ASSERT(source.attr # NIL, 21);
			s.SetAttr(Stores.CopyOf(source.attr)(Attributes))
				(* bkwd-comp hack to avoid link *)
				(* copy would not be necessary if Attributes were immutable (and assigned to an Immutable Domain) *)
		END
	END CopyFrom;
	
(*
	PROCEDURE (s: Style) InitFrom- (source: Models.Model), EXTENSIBLE;
	BEGIN
		WITH source: Style DO
			ASSERT(source.attr # NIL, 21);
			s.SetAttr(Stores.CopyOf(source.attr)(Attributes))
				(* bkwd-comp hack to avoid link *)
		END
	END InitFrom;
*)

	(** Directory **)

	PROCEDURE (d: Directory) SetAttr* (attr: Attributes), NEW, EXTENSIBLE;
	(** pre: attr.init **)
	(** post: d.attr = ModifiedAttr(attr, p)
		[ p.valid = {opts, tabs}, p.tabs.len = 0, p.opts.mask = {noBreakInside.. parJoin}, p.opts.val = {} ]
	**)
		VAR p: Prop;
	BEGIN
		ASSERT(attr.init, 20);
		IF attr.tabs.len > 0 THEN
			NEW(p);
			p.valid := {opts, tabs};
			p.opts.mask := {noBreakInside, pageBreak, parJoin}; p.opts.val := {};
			p.tabs.len := 0;
			attr := ModifiedAttr(attr, p)
		END;
		d.attr := attr
	END SetAttr;

	PROCEDURE (d: Directory) NewStyle* (attr: Attributes): Style, NEW, ABSTRACT;
	PROCEDURE (d: Directory) New* (style: Style): Ruler, NEW, ABSTRACT;

	PROCEDURE (d: Directory) NewFromProp* (p: Prop): Ruler, NEW, EXTENSIBLE;
	BEGIN
		RETURN d.New(d.NewStyle(ModifiedAttr(d.attr, p)))
	END NewFromProp;


	PROCEDURE Deposit*;
	BEGIN
		Views.Deposit(dir.New(NIL))
	END Deposit;


	(** Ruler **)

	PROCEDURE (r: Ruler) Externalize- (VAR wr: Stores.Writer), EXTENSIBLE;
	BEGIN
		ASSERT(r.style # NIL, 20);
		r.Externalize^(wr);
		wr.WriteVersion(maxRulerVersion); wr.WriteStore(r.style)
	END Externalize;
	
	PROCEDURE (r: Ruler) InitStyle* (s: Style), NEW;
	(** pre: r.style = NIL, s # NIL, style.attr # NIL **)
	(** post: r.style = s **)
	BEGIN
		ASSERT((r.style = NIL) OR (r.style = s), 20);
		ASSERT(s # NIL, 21); ASSERT(s.attr # NIL, 22);
		r.style := s; Stores.Join(r, s)
	END InitStyle;
	

	PROCEDURE (r: Ruler) Internalize- (VAR rd: Stores.Reader), EXTENSIBLE;
		VAR st: Stores.Store; thisVersion: INTEGER;
	BEGIN
		r.Internalize^(rd);
		IF rd.cancelled THEN RETURN END;
		rd.ReadVersion(minVersion, maxRulerVersion, thisVersion);
		IF rd.cancelled THEN RETURN END;
		rd.ReadStore(st);
		IF st IS Stores.Alien THEN rd.TurnIntoAlien(Stores.alienComponent); RETURN END;
		r.InitStyle(st(Style))
	END Internalize;

(*
	PROCEDURE (r: Ruler) InitModel* (m: Models.Model), EXTENSIBLE;
	(** pre: r.style = NIL, m # NIL, style.attr # NIL, m IS Style **)
	(** post: r.style = m **)
	BEGIN
		WITH m: Style DO
			ASSERT((r.style = NIL) OR (r.style = m), 20);
			ASSERT(m # NIL, 21); ASSERT(m.attr # NIL, 22);
			r.style := m
		ELSE HALT(23)
		END
	END InitModel;
*)

(*
	PROCEDURE (r: Ruler) PropagateDomain-, EXTENSIBLE;
	BEGIN
		ASSERT(r.style # NIL, 20);
		Stores.InitDomain(r.style, r.Domain())
	END PropagateDomain;
*)

	PROCEDURE CopyOf* (r: Ruler; shallow: BOOLEAN): Ruler;
		VAR v: Views.View;
	BEGIN
		ASSERT(r # NIL, 20);
		v := Views.CopyOf(r, shallow); RETURN v(Ruler)
	END CopyOf;


	(** Prop **)

	PROCEDURE (p: Prop) IntersectWith* (q: Properties.Property; OUT equal: BOOLEAN);
		VAR valid: SET;  i: INTEGER; c, m: SET; eq: BOOLEAN;
	BEGIN
		WITH q: Prop DO
			valid := p.valid * q.valid; equal := TRUE;
			i := 0;
			WHILE (i < p.tabs.len)
			& (p.tabs.tab[i].stop = q.tabs.tab[i].stop)
			& (p.tabs.tab[i].type = q.tabs.tab[i].type)
			DO
				INC(i)
			END;
			IF p.first # q.first THEN EXCL(valid, first) END;
			IF p.left # q.left THEN EXCL(valid, left) END;
			IF p.right # q.right THEN EXCL(valid, right) END;
			IF p.lead # q.lead THEN EXCL(valid, lead) END;
			IF p.asc # q.asc THEN EXCL(valid, asc) END;
			IF p.dsc # q.dsc THEN EXCL(valid, dsc) END;
			IF p.grid # q.grid THEN EXCL(valid, grid) END;
			Properties.IntersectSelections(p.opts.val, p.opts.mask, q.opts.val, q.opts.mask, c, m, eq);
			IF m = {} THEN EXCL(valid, opts)
			ELSIF (opts IN valid) & ~eq THEN p.opts.mask := m; equal := FALSE
			END;
			IF (p.tabs.len # q.tabs.len) OR (q.tabs.len # i) THEN EXCL(valid, tabs) END;
			IF p.valid # valid THEN p.valid := valid; equal := FALSE END
		END
	END IntersectWith;


	(** ruler construction **)

(*property-based facade procedures *)
(*
	PROCEDURE SetFirst* (p: Prop; x: INTEGER);
	BEGIN
		ASSERT(p # NIL, 20);
		INCL(p.valid, first); p.first := x
	END SetFirst;

	PROCEDURE SetLeft* (p: Prop; x: INTEGER);
	BEGIN
		ASSERT(p # NIL, 20);
		INCL(p.valid, left); p.left := x
	END SetLeft;

	PROCEDURE SetRight* (p: Prop; x: INTEGER);
	BEGIN
		ASSERT(p # NIL, 20);
		INCL(p.valid, right); p.right := x
	END SetRight;

	PROCEDURE SetFixedRight* (p: Prop; x: INTEGER);
	BEGIN
		ASSERT(p # NIL, 20);
		INCL(p.valid, right); p.right := x;
		INCL(p.valid, opts); INCL(p.opts.mask, rightFixed); INCL(p.opts.val, rightFixed)
	END SetFixedRight;


	PROCEDURE SetLead* (p: Prop; h: INTEGER);
	BEGIN
		ASSERT(p # NIL, 20);
		INCL(p.valid, lead); p.lead := h
	END SetLead;

	PROCEDURE SetAsc* (p: Prop; h: INTEGER);
	BEGIN
		ASSERT(p # NIL, 20);
		INCL(p.valid, asc); p.asc := h
	END SetAsc;

	PROCEDURE SetDsc* (p: Prop; h: INTEGER);
	BEGIN
		ASSERT(p # NIL, 20);
		INCL(p.valid, dsc); p.dsc := h
	END SetDsc;

	PROCEDURE SetGrid* (p: Prop; h: INTEGER);
	BEGIN
		ASSERT(p # NIL, 20);
		INCL(p.valid, grid); p.grid := h
	END SetGrid;


	PROCEDURE SetLeftFlush* (p: Prop);
	BEGIN
		ASSERT(p # NIL, 20);
		INCL(p.valid, opts);
		p.opts.mask := p.opts.mask + {leftAdjust, rightAdjust};
		p.opts.val := p.opts.val + {leftAdjust} - {rightAdjust}
	END SetLeftFlush;

	PROCEDURE SetRightFlush* (p: Prop);
	BEGIN
		ASSERT(p # NIL, 20);
		INCL(p.valid, opts);
		p.opts.mask := p.opts.mask + {leftAdjust, rightAdjust};
		p.opts.val := p.opts.val - {leftAdjust} + {rightAdjust}
	END SetRightFlush;

	PROCEDURE SetCentered* (p: Prop);
	BEGIN
		ASSERT(p # NIL, 20);
		INCL(p.valid, opts);
		p.opts.mask := p.opts.mask + {leftAdjust, rightAdjust};
		p.opts.val := p.opts.val - {leftAdjust, rightAdjust}
	END SetCentered;

	PROCEDURE SetJustified* (p: Prop);
	BEGIN
		ASSERT(p # NIL, 20);
		INCL(p.valid, opts);
		p.opts.mask := p.opts.mask + {leftAdjust, rightAdjust};
		p.opts.val := p.opts.val + {leftAdjust, rightAdjust}
	END SetJustified;


	PROCEDURE SetNoBreakInside* (p: Prop);
	BEGIN
		ASSERT(p # NIL, 20);
		INCL(p.valid, opts);
		INCL(p.opts.mask, noBreakInside); INCL(p.opts.val, noBreakInside)
	END SetNoBreakInside;

	PROCEDURE SetPageBreak* (p: Prop);
	BEGIN
		ASSERT(p # NIL, 20);
		INCL(p.valid, opts);
		INCL(p.opts.mask, pageBreak); INCL(p.opts.val, pageBreak)
	END SetPageBreak;

	PROCEDURE SetParJoin* (p: Prop);
	BEGIN
		ASSERT(p # NIL, 20);
		INCL(p.valid, opts);
		INCL(p.opts.mask, parJoin); INCL(p.opts.val, parJoin)
	END SetParJoin;


	PROCEDURE AddTab* (p: Prop; x: INTEGER);
		VAR i: INTEGER;
	BEGIN
		ASSERT(p # NIL, 20); i := p.tabs.len; ASSERT(i > 0, 21);
		ASSERT((i = 0) OR (p.tabs.tab[i - 1].stop < x), 22);
		INCL(p.valid, tabs);
		p.tabs.tab[i].stop := x; p.tabs.tab[i].type := {};
		p.tabs.len := i + 1
	END AddTab;

	PROCEDURE MakeCenterTab* (p: Prop);
		VAR i: INTEGER;
	BEGIN
		ASSERT(p # NIL, 20); i := p.tabs.len; ASSERT(i > 0, 21);
		p.tabs.tab[i - 1].type := p.tabs.tab[i - 1].type + {centerTab} - {rightTab}
	END MakeCenterTab;

	PROCEDURE MakeRightTab* (p: Prop);
		VAR i: INTEGER;
	BEGIN
		ASSERT(p # NIL, 20); i := p.tabs.len; ASSERT(i > 0, 21);
		p.tabs.tab[i - 1].type := p.tabs.tab[i - 1].type - {centerTab} + {rightTab}
	END MakeRightTab;

	PROCEDURE MakeBarTab* (p: Prop);
		VAR i: INTEGER;
	BEGIN
		ASSERT(p # NIL, 20); i := p.tabs.len; ASSERT(i > 0, 21);
		p.tabs.tab[i - 1].type := p.tabs.tab[i - 1].type + {barTab}
	END MakeBarTab;
*)

	PROCEDURE SetFirst* (r: Ruler; x: INTEGER);
	BEGIN
		ASSERT(r.style # NIL, 20);
		prop.valid := {first}; prop.first := x;
		r.style.SetAttr(ModifiedAttr(r.style.attr, prop))
	END SetFirst;

	PROCEDURE SetLeft* (r: Ruler; x: INTEGER);
	BEGIN
		ASSERT(r.style # NIL, 20);
		prop.valid := {left}; prop.left := x;
		r.style.SetAttr(ModifiedAttr(r.style.attr, prop))
	END SetLeft;

	PROCEDURE SetRight* (r: Ruler; x: INTEGER);
	BEGIN
		ASSERT(r.style # NIL, 20);
		prop.valid := {right}; prop.right := x;
		prop.opts.mask := {rightFixed}; prop.opts.val := {};
		r.style.SetAttr(ModifiedAttr(r.style.attr, prop))
	END SetRight;

	PROCEDURE SetFixedRight* (r: Ruler; x: INTEGER);
	BEGIN
		ASSERT(r.style # NIL, 20);
		prop.valid := {right, opts}; prop.right := x;
		prop.opts.mask := {rightFixed}; prop.opts.val := {rightFixed};
		r.style.SetAttr(ModifiedAttr(r.style.attr, prop))
	END SetFixedRight;


	PROCEDURE SetLead* (r: Ruler; h: INTEGER);
	BEGIN
		ASSERT(r.style # NIL, 20);
		prop.valid := {lead}; prop.lead := h;
		r.style.SetAttr(ModifiedAttr(r.style.attr, prop))
	END SetLead;

	PROCEDURE SetAsc* (r: Ruler; h: INTEGER);
	BEGIN
		ASSERT(r.style # NIL, 20);
		prop.valid := {asc}; prop.asc := h;
		r.style.SetAttr(ModifiedAttr(r.style.attr, prop))
	END SetAsc;

	PROCEDURE SetDsc* (r: Ruler; h: INTEGER);
	BEGIN
		ASSERT(r.style # NIL, 20);
		prop.valid := {dsc}; prop.dsc := h;
		r.style.SetAttr(ModifiedAttr(r.style.attr, prop))
	END SetDsc;

	PROCEDURE SetGrid* (r: Ruler; h: INTEGER);
	BEGIN
		ASSERT(r.style # NIL, 20);
		prop.valid := {grid}; prop.grid := h;
		r.style.SetAttr(ModifiedAttr(r.style.attr, prop))
	END SetGrid;


	PROCEDURE SetLeftFlush* (r: Ruler);
	BEGIN
		ASSERT(r.style # NIL, 20);
		prop.valid := {opts};
		prop.opts.mask := {leftAdjust, rightAdjust}; prop.opts.val := {leftAdjust};
		r.style.SetAttr(ModifiedAttr(r.style.attr, prop))
	END SetLeftFlush;

	PROCEDURE SetRightFlush* (r: Ruler);
	BEGIN
		ASSERT(r.style # NIL, 20);
		prop.valid := {opts};
		prop.opts.mask := {leftAdjust, rightAdjust}; prop.opts.val := {rightAdjust};
		r.style.SetAttr(ModifiedAttr(r.style.attr, prop))
	END SetRightFlush;

	PROCEDURE SetCentered* (r: Ruler);
	BEGIN
		ASSERT(r.style # NIL, 20);
		prop.valid := {opts};
		prop.opts.mask := {leftAdjust, rightAdjust}; prop.opts.val := {};
		r.style.SetAttr(ModifiedAttr(r.style.attr, prop))
	END SetCentered;

	PROCEDURE SetJustified* (r: Ruler);
	BEGIN
		ASSERT(r.style # NIL, 20);
		prop.valid := {opts};
		prop.opts.mask := {leftAdjust, rightAdjust}; prop.opts.val := {leftAdjust, rightAdjust};
		r.style.SetAttr(ModifiedAttr(r.style.attr, prop))
	END SetJustified;


	PROCEDURE SetNoBreakInside* (r: Ruler);
	BEGIN
		ASSERT(r.style # NIL, 20);
		prop.valid := {opts};
		prop.opts.mask := {noBreakInside}; prop.opts.val := {noBreakInside};
		r.style.SetAttr(ModifiedAttr(r.style.attr, prop))
	END SetNoBreakInside;

	PROCEDURE SetPageBreak* (r: Ruler);
	BEGIN
		ASSERT(r.style # NIL, 20);
		prop.valid := {opts};
		prop.opts.mask := {pageBreak}; prop.opts.val := {pageBreak};
		r.style.SetAttr(ModifiedAttr(r.style.attr, prop))
	END SetPageBreak;

	PROCEDURE SetParJoin* (r: Ruler);
	BEGIN
		ASSERT(r.style # NIL, 20);
		prop.valid := {opts};
		prop.opts.mask := {parJoin}; prop.opts.val := {parJoin};
		r.style.SetAttr(ModifiedAttr(r.style.attr, prop))
	END SetParJoin;


	PROCEDURE AddTab* (r: Ruler; x: INTEGER);
		VAR ra: Attributes; i: INTEGER;
	BEGIN
		ASSERT(r.style # NIL, 20); ra := r.style.attr; i := ra.tabs.len; ASSERT(i < maxTabs, 21);
		ASSERT((i = 0) OR (ra.tabs.tab[i - 1].stop < x), 22);
		prop.valid := {tabs};
		CopyTabs(ra.tabs, prop.tabs);
		prop.tabs.tab[i].stop := x; prop.tabs.tab[i].type := {}; INC(prop.tabs.len);
		r.style.SetAttr(ModifiedAttr(ra, prop))
	END AddTab;

	PROCEDURE MakeCenterTab* (r: Ruler);
		VAR ra: Attributes; i: INTEGER;
	BEGIN
		ASSERT(r.style # NIL, 20); ra := r.style.attr; i := ra.tabs.len; ASSERT(i > 0, 21);
		prop.valid := {tabs};
		CopyTabs(ra.tabs, prop.tabs);
		prop.tabs.tab[i - 1].type := prop.tabs.tab[i - 1].type + {centerTab} - {rightTab};
		r.style.SetAttr(ModifiedAttr(ra, prop))
	END MakeCenterTab;

	PROCEDURE MakeRightTab* (r: Ruler);
		VAR ra: Attributes; i: INTEGER;
	BEGIN
		ASSERT(r.style # NIL, 20); ra := r.style.attr; i := ra.tabs.len; ASSERT(i > 0, 21);
		prop.valid := {tabs};
		CopyTabs(ra.tabs, prop.tabs);
		prop.tabs.tab[i - 1].type := prop.tabs.tab[i - 1].type - {centerTab} + {rightTab};
		r.style.SetAttr(ModifiedAttr(ra, prop))
	END MakeRightTab;

	PROCEDURE MakeBarTab* (r: Ruler);
		VAR ra: Attributes; i: INTEGER;
	BEGIN
		ASSERT(r.style # NIL, 20); ra := r.style.attr; i := ra.tabs.len; ASSERT(i > 0, 21);
		prop.valid := {tabs};
		CopyTabs(ra.tabs, prop.tabs);
		prop.tabs.tab[i - 1].type := prop.tabs.tab[i - 1].type + {barTab};
		r.style.SetAttr(ModifiedAttr(ra, prop))
	END MakeBarTab;


	(* SetAttrOp *)

	PROCEDURE (op: SetAttrOp) Do;
		VAR s: Style; attr: Attributes; upd: UpdateMsg;
	BEGIN
		s := op.style;
		attr := s.attr; s.attr := op.attr; op.attr := attr;
		(*Stores.InitDomain(s.attr, s.Domain());*) (* Stores.Join(s, s.attr); *)
		ASSERT((s.attr=NIL) OR Stores.Joined(s, s.attr), 100);
		upd.style := s; upd.oldAttr := attr; Models.Domaincast(s.Domain(), upd)
	END Do;

	PROCEDURE DoSetAttrOp (s: Style; attr: Attributes);
		VAR op: SetAttrOp;
	BEGIN
		IF (s.attr # attr) OR ~s.attr.Equals(attr) THEN
			(* IF attr.Domain() # s.Domain() THEN attr := Stores.CopyOf(attr)(Attributes) END; *)
			IF ~Stores.Joined(s, attr) THEN
				IF ~Stores.Unattached(attr) THEN attr := Stores.CopyOf(attr)(Attributes) END;
				Stores.Join(s, attr)
			END;
			NEW(op); op.style := s; op.attr := attr;
			Models.Do(s, rulerChangeKey, op)
		END
	END DoSetAttrOp;


	(* grid definitions *)

	PROCEDURE MarginGrid (x: INTEGER): INTEGER;
	BEGIN
		RETURN (x + marginGrid DIV 2) DIV marginGrid * marginGrid
	END MarginGrid;

	PROCEDURE TabGrid (x: INTEGER): INTEGER;
	BEGIN
		RETURN (x + tabGrid DIV 2) DIV tabGrid * tabGrid
	END TabGrid;


	(* nice graphical primitives *)

	PROCEDURE DrawCenteredInt (f: Views.Frame; x, y, n: INTEGER);
		VAR sw: INTEGER; s: ARRAY 32 OF CHAR;
	BEGIN
		Strings.IntToString(n, s); sw := font.StringWidth(s);
		f.DrawString(x - sw DIV 2, y, Ports.defaultColor, s, font)
	END DrawCenteredInt;

	PROCEDURE DrawNiceRect (f: Views.Frame; l, t, r, b: INTEGER);
		VAR u: INTEGER;
	BEGIN
		u := f.dot;
		f.DrawRect(l, t, r - u, b - u, 0, Ports.defaultColor);
		f.DrawLine(l + u, b - u, r - u, b - u, u, Ports.grey25);
		f.DrawLine(r - u, t + u, r - u, b - u, u, Ports.grey25)
	END DrawNiceRect;

	PROCEDURE DrawScale (f: Views.Frame; l, t, r, b, clipL, clipR: INTEGER);
		VAR u, h, x, px, sw: INTEGER; i, n, d1, d2: INTEGER; s: ARRAY 32 OF CHAR;
	BEGIN
		f.DrawRect(l, t, r, b, Ports.fill, Ports.grey12);
		u := f.dot;
		IF Dialog.metricSystem THEN d1 := 2; d2 := 10 ELSE d1 := 2; d2 := 16 END;
		DEC(b, point);
		sw := 2*u + font.StringWidth("8888888888");
		x := l + tabGrid; i := 0; n := 0;
		WHILE x <= r DO
			INC(i); px := TabGrid(x);
			IF i = d2 THEN
				h := 6*point; i := 0; INC(n);
				IF (px >= clipL - sw) & (px < clipR) THEN
					Strings.IntToString(n, s);
					f.DrawString(px - 2*u - font.StringWidth(s), b - 3*point, Ports.defaultColor, s, font)
				END
			ELSIF i MOD d1 = 0 THEN
				h := 2*point
			ELSE
				h := 0
			END;
			IF (px >= clipL) & (px < clipR) & (h > 0) THEN
				f.DrawLine(px, b, px, b - h, 0, Ports.defaultColor)
			END;
			INC(x, tabGrid)
		END
	END DrawScale;

	PROCEDURE InvertTabMark (f: Views.Frame; l, t, r, b: INTEGER; type: SET; show: BOOLEAN);
		VAR u, u2, u3, yc, i, ih: INTEGER;
	BEGIN
		u := f.dot; u2 := 2*u; u3 := 3*u;
		IF ~ODD((r - l) DIV u) THEN DEC(r, u) END;
		yc := l + (r - l) DIV u DIV 2 * u;
		IF barTab IN type THEN
			f.MarkRect(yc, b - u3, yc + u, b - u2, Ports.fill, Ports.invert, show);
			f.MarkRect(yc, b - u, yc + u, b, Ports.fill, Ports.invert, show)
		END;
		IF centerTab IN type THEN
			f.MarkRect(l + u, b - u2, r - u, b - u, Ports.fill, Ports.invert, show)
		ELSIF rightTab IN type THEN
			f.MarkRect(l, b - u2, yc + u, b - u, Ports.fill, Ports.invert, show)
		ELSE
			f.MarkRect(yc, b - u2, r, b - u, Ports.fill, Ports.invert, show)
		END;
		DEC(b, u3); INC(l, u2); DEC(r, u2);
		ih := (r - l) DIV 2;
		i := b - t; t := b - u;
		WHILE (i > 0) & (r > l) DO
			DEC(i, u);
			f.MarkRect(l, t, r, b, Ports.fill, Ports.invert, show);
			IF i <= ih THEN INC(l, u); DEC(r, u) END;
			DEC(t, u); DEC(b, u)
		END
	END InvertTabMark;

	PROCEDURE InvertFirstMark (f: Views.Frame; l, t, r, b: INTEGER; show: BOOLEAN);
		VAR u, i, ih: INTEGER;
	BEGIN
		u := f.dot;
		i := b - t; t := b - u;
		ih := r - l;
		WHILE (i > 0) & (r > l) DO
			DEC(i, u);
			f.MarkRect(l, t, r, b, Ports.fill, Ports.invert, show);
			IF i <= ih THEN DEC(r, u) END;
			DEC(t, u); DEC(b, u)
		END
	END InvertFirstMark;

	PROCEDURE InvertLeftMark (f: Views.Frame; l, t, r, b: INTEGER; show: BOOLEAN);
		VAR u, i, ih: INTEGER;
	BEGIN
		u := f.dot;
		i := b - t; b := t + u;
		ih := r - l;
		WHILE (i > 0) & (r > l) DO
			DEC(i, u);
			f.MarkRect(l, t, r, b, Ports.fill, Ports.invert, show);
			IF i <= ih THEN DEC(r, u) END;
			INC(t, u); INC(b, u)
		END
	END InvertLeftMark;

	PROCEDURE InvertRightMark (f: Views.Frame; l, t, r, b: INTEGER; show: BOOLEAN);
		VAR u, i, ih: INTEGER;
	BEGIN
		u := f.dot;
		IF ~ODD((b - t) DIV u) THEN INC(t, u) END;
		ih := r - l; l := r - u;
		i := b - t; b := t + u;
		WHILE (i > 0) & (i > ih) DO
			DEC(i, u);
			f.MarkRect(l, t, r, b, Ports.fill, Ports.invert, show);
			DEC(l, u);
			INC(t, u); INC(b, u)
		END;
		WHILE (i > 0) & (r > l) DO
			DEC(i, u);
			f.MarkRect(l, t, r, b, Ports.fill, Ports.invert, show);
			INC(l, u);
			INC(t, u); INC(b, u)
		END
	END InvertRightMark;


	(* marks *)

	PROCEDURE SetMark (VAR m: Mark; r: StdRuler; px, py: INTEGER; kind, index: INTEGER);
	BEGIN
		m.ruler := r; m.kind := kind;
		m.px := px; m.py := py;
		CASE kind OF
		  first:
			m.l := px; m.r := m.l + 4*point;
			m.b := py - 7*point; m.t := m.b - 4*point
		| left:
			m.l := px; m.r := m.l + 4*point;
			m.b := py - 2*point; m.t := m.b - 4*point
		| right:
			m.r := px; m.l := m.r - 4*point;
			m.b := py - 3*point; m.t := m.b - 7*point
		| tabs:
			m.l := px - 4*point; m.r := m.l + 9*point;
			m.b := py - 5*point; m.t := m.b - 6*point;
			m.type := r.style.attr.tabs.tab[index].type
		| firstIcon .. lastIcon:
			m.l := px; m.r := px + iconWidth;
			m.t := py; m.b := py + iconHeight
		ELSE HALT(100)
		END
	END SetMark;

	PROCEDURE Try (VAR m: Mark; r: StdRuler; px, py, x, y: INTEGER; kind, index: INTEGER);
	BEGIN
		IF m.kind = invalid THEN
			SetMark(m, r, px, py, kind, index);
			IF (m.l - point <= x) & (x < m.r + point) & (m.t - point <= y) & (y < m.b + point) THEN
				m.px0 := m.px; m.py0 := m.py; m.x := x; m.y := y;
				IF kind = tabs THEN
					m.index := index; CopyTabs(r.style.attr.tabs, m.tabs)
				END
			ELSE
				m.kind := invalid
			END
		END
	END Try;

	PROCEDURE InvertMark (VAR m: Mark; f: Views.Frame; show: BOOLEAN);
	(* pre: kind # invalid *)
	BEGIN
		CASE m.kind OF
		  first: InvertFirstMark(f, m.l, m.t, m.r, m.b, show)
		| left: InvertLeftMark(f, m.l, m.t, m.r, m.b, show)
		| right: InvertRightMark(f, m.l, m.t, m.r, m.b, show)
		| tabs: InvertTabMark(f, m.l, m.t, m.r, m.b, m.type, show)
		END
	END InvertMark;

	PROCEDURE HiliteMark (VAR m: Mark; f: Views.Frame; show: BOOLEAN);
	BEGIN
		f.MarkRect(m.l, m.t, m.r - point, m.b - point, Ports.fill, Ports.hilite, show)
	END HiliteMark;

	PROCEDURE HiliteThisMark (r: StdRuler; f: Views.Frame; kind: INTEGER; show: BOOLEAN);
		VAR m: Mark; px,  w, h: INTEGER;
	BEGIN
		IF (kind # invalid) & (kind IN validIcons) THEN
			px := iconGap + (kind - firstIcon) * (iconWidth + iconGap);
			r.context.GetSize(w, h);
			SetMark(m, r, px, h - iconPin, kind, -1);
			HiliteMark(m, f, show)
		END
	END HiliteThisMark;

	PROCEDURE DrawMark (VAR m: Mark; f: Views.Frame);
	(* pre: kind # invalid *)
		VAR a: Attributes; l, t, r, b, y, d, e, asc, dsc, fw: INTEGER; i: INTEGER;
			w: ARRAY 4 OF INTEGER;
	BEGIN
		a := m.ruler.style.attr;
		l := m.l + 2 * point; t := m.t + 2 * point; r := m.r - 4 * point; b := m.b - 3 * point;
		font.GetBounds(asc, dsc, fw);
		y := (m.t + m.b + asc) DIV 2;
		w[0] := (r - l) DIV 2; w[1] := r - l; w[2] := (r - l) DIV 3; w[3] := (r - l) * 2 DIV 3;
		CASE m.kind OF
		  rightToggle:
			IF rightFixed IN a.opts THEN
				d := 0; y := (t + b) DIV 2 - point; e := (l + r) DIV 2 + point;
				WHILE t < y DO
					f.DrawLine(e - d, t, e, t, point, Ports.defaultColor); INC(d, point); INC(t, point)
				END;
				WHILE t < b DO
					f.DrawLine(e - d, t, e, t, point, Ports.defaultColor); DEC(d, point); INC(t, point)
				END
			ELSE
				DEC(b, point);
				f.DrawLine(l, t, r, t, point, Ports.defaultColor);
				f.DrawLine(l, b, r, b, point, Ports.defaultColor);
				f.DrawLine(l, t, l, b, point, Ports.defaultColor);
				f.DrawLine(r, t, r, b, point, Ports.defaultColor)
			END
		| gridDec:
			WHILE t < b DO f.DrawLine(l, t, r, t, point, Ports.defaultColor); INC(t, 2 * point) END
		| gridVal:
			DrawCenteredInt(f, (l + r) DIV 2, y, a.grid DIV point)
		| gridInc:
			WHILE t < b DO f.DrawLine(l, t, r, t, point, Ports.defaultColor); INC(t, 3 * point) END
		| leftFlush:
			i := 0;
			WHILE t < b DO
				d := w[i]; i := (i + 1) MOD LEN(w);
				f.DrawLine(l, t, l + d, t, point, Ports.defaultColor); INC(t, 2 * point)
			END
		| centered:
			i := 0;
			WHILE t < b DO
				d := (r - l - w[i]) DIV 2; i := (i + 1) MOD LEN(w);
				f.DrawLine(l + d, t, r - d, t, point, Ports.defaultColor); INC(t, 2 * point)
			END
		| rightFlush:
			i := 0;
			WHILE t < b DO
				d := w[i]; i := (i + 1) MOD LEN(w);
				f.DrawLine(r - d, t, r, t, point, Ports.defaultColor); INC(t, 2 * point)
			END
		| justified:
			WHILE t < b DO f.DrawLine(l, t, r, t, point, Ports.defaultColor); INC(t, 2 * point) END
		| leadDec:
			f.DrawLine(l, t, l, t + point, point, Ports.defaultColor); INC(t, 2 * point);
			WHILE t < b DO f.DrawLine(l, t, r, t, point, Ports.defaultColor); INC(t, 2 * point) END
		| leadVal:
			DrawCenteredInt(f, (l + r) DIV 2, y, m.ruler.style.attr.lead DIV point)
		| leadInc:
			f.DrawLine(l, t, l, t + 3 * point, point, Ports.defaultColor); INC(t, 4 * point);
			WHILE t < b DO f.DrawLine(l, t, r, t, point, Ports.defaultColor); INC(t, 2 * point) END
		| pageBrk:
			DEC(b, point);
			IF pageBreak IN a.opts THEN
				y := (t + b) DIV 2 - point;
				f.DrawLine(l, t, l, y, point, Ports.defaultColor);
				f.DrawLine(r, t, r, y, point, Ports.defaultColor);
				f.DrawLine(l, y, r, y, point, Ports.defaultColor);
				INC(y, 2 * point);
				f.DrawLine(l, y, r, y, point, Ports.defaultColor);
				f.DrawLine(l, y, l, b, point, Ports.defaultColor);
				f.DrawLine(r, y, r, b, point, Ports.defaultColor)
			ELSE
				f.DrawLine(l, t, l, b, point, Ports.defaultColor);
				f.DrawLine(r, t, r, b, point, Ports.defaultColor)
			END
		ELSE
			HALT(100)
		END;
		IF ~(m.kind IN {gridVal, leadVal}) THEN
			DrawNiceRect(f, m.l, m.t, m.r, m.b)
		END
	END DrawMark;

	PROCEDURE GetMark (VAR m: Mark; r: StdRuler; f: Views.Frame;
		x, y: INTEGER; canCreate: BOOLEAN
	);
	(* pre: ~canCreate OR (f # NIL) *)
		VAR a: Attributes; px,  w, h: INTEGER; i: INTEGER;
	BEGIN
		m.kind := invalid; m.dirty := FALSE;
		a := r.style.attr;
		r.context.GetSize(w, h);

		(* first try scale *)
		Try(m, r, a.first, h, x, y, first, 0);
		Try(m, r, a.left, h, x, y, left, 0);
		IF rightFixed IN a.opts THEN
			Try(m, r, a.right, h, x, y, right, 0)
		END;
		i := 0;
		WHILE (m.kind = invalid) & (i < a.tabs.len) DO
			Try(m, r, a.tabs.tab[i].stop, h, x, y, tabs, i);
			INC(i)
		END;
		IF (m.kind = invalid) & (y >= h - tabBarHeight) & (a.tabs.len < maxTabs) THEN
			i := 0; px := TabGrid(x);
			WHILE (i < a.tabs.len) & (a.tabs.tab[i].stop < px) DO INC(i) END;
			IF (i = 0) OR (px - a.tabs.tab[i - 1].stop >= minTabWidth) THEN
				IF (i = a.tabs.len) OR (a.tabs.tab[i].stop - px >= minTabWidth) THEN
					IF canCreate THEN	(* set new tab stop, initially at end of list *)
						m.kind := tabs; m.index := a.tabs.len; m.dirty := TRUE;
						CopyTabs(a.tabs, m.tabs); m.tabs.len := a.tabs.len + 1;
						m.tabs.tab[a.tabs.len].stop := px; m.tabs.tab[a.tabs.len].type := {};
						a.tabs.tab[a.tabs.len].stop := px; a.tabs.tab[a.tabs.len].type := {};
						SetMark(m, r, px, h, tabs, m.index); InvertMark(m, f, Ports.show);
						m.px0 := m.px; m.py0 := m.py; m.x := x; m.y := y
					END
				END
			END
		END;

		(* next try icon bar *)
		px := iconGap; i := firstIcon;
		WHILE i <= lastIcon DO
			IF i IN validIcons THEN
				Try(m, r, px, h - iconPin, x, y, i, 0)
			END;
			INC(px, iconWidth + iconGap); INC(i)
		END
	END GetMark;

	PROCEDURE SelectMark (r: StdRuler; f: Views.Frame; IN m: Mark);
	BEGIN
		r.sel := m.kind; r.px := m.px; r.py := m.py
	END SelectMark;

	PROCEDURE DeselectMark (r: StdRuler; f: Views.Frame);
	BEGIN
		HiliteThisMark(r, f, r.sel, Ports.hide); r.sel := invalid
	END DeselectMark;


	(* mark interaction *)

	PROCEDURE Mode (r: StdRuler): INTEGER;
		VAR a: Attributes; i: INTEGER;
	BEGIN
		a := r.style.attr;
		IF a.opts * adjMask = {leftAdjust} THEN
			i := leftFlush
		ELSIF a.opts * adjMask = {} THEN
			i := centered
		ELSIF a.opts * adjMask = {rightAdjust} THEN
			i := rightFlush
		ELSE (* a.opts * adjMask = adjMask *)
			i := justified
		END;
		RETURN i
	END Mode;

	PROCEDURE GrabMark (VAR m: Mark; r: StdRuler; f: Views.Frame; x, y: INTEGER);
	BEGIN
		GetMark(m, r, f, x, y, TRUE);
		DeselectMark(r, f);
		IF m.kind = Mode(r) THEN m.kind := invalid END
	END GrabMark;

	PROCEDURE TrackMark (VAR m: Mark; f: Views.Frame; x, y: INTEGER; modifiers: SET);
		VAR px, py,  w, h: INTEGER;
	BEGIN
		IF m.kind # invalid THEN
			px := m.px + x - m.x; py := m.py + y - m.y;
			IF m.kind = tabs THEN
				px := TabGrid(px)
			ELSIF m.kind IN validIcons THEN
				IF (m.l <= x) & (x < m.r) THEN px := 1 ELSE px := 0 END
			ELSE
				px := MarginGrid(px)
			END;
			IF m.kind IN {right, tabs} THEN
				m.ruler.context.GetSize(w, h);
				IF (0 <= y) & (y < h + scaleHeight) OR (Controllers.extend IN modifiers) THEN 
					py := h
				ELSE
					py := -1	(* moved mark out of ruler: delete tab stop or fixed right margin *)
				END
			ELSIF m.kind IN validIcons THEN
				IF (m.t <= y) & (y < m.b) THEN py := 1 ELSE py := 0 END
			ELSE
				py := MarginGrid(py)
			END;
			IF (m.kind IN {right, tabs}) & ((m.px # px) OR (m.py # py)) THEN
				INC(m.x, px - m.px); INC(m.y, py - m.py);
				InvertMark(m, f, Ports.hide); SetMark(m, m.ruler, px, py, m.kind, m.index);
				InvertMark(m, f, Ports.show);
				m.dirty := TRUE
			ELSIF (m.kind IN {first, left}) & (m.px # px) THEN
				INC(m.x, px - m.px);
				InvertMark(m, f, Ports.hide); SetMark(m, m.ruler, px, m.py, m.kind, m.index);
				InvertMark(m, f, Ports.show)
			ELSIF (m.kind IN validIcons) & (m.px * m.py # px * py) THEN
				HiliteMark(m, f, Ports.show);
				IF m.kind IN modeIcons THEN HiliteThisMark(m.ruler, f, Mode(m.ruler), Ports.hide) END;
				m.px := px; m.py := py
			END
		END
	END TrackMark;

	PROCEDURE ShiftMarks (a: Attributes; p: Prop; mask: SET; x0, dx: INTEGER);
		VAR new: SET; i, j, t0, t1: INTEGER; tab0, tab1: TabArray;
	BEGIN
		new := mask - p.valid;
		IF first IN new THEN p.first := a.first END;
		IF tabs IN new THEN CopyTabs(a.tabs, p.tabs) END;
		p.valid := p.valid + mask;
		IF first IN mask THEN INC(p.first, dx) END;
		IF tabs IN mask THEN
			i := 0;
			WHILE (i < p.tabs.len) & (p.tabs.tab[i].stop < x0) DO tab0.tab[i] := p.tabs.tab[i]; INC(i) END;
			t0 := i;
			t1 := 0;
			WHILE i < p.tabs.len DO
				tab1.tab[t1].stop := p.tabs.tab[i].stop + dx;
				tab1.tab[t1].type := p.tabs.tab[i].type;
				INC(t1); INC(i)
			END;
			i := 0; j := 0; p.tabs.len := 0;
			WHILE i < t0 DO	(* merge sort *)
				WHILE (j < t1) & (tab1.tab[j].stop < tab0.tab[i].stop) DO
					p.tabs.tab[p.tabs.len] := tab1.tab[j]; INC(p.tabs.len); INC(j)
				END;
				IF (j < t1) & (tab1.tab[j].stop = tab0.tab[i].stop) THEN INC(j) END;
				p.tabs.tab[p.tabs.len] := tab0.tab[i]; INC(p.tabs.len); INC(i)
			END;
			WHILE j < t1 DO
				p.tabs.tab[p.tabs.len] := tab1.tab[j]; INC(p.tabs.len); INC(j)
			END
		END
	END ShiftMarks;

	PROCEDURE ShiftDependingMarks (VAR m: Mark; p: Prop);
		VAR a: Attributes; dx: INTEGER;
	BEGIN
		a := m.ruler.style.attr; dx := m.px - m.px0;
		CASE m.kind OF
		  first: ShiftMarks(a, p, {tabs}, 0, dx)
		| left: ShiftMarks(a, p, {first, tabs}, 0, dx)
		| tabs: ShiftMarks(a, p, {tabs}, m.px0, dx)
		ELSE
		END
	END ShiftDependingMarks;

	PROCEDURE AdjustMarks (VAR m: Mark; f: Views.Frame; modifiers: SET);
		VAR r: StdRuler; a: Attributes; p: Prop;
			g: INTEGER; i, j: INTEGER; shift: BOOLEAN; type: SET;
	BEGIN
		r := m.ruler;
		IF  (m.kind # invalid) & (m.kind IN validIcons)
				& (m.px = 1) & (m.py = 1)
		OR (m.kind # invalid) & ~(m.kind IN validIcons)
				& ((m.px # m.px0) OR (m.py # m.py0)
					OR (m.kind = tabs) (*(m.tabs.len # r.style.attr.tabs.len)*) )
		THEN
			a := r.style.attr; NEW(p);
			p.valid := {};
			shift := (Controllers.modify IN modifiers) & (m.tabs.len = r.style.attr.tabs.len);
			CASE m.kind OF
			  first:
				p.valid := {first}; p.first := m.px
			| left:
				p.valid := {left}; p.left := m.px
			| right:
				IF m.py >= 0 THEN
					p.valid := {right}; p.right := m.px
				ELSE
					p.valid := {opts}; p.opts.val := {}; p.opts.mask := {rightFixed}
				END
			| tabs:
				IF ~m.dirty THEN
					p.valid := {tabs}; CopyTabs(m.tabs, p.tabs);
					i := m.index; type := m.tabs.tab[i].type;
					IF shift THEN
						type := type * {barTab};
						IF type = {} THEN type := {barTab}
						ELSE type := {}
						END;
						p.tabs.tab[i].type := p.tabs.tab[i].type - {barTab} + type
					ELSE
						type := type * {centerTab, rightTab};
						IF type = {} THEN type := {centerTab}
						ELSIF type = {centerTab} THEN type := {rightTab}
						ELSE type := {}
						END;
						p.tabs.tab[i].type := p.tabs.tab[i].type - {centerTab, rightTab} + type
					END
				ELSIF ~shift THEN
					p.valid := {tabs}; p.tabs.len := m.tabs.len - 1;
					i := 0;
					WHILE i < m.index DO p.tabs.tab[i] := m.tabs.tab[i]; INC(i) END;
					INC(i);
					WHILE i < m.tabs.len DO p.tabs.tab[i - 1] := m.tabs.tab[i]; INC(i) END;
					i := 0;
					WHILE (i < p.tabs.len) & (p.tabs.tab[i].stop < m.px) DO INC(i) END;
					IF (m.px >= MIN(a.first, a.left)) & (m.px <= f.r) & (m.py >= 0)
					 & ((i = 0) OR (m.px - p.tabs.tab[i - 1].stop >= minTabWidth))
					 & ((i = p.tabs.len) OR (p.tabs.tab[i].stop - m.px >= minTabWidth)) THEN
						j := p.tabs.len;
						WHILE j > i DO p.tabs.tab[j] := p.tabs.tab[j - 1]; DEC(j) END;
						p.tabs.tab[i].stop := m.px; p.tabs.tab[i].type := m.tabs.tab[m.index].type;
						INC(p.tabs.len)
					END;
					i := 0;
					WHILE (i < p.tabs.len)
					 & (p.tabs.tab[i].stop = a.tabs.tab[i].stop)
					 & (p.tabs.tab[i].type = a.tabs.tab[i].type) DO
						INC(i)
					END;
					IF (i = p.tabs.len) & (p.tabs.len = a.tabs.len) THEN RETURN END	(* did not change *)
				END
			| rightToggle:
				p.valid := {right, opts};
				IF ~(rightFixed IN a.opts) THEN
					p.right := f.r DIV marginGrid * marginGrid
				END;
				p.opts.val := a.opts / {rightFixed}; p.opts.mask := {rightFixed}
			| gridDec:
				p.valid := {asc, grid}; g := a.grid - point;
				IF g = 0 THEN p.grid := 1; p.asc := 0 ELSE p.grid := g; p.asc := g - a.dsc END
			| gridVal:
				SelectMark(r, f, m); RETURN
			| gridInc:
				p.valid := {asc, grid}; g := a.grid + point; DEC(g, g MOD point);
				p.grid := g; p.asc := g - a.dsc
			| leftFlush:
				p.valid := {opts}; p.opts.val := {leftAdjust}; p.opts.mask := adjMask
			| centered:
				p.valid := {opts}; p.opts.val := {}; p.opts.mask := adjMask
			| rightFlush:
				p.valid := {opts}; p.opts.val := {rightAdjust}; p.opts.mask := adjMask
			| justified:
				p.valid := {opts}; p.opts.val := adjMask; p.opts.mask := adjMask
			| leadDec:
				p.valid := {lead}; p.lead := a.lead - point
			| leadVal:
				SelectMark(r, f, m); RETURN
			| leadInc:
				p.valid := {lead}; p.lead := a.lead + point
			| pageBrk:
				p.valid := {opts}; p.opts.val := a.opts / {pageBreak}; p.opts.mask := {pageBreak}
			ELSE HALT(100)
			END;
			IF shift THEN ShiftDependingMarks(m, p) END;
			IF m.kind IN validIcons - modeIcons THEN HiliteMark(m, f, Ports.hide) END;

			r.style.SetAttr(ModifiedAttr(a, p))
		END
	END AdjustMarks;


	(* primitivies for standard ruler *)

	PROCEDURE Track (r: StdRuler; f: Views.Frame; IN msg: Controllers.TrackMsg);
		VAR m: Mark; x, y, res: INTEGER; modifiers: SET; isDown: BOOLEAN;
			cmd: ARRAY 128 OF CHAR;
	BEGIN
		GrabMark(m, r, f, msg.x, msg.y);
		REPEAT
			f.Input(x, y, modifiers, isDown); TrackMark(m, f, x, y, modifiers)
		UNTIL ~isDown;
		AdjustMarks(m, f, modifiers);
		IF Controllers.doubleClick IN msg.modifiers THEN
			CASE m.kind OF
			| invalid:
				Dialog.MapString("#Text:OpenRulerDialog", cmd); Dialog.Call(cmd, "", res)
			| gridVal, leadVal:
				Dialog.MapString("#Text:OpenSizeDialog", cmd); Dialog.Call(cmd, "", res)
			ELSE
			END
		END
	END Track;

	PROCEDURE Edit (r: StdRuler; f: Views.Frame; VAR msg: Controllers.EditMsg);
		VAR v: Views.View;
	BEGIN
		CASE msg.op OF
		  Controllers.copy:
			msg.view := Views.CopyOf(r, Views.deep);
			msg.isSingle := TRUE
		| Controllers.paste:
			v := msg.view;
			WITH v: Ruler DO r.style.SetAttr(v.style.attr) ELSE END
		ELSE
		END
	END Edit;

	PROCEDURE PollOps (r: StdRuler; f: Views.Frame; VAR msg: Controllers.PollOpsMsg);
	BEGIN
		msg.type := "TextRulers.Ruler";
		msg.pasteType := "TextRulers.Ruler";
		msg.selectable := FALSE;
		msg.valid := {Controllers.copy, Controllers.paste}
	END PollOps;

	PROCEDURE SetProp (r: StdRuler; VAR msg: Properties.SetMsg; VAR requestFocus: BOOLEAN);
		VAR a1: Attributes; px, py, g: INTEGER; sel: INTEGER;
			p: Properties.Property; sp: Properties.StdProp; rp: Prop;
	BEGIN
		p := msg.prop; sel := r.sel; px := r.px; py := r.py;
		IF sel # invalid THEN
			WHILE (p # NIL) & ~(p IS Properties.StdProp) DO p := p.next END;
			IF p # NIL THEN
				sp := p(Properties.StdProp);
				IF (r.sel = leadVal) & (Properties.size IN sp.valid) THEN
					NEW(rp); rp.valid := {lead};
					rp.lead := sp.size
				ELSIF (r.sel = gridVal) & (Properties.size IN sp.valid) THEN
					g := sp.size; DEC(g, g MOD point);
					NEW(rp); rp.valid := {asc, grid};
					IF g = 0 THEN rp.asc := 0; rp.grid := 1
					ELSE rp.asc := g - r.style.attr.dsc; rp.grid := g
					END
				ELSE
					rp := NIL
				END
			END;
			p := rp
		END;
		a1 := ModifiedAttr(r.style.attr, p);
		IF ~a1.Equals(r.style.attr) THEN
			r.style.SetAttr(a1);
			IF requestFocus & (r.sel = invalid) THEN	(* restore mark selection *)
				r.sel := sel; r.px := px; r.py := py 
			END
		ELSE requestFocus := FALSE
		END
	END SetProp;

	PROCEDURE PollProp (r: StdRuler; VAR msg: Properties.PollMsg);
		VAR p: Properties.StdProp;
	BEGIN
		CASE r.sel OF
		  invalid:
			msg.prop := r.style.attr.Prop()
		| leadVal:
			NEW(p); p.known := {Properties.size}; p.valid := p.known;
			p.size := r.style.attr.lead;
			msg.prop := p
		| gridVal:
			NEW(p); p.known := {Properties.size}; p.valid := p.known;
			p.size := r.style.attr.grid;
			msg.prop := p
		ELSE HALT(100)
		END
	END PollProp;


	(* StdStyle *)

	PROCEDURE (r: StdStyle) Internalize (VAR rd: Stores.Reader);
		VAR thisVersion: INTEGER;
	BEGIN
		r.Internalize^(rd);
		IF rd.cancelled THEN RETURN END;
		rd.ReadVersion(minVersion, maxStdStyleVersion, thisVersion)
	END Internalize;

	PROCEDURE (r: StdStyle) Externalize (VAR wr: Stores.Writer);
	BEGIN
		r.Externalize^(wr);
		wr.WriteVersion(maxStdStyleVersion)
	END Externalize;
(*	
	PROCEDURE (r: StdStyle) CopyFrom (source: Stores.Store);
	BEGIN
		r.SetAttr(source(StdStyle).attr)
	END CopyFrom;
*)

	(* StdRuler *)

	PROCEDURE (r: StdRuler) Internalize (VAR rd: Stores.Reader);
		VAR thisVersion: INTEGER;
	BEGIN
		r.Internalize^(rd);
		IF rd.cancelled THEN RETURN END;
		rd.ReadVersion(minVersion, maxStdRulerVersion, thisVersion);
		IF rd.cancelled THEN RETURN END;
		r.sel := invalid
	END Internalize;

	PROCEDURE (r: StdRuler) Externalize (VAR wr: Stores.Writer);
	BEGIN
		r.Externalize^(wr);
		wr.WriteVersion(maxStdRulerVersion)
	END Externalize;

	PROCEDURE (r: StdRuler) ThisModel (): Models.Model;
	BEGIN
		RETURN r.style
	END ThisModel;

	PROCEDURE (r: StdRuler) CopyFromModelView (source: Views.View; model: Models.Model);
	BEGIN
		r.sel := invalid; r.InitStyle(model(Style))
	END CopyFromModelView;

	PROCEDURE (ruler: StdRuler) Restore (f: Views.Frame; l, t, r, b: INTEGER);
		VAR a: Attributes; m: Mark; u, scale, tabBar,  px,  w, h: INTEGER; i: INTEGER;
	BEGIN
		u := f.dot; a := ruler.style.attr;
		ruler.context.GetSize(w, h);
		tabBar := h - tabBarHeight; scale := tabBar - scaleHeight;
		w := MIN(f.r + 10 * mm, 10000 * mm);	(* high-level clipping *)
		f.DrawLine(0, scale - u, w - u, scale - u, u, Ports.grey25);
		f.DrawLine(0, tabBar - u, w - u, tabBar - u, u, Ports.grey50);
		DrawScale(f, 0, scale, w, tabBar, l, r);
		DrawNiceRect(f, 0, h - rulerHeight, w, h);
		SetMark(m, ruler, a.first, h, first, -1); InvertMark(m, f, Ports.show);
		SetMark(m, ruler, a.left, h, left, -1); InvertMark(m, f, Ports.show);
		IF rightFixed IN a.opts THEN
			SetMark(m, ruler, a.right, h, right, -1); InvertMark(m, f, Ports.show)
		END;
		i := 0;
		WHILE i < a.tabs.len DO
			SetMark(m, ruler, a.tabs.tab[i].stop, h, tabs, i); InvertMark(m, f, Ports.show); INC(i)
		END;
		px := iconGap; i := firstIcon;
		WHILE i <= lastIcon DO
			IF i IN validIcons THEN
				SetMark(m, ruler, px, h - iconPin, i, -1); DrawMark(m, f)
			END;
			INC(px, iconWidth + iconGap); INC(i)
		END;
		HiliteThisMark(ruler, f, Mode(ruler), Ports.show)
	END Restore;

	PROCEDURE (ruler: StdRuler) RestoreMarks (f: Views.Frame; l, t, r, b: INTEGER);
	BEGIN
		HiliteThisMark(ruler, f, ruler.sel, Ports.show)
	END RestoreMarks;

	PROCEDURE (r: StdRuler) GetBackground (VAR color: Ports.Color);
	BEGIN
		color := Ports.background
	END GetBackground;

	PROCEDURE (r: StdRuler) Neutralize;
		VAR msg: NeutralizeMsg;
	BEGIN
		Views.Broadcast(r, msg)
	END Neutralize;

	PROCEDURE (r: StdRuler) HandleModelMsg (VAR msg: Models.Message);
	BEGIN
		WITH msg: UpdateMsg DO
			Views.Update(r, Views.keepFrames)
		ELSE
		END
	END HandleModelMsg;

	PROCEDURE (r: StdRuler) HandleViewMsg (f: Views.Frame; VAR msg: Views.Message);
	BEGIN
		WITH msg: NeutralizeMsg DO
			DeselectMark(r, f)
		ELSE
		END
	END HandleViewMsg;

	PROCEDURE (r: StdRuler) HandleCtrlMsg (f: Views.Frame;
		VAR msg: Controllers.Message; VAR focus: Views.View
	);
		VAR requestFocus: BOOLEAN;
	BEGIN
		WITH msg: Controllers.TrackMsg DO
			Track(r, f, msg)
		| msg: Controllers.EditMsg DO
			Edit(r, f, msg)
		| msg: Controllers.MarkMsg DO
			r.RestoreMarks(f, f.l, f.t, f.r, f.b)
		| msg: Controllers.SelectMsg DO
			IF ~msg.set THEN DeselectMark(r, f) END
		| msg: Controllers.PollOpsMsg DO
			PollOps(r, f, msg)
		| msg: Properties.CollectMsg DO
			PollProp(r, msg.poll)
		| msg: Properties.EmitMsg DO
			requestFocus := f.front;
			SetProp(r, msg.set, requestFocus);
			msg.requestFocus := requestFocus
		ELSE
		END
	END HandleCtrlMsg;

	PROCEDURE (r: StdRuler) HandlePropMsg (VAR msg: Properties.Message);
		VAR m: Mark; requestFocus: BOOLEAN; w, h: INTEGER;
	BEGIN
		WITH msg: Properties.SizePref DO
			msg.w := 10000 * Ports.mm; msg.h := rulerHeight
		| msg: Properties.ResizePref DO
			msg.fixed := TRUE
		| msg: Properties.FocusPref DO
			IF msg.atLocation THEN
				r.context.GetSize(w, h);
				GetMark(m, r, NIL, msg.x, msg.y, FALSE);
				msg.hotFocus := (m.kind # invalid) & ~(m.kind IN fieldIcons) OR (msg.y >= h - tabBarHeight);
				msg.setFocus := ~msg.hotFocus
			END
		| msg: TextModels.Pref DO
			msg.opts := {TextModels.maskChar, TextModels.hideable};
			msg.mask := TextModels.para
		| msg: Properties.SetMsg DO
			requestFocus := FALSE;
			SetProp(r, msg, requestFocus)
		| msg: Properties.PollMsg DO
			PollProp(r, msg)
		ELSE
		END
	END HandlePropMsg;


	(* StdDirectory *)

	PROCEDURE (d: StdDirectory) NewStyle (attr: Attributes): Style;
		VAR s: StdStyle;
	BEGIN
		IF attr = NIL THEN attr := d.attr END;
		NEW(s); s.SetAttr(attr); RETURN s
	END NewStyle;

	PROCEDURE (d: StdDirectory) New (style: Style): Ruler;
		VAR r: StdRuler;
	BEGIN
		IF style = NIL THEN style := d.NewStyle(NIL) END;
		NEW(r); r.InitStyle(style); r.sel := invalid; RETURN r
	END New;


	(** miscellaneous **)

	PROCEDURE GetValidRuler* (text: TextModels.Model; pos, hint: INTEGER;
		VAR ruler: Ruler; VAR rpos: INTEGER
	);
		(** pre: (hint < 0   OR   (ruler, rpos) is first ruler before hint  &  0 <= pos <= t.Length() **)
		(** post: hint < rpos <= pos & rpos = Pos(ruler) & (no ruler in (rpos, pos])
				OR   ((ruler, rpos) unmodified)
		**)
		VAR view: Views.View;
	BEGIN
		IF pos < text.Length() THEN INC(pos) END;	(* let a ruler dominate its own position *)
		IF pos < hint THEN hint := -1 END;
		globRd := text.NewReader(globRd); globRd.SetPos(pos);
		REPEAT
			globRd.ReadPrevView(view)
		UNTIL globRd.eot OR (view IS Ruler) OR (globRd.Pos() < hint);
		IF (view # NIL) & (view IS Ruler) THEN
			ruler := view(Ruler); rpos := globRd.Pos()
		END
	END GetValidRuler;

	PROCEDURE SetDir* (d: Directory);
	(** pre: d # NIL, d.attr # NIL **)
	(** post: dir = d **)
	BEGIN
		ASSERT(d # NIL, 20); ASSERT(d.attr.init, 21); dir := d
	END SetDir;


	PROCEDURE Init;
		VAR d: StdDirectory; fnt: Fonts.Font; asc, dsc, w: INTEGER;
	BEGIN
		IF Dialog.metricSystem THEN
			marginGrid := 1*mm; minTabWidth := 1*mm; tabGrid := 1*mm
		ELSE
			marginGrid := inch16; minTabWidth := inch16; tabGrid := inch16
		END;

		fnt := Fonts.dir.Default();
		font := Fonts.dir.This(fnt.typeface, 7*point, {}, Fonts.normal);	(* font for ruler scales *)
		NEW(prop);
		prop.valid := {first .. tabs};
		prop.first := 0; prop.left := 0;
		IF Dialog.metricSystem THEN
			prop.right := 165*mm
		ELSE
			prop.right := 104*inch16
		END;
		fnt.GetBounds(asc, dsc, w);
		prop.lead := 0; prop.asc := asc; prop.dsc := dsc; prop.grid := 1;
		prop.opts.val := {leftAdjust}; prop.opts.mask := options;
		prop.tabs.len := 0;

		NEW(def); def.InitFromProp(prop);
		NEW(d); d.attr := def; dir := d; stdDir := d
	END Init;

	PROCEDURE Cleaner;
	BEGIN
		globRd := NIL
	END Cleaner;

BEGIN
	Init;
	Kernel.InstallCleaner(Cleaner)
CLOSE
	Kernel.RemoveCleaner(Cleaner)
END TextRulers.
