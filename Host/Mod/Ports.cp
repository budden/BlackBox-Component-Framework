MODULE HostPorts;
(**
	project	= "BlackBox"
	organization	= "www.oberon.ch"
	contributors	= "Oberon microsystems, Marco Ciot, Alexander Iljin"
	version	= "System/Rsrc/About"
	copyright	= "System/Rsrc/About"
	license	= "Docu/BB-License"
	changes	= "
	- 20060325, mc, Rider.Input changed for the benefit of background task response.
	- 20060903, ai, call to ValidateRect from Port.CloseBuffer to fix painting bug deleted
	- 20060915, ai, call to UpdateWindow in Rider.Input to fix ScrollWhileTracking painting bug added
	- 20070130, bh, Unicode support
	- 20070205, bh, Win32s handling removed
	- 20070827, bh, improved width and figureSpace handling in long string ops
	"
	issues	= ""
**)

	(* TODO:
		- OpenBuffer can be optimized to only open a buffer large enough for the clipingrectangle
		- MarkRect needs to be looked at for some parameter values
		- ResetColors should find system values
	*)

	IMPORT SYSTEM,
		Gdk := LibsGdk, Gtk := LibsGtk, Key := Gtk2Keysyms, Pango := LibsPango, GLib := LibsGlib,
		Utf8 := HostUtf8, Fonts, Ports, Dialog, HostFonts;

	CONST
		resizeHCursor* = 16; resizeVCursor* = 17; resizeLCursor* = 18;
		resizeRCursor* = 19; resizeCursor* = 20;
		busyCursor* = 21; stopCursor* = 22;
		moveCursor* = 23; copyCursor* = 24; linkCursor* = 25; pickCursor* = 26;
		focusPat* = 5;

		extend = 1; modify = 2; (* same as Controllers.extend and Controllers.modify  !!! *)
		off = 0; on = 1;

		(** buttons **)
		left* = 16; middle* = 17; right* = 18;
		shift* = 24; ctrl* = 25; opt* = 26; cmd* = 27; alt* = 28;

	TYPE

		Port* = POINTER TO RECORD (Ports.Port)
			bl, bt, br, bb: INTEGER;	(* buffer rectangle *)
			w, h: INTEGER; (* size of port *)
			da-: Gtk.GtkDrawingArea; (* drawing widget *)
			fixed-: Gtk.GtkFixed; (* fixed container for absolute positioning *)
			map: Gdk.GdkPixmap; (* off screen buffer *)
			gc: Gdk.GdkGC; (* graphic context *)
		END;

		Rider* = POINTER TO RECORD (Ports.Rider)
			l-, t-, r-, b-: INTEGER;
			dx, dy: INTEGER;	(* scroll offset *)
			port-: Port; (* port for the rider *)
			map: Gdk.GdkPixmap;	(* save bitmap *)
			gc: Gdk.GdkGC;	(* save gc *)
			sl, st, sr, sb: INTEGER	(* save rect *)
		END;

	VAR
		(* system colors *)
		textCol-, selBackground-, selTextCol-,
		dialogTextCol-, dialogShadowCol-, dialogLightCol-: Ports.Color;

		mx, my: INTEGER;	(* actual mouse coordinates *)
		mb: SET;	(* actual mouse buttons & modifiers *)
		dim25Col, dim50Col, dim75Col: Ports.Color;

		cursors-: ARRAY 32 OF Gdk.GdkCursor;

	(* Auxiliary procedures *)

	PROCEDURE UnsignedShortInt(i: INTEGER): SHORTINT;
	BEGIN
		ASSERT(i < 2 * MAX(SHORTINT) + 2, 20);
		RETURN (SHORT(i))
	END UnsignedShortInt;

	PROCEDURE AllocateColor (bbColor: Ports.Color; VAR gdkColor: Gdk.GdkColor);
		VAR cm: Gdk.GdkColormap; i: INTEGER;
	BEGIN
		cm := Gdk.gdk_colormap_get_system(); (* TODO: keep in global variable? *)
		i := bbColor MOD 256;
			gdkColor.red := UnsignedShortInt(i * 257);
		i := (bbColor DIV 256) MOD 256;
			gdkColor.green := UnsignedShortInt(i * 257);
		i := (bbColor DIV (256 * 256)) MOD 256;
			gdkColor.blue := UnsignedShortInt(i * 257);
		IF ~Gdk.gdk_colormap_alloc_color(cm, gdkColor, 1, 1)  THEN
			Dialog.ShowMsg("gdk_colormap_alloc_color failed")
		END
	END AllocateColor;

	(* Port *)

	PROCEDURE (p: Port) SetSize* (w, h: INTEGER);
	BEGIN
		ASSERT(w >= 0, 20); ASSERT(h >= 0, 21);
		p.w := w; p.h := h
	END SetSize;

	PROCEDURE (p: Port) GetSize* (OUT w, h: INTEGER);
	BEGIN
		w := p.w; h := p.h
	END GetSize;

	PROCEDURE (p: Port) NewRider* (): Rider;
		VAR h: Rider;
	BEGIN
		NEW(h); h.port := p; RETURN h
	END NewRider;

	PROCEDURE (p: Port) SetDA* (da: Gtk.GtkDrawingArea), NEW;
	BEGIN
		p.da := da
	END SetDA;

	PROCEDURE (p: Port) SetFW* (fixed: Gtk.GtkFixed), NEW;
	BEGIN
		p.fixed := fixed
	END SetFW;

	PROCEDURE (p: Port) OpenBuffer* (l, t, r, b: INTEGER);
		(*VAR rect: Gdk.GdkRectangle;*)
	BEGIN
		ASSERT(p.da # NIL, 20);
		IF l < 0 THEN l := 0 END;
		IF t < 0 THEN t := 0 END;
		IF r > p.w THEN r := p.w END;
		IF b > p.h THEN b := p.h END;
		IF (l < r) & (t < b) THEN
			p.bl := l; p.bt := t; p.br := r; p.bb := b;
			p.map := Gdk.gdk_pixmap_new(p.da.window, p.w, p.h , -1);
			p.gc := Gdk.gdk_gc_new(p.map)
		END
	END OpenBuffer;

	PROCEDURE (p: Port) CloseBuffer*;
	BEGIN
		IF p.map # NIL THEN
			Gdk.gdk_draw_drawable(p.da.window, p.da.style.white_gc, p.map, p.bl, p.bt, p.bl, p.bt, p.br - p.bl, p.bb - p.bt);
			Gdk.gdk_drawable_unref(p.map);
			p.map := NIL;
			Gdk.gdk_gc_unref(p.gc);
			p.gc := NIL
		END
	END CloseBuffer;


	(** Rider **)

	PROCEDURE (rd: Rider) Base* (): Ports.Port;
	BEGIN
		(* ASSERT(rd.port # NIL, 20); ASSERT(rd.port.dc # 0, 21); *)
		RETURN rd.port
	END Base;

	PROCEDURE (rd: Rider) SetRect* (l, t, r, b: INTEGER);
	BEGIN
		ASSERT((l <= r) & (t <= b), 20);
		ASSERT(rd.port # NIL, 21);
		rd.l := l; rd.t := t; rd.r := r; rd.b := b
	END SetRect;

	PROCEDURE (rd: Rider) GetRect* (OUT l, t, r, b: INTEGER);
	BEGIN
		l := rd.l; t := rd.t; r := rd.r; b := rd.b
	END GetRect;

	PROCEDURE (rd: Rider) InitPort* (port: Port), NEW;
	BEGIN
		ASSERT(rd.port = NIL, 20); ASSERT(port # NIL, 21); ASSERT(port.da # NIL, 22);
		rd.port := port; rd.dx := 0; rd.dy := 0
	END InitPort;

	PROCEDURE (rd: Rider) Move* (dx, dy: INTEGER);
	BEGIN
		INC(rd.dx, dx); INC(rd.dy, dy)
	END Move;

	PROCEDURE (rd: Rider) DrawingBuf (VAR map: Gdk.GdkDrawable; VAR gc: Gdk.GdkGC), NEW;
		VAR r: Gdk.GdkRectangle;
	BEGIN
		ASSERT(rd.port # NIL, 20);
		IF (rd.port.map # NIL) & (rd.port.gc # NIL) THEN (* buffered drawing *)
			map := rd.port.map; gc := rd.port.gc; Gdk.gdk_gc_ref(gc)
		ELSE (* unbuffered drawing *)
			map := rd.port.da.window; gc := Gdk.gdk_gc_new(map)
		END;
		r.x := (rd.l);
		r.y := (rd.t);
		r.width := (rd.r - rd.l + 1);
		r.height := (rd.b - rd.t + 1) ;
		Gdk.gdk_gc_set_clip_rectangle(gc, SYSTEM.ADR(r));
		Gdk.gdk_gc_set_clip_origin(gc, 0, 0)
	END DrawingBuf;

	PROCEDURE (rd: Rider) DrawRect* (l, t, r, b, s: INTEGER; col: Ports.Color);
		VAR gdkColor: Gdk.GdkColor; map: Gdk.GdkDrawable; gc: Gdk.GdkGC; h: INTEGER;
	BEGIN
		ASSERT(rd.port # NIL, 20);
		rd.DrawingBuf(map, gc);
		AllocateColor(col, gdkColor);
		Gdk.gdk_gc_set_foreground(gc, gdkColor);
		Gdk.gdk_gc_set_background(gc, gdkColor);
		IF s = Ports.fill THEN
			Gdk.gdk_gc_set_line_attributes(gc, 1, Gdk.GDK_LINE_SOLID, Gdk.GDK_CAP_ROUND, Gdk.GDK_JOIN_MITER);
			Gdk.gdk_draw_rectangle(map, gc, on,  l, t, r - l, b - t)
		ELSE
			h := s DIV 2; INC(l, h); INC(t, h); h := (s-1) DIV 2; DEC(r, h); DEC(b, h);
			Gdk.gdk_gc_set_line_attributes(gc, s, Gdk.GDK_LINE_SOLID, Gdk.GDK_CAP_ROUND, Gdk.GDK_JOIN_MITER);
			Gdk.gdk_draw_rectangle(map, gc, off,  l, t, r - l, b - t)
		END;
		Gdk.gdk_gc_unref(gc)
	END DrawRect;

	PROCEDURE (rd: Rider) DrawOval* (l, t, r, b, s: INTEGER; col: Ports.Color);
		VAR gdkColor: Gdk.GdkColor;map: Gdk.GdkDrawable; gc: Gdk.GdkGC; h: INTEGER;
	BEGIN
		ASSERT(rd.port # NIL, 20);
		rd.DrawingBuf(map, gc);
		AllocateColor(col, gdkColor);
		Gdk.gdk_gc_set_foreground(gc, gdkColor);
		Gdk.gdk_gc_set_background(gc, gdkColor);
		IF s = Ports.fill THEN
			Gdk.gdk_gc_set_line_attributes(gc, 1, Gdk.GDK_LINE_SOLID, Gdk.GDK_CAP_ROUND, Gdk.GDK_JOIN_ROUND);
			Gdk.gdk_draw_arc(map, gc, on,  l, t, r - l, b - t, 0, 360 * 64)
		ELSE
			IF s = 0 THEN s := 1 END;
			h := s DIV 2; INC(l, h); INC(t, h);
			h := (s-1) DIV 2; DEC(r, h); DEC(b, h);
			(* ShiryaevAV: *)
				IF r < l THEN h := l; l := r; r := h END;
				IF b < t THEN h := t; t := b; b := h END;
			Gdk.gdk_gc_set_line_attributes(gc, s, Gdk.GDK_LINE_SOLID, Gdk.GDK_CAP_ROUND, Gdk.GDK_JOIN_ROUND);
			Gdk.gdk_draw_arc(map, gc, off,  l, t, r - l, b - t, 0, 360  * 64)
		END;
		Gdk.gdk_gc_unref(gc)
	END DrawOval;

	PROCEDURE (rd: Rider) DrawLine* (x0, y0, x1, y1, s: INTEGER; col: Ports.Color);
		VAR gdkColor: Gdk.GdkColor; map: Gdk.GdkDrawable; gc: Gdk.GdkGC;
	BEGIN
		ASSERT(s >= 0, 21); ASSERT(rd.port # NIL, 100);
		rd.DrawingBuf(map, gc);
		AllocateColor(col, gdkColor);
		Gdk.gdk_gc_set_foreground(gc, gdkColor);
		Gdk.gdk_gc_set_line_attributes(gc, s, Gdk.GDK_LINE_SOLID, Gdk.GDK_CAP_ROUND, Gdk.GDK_JOIN_ROUND);
		Gdk.gdk_draw_line(map, gc, x0, y0, x1, y1);
		Gdk.gdk_gc_unref(gc)
	END DrawLine;

	PROCEDURE (rd: Rider) DrawPath* (IN pts: ARRAY OF Ports.Point; n, s: INTEGER; col: Ports.Color; path: INTEGER);
		TYPE
			PAP = POINTER TO Gdk.GdkPoints;
		VAR
			i, j, k: INTEGER;
			map: Gdk.GdkDrawable; gc: Gdk.GdkGC;
			gdkColor: Gdk.GdkColor;
			polyBuf: ARRAY 256 OF Gdk.GdkPoint;
			polyPtr: POINTER TO ARRAY OF Ports.Point;
			polyLen: INTEGER;
			pap: PAP;

		PROCEDURE Bezier(x0, y0, xd0, yd0, x1, y1, xd1, yd1: INTEGER);
			VAR x, y, xd, yd, i: INTEGER;
		BEGIN
			IF ABS(xd0 - xd1) + ABS(yd0 - yd1) < 8 THEN
				IF k > polyLen - 2 THEN
					NEW(polyPtr, polyLen * 2);
					i := 0; WHILE i < polyLen DO
						polyPtr[i] := SYSTEM.VAL(Ports.Point, pap[i]); INC(i)
					END;
					polyLen := polyLen * 2; pap := SYSTEM.VAL(PAP, SYSTEM.ADR(polyPtr^))
				END;
				pap[k].x := x0; pap[k].y := y0; INC(k)
			ELSE
				x := ((xd0 - xd1) DIV 4 + x0 + x1 + 1) DIV 2;
				y := ((yd0 - yd1) DIV 4 + y0 + y1 + 1) DIV 2;
				xd := ((x1 - x0) * 3 - (xd0 + xd1) DIV 2 + 2) DIV 4;
				yd := ((y1 - y0) * 3 - (yd0 + yd1) DIV 2 + 2) DIV 4;
				Bezier(x0, y0, xd0 DIV 2, yd0 DIV 2, x, y, xd, yd);
				Bezier(x, y, xd, yd, x1, y1, xd1 DIV 2, yd1 DIV 2)
			END
		END Bezier;

	BEGIN
		ASSERT(rd.port # NIL, 20);
		rd.DrawingBuf(map, gc);
		AllocateColor(col, gdkColor);
		Gdk.gdk_gc_set_foreground(gc, gdkColor);

		ASSERT(n >= 0, 20); ASSERT(n <= LEN(pts), 21);
		ASSERT(s >= Ports.fill, 23);
		IF s < 0 THEN
			Gdk.gdk_gc_set_line_attributes(gc, 1, Gdk.GDK_LINE_SOLID, Gdk.GDK_CAP_ROUND, Gdk.GDK_JOIN_ROUND);
			Gdk.gdk_gc_set_background(gc, gdkColor);
			IF path = Ports.closedPoly THEN
				ASSERT(n >= 2, 20);
				Gdk.gdk_draw_polygon(map, gc, on, SYSTEM.VAL(Gdk.GdkPoints, pts), n)
			ELSE
				ASSERT(n >= 3, 20);
				ASSERT(path = Ports.closedBezier, 22);
				ASSERT(n MOD 3 = 0, 24);
				pap := SYSTEM.VAL(PAP, SYSTEM.ADR(polyBuf)); polyLen := LEN(polyBuf);
				i := 0; k := 0;
				WHILE i < n DO
					j := i+3;
					IF j = n THEN j := 0 END;
					Bezier(pts[i].x, pts[i].y, (pts[i+1].x - pts[i].x) * 3, (pts[i+1].y - pts[i].y) * 3,
							pts[j].x, pts[j].y, (pts[j].x - pts[i+2].x) * 3, (pts[j].y - pts[i+2].y) * 3);
					INC(i, 3)
				END;
				Gdk.gdk_draw_polygon(map, gc, on, pap, k);
			END;
		ELSE
			Gdk.gdk_gc_set_line_attributes(gc, s, Gdk.GDK_LINE_SOLID, Gdk.GDK_CAP_ROUND, Gdk.GDK_JOIN_ROUND);
			IF s = 0 THEN s := 1 END;
			IF path = Ports.closedPoly THEN
				ASSERT(n >= 2, 20);
				Gdk.gdk_draw_polygon(map, gc, off, SYSTEM.VAL(Gdk.GdkPoints, pts), n);
			ELSIF path = Ports.openPoly THEN
				ASSERT(n >= 2, 20);
				Gdk.gdk_draw_lines(map, gc,  SYSTEM.VAL(Gdk.GdkPoints, pts), n);
			ELSE
				IF path = Ports.closedBezier THEN
					ASSERT(n >= 3, 20);
					ASSERT(n MOD 3 = 0, 24)
				ELSE
					ASSERT(n >= 4, 20);
					ASSERT(path = Ports.openBezier, 25);
					ASSERT(n MOD 3 = 1, 24)
				END;
				pap := SYSTEM.VAL(PAP, SYSTEM.ADR(polyBuf)); polyLen := LEN(polyBuf);
				i := 0;
				WHILE i < n-2 DO
					k := 0; j := i+3;
					IF j = n THEN j := 0 END;
					Bezier(pts[i].x, pts[i].y, (pts[i+1].x - pts[i].x) * 3, (pts[i+1].y - pts[i].y) * 3,
							pts[j].x, pts[j].y, (pts[j].x - pts[i+2].x) * 3, (pts[j].y - pts[i+2].y) * 3);
					pap[k].x := (pts[j].x);
					pap[k].y := (pts[j].y);
					INC(k);
					Gdk.gdk_draw_lines(map, gc, pap, k);
					INC(i, 3)
				END
			END;
		END;
		Gdk.gdk_gc_unref(gc)
	END DrawPath;

	PROCEDURE (rd: Rider) MarkRect* (l, t, r, b, s, mode: INTEGER; show: BOOLEAN);
	VAR gdkColor: Gdk.GdkColor; gc: Gdk.GdkGC;
			vals: Gdk.GdkGCValues; map: Gdk.GdkDrawable;
	BEGIN
		IF rd.port.map # NIL THEN
			map := rd.port.map; (* buffered drawing *)
		ELSE
			map := rd.port.da.window; (* unbuffered drawing *)
		END;
		IF (mode = Ports.invert) OR (mode = Ports.hilite) THEN	vals.foreground := rd.port.da.style.white;
		ELSIF mode = Ports.dim25 THEN AllocateColor(dim25Col, gdkColor); vals.foreground := gdkColor
		ELSIF mode = Ports.dim50 THEN AllocateColor(dim50Col, gdkColor); vals.foreground := gdkColor
		ELSIF mode = Ports.dim75 THEN AllocateColor(dim75Col, gdkColor); vals.foreground := gdkColor
		ELSE (* mode = focusPat *)
			(* TODO: which color should be used here? *)
			AllocateColor(Ports.red, gdkColor);
			vals.foreground := gdkColor;
		END;
		vals.function := Gdk.GDK_XOR;
		vals.line_style := Gdk.GDK_LINE_SOLID;
		vals.line_width := 1;
		vals.subwindow_mode := Gdk.GDK_INCLUDE_INFERIORS;
		gc := Gdk.gdk_gc_new_with_values(map, vals,
			Gdk.GDK_GC_FOREGROUND + Gdk.GDK_GC_LINE_WIDTH +
			Gdk.GDK_GC_LINE_STYLE + Gdk.GDK_GC_SUBWINDOW + Gdk.GDK_GC_FUNCTION);
		IF s = 0 THEN s := 1 END;
		IF (s < 0) OR (r-l < 2*s) OR (b-t < 2*s) THEN
			Gdk.gdk_gc_set_line_attributes(gc, 1, Gdk.GDK_LINE_SOLID, Gdk.GDK_CAP_ROUND, Gdk.GDK_JOIN_MITER);
			Gdk.gdk_draw_rectangle(map, gc, on,  l, t, r - l, b - t)
		ELSE
			Gdk.gdk_gc_set_line_attributes(gc, 1, Gdk.GDK_LINE_SOLID, Gdk.GDK_CAP_ROUND, Gdk.GDK_JOIN_MITER);
			Gdk.gdk_draw_rectangle(map, gc, off,  l, t, s, b-t); DEC(r, s);
			Gdk.gdk_draw_rectangle(map, gc, off,  r, t, s, b-t); INC(l, s);
			Gdk.gdk_draw_rectangle(map, gc, off,  l, t, r-l, s); DEC(b, s);
			Gdk.gdk_draw_rectangle(map, gc, off,  l, b, r-l, s)
		END;
		Gdk.gdk_gc_unref(gc);
	END MarkRect;

	PROCEDURE (rd: Rider) Scroll* (dx, dy: INTEGER);
		VAR width, height, x, y, sx, sy: INTEGER; gc: Gdk.GdkGC;
	BEGIN
		ASSERT(rd.port # NIL, 100);
		IF dx < 0 THEN
			sx := rd.l - dx; x := rd.l; width := rd.r - rd.l + dx
		ELSE
			sx := rd.l; x := rd.l + dx; width := rd.r - rd.l - dx
		END;
		IF dy < 0 THEN
			sy := rd.t - dy; y := rd.t; height := rd.b - rd.t + dy
		ELSE
			sy := rd.t; y := rd.t + dy; height := rd.b - rd.t - dy
		END;
		gc := Gdk.gdk_gc_new(rd.port.da.window);
		Gdk.gdk_gc_set_exposures(gc, TRUE);
		INC(width); INC(height);
		Gdk.gdk_draw_drawable(rd.port.da.window, gc, rd.port.da.window, sx, sy, x, y, width, height);
		Gdk.gdk_gc_unref(gc);

		(* Invalidate the new area *)

		IF dy < 0 THEN
			Gtk.gtk_widget_queue_draw_area(rd.port.da, rd.l, rd.b + dy, rd.r - rd.l + 1,  -dy + 1)
		ELSIF dy > 0 THEN
			Gtk.gtk_widget_queue_draw_area(rd.port.da, rd.l, rd.t, rd.r - rd.l + 1,  dy + 1)
		END;
		IF dx < 0 THEN
			Gtk.gtk_widget_queue_draw_area(rd.port.da, rd.r + dx, rd.t, -dx + 1, rd.b - rd.t + 1)
		ELSIF dx > 0 THEN
			Gtk.gtk_widget_queue_draw_area(rd.port.da, rd.l, rd.t, dx + 1,  rd.b - rd.t + 1)
		END;

		(* pattern origin correction *)
		INC(rd.dx, dx); INC(rd.dy, dy)
	END Scroll;

	PROCEDURE (rd: Rider) SetCursor* (cursor: INTEGER);
	BEGIN
		Gdk.gdk_window_set_cursor(rd.port.da.window, cursors[cursor]);
	END SetCursor;

	PROCEDURE SetMouseState* (x, y: INTEGER; but: SET; isDown: BOOLEAN);
	BEGIN
		mx := x; my := y; mb := but
	END SetMouseState;

	PROCEDURE (rd: Rider) Input* (OUT x, y: INTEGER; OUT modifiers: SET; OUT isDown: BOOLEAN);
		VAR
			event: Gdk.GdkEvent;
			motion: Gdk.GdkEventMotion;
			button: Gdk.GdkEventButton;
			key: Gdk.GdkEventKey;
			state: SET; gotState: BOOLEAN;
	BEGIN
		IF Gdk.gdk_events_pending() THEN
			gotState := FALSE;
			event := Gdk.gdk_event_get();
			IF event # NIL THEN
				IF event.type = Gdk.GDK_MOTION_NOTIFY THEN
					motion :=  event(Gdk.GdkEventMotion);
					Gdk.gdk_window_get_position(rd.port.fixed.parent.window, mx, my);
					mx := SHORT(ENTIER(motion.x_root)) - mx;
					my := SHORT(ENTIER(motion.y_root)) - my;
					state := motion.state; gotState := TRUE;
				ELSIF event.type IN {Gdk.GDK_BUTTON_PRESS, Gdk.GDK_BUTTON_RELEASE} THEN
					button := event(Gdk.GdkEventButton);
					Gdk.gdk_window_get_position(rd.port.fixed.parent.window, mx, my);
					mx := SHORT(ENTIER(button.x_root)) - mx;
					my := SHORT(ENTIER(button.y_root)) - my;
					IF event.type = Gdk.GDK_BUTTON_PRESS THEN
						IF button.button = 1 THEN INCL(mb, left) END;
						IF button.button = 2 THEN INCL(mb, middle) END;
						IF button.button = 3 THEN INCL(mb, right) END
					ELSE
						IF button.button = 1 THEN EXCL(mb, left) END;
						IF button.button = 2 THEN EXCL(mb, middle) END;
						IF button.button = 3 THEN EXCL(mb, right) END
					END;
					state := button.state; gotState := TRUE;
				ELSIF event.type IN {Gdk.GDK_KEY_PRESS, Gdk.GDK_KEY_RELEASE} THEN
					key := event(Gdk.GdkEventKey);
					IF (key.keyval = Key.GDK_Shift_L) OR  (key.keyval = Key.GDK_Shift_R) THEN
						IF key.type = Gdk.GDK_KEY_PRESS THEN mb := mb + {shift, extend} ELSE  mb := mb - {shift, extend} END
					END;
					IF (key.keyval = Key.GDK_Control_L) OR (key.keyval = Key.GDK_Control_R) THEN
						IF key.type = Gdk.GDK_KEY_PRESS THEN mb := mb + {ctrl, modify} ELSE  mb := mb - {ctrl, modify} END
					END;
					IF (key.keyval = Key.GDK_Alt_L) OR (key.keyval = Key.GDK_Alt_R) THEN
						IF key.type = Gdk.GDK_KEY_PRESS THEN INCL(mb, alt); ELSE  EXCL(mb, alt) END
					END;
				ELSE
					Gtk.gtk_main_do_event(event);
				END;
				IF gotState THEN
					IF Gdk.GDK_SHIFT_BIT IN state THEN mb := mb + {shift, extend} ELSE  mb := mb - {shift, extend} END;
					IF Gdk.GDK_CONTROL_BIT IN state THEN mb := mb + {ctrl, modify} ELSE  mb := mb - {ctrl, modify} END;
					IF Gdk.GDK_MOD1_BIT IN state THEN INCL(mb, alt) ELSE EXCL(mb, alt) END;
				END;
				Gdk.gdk_event_free(event)
			END
		END;
		x := mx; y := my; modifiers := mb; isDown := mb * {left, middle, right} # {}
	END Input;

	PROCEDURE (rd: Rider) DrawSString* (x, y: INTEGER; col: Ports.Color; IN s: ARRAY OF SHORTCHAR; font: Fonts.Font);
	VAR gdkColor: Gdk.GdkColor; map: Gdk.GdkDrawable; gc: Gdk.GdkGC;
			layout:Pango.PangoLayout;
	BEGIN
		ASSERT(rd.port # NIL, 20);
		rd.DrawingBuf(map, gc);
		AllocateColor(col, gdkColor);
		Gdk.gdk_gc_set_foreground(gc, gdkColor);
		layout := HostFonts.layout; (* (layout); *)
		HostFonts.ShapeSString(layout, s, font);
		Gdk.gdk_draw_layout_line(map, gc, x, y, Pango.pango_layout_get_line(layout,0));
		(* GLib.g_object_unref(layout); *)
		Gdk.gdk_gc_unref(gc);
	END DrawSString;

	PROCEDURE (rd: Rider) SCharIndex* (x, pos: INTEGER; IN s: ARRAY OF SHORTCHAR; font: Fonts.Font): INTEGER;
	VAR layout:Pango.PangoLayout;
			x_pos, index,trailing: INTEGER; out: BOOLEAN;
	BEGIN
		ASSERT(rd.port # NIL, 100);

		layout := HostFonts.layout;
		HostFonts.ShapeSString(layout, s, font);
		x_pos:=(pos-x) * Pango.PANGO_SCALE;
		out:=Pango.pango_layout_line_x_to_index(Pango.pango_layout_get_line(layout,0),x_pos ,index,trailing);
		RETURN index;
	END SCharIndex;

	PROCEDURE (rd: Rider) SCharPos* (x, index: INTEGER; IN s: ARRAY OF SHORTCHAR; font: Fonts.Font): INTEGER;
	VAR layout:Pango.PangoLayout;
			x_pos: INTEGER;
	BEGIN
		ASSERT(rd.port # NIL, 100); ASSERT(index <= LEN(s), 101);

		layout := HostFonts.layout;
		HostFonts.ShapeSString(layout, s, font);

		Pango.pango_layout_line_index_to_x(
			Pango.pango_layout_get_line(layout,0),index,0,x_pos);
		RETURN x + x_pos DIV Pango.PANGO_SCALE;
	END SCharPos;

	PROCEDURE (rd: Rider) DrawString* (x, y: INTEGER; col: Ports.Color; IN s: ARRAY OF CHAR; font: Fonts.Font);
	VAR gdkColor: Gdk.GdkColor; map: Gdk.GdkDrawable; gc: Gdk.GdkGC;
			layout:Pango.PangoLayout;
	BEGIN
		ASSERT(rd.port # NIL, 20);
		rd.DrawingBuf(map, gc);
		AllocateColor(col, gdkColor);
		Gdk.gdk_gc_set_foreground(gc, gdkColor);
		layout := HostFonts.layout;
		HostFonts.ShapeString(layout, s, font);
		Gdk.gdk_draw_layout_line(map, gc, x, y, Pango.pango_layout_get_line(layout,0));
		(* GLib.g_object_unref(layout); *)
		Gdk.gdk_gc_unref(gc);
	END DrawString;

	PROCEDURE (rd: Rider) CharIndex* (x, pos: INTEGER; IN s: ARRAY OF CHAR; font: Fonts.Font): INTEGER;
	VAR layout:Pango.PangoLayout;
			x_pos, shortIndex, index, trailing: INTEGER; out: BOOLEAN;
	BEGIN
		ASSERT(rd.port # NIL, 100);

		layout := HostFonts.layout;
		HostFonts.ShapeString(layout, s, font);
		x_pos:=(pos-x) * Pango.PANGO_SCALE;
		out:=Pango.pango_layout_line_x_to_index(
			Pango.pango_layout_get_line(layout,0), x_pos, shortIndex, trailing);

		Utf8.ShortLenToLongLen(s, shortIndex, index);

		RETURN index
	END CharIndex;

	PROCEDURE (rd: Rider) CharPos* (x, index: INTEGER; IN s: ARRAY OF CHAR; font: Fonts.Font): INTEGER;
		VAR layout:Pango.PangoLayout;
			x_pos: INTEGER;
	BEGIN
		ASSERT(rd.port # NIL, 100); ASSERT(index <= LEN(s), 101);

		layout := HostFonts.layout;
		HostFonts.ShapeString(layout, s, font);

		Pango.pango_layout_line_index_to_x(
			Pango.pango_layout_get_line(layout,0), Utf8.ShortLen(s,index), 0, x_pos);
		RETURN x + x_pos DIV Pango.PANGO_SCALE;
	END CharPos;

	PROCEDURE (rd: Rider) SaveRect* (l, t, r, b: INTEGER; VAR res: INTEGER);
		VAR p: Port;
	BEGIN
		res := 1; p := rd.port;
		IF l < 0 THEN l := 0 END;
		IF t < 0 THEN t := 0 END;
		IF r > p.w THEN r := p.w END;
		IF b > p.h THEN b := p.h END;
		IF (l < r) & (t < b) THEN
			rd.sl := l; rd.st := t; rd.sr := r; rd.sb := b;
			rd.gc := Gdk.gdk_gc_new(p.da.window);
			IF rd.gc # NIL THEN
				rd.map := Gdk.gdk_pixmap_new(p.da.window, r - l, b - t, -1);
				IF rd.map # NIL THEN
					Gdk.gdk_gc_set_exposures(rd.gc, TRUE);
					Gdk.gdk_draw_drawable(rd.map, rd.gc, p.da.window, l, t, 0, 0, r - l, b- t);
					res := 0
				ELSE
					Gdk.gdk_gc_unref(rd.gc);
					rd.gc := NIL
				END
			END
		END
	END SaveRect;

	PROCEDURE (rd: Rider) RestoreRect* (l, t, r, b: INTEGER; dispose: BOOLEAN);
	BEGIN
		IF rd.gc # NIL THEN
			IF l < rd.sl THEN l := rd.sl END;
			IF t < rd.st THEN t := rd.st END;
			IF r > rd.sr THEN r := rd.sr END;
			IF b > rd.sb THEN b := rd.sb END;
			Gdk.gdk_draw_drawable(rd.port.da.window, rd.port.da.style.white_gc, rd.map, l - rd.sl, t - rd.st, l, t, r - l, b - t);
			IF dispose THEN
				Gdk.gdk_drawable_unref(rd.map);
				rd.map := NIL;
				Gdk.gdk_gc_unref(rd.gc);
				rd.gc := NIL
			END
		END
	END RestoreRect;


	(** miscellaneous **)

	PROCEDURE ResetColors* (basewidget: Gtk.GtkWidget);
	BEGIN
		ASSERT(basewidget # NIL, 20);
	(* TODO: Use the actual system colors  *)
		Ports.background := Ports.white;
		textCol := Ports.black;
		selBackground := Ports.black;
		selTextCol := Ports.white;

		Ports.dialogBackground := Ports.RGBColor(
			(basewidget.style.bg[Gtk.GTK_STATE_NORMAL].red DIV 256) MOD 256,
			(basewidget.style.bg[Gtk.GTK_STATE_NORMAL].green DIV 256) MOD 256,
			(basewidget.style.bg[Gtk.GTK_STATE_NORMAL].blue DIV 256) MOD 256);
		dialogTextCol := Ports.black;
		dialogShadowCol := Ports.grey75;
		dialogLightCol := Ports.grey50;
		dim25Col := Ports.white - Ports.grey75;
		dim50Col := Ports.white - Ports.grey75; (* TODO: On Windows this is a dashed line... *)
		dim75Col := Ports.white - Ports.grey25;
	END ResetColors;


	PROCEDURE Init;
		VAR i: INTEGER; wnd: Gtk.GtkWidget;
	BEGIN
		wnd := Gtk.gtk_window_new(Gtk.GTK_WINDOW_TOPLEVEL);
		Gtk.gtk_widget_ref(wnd);
		ResetColors(wnd);
		Gtk.gtk_widget_unref(wnd);
		cursors[Ports.arrowCursor] := Gdk.gdk_cursor_new(Gdk.GDK_LEFT_PTR);
		cursors[Ports.textCursor] := Gdk.gdk_cursor_new(Gdk.GDK_XTERM);
		cursors[Ports.graphicsCursor] := Gdk.gdk_cursor_new(Gdk.GDK_CROSSHAIR);
		cursors[Ports.bitmapCursor] := Gdk.gdk_cursor_new(Gdk.GDK_CROSSHAIR);
		cursors[Ports.tableCursor] := Gdk.gdk_cursor_new(Gdk.GDK_CROSS);
		cursors[Ports.refCursor] := Gdk.gdk_cursor_new(Gdk.GDK_HAND2);
		cursors[resizeHCursor] := Gdk.gdk_cursor_new(Gdk.GDK_SB_H_DOUBLE_ARROW);
		cursors[resizeVCursor] := Gdk.gdk_cursor_new(Gdk.GDK_SB_V_DOUBLE_ARROW);
		cursors[resizeLCursor] := Gdk.gdk_cursor_new(Gdk.GDK_TOP_LEFT_CORNER);
		cursors[resizeRCursor] := Gdk.gdk_cursor_new(Gdk.GDK_TOP_RIGHT_CORNER);
		cursors[resizeCursor] := Gdk.gdk_cursor_new(Gdk.GDK_FLEUR);
		cursors[busyCursor] := Gdk.gdk_cursor_new(Gdk.GDK_WATCH);
		cursors[stopCursor] := Gdk.gdk_cursor_new(Gdk.GDK_CIRCLE);
		(* TODO: On windows these are bitmaps stored as resources... *)
		cursors[moveCursor] := Gdk.gdk_cursor_new(Gdk.GDK_SAILBOAT);
		cursors[copyCursor] := Gdk.gdk_cursor_new(Gdk.GDK_SAILBOAT);
		cursors[linkCursor] := Gdk.gdk_cursor_new(Gdk.GDK_SAILBOAT);
		cursors[pickCursor] := Gdk.gdk_cursor_new(Gdk.GDK_SAILBOAT);
		i := 0;
		WHILE i < LEN(cursors) DO
			IF cursors[i] = NIL THEN cursors[i] := cursors[Ports.arrowCursor] END;
			INC(i)
		END
	END Init;

BEGIN
	Init
END HostPorts.
