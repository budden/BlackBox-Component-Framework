MODULE HostTabFrames;

	(*
		DONE:
			ShiryaevAV: 2012.11: UTF-8
			ShiryaevAV: 2012.11: gtk_widget_draw_default and gtk_widget_draw_focus calls commented
	*)

	(*
		TODO:
			This is more or less a dummy implementation. The major problem is to make the GtkNotebook transparent.
			The rest can be copied from HostCFrames.
	*)


	IMPORT
		SYSTEM,
		GLib := LibsGlib, Gtk:= LibsGtk, Gdk := LibsGdk,  GtkU := Gtk2Util,
		Kernel, Dialog, Services, Fonts, Ports, Views,
		HostFonts, HostPorts, StdTabViews;

	CONST
		RDEL = 07X; LDEL = 08X; ESC = 1BX;

	TYPE

		Info = POINTER TO RECORD
			frame: StdTabViews.Frame; (* pointer back to the frame that owns the Info *)
			mainWidget: Gtk.GtkWidget; (* the outer most widget *)
			eventWidget: Gtk.GtkWidget; (* widget to recieve events, not necessarily the same as mainWidget  *)
			allowMouse, allowKey, hasFocus: BOOLEAN
		END;

		Tab = POINTER TO RECORD (StdTabViews.Frame)
			noteBook: Gtk.GtkWidget;
			i: Info;
			dispLeft, dispTop, dispWidth, dispHeight: INTEGER;
		END;

		Directory = POINTER TO RECORD (StdTabViews.FrameDirectory) END;

		(* Delays mouse events until the end of the command *)
		DelayedMouseEvent = POINTER TO RECORD (Services.Action)
			eventPending: BOOLEAN;
			forwardWidget: Gtk.GtkWidget;
			forwardEvent: Gdk.GdkEventButtonDesc;
			delayedEvent: Gdk.GdkEventButtonDesc;
		END;

		TrapCleaner = POINTER TO RECORD (Kernel.TrapCleaner) END;

	VAR
		mouseDelayer: DelayedMouseEvent;

		(* used in HandleMouse *)
		released: BOOLEAN;
		currentInfo: Info;

		(* holds the state of the last key event. this is used later widthin the same command. *)
		lastKeyState: SET;


	(* Common mouse handling (filter mouse events throught the BlackBox framework) *)

	PROCEDURE (a: DelayedMouseEvent) Do-;
	BEGIN
		ASSERT(a.forwardWidget # NIL, 20); ASSERT(a.eventPending, 21);
		Gtk.gtk_widget_event(a.forwardWidget, a.forwardEvent);
		a.forwardWidget := NIL; a.eventPending := FALSE;
	END Do;

	PROCEDURE [ccall] EventSpy (widget: Gtk.GtkWidget; event: Gdk.GdkEvent; user_data: INTEGER): INTEGER;
	BEGIN
		IF event.type = Gdk.GDK_EXPOSE THEN
			Dialog.ShowMsg("expose");
			 RETURN 1
		END;
		Dialog.ShowMsg("event");
		RETURN 0
	END EventSpy;

	PROCEDURE [ccall] MouseHandler (widget: Gtk.GtkWidget; event: Gdk.GdkEventButton; user_data: INTEGER): INTEGER;
		VAR i: Info;
	BEGIN
		i := SYSTEM.VAL(Info, user_data);
		IF ~i.allowMouse THEN
			Gtk.gtk_signal_emit_stop_by_name(widget, "button-press-event");
			IF (i.frame.rider(HostPorts.Rider).port.da # NIL) & (event.type = Gdk.GDK_BUTTON_PRESS) THEN
				ASSERT(mouseDelayer.forwardWidget = NIL, 100); ASSERT(~mouseDelayer.eventPending, 101);
				mouseDelayer.delayedEvent := event;
				mouseDelayer.forwardEvent := event;
				mouseDelayer.forwardEvent.window := i.frame.rider(HostPorts.Rider).port.da.window;
				mouseDelayer.forwardEvent.x := mouseDelayer.forwardEvent.x + i.mainWidget.allocation.x;
				mouseDelayer.forwardEvent.y := mouseDelayer.forwardEvent.y + i.mainWidget.allocation.y;
				mouseDelayer.forwardWidget := i.frame.rider(HostPorts.Rider).port.da;
				mouseDelayer.eventPending := TRUE;
				(* wait with propagating event to parent until after this procedure has returned *)
				Services.DoLater(mouseDelayer, Services.immediately)
			END;
			RETURN 1
		ELSE
			RETURN 1
		END
	END MouseHandler;

	PROCEDURE [ccall] MouseRelease (widget: Gtk.GtkWidget; event: Gdk.GdkEventButton; user_data: INTEGER): INTEGER;
	BEGIN
		released := TRUE;
		RETURN 0
	END MouseRelease;

	PROCEDURE (t: TrapCleaner) Cleanup;
	BEGIN
		released := TRUE;
		mouseDelayer.eventPending := FALSE;
		currentInfo.allowMouse := FALSE;
		currentInfo := NIL
	END Cleanup;

	PROCEDURE HandleMouse (i: Info; x, y: INTEGER; buttons: SET);
		VAR res: INTEGER; tc: TrapCleaner;
	BEGIN
		IF mouseDelayer.eventPending THEN (* BlackBox tries to propagate an Event that the Gtk control didn't want *)
			NEW(tc); Kernel.PushTrapCleaner(tc);
			currentInfo := i;
			Gtk.gtk_grab_remove(i.frame.rider(HostPorts.Rider).port.da);
			ASSERT((mouseDelayer.delayedEvent.x = x + 1) & (mouseDelayer.delayedEvent.y = y + 1), 20); (* TODO: why "+ 1" ? *)
			i.allowMouse := TRUE;
			Gtk.gtk_widget_event(i.eventWidget, mouseDelayer.delayedEvent);
			released := FALSE;
			REPEAT
				Dialog.Beep;
				res := Gtk.gtk_main_iteration()
			UNTIL released;
			i.allowMouse := FALSE;
			currentInfo := NIL;
			Kernel.PopTrapCleaner(tc)
		END
	END HandleMouse;


	(* Common key handling (filter key events throught the BlackBox framework) *)

	PROCEDURE [ccall] KeyHandler(widget: Gtk.GtkWidget; event: Gdk.GdkEventKey; user_data: INTEGER): INTEGER;
		VAR i: Info;
	BEGIN
		i := SYSTEM.VAL(Info, user_data);
		IF ~i.allowKey THEN
			lastKeyState := event.state;
			Gtk.gtk_signal_emit_stop_by_name(widget, "key-press-event");
			RETURN 1
		ELSE
			RETURN 0
		END
	END KeyHandler;

	PROCEDURE HandleKey (i: Info; char: CHAR);
		VAR ne: Gdk.GdkEventKeyDesc; code: INTEGER;
	BEGIN
		i.allowKey := TRUE;
		CASE char OF
			| 10X: code := 0FF55H
			| 11X: code := 0FF56H
			| 12X: code := 0FF55H
			| 13X: code := 0FF56H
			| 14X: code := 0FF50H
			| 15X: code := 0FF57H
			| 16X: code := 0FF50H
			| 17X: code := 0FF57H
			| 1CX: code := 0FF51H
			| 1DX: code := 0FF53H
			| 1EX: code := 0FF52H
			| 1FX: code := 0FF54H
			| LDEL: code := 0FF08H
			| RDEL: code := 0FFFFH
			| ESC: code := 0FF1BH
		ELSE code := 0
		END;
		IF code # 0 THEN
			ne.keyval := code
		ELSE
			ne.keyval := Gdk.gdk_unicode_to_keyval(ORD(char));
		END;
		ne.length := 1;
		ne.string := SYSTEM.VAL(Gtk.PString, SYSTEM.ADR(char));
		ne.type := Gdk.GDK_KEY_PRESS;
		ne.window := i.eventWidget.window;
		ne.send_event := 1;
		ne.time := Gdk.GDK_CURRENT_TIME;
		ne.state := lastKeyState;
		Gtk.gtk_widget_event(i.eventWidget, ne);
		i.allowKey := FALSE
	END HandleKey;


	PROCEDURE NewInfo (frame: StdTabViews.Frame; mainWidget, eventWidget: Gtk.GtkWidget): Info;
		VAR i: Info; res: INTEGER;
	BEGIN
		ASSERT(frame # NIL, 20); ASSERT(mainWidget # NIL, 21); ASSERT(eventWidget # NIL, 22);
		NEW(i);
		i.allowMouse := FALSE; i.frame := frame; i.mainWidget := mainWidget; i.eventWidget := eventWidget;
		(* connect signals for the mouse handling - the new info is passed as user_data *)
		res := GtkU.gtk_signal_connect(eventWidget, "button_press_event", 
						SYSTEM.ADR(MouseHandler), SYSTEM.VAL(INTEGER, i));
		res := GtkU.gtk_signal_connect_after(eventWidget, "button-release-event", 
						SYSTEM.ADR(MouseRelease), SYSTEM.VAL(INTEGER, i));
		res := GtkU.gtk_signal_connect(eventWidget, "key-press-event", 
						SYSTEM.ADR(KeyHandler), SYSTEM.VAL(INTEGER, i));
		RETURN i
	END NewInfo;

	PROCEDURE Mark (on, focus: BOOLEAN; i: Info);
	BEGIN
		IF focus & (i # NIL) THEN
			IF on THEN
				IF ~i.hasFocus THEN
					Gtk.gtk_container_set_focus_child(i.frame.rider(HostPorts.Rider).port.fixed,
						 i.eventWidget);
					Gtk.gtk_widget_grab_focus(i.eventWidget);
(* ShiryaevAV
					Gtk.gtk_widget_draw_focus(i.eventWidget);
*)
					i.hasFocus := TRUE
				END
			ELSE
				IF i.hasFocus THEN
					Gtk.gtk_container_set_focus_child(i.frame.rider(HostPorts.Rider).port.fixed, NIL);
(* ShiryaevAV
					Gtk.gtk_widget_draw_default(i.eventWidget); (* *)
*)
					i.hasFocus := FALSE
				END
			END
		END
	END Mark;

	PROCEDURE Paint (i: Info);
	BEGIN
		i.frame.rider(HostPorts.Rider).port.CloseBuffer;
		Gtk.gtk_widget_show_all(i.mainWidget);
	END Paint;

	PROCEDURE NewStyle (font: Fonts.Font): Gtk.GtkRcStyle;
		VAR style: Gtk.GtkRcStyle;
	BEGIN
		style := Gtk.gtk_rc_style_new();
(*		style.fontset_name := font(HostFonts.Font).dev.xlfd;
		-> style.font_desc :=  ... *)
		RETURN style
	END NewStyle;

	(* On Windows & is used to mark an Alt-shortcut. This is not available on Linux. *)
	PROCEDURE RemoveAmpersand (VAR ss: ARRAY OF CHAR);
		VAR i, j: INTEGER;
	BEGIN
		i := 0; j := 0;
		WHILE ss[i] # 0X DO
			IF ss[i] # "&" THEN
				ss[j] := ss[i]; INC(j);
				INC(i)
			ELSE
				INC(i);
				IF ss[i] = "&" THEN ss[j] := ss[i]; INC(j); INC(i) END
			END
		END;
		ss[j] := 0X
	END RemoveAmpersand;

	PROCEDURE NewLabel (VAR text: ARRAY OF CHAR; style: Gtk.GtkRcStyle; left, right: BOOLEAN): Gtk.GtkLabel;
		VAR l: Gtk.GtkLabel; us: GLib.PString;
	BEGIN
		Dialog.MapString(text, text);
		RemoveAmpersand(text);
		us := GLib.g_utf16_to_utf8(text, -1, NIL, NIL, NIL);
		l := Gtk.gtk_label_new(us);
		GLib.g_free(SYSTEM.VAL(GLib.gpointer, us));
		Gtk.gtk_widget_modify_style(l, style);
		IF right & ~left THEN
			Gtk.gtk_misc_set_alignment(l, 1, 1);
			Gtk.gtk_label_set_justify(l, Gtk.GTK_JUSTIFY_RIGHT)
		ELSIF ~left THEN
			Gtk.gtk_misc_set_alignment(l, 0.5, 0.5);
			Gtk.gtk_label_set_justify(l, Gtk.GTK_JUSTIFY_CENTER)
		ELSE
			Gtk.gtk_misc_set_alignment(l, 0, 0);
			Gtk.gtk_label_set_justify(l, Gtk.GTK_JUSTIFY_LEFT)
		END;
		RETURN l
	END NewLabel;

	(* Tab *)

	PROCEDURE (f: Tab) SetOffset (x, y: INTEGER);
	BEGIN
		f.SetOffset^(x, y);
(*		Adapt(f, f.i)*)
	END SetOffset;

	PROCEDURE (f: Tab) Mark (on, focus: BOOLEAN);
	BEGIN
		Mark(on, f.front, f.i);
	END Mark;

	PROCEDURE TabSelect (clist: Gtk.GtkWidget; row, column: INTEGER;
									event: Gdk.GdkEventButton; user_data: INTEGER);
		VAR f: Tab; cur: INTEGER;
	BEGIN
		f := SYSTEM.VAL(Tab, user_data);
		IF f.noteBook # NIL THEN
			IF row # cur THEN f.SetIndex( row) END
		END
	END TabSelect;

	PROCEDURE (c: Tab) Update;
		VAR i, j: INTEGER;
	BEGIN
(*
		IF ~c.disabled THEN
			Gtk.gtk_widget_set_sensitive(c.list, TRUE);
			IF c.undef THEN i := -1 ELSE c.Get(c, i) END; j := i;
			IF i >= 0 THEN
				Gtk.gtk_clist_select_row(c.list, i, 0)
			ELSE
				Gtk.gtk_clist_unselect_all(c.list)
			END
		ELSE
			Gtk.gtk_widget_set_sensitive(c.list, FALSE);
		END;
*)
		Gtk.gtk_widget_show_all(c.noteBook)
	END Update;

	PROCEDURE (f: Tab) UpdateList;
		VAR
			s: Dialog.String; (*ss: ARRAY LEN(Dialog.String) OF SHORTCHAR;*)
			len, i, sel: INTEGER; tv: StdTabViews.View; v: Views.View; w: Gtk.GtkWidget;
			(*pm: Gdk.Pixmap;*)
	BEGIN
		IF f.noteBook # NIL THEN
			tv := f.view(StdTabViews.View); sel := tv.Index();
			len := tv.NofTabs();
			(* Add the tabs to the tabcontrol *)
			IF len > 0 THEN
				i := 0; tv.GetItem(i, s, v); Dialog.MapString(s, s); (*ss := SHORT(s$);*)
				WHILE (i < len) & (s # "") & (v # NIL) DO
				(*
					pm := Gdk.gdk_pixmap_new(f.noteBook.window, 10, 10, 10);
					w := Gtk.gtk_pixmap_new(pm, Gdk.gdk_pixmap_new(f.noteBook.window, 10, 10, 1));
				*)
					w := Gtk.gtk_event_box_new();
					Gtk.gtk_notebook_append_page(f.noteBook, w, NewLabel(s, NewStyle(f.font), TRUE, FALSE));
					INC(i);
					IF i < len THEN
						tv.GetItem(i, s, v); Dialog.MapString(s, s)
						(*; ss := SHORT(s$) *)
					END
				END
			END;
			f.Update
		END
(*	*)
	END UpdateList;

	PROCEDURE (c: Tab) Restore (l, t, r, b: INTEGER);
		VAR cx, cy, cw, ch, res: INTEGER; s: Dialog.String;
	BEGIN
		IF c.noteBook = NIL THEN
			c.noRedraw := TRUE;
			c.noteBook := Gtk.gtk_notebook_new();
			Gtk.gtk_widget_ref(c.noteBook);
			Gtk.gtk_widget_modify_style(c.noteBook, NewStyle(c.font));
(*			s := "one";
			Gtk.gtk_notebook_append_page(c.noteBook, NewLabel(s, NewStyle(c.font), TRUE, FALSE), NewLabel(s, NewStyle(c.font), TRUE, FALSE));*)
		(*
			res := GtkU.gtk_signal_connect(c.list, "select-row",
											 SYSTEM.ADR(ListSelect)),
											SYSTEM.VAL(INTEGER, c));
		*)
			res := GtkU.gtk_signal_connect(c.noteBook, "event", SYSTEM.ADR(EventSpy), 0);
			c.i := NewInfo(c, c.noteBook, c.noteBook);
			c.rider(HostPorts.Rider).GetRect(cx, cy, cw, ch); cw := cw - cx; ch := ch - cy;
			Gtk.gtk_widget_set_usize(c.noteBook, cw, ch);
			Gtk.gtk_fixed_put(c.rider(HostPorts.Rider).port.fixed, c.noteBook, SHORT(cx), SHORT(cy));
			c.UpdateList
		ELSE
			c.Update;
		END;
		Paint(c.i)
	END Restore;

	PROCEDURE (c: Tab) Close;
	BEGIN
		IF c.noteBook # NIL THEN
			Gtk.gtk_widget_unref(c.noteBook);
			Gtk.gtk_container_remove(c.rider(HostPorts.Rider).port.fixed, c.noteBook);
		END;
		c.i := NIL; c.noteBook := NIL
	END Close;

	PROCEDURE (f: Tab) MouseDown (x, y: INTEGER; buttons: SET);
	BEGIN
		ASSERT(~f.disabled, 100);
		IF f.rider # NIL THEN
			HandleMouse(f.i, x DIV f.unit, y DIV f.unit, buttons)
		END
	END MouseDown;

	PROCEDURE (f: Tab) KeyDown (ch: CHAR);
	BEGIN
		ASSERT(~f.disabled, 100);
		HandleKey(f.i, ch)
	END KeyDown;

	PROCEDURE (f: Tab) InDispArea (x, y: INTEGER): BOOLEAN;
	BEGIN
		IF ((x >= f.dispLeft) & (x <= f.dispLeft + f.dispWidth) &
			(y >= f.dispTop) & (y <= f.dispTop + f.dispHeight)) THEN
			RETURN TRUE
		ELSE
			RETURN FALSE
		END
	END InDispArea;

	PROCEDURE (f: Tab) GetDispSize (OUT x, y, w, h: INTEGER);
	(*
		VAR
			rect: WinApi.RECT;
			res: INTEGER;
	BEGIN
		IF ~Views.IsPrinterFrame(f) THEN
			rect.left := 0; rect.top := 0; rect.right := f.i.w2; rect.bottom := f.i.h2;
			res := WinApi.SendMessageA(f.i.ctrl, WinCtl.TCM_ADJUSTRECT, WinApi.FALSE, SYSTEM.ADR(rect));
			f.dispLeft := rect.left * f.unit; f.dispTop := rect.top * f.unit;
			f.dispWidth := (rect.right - rect.left) * f.unit; f.dispHeight := (rect.bottom - rect.top) * f.unit
		END;
		x := f.dispLeft; y := f.dispTop; w := f.dispWidth; h := f.dispHeight
	*)
	BEGIN
		f.dispLeft := 10  * f.unit; f.dispTop := 10 * f.unit; f.dispWidth := 100 * f.unit; f.dispHeight := 100 * f.unit;
		x := f.dispLeft; y := f.dispTop; w := f.dispWidth; h := f.dispHeight
	END GetDispSize;

	(* Directory *)
	PROCEDURE (d: Directory) GetTabSize (VAR w, h: INTEGER);
	BEGIN
		IF w = Views.undefined THEN w := 150 * Ports.point END;
		IF h = Views.undefined THEN h := 100 * Ports.point END
	END GetTabSize;

	PROCEDURE (d: Directory) New (): StdTabViews.Frame;
		VAR f: Tab;
	BEGIN
		NEW(f); RETURN f
	END New;

	PROCEDURE Init*;
		VAR dir: Directory;
	BEGIN
		NEW(mouseDelayer);
		StdTabViews.setFocus := TRUE;
		NEW(dir); StdTabViews.SetFrameDir(dir)
	END Init;

END HostTabFrames.
