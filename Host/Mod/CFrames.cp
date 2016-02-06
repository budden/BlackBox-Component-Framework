MODULE HostCFrames;
(**
	project	= "BlackBox"
	organization	= "www.oberon.ch"
	contributors	= "Oberon microsystems"
	version	= "System/Rsrc/About"
	copyright	= "System/Rsrc/About"
	license	= "Docu/BB-License"
	changes	= "
	- 20060407, mf, ListBox.DblClickOk: case f.sorted handled
	- 20070131, bh, Unicode support
	- 20070209, bh, Euro handling removed
	"
	issues	= ""

**)

	(* ColorField, DateField, TimeField, UpDownField *)

	(* TODO:
		- Mouse events are not received when the widget is disabled
		- TimeField, DateField and ColorField are not implemented
		- GroupBox: Would be nice to use GtkFrame, but how can this one be made transparent?
	*)


	(* Implementation docu
		Most controls fit fine with widgets from Gtk.
		Some controls (ListBoxes, Fields) need a scrolled window around their widget to allow for scrolling.
		Some other widgets (Captions, CheckBoxes) do not have their own window and need to be wrapped in an event box.

		BlackBox wants to filter mouse and keyboard events. To do this special handlers for these events are installed (done in NewInfo).
		These handlers basically stops the events and resends the event only if the event comes back through the BlackBox framework.
		 *)

	IMPORT
		SYSTEM,
		GLib := LibsGlib, Gdk := LibsGdk, Key:=Gtk2Keysyms, Gtk := LibsGtk, GtkU:=Gtk2Util,

		Kernel, Services, Strings, Dates, Files, Fonts, Ports, Views,
		Controllers, Dialog, HostFiles, HostFonts, HostRegistry, HostPorts, HostWindows, StdCFrames,
		(*Kernel, Files, Dialog, Fonts, Ports,  Views, Services,
		HostFiles, HostFonts, HostPorts, StdCFrames,*)
		Log, Utf8 := HostUtf8;

	CONST
(*
		ENTER = 0DX; ESC = 1BX; DEL = 07X; BS = 08X; TAB = 09X; LTAB = 0AX;
		AL = 1CX; AR = 1DX; AU = 1EX; AD = 1FX;
		PL = 10X; PR = 11X; PU = 12X; PD = 13X;
		DL = 14X; DR = 15X; DU = 16X; DD = 17X;
		dlgWindowExtra = 30;
		dropDownHeight = 30 * Ports.point;
		upDownWidth = 11 * Ports.point;
		numColors = 7;
*)
		dropDownHeight = 30 * Ports.point;

		(* possible values for eventMaskState *)
		eUndef = 0; eOn = 1; eOff = 2;

		(* alignment  *)
		none = 0; left = 1; right = 2; center = 3;

		scrollRange = 16384; lineInc = 1; pageInc = 100; defThumbSize = scrollRange DIV 20;

		off = 0; on = 1;

	TYPE
		Info = POINTER TO RECORD
			frame: StdCFrames.Frame; (* pointer back to the frame that owns the Info *)
			mainWidget: Gtk.GtkWidget; (* the outer most widget *)
			eventWidget: Gtk.GtkWidget; (* widget to recieve events, not necessarily the same as mainWidget  *)
			allowMouse, allowKey, hasFocus: BOOLEAN;
			eventMaskState: INTEGER;
		END;

		PushButton = POINTER TO RECORD (StdCFrames.PushButton)
			button: Gtk.GtkButton;
			ebox: Gtk.GtkEventBox;
			lbl: Gtk.GtkLabel;
			hasFocus: BOOLEAN;
			i: Info
		END;

		CheckBox = POINTER TO RECORD (StdCFrames.CheckBox)
			checkButton:Gtk.GtkCheckButton;
			ebox: Gtk.GtkEventBox;
			lbl: Gtk.GtkLabel;
			hasFocus: BOOLEAN;
			i: Info
		END;

		RadioButton = POINTER TO RECORD (StdCFrames.RadioButton)
			radioButton: Gtk.GtkRadioButton; (* RadioButton? *)
			ebox: Gtk.GtkEventBox;
			lbl: Gtk.GtkLabel;
			i: Info
		END;

		ScrollBar = POINTER TO RECORD (StdCFrames.ScrollBar)
			scrollBar: Gtk.GtkScrollbar;
			isUpdate: BOOLEAN;
			oldPos: REAL;
			i: Info
		END;

		Field = POINTER TO RECORD (StdCFrames.Field)
			i: Info;
			text: Gtk.GtkWidget; (* GtkText/GtkEntry *);
			scrlw: Gtk.GtkScrolledWindow;
			TextLen: Gtk.TextLenProc;
			isUpdate: BOOLEAN
		END;

		UpDownField = POINTER TO RECORD (StdCFrames.UpDownField)
			i: Info;
			spin: Gtk.GtkSpinButton;
			val: INTEGER;
			hasFocus, isUpdate: BOOLEAN
		END;

		DateField = POINTER TO RECORD (StdCFrames.DateField)
			(*
			i: Info;
			isUpdate: BOOLEAN;
			cnt, val: INTEGER	(* input state: val = current val, cnt = number of key strokes *)
			*)
		END;

		TimeField = POINTER TO RECORD (StdCFrames.TimeField)
			(*
			i: Info;
			isUpdate: BOOLEAN;
			cur: INTEGER
			*)
		END;

		ColorField = POINTER TO RECORD (StdCFrames.ColorField)
			(*
			i: Info;
			color: Ports.Color
			*)
		END;

		ListBox = POINTER TO RECORD (StdCFrames.ListBox)
			list: Gtk.GtkCList;
			scrlw: Gtk.GtkScrolledWindow;
			i: Info
		END;

		SelectionBox = POINTER TO RECORD (StdCFrames.SelectionBox)
			i: Info;
			list: Gtk.GtkCList;
			scrlw: Gtk.GtkScrolledWindow;
			num: INTEGER (* the number of items in the list, updated by UpdateList *)
		END;

		ComboBox = POINTER TO RECORD (StdCFrames.ComboBox)
			i: Info;
			combo: Gtk.GtkCombo;
			isUpdate: BOOLEAN
		END;

		Caption = POINTER TO RECORD (StdCFrames.Caption)
			labelWidget: Gtk.GtkLabel;
			ebox: Gtk.GtkEventBox;
			(* a label widget does not have its own GdkWindow, so it needs to be wrapped in an event box *)
			i: Info
		END;

		Group = POINTER TO RECORD (StdCFrames.Group)
			(*
			i: Info
			*)
		END;

		TreeFrame = POINTER TO RECORD (StdCFrames.TreeFrame)
			i: Info;
			ctree: Gtk.GtkCTree;
			scrlw: Gtk.GtkScrolledWindow;
			hasFocus: BOOLEAN
		END;

		Directory = POINTER TO RECORD (StdCFrames.Directory) END;

		TrapCleaner = POINTER TO RECORD (Kernel.TrapCleaner) END;

		(* Delays mouse events until the end of the command *)
		DelayedMouseEvent = POINTER TO RECORD (Services.Action)
			eventPending: BOOLEAN;
			forwardWidget: Gtk.GtkWidget;
			forwardEvent: Gdk.GdkEventButtonDesc;
			delayedEvent: Gdk.GdkEventButtonDesc
		END;

	VAR
		(* date format *)
		dateSep: CHAR;	(* separator character *)
		yearPart, monthPart, dayPart: INTEGER;	(* first = 1, last = 3 *)
		del1, del2: INTEGER;	(* position of separators *)
		(* time format *)
		timeSep: CHAR;	(* separator character *)
		lastPart: INTEGER;	(* 3 for 24h format, 4 for 12h format *)

		mouseDelayer: DelayedMouseEvent;
		(* icons for the tree *)
		tree_closed, tree_open, tree_leaf: Gdk.GdkPixmap;
		tree_closed_mask, tree_open_mask, tree_leaf_mask: Gdk.GdkBitmap;

		(* used in HandleMouse *)
		released: BOOLEAN;
		currentInfo: Info;
		(* holds the state of the last key event. this is used later widthin the same command. *)
		lastKeyState: SET;


	(* auxiliary procedures *)

	PROCEDURE (t: TrapCleaner) Cleanup;
	BEGIN
		released := TRUE;
		mouseDelayer.eventPending := FALSE;
		currentInfo.allowMouse := FALSE;
		currentInfo := NIL
	END Cleanup;

	(* Common mouse handling (filter mouse events throught the BlackBox framework) *)

	PROCEDURE (a: DelayedMouseEvent) Do-;
	BEGIN
		ASSERT(a.forwardWidget # NIL, 20);
		ASSERT(a.eventPending, 21);
		Gtk.gtk_widget_event(a.forwardWidget,  a.forwardEvent);
		a.forwardWidget := NIL;
		a.eventPending := FALSE
	END Do;

	PROCEDURE [ccall] MouseHandler (widget: Gtk.GtkWidget; event: Gdk.GdkEventButton; user_data: INTEGER): INTEGER;
		VAR i: Info;
	BEGIN
		i := SYSTEM.VAL(Info, user_data);
		IF ~i.allowMouse THEN
			Gtk.gtk_signal_emit_stop_by_name(widget, "button-press-event");
			IF (i.frame.rider(HostPorts.Rider).port.da # NIL) & (event.type = Gdk.GDK_BUTTON_PRESS) THEN
				ASSERT(mouseDelayer.forwardWidget = NIL, 100);
				ASSERT(~mouseDelayer.eventPending, 101);

				mouseDelayer.delayedEvent := event;
				mouseDelayer.delayedEvent.send_event := 1;
				mouseDelayer.forwardEvent := event;
				mouseDelayer.forwardEvent.send_event := 1;
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

	PROCEDURE HandleMouse (i: Info; x, y: INTEGER; buttons: SET);
		VAR res: INTEGER; tc: TrapCleaner;
	BEGIN
		IF mouseDelayer.eventPending THEN
			(* BlackBox tries to propagate an Event that the Gtk control didn't want *)
			NEW(tc);
			Kernel.PushTrapCleaner(tc);
			currentInfo := i;
			Gtk.gtk_grab_remove(i.frame.rider(HostPorts.Rider).port.da);

	(*	ASSERT((mouseDelayer.delayedEvent.x = x + 1) & (mouseDelayer.delayedEvent.y = y + 1), 20);
		(* TODO: why "+ 1" ? *)
	*)

			i.allowMouse := TRUE;
			Gtk.gtk_widget_event(i.eventWidget, mouseDelayer.delayedEvent);
			released := FALSE;
			REPEAT
				res := Gtk.gtk_main_iteration()
			UNTIL released;
			i.allowMouse := FALSE;
			currentInfo := NIL;
			Kernel.PopTrapCleaner(tc)
		END
	END HandleMouse;

	PROCEDURE UpdateEventMask (i: Info);
	(* This is a hack to get mouse events propagated to the parent when the widget is disabled *)
	(* TODO: It only seems to work with buttons... *)
		VAR mask: Gdk.GdkEventMask;
	BEGIN
		IF  i.eventWidget.window # NIL THEN
			IF ~i.frame.disabled & ~i.frame.readOnly THEN
				IF i.eventMaskState # eOn THEN
					mask := Gdk.gdk_window_get_events(i.eventWidget.window);
					mask := mask + Gdk.GDK_BUTTON_PRESS_MASK;
					Gdk.gdk_window_set_events(i.eventWidget.window, mask);
					i.eventMaskState := eOn
				END
			ELSIF i.eventMaskState # eOff THEN
				mask := Gdk.gdk_window_get_events(i.eventWidget.window);
				mask := mask - Gdk.GDK_BUTTON_PRESS_MASK;
				Gdk.gdk_window_set_events(i.eventWidget.window, mask);
				i.eventMaskState := eOff
			END
		ELSE
			i.eventMaskState := eUndef
		END
	END UpdateEventMask;

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
	CONST
			RDEL = 07X; LDEL = 08X;  TAB = 09X; LTAB = 0AX; ENTER = 0DX;
			PL = 10X; PR = 11X; PU = 12X; PD = 13X;
			DL = 14X; DR = 15X; DU = 16X; DD = 17X; ESC = 1BX;
			AL = 1CX; AR = 1DX; AU = 1EX; AD = 1FX;
	VAR ne: Gdk.GdkEventKeyDesc;
			code: INTEGER;
	BEGIN
		i.allowKey := TRUE;
		CASE char OF
			| PL, PU: code := Key.GDK_Page_Up
			| PR, PD: code := Key.GDK_Page_Down
			| DL, DU: code := Key.GDK_Home
			| DR, DD: code := Key.GDK_End
			| AL: code := Key.GDK_Left
			| AR: code := Key.GDK_Right
			| AU: code := Key.GDK_Up
			| AD: code := Key.GDK_Down
			| LDEL: code := Key.GDK_BackSpace
			| RDEL: code := Key.GDK_Delete
			| ESC: code := Key.GDK_Escape
			| ENTER: code := Key.GDK_Return
			| TAB : code := Key.GDK_Tab	(* ?*)
			| LTAB : code := Key.GDK_Tab; (* ?*)
		ELSE code := 0
		END;
		IF code # 0 THEN
			ne.keyval := code
		ELSE
			ne.keyval := Gdk.gdk_unicode_to_keyval(ORD(char))
		END;
		ne.length := 1;
		ne.string := SYSTEM.VAL(Gdk.PString, SYSTEM.ADR(char));
		ne.type := Gdk.GDK_KEY_PRESS;
		ne.window := i.eventWidget.window;
		ne.send_event := 1;
		ne.time := Gdk.GDK_CURRENT_TIME;
		ne.state := lastKeyState;
		Gtk.gtk_widget_event(i.eventWidget, ne);
		i.allowKey := FALSE
	END HandleKey;


	PROCEDURE NewInfo (frame: StdCFrames.Frame; mainWidget, eventWidget: Gtk.GtkWidget): Info;
		VAR i: Info; res: INTEGER;
	BEGIN
		ASSERT(frame # NIL, 20); ASSERT(mainWidget # NIL, 21); ASSERT(eventWidget # NIL, 22);
		NEW(i);
		i.allowMouse := FALSE;
		i.frame := frame; i.mainWidget := mainWidget; i.eventWidget := eventWidget;
		i.eventMaskState := eUndef;
		(* connect signals for the mouse handling - the new info is passed as user_data *)
		res := GtkU.gtk_signal_connect(eventWidget, "button_press_event", SYSTEM.ADR(MouseHandler), SYSTEM.VAL(INTEGER, i));
		res := GtkU.gtk_signal_connect_after(eventWidget, "button-release-event", SYSTEM.ADR(MouseRelease), SYSTEM.VAL(INTEGER, i));
		res := GtkU.gtk_signal_connect(eventWidget, "key-press-event", SYSTEM.ADR(KeyHandler), SYSTEM.VAL(INTEGER, i));
		RETURN i
	END NewInfo;

	PROCEDURE [ccall] EditableTextLen (editable: Gtk.GtkWidget): INTEGER;
		VAR s: Gtk.PString; res: INTEGER;
	BEGIN
		res := 0;
		s := Gtk.gtk_editable_get_chars(editable, 0, -1);
		IF s # NIL THEN
			res := LEN(s$);
			GLib.g_free(SYSTEM.VAL(GLib.gpointer, s))
		END;
		RETURN res
	END EditableTextLen;

	PROCEDURE Mark (on, focus: BOOLEAN; i: Info);
	BEGIN
		IF focus & (i # NIL) & (i.eventWidget # NIL) THEN
			IF on THEN
				IF ~i.hasFocus THEN
					Gtk.gtk_container_set_focus_child(i.frame.rider(HostPorts.Rider).port.fixed,
						 i.eventWidget);
					Gtk.gtk_widget_grab_focus(i.eventWidget);
(*				Gtk.gtk_widget_draw_focus(i.eventWidget); *)
					i.hasFocus := TRUE
				END
			ELSE
				IF i.hasFocus THEN
					Gtk.gtk_container_set_focus_child(i.frame.rider(HostPorts.Rider).port.fixed, NIL);
					(* Gtk.gtk_widget_draw_default(i.eventWidget);   *)
					i.hasFocus := FALSE
				END
			END
		END
	END Mark;

	PROCEDURE NewStyle (font: Fonts.Font): Gtk.GtkRcStyle;
		VAR style: Gtk.GtkRcStyle;
	BEGIN
		style := Gtk.gtk_rc_style_new();
		style.font_desc := font(HostFonts.Font).desc; (*!!!*)
		RETURN style
	END NewStyle;

	(* On Windows & is used to mark an Alt-shortcut. This is not available on Linux. *)
	PROCEDURE AmpersandToUline (VAR ss: ARRAY OF CHAR);
		VAR i: INTEGER;
	BEGIN
	i:=0;
		WHILE (ss[i] # 0X) DO IF ss[i] = "&" THEN ss[i] := "_" END ;	INC(i)	END;
	END AmpersandToUline;

	PROCEDURE NewLabel (VAR text: ARRAY OF CHAR; style: Gtk.GtkRcStyle): Gtk.GtkLabel;
		VAR lbl: Gtk.GtkLabel;
			us: GLib.PString;
	BEGIN
		Dialog.MapString(text, text);
		AmpersandToUline(text);
		us := GLib.g_utf16_to_utf8(text, -1, NIL, NIL, NIL);
		lbl := Gtk.gtk_label_new_with_mnemonic(us);
		GLib.g_free(SYSTEM.VAL(GLib.gpointer, us));
		Gtk.gtk_widget_ref(lbl);
		Gtk.gtk_widget_modify_style(lbl, style);
		RETURN lbl
	END NewLabel;

	PROCEDURE DummyRestore (f: StdCFrames.Frame; lbl: Dialog.String; l, t, r, b: INTEGER);
		VAR w, h, mx, my, dw, dh, asc, dsc: INTEGER;
	BEGIN
		AmpersandToUline(lbl);
		f.view.context.GetSize(w, h);
		mx := w DIV 2; my := h DIV 2;
		dw := HostFonts.dlgFont.StringWidth(lbl) DIV 2;
		HostFonts.dlgFont.GetBounds(asc, dsc, dh); dh := (asc + dsc) DIV 2;
		f.DrawRect(0, 0, w, h, Ports.fill, Ports.grey50);
		f.DrawRect(0, 0, w, h, 1, Ports.black);
		f.DrawString(mx - dw, my + dh, Ports.white, lbl, HostFonts.dlgFont)
	END DummyRestore;

	PROCEDURE Paint (i: Info);
	BEGIN
		i.frame.rider(HostPorts.Rider).port.CloseBuffer;
		Gtk.gtk_widget_show_all(i.mainWidget)
	END Paint;


	(* PushButton *)

	PROCEDURE Execute (f: PushButton);
	BEGIN
		IF f.Do # NIL THEN
			Dialog.ShowStatus("");
			f.Do(f);
		END
	END Execute;
(*
	PROCEDURE (f: PushButton) SetOffset (x, y: INTEGER);
	BEGIN
		f.SetOffset^(x, y);
		(*Adapt(f, f.i)*)
	END SetOffset;
*)
	PROCEDURE (c: PushButton) Close;
	BEGIN
		IF c.button # NIL THEN
			Gtk.gtk_widget_unref(c.button);
			Gtk.gtk_widget_unref(c.lbl);
			Gtk.gtk_widget_unref(c.ebox);
			Gtk.gtk_container_remove(c.rider(HostPorts.Rider).port.fixed, c.ebox)
		END;
	  c.button := NIL; c.i := NIL
	END Close;

	PROCEDURE [ccall] ButtonClick (button: Gtk.GtkWidget; user_data: INTEGER);
		VAR f: PushButton;
	BEGIN
		f := SYSTEM.VAL(PushButton, user_data);
		ASSERT(~f.disabled, 100);
		Execute(f);
		released := TRUE
	END ButtonClick;

	PROCEDURE (c: PushButton) Update;
		VAR mask: Gdk.GdkEventMask;
	BEGIN
		ASSERT(c.button # NIL);
		IF ~c.disabled & ~c.readOnly THEN
			Gtk.gtk_widget_set_sensitive(c.button, on);
			IF c.default THEN
				INCL(c.button.flags, 13); (* GTK_CAN_DEFAULT *)
				Gtk.gtk_widget_grab_default(c.button)
			END
		ELSE
			Gtk.gtk_widget_set_sensitive(c.button, off)
		END;
(**)
		Gtk.gtk_widget_show_all(c.button);
		UpdateEventMask(c.i)
	END Update;

	PROCEDURE (c: PushButton) Restore (l, t, r, b: INTEGER);
		VAR w, h, cx, cy, cw, ch, res: INTEGER;
		s: Dialog.String;
	BEGIN
		IF c.button = NIL THEN
			c.noRedraw := TRUE;
			c.rider(HostPorts.Rider).GetRect(cx, cy, cw, ch);
			cw := cw - cx; ch := ch - cy;
			c.lbl := NewLabel(c.label, NewStyle(c.font));
			c.button := Gtk.gtk_button_new(); Gtk.gtk_widget_ref(c.button);
			Gtk.gtk_container_add(c.button, c.lbl);
			c.ebox := Gtk.gtk_event_box_new(); 	Gtk.gtk_widget_ref(c.ebox);

			res := GtkU.gtk_signal_connect(c.button, "clicked", SYSTEM.ADR(ButtonClick), SYSTEM.VAL(INTEGER, c));
			c.i := NewInfo(c, c.ebox, c.ebox);
			Gtk.gtk_container_add(c.ebox, c.button);
			Gtk.gtk_widget_set_usize(c.ebox, cw, ch); (*	*)
			Gtk.gtk_fixed_put(c.rider(HostPorts.Rider).port.fixed, c.ebox, cx, cy)
		END;
		c.Update;
		Paint(c.i)
	END Restore;

	PROCEDURE (f: PushButton) MouseDown (x, y: INTEGER; buttons: SET);
	BEGIN
		ASSERT(~f.disabled, 100);
		IF f.rider # NIL THEN
			HandleMouse(f.i, x DIV f.unit, y DIV f.unit, buttons)
		END
	END MouseDown;

	PROCEDURE (f: PushButton) KeyDown (ch: CHAR);
	BEGIN
		ASSERT(~f.disabled, 100);
		Execute(f)
	END KeyDown;

	PROCEDURE (f: PushButton) Mark (on, focus: BOOLEAN);
	BEGIN
(*		Mark(on, f.front, f.button, f.hasFocus);*)
		Mark(on, f.front, f.i);
	END Mark;



	(* CheckBox *)

	PROCEDURE [ccall] CheckToggled (checkButton: Gtk.GtkWidget; user_data: INTEGER);
		VAR f: CheckBox;
	BEGIN
		f := SYSTEM.VAL(CheckBox, user_data);
		IF ~f.disabled THEN
			f.Set(f, Gtk.gtk_toggle_button_get_active(f.checkButton) # 0)
		END
	END CheckToggled;
(*
	PROCEDURE (f: CheckBox) SetOffset (x, y: INTEGER);
	BEGIN
		f.SetOffset^(x, y);
		(*Adapt(f, f.i)*)
	END SetOffset;
*)
	PROCEDURE (c: CheckBox) Close;
	BEGIN
		IF c.checkButton # NIL THEN
			Gtk.gtk_widget_unref(c.lbl);
			Gtk.gtk_widget_unref(c.checkButton);
			Gtk.gtk_widget_unref(c.ebox);
			Gtk.gtk_container_remove(c.rider(HostPorts.Rider).port.fixed, c.ebox)
		END;
		c.i := NIL; c.lbl := NIL; c.checkButton := NIL; c.ebox := NIL
	END Close;

	PROCEDURE (c: CheckBox) Update;
		VAR res: INTEGER; value, mixed: BOOLEAN; mask: Gdk.GdkEventMask;
	BEGIN
		IF ~c.disabled THEN
			c.Get(c, value);
			IF c.undef THEN
(*				res := USER32.SendMessageA(f.i.ctrl, USER32.BMSetCheck, 2, 0) TODO: Here what? *)
			ELSIF value THEN Gtk.gtk_toggle_button_set_active(c.checkButton, 1)
			ELSE Gtk.gtk_toggle_button_set_active(c.checkButton, 0)
			END;
			IF c.readOnly THEN Gtk.gtk_widget_set_sensitive(c.checkButton, off)
			ELSE Gtk.gtk_widget_set_sensitive(c.checkButton, on)
			END
		ELSE
			Gtk.gtk_toggle_button_set_active(c.checkButton, 0);
			Gtk.gtk_widget_set_sensitive(c.checkButton, off)
		END;
(*		CheckLabel(f.label, f.i.ctrl);*)
(*
		UpdateEventMask(c.i);
		IF c.lbl.window # NIL THEN
			mask := Gdk.gdk_window_get_events(c.lbl.window);
			mask := mask - Gdk.GDK_BUTTON_PRESS_MASK;
			Gdk.gdk_window_set_events(c.lbl.window, mask)
		END
*)
	END Update;

	PROCEDURE (c: CheckBox) Restore (l, t, r, b: INTEGER);
		VAR w, h, cx, cy, cw, ch, res: INTEGER;
		s: Dialog.String;
	BEGIN
		IF c.checkButton = NIL THEN
			c.noRedraw := TRUE;
			c.rider(HostPorts.Rider).GetRect(cx, cy, cw, ch);
			cw := cw - cx; ch := ch - cy;

			c.lbl := NewLabel(c.label, NewStyle(c.font));
			c.checkButton := Gtk.gtk_check_button_new();		Gtk.gtk_widget_ref(c.checkButton);
			Gtk.gtk_container_add(c.checkButton, c.lbl);

			res := GtkU.gtk_signal_connect(c.checkButton, "toggled", SYSTEM.ADR(CheckToggled),SYSTEM.VAL(INTEGER, c));
			c.ebox := Gtk.gtk_event_box_new(); 	Gtk.gtk_widget_ref(c.ebox);
			c.i := NewInfo(c, c.ebox, c.checkButton);
			Gtk.gtk_widget_set_usize(c.ebox, cw, ch);
			Gtk.gtk_container_add(c.ebox, c.checkButton);
			Gtk.gtk_fixed_put(c.rider(HostPorts.Rider).port.fixed, c.ebox, cx, cy)
		END;
		c.Update;
		Paint(c.i)
	END Restore;

	PROCEDURE (f: CheckBox) MouseDown (x, y: INTEGER; buttons: SET);
	BEGIN
		ASSERT(~f.disabled, 100);
		IF f.rider # NIL THEN
			HandleMouse(f.i, x DIV f.unit, y DIV f.unit, buttons)
		END
	END MouseDown;

	PROCEDURE (f: CheckBox) KeyDown (ch: CHAR);
	BEGIN
		ASSERT(~f.disabled, 100);
		IF ch = " " THEN
			Gtk.gtk_button_clicked(f.checkButton)
		END
	END KeyDown;

	PROCEDURE (f: CheckBox) Mark (on, focus: BOOLEAN);
	BEGIN
		Mark(on, f.front, f.i)
	END Mark;



	(* RadioButton *)

	PROCEDURE [ccall] RadioToggled (radioButton: Gtk.GtkWidget; user_data: INTEGER);
		VAR f: RadioButton;
	BEGIN
		f := SYSTEM.VAL(RadioButton, user_data);
		IF ~f.disabled THEN
			(* f.Set(f, Gtk.gtk_radio_button_get_active(f.radioButton) # 0) *)
		END
	END RadioToggled;
(*
	PROCEDURE (f: RadioButton) SetOffset (x, y: INTEGER);
	BEGIN
		f.SetOffset^(x, y);
		(*Adapt(f, f.i)*)
	END SetOffset;
*)
	PROCEDURE (c: RadioButton) Close;
	BEGIN
		IF c.radioButton # NIL THEN
			Gtk.gtk_widget_unref(c.lbl);
			Gtk.gtk_widget_unref(c.radioButton);
			Gtk.gtk_widget_unref(c.ebox);
			Gtk.gtk_container_remove(c.rider(HostPorts.Rider).port.fixed, c.ebox)
		END;
		c.i := NIL; c.lbl := NIL; c.radioButton := NIL; c.ebox := NIL
	END Close;

	PROCEDURE (c: RadioButton) Update;
		VAR res: INTEGER; value, mixed: BOOLEAN; mask: Gdk.GdkEventMask;
	BEGIN
		IF ~c.disabled THEN
			c.Get(c, value);
(*			IF USER32.IsWindowEnabled(f.i.ctrl) = 0 THEN res := USER32.EnableWindow(f.i.ctrl, 1) END;*)
			IF c.undef THEN
(*				res := USER32.SendMessageA(f.i.ctrl, USER32.BMSetCheck, 2, 0) TODO: Here what? *)
			ELSIF value THEN Gtk.gtk_toggle_button_set_active(c.radioButton, 1)
			ELSE Gtk.gtk_toggle_button_set_active(c.radioButton, 0)
			END;

			IF c.readOnly THEN Gtk.gtk_widget_set_sensitive(c.radioButton, off)
			ELSE Gtk.gtk_widget_set_sensitive(c.radioButton, on)
			END
		ELSE
			Gtk.gtk_toggle_button_set_active(c.radioButton, 0);
			Gtk.gtk_widget_set_sensitive(c.radioButton, off)
		END;
(*		CheckLabel(f.label, f.i.ctrl);*)
		UpdateEventMask(c.i);
	(*
		IF c.lbl.window # NIL THEN
			mask := Gdk.gdk_window_get_events(c.lbl.window);
			mask := mask - Gdk.GDK_BUTTON_PRESS_MASK;
			Gdk.gdk_window_set_events(c.lbl.window, mask)
		END
	*)
	END Update;

	PROCEDURE (c: RadioButton) Restore (l, t, r, b: INTEGER);
		VAR w, h, cx, cy, cw, ch, res: INTEGER;
	BEGIN
		IF c.radioButton = NIL THEN
			c.noRedraw := TRUE;
			c.rider(HostPorts.Rider).GetRect(cx, cy, cw, ch); cw := cw - cx; ch := ch - cy;
			c.lbl := NewLabel(c.label, NewStyle(c.font));
			c.radioButton := Gtk.gtk_radio_button_new(NIL);
			Gtk.gtk_widget_ref(c.radioButton);
			Gtk.gtk_container_add(c.radioButton, c.lbl);
			res := GtkU.gtk_signal_connect(c.radioButton, "toggled", SYSTEM.ADR(RadioToggled), SYSTEM.ADR(c));
			c.ebox := Gtk.gtk_event_box_new(); 	Gtk.gtk_widget_ref(c.ebox);
			c.i := NewInfo(c, c.ebox, c.radioButton);
			Gtk.gtk_widget_set_usize(c.ebox, cw, ch);
			Gtk.gtk_container_add(c.ebox, c.radioButton);
			Gtk.gtk_fixed_put(c.rider(HostPorts.Rider).port.fixed, c.ebox, cx, cy)
		END;
		c.Update;
		Paint(c.i)
	END Restore;

	PROCEDURE (f: RadioButton) MouseDown (x, y: INTEGER; buttons: SET);
	BEGIN
		ASSERT(~f.disabled, 100);
		IF f.rider # NIL THEN
			HandleMouse(f.i, x DIV f.unit, y DIV f.unit, buttons)
		END
	END MouseDown;

	PROCEDURE (f: RadioButton) KeyDown (ch: CHAR);
	BEGIN
		ASSERT(~f.disabled, 100);
		IF ch = " "  THEN
			Gtk.gtk_button_clicked(f.radioButton)
		END
	END KeyDown;

	PROCEDURE (f: RadioButton) Mark (on, focus: BOOLEAN);
	BEGIN
		Mark(on, f.front, f.i)
	END Mark;



	(* ScrollBar *)

	PROCEDURE [ccall] ScrollChanged (adjustment: Gtk.GtkAdjustment; user_data: INTEGER);
		VAR c: ScrollBar; size, sect, pos: INTEGER;
	BEGIN
		c := SYSTEM.VAL(ScrollBar, user_data);
		IF ~c.isUpdate THEN
			Views.ValidateRoot(Views.RootOf(c));
			c.Get(c, size, sect, pos);
			IF adjustment.value = c.oldPos - lineInc THEN
				c.Track(c, StdCFrames.lineUp, pos)
			ELSIF adjustment.value = c.oldPos + lineInc THEN
				c.Track(c, StdCFrames.lineDown, pos)
			ELSIF adjustment.value = c.oldPos - pageInc THEN
				c.Track(c, StdCFrames.pageUp, pos)
			ELSIF adjustment.value = c.oldPos + pageInc THEN
				c.Track(c, StdCFrames.pageDown, pos)
			ELSE
				c.Set(c, SHORT(ENTIER((size / scrollRange) * adjustment.value)))
			END;
			c.oldPos := adjustment.value
		END
	END ScrollChanged;
(*
	PROCEDURE (f: ScrollBar) SetOffset (x, y: INTEGER);
	BEGIN
		f.SetOffset^(x, y);
		(*Adapt(f, f.i)*)
	END SetOffset;
*)
	PROCEDURE (c: ScrollBar) Close;
	BEGIN
		IF c.scrollBar # NIL THEN
			Gtk.gtk_widget_unref(c.scrollBar);
			Gtk.gtk_container_remove(c.rider(HostPorts.Rider).port.fixed, c.scrollBar)
		END;
		c.scrollBar := NIL; c.i := NIL
	END Close;

	PROCEDURE (c: ScrollBar) Update;
		VAR  size, sect, pos, q, m: INTEGER; adj: Gtk.GtkAdjustment; trans: REAL;
	BEGIN
		IF ~c.disabled THEN
			c.Get(c, size, sect, pos);
			IF sect > size THEN sect := size END; (* c.Get does not fullfill the invariants from Controllers.PollSectionMsg *)
			ASSERT(size >= 1, 100); ASSERT((sect >= 0) & (sect <= size), 101);  (* Invariants from Controllers.PollSectionMsg *)
			ASSERT((pos >= 0) & (pos <= size - sect), 102);
			IF size > sect THEN
				Gtk.gtk_widget_set_sensitive(c.scrollBar, on);
				adj := Gtk.gtk_range_get_adjustment(c.scrollBar);
				trans := scrollRange / (size - sect);
				adj.value := SHORT(ENTIER(pos * trans));
				IF sect > 0 THEN
					adj.page_size := SHORT(ENTIER(sect * trans))
				ELSE
					adj.page_size := defThumbSize
				END;
				adj.lower := 0;
				adj.upper := scrollRange + adj.page_size;
				c.isUpdate := TRUE;
				Gtk.gtk_adjustment_changed(adj);
				c.oldPos := adj.value;
				c.isUpdate := FALSE
			ELSE
				Gtk.gtk_widget_set_sensitive(c.scrollBar, off)
			END
		ELSE
			Gtk.gtk_widget_set_sensitive(c.scrollBar, off)
		END;
		Gtk.gtk_widget_show_all(c.scrollBar)
	END Update;

	PROCEDURE (c: ScrollBar) Restore (l, t, r, b: INTEGER);
		VAR w, h, cx, cy, cw, ch, res: INTEGER; lbl: Gtk.GtkWidget; adj: Gtk.GtkAdjustment;
	BEGIN
		IF c.scrollBar = NIL THEN
			c.noRedraw := TRUE;
			c.rider(HostPorts.Rider).GetRect(cx, cy, cw, ch); cw := cw - cx; ch := ch - cy;

			adj := Gtk.gtk_adjustment_new(0, 0, scrollRange, lineInc, pageInc, defThumbSize);
			res := GtkU.gtk_signal_connect(adj, "value_changed", SYSTEM.ADR(ScrollChanged),SYSTEM.VAL(INTEGER, c));
			c.view.context.GetSize(w, h);
			IF h > w THEN
				c.scrollBar := Gtk.gtk_vscrollbar_new(adj)
			ELSE
				c.scrollBar := Gtk.gtk_hscrollbar_new(adj)
			END;

			Gtk.gtk_widget_ref(c.scrollBar);
			c.i := NewInfo(c, c.scrollBar, c.scrollBar);
			Gtk.gtk_widget_set_usize(c.scrollBar, cw, ch);
			Gtk.gtk_fixed_put(c.rider(HostPorts.Rider).port.fixed, c.scrollBar, cx, cy);
		END;
		c.Update;
		Paint(c.i)
	END Restore;

	PROCEDURE (f: ScrollBar) MouseDown (x, y: INTEGER; buttons: SET);
	BEGIN
		ASSERT(~f.disabled, 100);
		IF f.rider # NIL THEN
			HandleMouse(f.i, x DIV f.unit, y DIV f.unit, buttons)
		END
	END MouseDown;

	PROCEDURE (f: ScrollBar) KeyDown (ch: CHAR);
	BEGIN
		ASSERT(~f.disabled, 100);
		HandleKey(f.i, ch)
	END KeyDown;

	PROCEDURE (f: ScrollBar) Mark (on, focus: BOOLEAN);
	BEGIN
		Mark(on, f.front, f.i)
	END Mark;



	(* Field *)

	PROCEDURE [ccall] TextChanged (editable: Gtk.GtkWidget; user_data: INTEGER);
		VAR s: Gtk.PString; c: Field;
	BEGIN
		c := SYSTEM.VAL(Field, user_data);
		ASSERT(c # NIL, 20);
		IF ~c.isUpdate THEN
			s := Gtk.gtk_editable_get_chars(editable, 0, -1);
			IF s= NIL THEN
				c.Set(c, "");
			ELSE
				c.Set(c, s$);
				GLib.g_free(SYSTEM.VAL(GLib.gpointer, s))
			END
		END
	END TextChanged;
(*
	PROCEDURE (f: Field) SetOffset (x, y: INTEGER);
	BEGIN
		f.SetOffset^(x, y);
		(*Adapt(f, f.i)*)
	END SetOffset;
*)
	PROCEDURE (c: Field) Close;
	BEGIN
		IF c.text # NIL THEN
			Gtk.gtk_widget_unref(c.text);
			IF c.scrlw # NIL THEN Gtk.gtk_widget_unref(c.scrlw) END;
			Gtk.gtk_container_remove(c.rider(HostPorts.Rider).port.fixed, c.i.mainWidget)
		END;
		c.i := NIL; c.text := NIL; c.scrlw := NIL
	END Close;

	PROCEDURE InsLF (VAR x: ARRAY OF CHAR);
		VAR i, j: INTEGER;
	BEGIN
		i := 0; j := 0;
		WHILE x[i] # 0X DO
			IF x[i] = 0DX THEN INC(j) END;
			INC(i); INC(j)
		END;
		x[j] := 0X;
		WHILE i # j DO
			DEC(i); DEC(j);
			IF x[i] = 0DX THEN x[j] := 0AX; DEC(j) END;
			x[j] := x[i]
		END
	END InsLF;

	PROCEDURE DelLF (VAR x: ARRAY OF CHAR);
		VAR i, j: INTEGER;
	BEGIN
		i := 0; j := 0;
		WHILE x[i] # 0X DO
			IF x[i] = 0AX THEN INC(i) END;
			x[j] := x[i]; INC(i); INC(j)
		END;
		x[j] := 0X
	END DelLF;

	PROCEDURE Equal (f: Field; VAR x, y: ARRAY OF CHAR): BOOLEAN;
	BEGIN
		DelLF(y);
		RETURN f.Equal(f, x, y)
	END Equal;

	PROCEDURE (f: Field) Update;
		VAR pos: INTEGER; style: SET;
			s, s1: ARRAY 512 OF CHAR;
			ps, ps1: POINTER TO ARRAY OF CHAR;
			pss: POINTER TO ARRAY OF SHORTCHAR;
			us: GLib.PString;
			pstr: Gtk.PString;
	BEGIN
		ASSERT(f.text # NIL, 20);
		IF f.maxLen < 256 THEN
			IF f.undef OR f.disabled THEN s := "" ELSE f.Get(f, s) END;
			pstr :=  Gtk.gtk_editable_get_chars(f.text, 0, -1);
			IF pstr= NIL THEN
				s1 := ""
			ELSE
				s1 := pstr$; (* FIXME: UTF-8 -> UCS-2 ? *)
				GLib.g_free(SYSTEM.VAL(GLib.gpointer, pstr));
			END;

			IF (f.TextLen(f.text) >= LEN(s)) OR ~Equal(f, s, s1) THEN
				f.isUpdate := TRUE;
				IF f.multiLine THEN InsLF(s) END;
				us := GLib.g_utf16_to_utf8(s, -1, NIL, NIL, NIL);
				Gtk.gtk_editable_delete_text(f.text, 0, -1);
				pos := 0;
				Gtk.gtk_editable_insert_text(f.text, us, LEN(us$), pos);
				GLib.g_free(SYSTEM.VAL(GLib.gpointer, us));
				f.isUpdate := FALSE
			END
		ELSE
			NEW(ps, 2 * f.maxLen + 1);
			NEW(ps1, 2 * f.maxLen + 1);
			NEW(pss, 2 * f.maxLen + 1);

			IF f.undef OR f.disabled THEN ps^ := "" ELSE f.Get(f, ps^) END;

			pstr :=  Gtk.gtk_editable_get_chars(f.text, 0, -1);
			IF pstr= NIL THEN
				pss^ := "";
			ELSE
				pss^ := pstr$;
				GLib.g_free(SYSTEM.VAL(GLib.gpointer, pstr));
			END;

			ps1^ := pss^$;
			IF (f.TextLen(f.text) >= LEN(ps^)) OR ~Equal(f, ps^, ps1^) THEN
				f.isUpdate := TRUE;
				IF f.multiLine THEN InsLF(ps^) END;
				us := GLib.g_utf16_to_utf8(ps, -1, NIL, NIL, NIL);
				Gtk.gtk_editable_delete_text(f.text, 0, -1);
				pos := 0;
				Gtk.gtk_editable_insert_text(f.text, us, LEN(us$), pos);
				GLib.g_free(SYSTEM.VAL(GLib.gpointer, us));
				f.isUpdate := FALSE
			END
		END;
		IF (~f.readOnly & ~f.undef) THEN
			Gtk.gtk_editable_set_editable(f.text, on);
		ELSE
			Gtk.gtk_editable_set_editable(f.text, off);
		END;
		IF (~f.disabled & ~f.readOnly) THEN
			Gtk.gtk_widget_set_sensitive(f.text, on);
		ELSE
			Gtk.gtk_widget_set_sensitive(f.text, off);
		END;
		Gtk.gtk_widget_show_all(f.i.mainWidget);
		UpdateEventMask(f.i);
	END Update;

	PROCEDURE (c: Field) Restore (l, t, r, b: INTEGER);
		VAR w, h, cx, cy, cw, ch, res: INTEGER;
	BEGIN
		IF c.text = NIL THEN
			c.noRedraw := TRUE;
			c.view.context.GetSize(w, h);
			IF c.multiLine OR (h > dropDownHeight) THEN
				c.TextLen := Gtk.gtk_text_get_length;
				c.text := Gtk.gtk_text_new(NIL, NIL);
				c.scrlw := Gtk.gtk_scrolled_window_new(NIL, NIL); Gtk.gtk_widget_ref(c.scrlw);
				Gtk.gtk_container_add(c.scrlw, c.text);
				c.i := NewInfo(c, c.scrlw, c.text);
				Gtk.gtk_text_set_line_wrap(c.text, 1);
				IF c.multiLine THEN
					Gtk.gtk_scrolled_window_set_policy(c.scrlw,Gtk.GTK_POLICY_NEVER, Gtk.GTK_POLICY_ALWAYS)
				ELSE
					Gtk.gtk_scrolled_window_set_policy(c.scrlw,Gtk.GTK_POLICY_NEVER, Gtk.GTK_POLICY_NEVER)
				END
			ELSE
				(* Use a GtkEntry instead of a GtkText. No GtkScrolledWindow is needed. *)
				c.TextLen := EditableTextLen;
				c.text := Gtk.gtk_entry_new();
				c.scrlw := NIL;
				c.i := NewInfo(c, c.text, c.text);
			END;
			Gtk.gtk_widget_ref(c.text);
			Gtk.gtk_widget_modify_style(c.text, NewStyle(c.font));
			res := GtkU.gtk_signal_connect(c.text, "changed",SYSTEM.ADR(TextChanged), SYSTEM.VAL(INTEGER, c));
			c.rider(HostPorts.Rider).GetRect(cx, cy, cw, ch); cw := cw - cx; ch := ch - cy;
			Gtk.gtk_widget_set_usize(c.i.mainWidget, cw, ch);
			Gtk.gtk_fixed_put(c.rider(HostPorts.Rider).port.fixed, c.i.mainWidget, cx, cy);
		END;
		c.Update;
		Paint(c.i)
	END Restore;

	PROCEDURE (f: Field) MouseDown (x, y: INTEGER; buttons: SET);
	BEGIN
		ASSERT(~f.disabled, 100);
		IF f.rider # NIL THEN
			HandleMouse(f.i, x DIV f.unit, y DIV f.unit, buttons)
		END
	END MouseDown;

	PROCEDURE (f: Field) KeyDown (ch: CHAR);
	BEGIN
		ASSERT(~f.disabled, 100);
		IF f.multiLine OR (ch # 0DX) THEN
			HandleKey(f.i, ch)
		END
	END KeyDown;

	PROCEDURE (f: Field) Edit (op: INTEGER; VAR v: Views.View; VAR w, h: INTEGER;
										VAR singleton, clipboard: BOOLEAN);
	BEGIN
	END Edit;

	PROCEDURE (f: Field) Idle, EMPTY;

	PROCEDURE (f: Field) Select (from, to: INTEGER);
	BEGIN
		IF f.text # NIL THEN
			IF to = MAX(INTEGER) THEN to := -1 END;
			Gtk.gtk_editable_select_region(f.text, from, to)
		END
	END Select;

	PROCEDURE (f: Field) GetSelection (OUT from, to: INTEGER);
		VAR e: Gtk.GtkOldEditable;
	BEGIN
		(*
		e := f.text(Gtk.GtkOldEditable);
		IF 0 IN e.has_selection (* testing bit field *) THEN
			from := e.selection_start_pos; to := e.selection_end_pos
		ELSE
		*)
			from := -1; to := -1
		(*
			from := -1; to := -1;
		END
		*)
	END GetSelection;

	PROCEDURE (f: Field) Mark (on, focus: BOOLEAN);
	BEGIN
		Mark(on, f.front, f.i)
	END Mark;

	PROCEDURE (f: Field) Length (): INTEGER;
	BEGIN
		RETURN f.TextLen(f.text)
	END Length;

(*
	PROCEDURE (f: Field) GetCursor (x, y: INTEGER; modifiers: SET; VAR cursor: INTEGER);
	BEGIN
		(* TODO *)
	END GetCursor;
*)


	(* UpDownField *)

	PROCEDURE [ccall] SpinChanged (spin: Gtk.GtkWidget; user_data: INTEGER);
		VAR s: Gtk.PString; c: UpDownField;
	BEGIN
		c := SYSTEM.VAL(UpDownField, user_data);
		IF ~c.isUpdate THEN
			c.val := Gtk.gtk_spin_button_get_value_as_int(spin);
			c.Set(c, c.val)
		END
	END SpinChanged;
(*
	PROCEDURE (f: UpDownField) SetOffset (x, y: INTEGER);
	BEGIN
		f.SetOffset^(x, y);
		(*Adapt(f, f.i)*)
	END SetOffset;
*)
	PROCEDURE (c: UpDownField) Close;
	BEGIN
		IF c.spin # NIL THEN
			Gtk.gtk_widget_unref(c.spin);
			Gtk.gtk_container_remove(c.rider(HostPorts.Rider).port.fixed, c.spin)
		END;
		c.spin := NIL; c.i := NIL
	END Close;

	PROCEDURE (f: UpDownField) Update;
		VAR val: INTEGER;
	BEGIN
		f.isUpdate := TRUE;
		IF ~f.disabled THEN
			f.Get(f, val);
			Gtk.gtk_spin_button_set_value(f.spin, val);
			IF ~f.readOnly THEN
				Gtk.gtk_widget_set_sensitive(f.spin, on)
			ELSE
				Gtk.gtk_widget_set_sensitive(f.spin, off)
			END
		ELSE
			Gtk.gtk_entry_set_text(f.spin, "");
			Gtk.gtk_widget_set_sensitive(f.spin, off)
		END;
		f.isUpdate := FALSE
		;UpdateEventMask (f.i)
	END Update;

	PROCEDURE (c: UpDownField) Restore (l, t, r, b: INTEGER);
		VAR w, h, cx, cy, cw, ch, res: INTEGER; lbl: Gtk.GtkWidget; adj: Gtk.GtkAdjustment;
	BEGIN
		IF c.spin = NIL THEN
			c.noRedraw := TRUE;
			c.val := 0;
			c.rider(HostPorts.Rider).GetRect(cx, cy, cw, ch); cw := cw - cx; ch := ch - cy;
			adj := Gtk.gtk_adjustment_new(c.val, c.min, c.max, c.inc,
				c.inc, (c.max - c.min) DIV MAX(1, c.inc)); (* waste of pgup & pgdn *)
			c.spin := Gtk.gtk_spin_button_new(adj, 1, 0);			Gtk.gtk_widget_ref(c.spin);
			Gtk.gtk_widget_modify_style(c.spin, NewStyle(c.font));
			Gtk.gtk_spin_button_set_numeric(c.spin, 1);
(*		Gtk.gtk_spin_button_set_shadow_type(c.spin, Gtk.GTK_SHADOW_NONE); *)
(*		Gtk.gtk_spin_button_set_snap_to_ticks(c.spin, 1);*)
			Gtk.gtk_spin_button_set_wrap(c.spin, 1);
			res := GtkU.gtk_signal_connect(c.spin, "changed", SYSTEM.ADR(SpinChanged), SYSTEM.VAL(INTEGER, c));
			c.i := NewInfo(c, c.spin, c.spin);
			Gtk.gtk_widget_set_usize(c.spin, cw, ch);
			Gtk.gtk_fixed_put(c.rider(HostPorts.Rider).port.fixed, c.spin, cx, cy)
		END;
		c.Update;
		Paint(c.i)
	END Restore;

	PROCEDURE (f: UpDownField) MouseDown (x, y: INTEGER; buttons: SET);
	BEGIN
		ASSERT(~f.disabled, 100);
		IF f.rider # NIL THEN
			HandleMouse(f.i, x DIV f.unit, y DIV f.unit, buttons)
		END
	END MouseDown;

	PROCEDURE (f: UpDownField) KeyDown (ch: CHAR);
	BEGIN
		ASSERT(~f.disabled, 100);
		HandleKey(f.i, ch)
	END KeyDown;

(*
	PROCEDURE (f: UpDownField) Edit (op: INTEGER; VAR v: Views.View; VAR w, h: INTEGER;
										VAR singleton, clipboard: BOOLEAN);
	BEGIN
		(* TODO *)
	END Edit;
*)

	PROCEDURE (f: UpDownField) Idle, EMPTY;

	PROCEDURE (f: UpDownField) Select (from, to: INTEGER);
	BEGIN
		IF to = MAX(INTEGER) THEN to := -1 END;
		Gtk.gtk_editable_select_region(f.spin, from, to)
	END Select;

	PROCEDURE (f: UpDownField) GetSelection (OUT from, to: INTEGER);
		VAR e: Gtk.GtkOldEditable;
	BEGIN
		(*
		e := f.spin(Gtk.GtkOldEditable);
		IF 0 IN e.has_selection (* testing bit field *) THEN
			from := e.selection_start_pos; to := e.selection_end_pos
		ELSE
		*)
			from := -1; to := -1
		(*
		END
		*)
	END GetSelection;

	PROCEDURE (f: UpDownField) Mark (on, focus: BOOLEAN);
	BEGIN
		Mark(on, f.front, f.i)
	END Mark;

(*
	PROCEDURE (f: UpDownField) GetCursor (x, y: INTEGER; modifiers: SET; VAR cursor: INTEGER);
	BEGIN
		(* TODO *)
	END GetCursor;
*)


	(* TimeField *)

(*
	PROCEDURE GetTimePart (VAR time: Dates.Time; part: INTEGER; VAR val, min, max: INTEGER);
	BEGIN
		IF part = -1 THEN part := lastPart END;
		IF part = 4 THEN val := time.hour DIV 12; min := 0; max := 1
		ELSIF part = 3 THEN val := time.second; min := 0; max := 59
		ELSIF part = 2 THEN val := time.minute; min := 0; max := 59
		ELSIF lastPart = 3 THEN val := time.hour; min := 0; max := 23
		ELSE val := (time.hour - 1) MOD 12 + 1; min := 1; max := 12
		END
	END GetTimePart;

	PROCEDURE SetTimePart (VAR time: Dates.Time; part: INTEGER; val: INTEGER);
	BEGIN
		IF part = -1 THEN part := lastPart END;
		IF part = 4 THEN time.hour := val * 12 + time.hour MOD 12
		ELSIF part = 3 THEN time.second := val
		ELSIF part = 2 THEN time.minute := val
		ELSIF lastPart = 3 THEN time.hour := val
		ELSE time.hour := time.hour DIV 12 * 12 + val MOD 12
		END
	END SetTimePart;

	PROCEDURE TimeToString (VAR time: Dates.Time; VAR str: ARRAY OF CHAR);
		VAR val, min, max, i, j, k: INTEGER;
	BEGIN
		GetTimePart(time, 1, val, min, max);
		str[0] := CHR(val DIV 10 + ORD("0"));
		str[1] := CHR(val MOD 10 + ORD("0"));
		str[2] := timeSep;
		GetTimePart(time, 2, val, min, max);
		str[3] := CHR(val DIV 10 + ORD("0"));
		str[4] := CHR(val MOD 10 + ORD("0"));
		str[5] := timeSep;
		GetTimePart(time, 3, val, min, max);
		str[6] := CHR(val DIV 10 + ORD("0"));
		str[7] := CHR(val MOD 10 + ORD("0"));
		IF lastPart = 3 THEN
			str[8] := 0X
		ELSE
			str[8] := " ";
			i := 9; j := 0; k := time.hour DIV 12;
			WHILE desig[k, j] # 0X DO str[i] := desig[k, j]; INC(i); INC(j) END;
			str[i] := 0X
		END
	END TimeToString;
*)
(*
	PROCEDURE (f: TimeField) Select, NEW;
	BEGIN
		(* TODO *)
	END Select;
*)
	PROCEDURE (f: TimeField) SetOffset (x, y: INTEGER);
	BEGIN
		(*
		f.SetOffset^(x, y);
		(*Adapt(f, f.i)*)
		*)
	END SetOffset;

	PROCEDURE (f: TimeField) Close;
	BEGIN
		(* TODO *)
	END Close;

	PROCEDURE (f: TimeField) Update;
	BEGIN
		(* TODO *)
	END Update;

	PROCEDURE (f: TimeField) Restore (l, t, r, b: INTEGER);
	BEGIN
		DummyRestore(f, "TimeField", l, t, r, b)
	END Restore;

	PROCEDURE (f: TimeField) MouseDown (x, y: INTEGER; buttons: SET);
		VAR res: INTEGER; sel: INTEGER;
	BEGIN
		(* TODO *)
	END MouseDown;

	PROCEDURE (f: TimeField) KeyDown (ch: CHAR);
	BEGIN
		(* TODO *)
	END KeyDown;

	PROCEDURE (f: TimeField) Edit (op: INTEGER; VAR v: Views.View; VAR w, h: INTEGER;
										VAR singleton, clipboard: BOOLEAN);
	BEGIN
		(* TODO *)
	END Edit;

	PROCEDURE (f: TimeField) Mark (on, focus: BOOLEAN);
	BEGIN
		(* TODO *)
	END Mark;

	PROCEDURE (f: TimeField) GetCursor (x, y: INTEGER; modifiers: SET; VAR cursor: INTEGER);
	BEGIN
		(* TODO *)
	END GetCursor;



	(* DateField *)

	PROCEDURE GetDatePart (VAR date: Dates.Date; part: INTEGER; OUT val, min, max: INTEGER);
	(* GetDatePart picks the day, month or year part out of a given date and asigns it to the out
		parameter val, together with the min and max possible values for this part
	*)
	BEGIN
		IF part = -1 THEN part := 3 END;
		IF part = yearPart THEN val := date.year; min := 1; max := 9999
		ELSIF part = monthPart THEN val := date.month; min := 1; max := 12
		ELSE
			val := date.day; min := 1;
			IF date.month = 0 THEN
				max := 31
			ELSIF date.month = 2 THEN
				IF (date.year MOD 4 = 0)
					& ((date.year < 1583) OR (date.year MOD 100 # 0) OR (date.year MOD 400 = 0))
				THEN
					max := 29
				ELSE max := 28
				END
			ELSIF date.month IN {1, 3, 5, 7, 8, 10, 12} THEN max := 31
			ELSE max := 30
			END
		END
	END GetDatePart;

	PROCEDURE SetDatePart (VAR date: Dates.Date; part: INTEGER; val: INTEGER);
	(* SetDatePart sets the day, month or year part in a given date to the value specivied
		by the parameter val.
		If the month is set, the day is adjusted to the possible range of days in this month.
		If the day is set, then the month may be changed in order to obtain a valid date
	*)
		VAR v, min, max: INTEGER;
	BEGIN
		IF part = -1 THEN part := 3 END;
		IF part = yearPart THEN date.year := val
		ELSIF part = monthPart THEN date.month := val
		ELSE date.day := val
		END;
		GetDatePart(date, dayPart, v, min, max);
		IF (part = monthPart) THEN (* adjust day if month value is set and day > max *)
			IF v > max THEN date.day := max END
		ELSIF part = yearPart THEN (* adjust month is day value is set and day > max *)
			IF v > max THEN INC(date.month) END
		END
	END SetDatePart;

	PROCEDURE DateToString (VAR date: Dates.Date; VAR str: ARRAY OF CHAR);
		VAR val, min, max, p, i: INTEGER;
	BEGIN
		p := 1; i := 0;
		WHILE p <= 3 DO
			IF p > 1 THEN str[i] := dateSep; INC(i) END;
			GetDatePart(date, p, val, min, max);
			IF max = 9999 THEN
				str[i] := CHR(val DIV 1000 MOD 10 + ORD("0")); INC(i);
				str[i] := CHR(val DIV 100 MOD 10 + ORD("0")); INC(i)
			END;
			str[i] := CHR(val DIV 10 MOD 10 + ORD("0")); INC(i);
			str[i] := CHR(val MOD 10 + ORD("0")); INC(i);
			INC(p)
		END;
		str[i] := 0X
	END DateToString;

	PROCEDURE (f: DateField) Select, NEW;
	BEGIN
		(* TODO *)
	END Select;

	PROCEDURE (f: DateField) SetOffset (x, y: INTEGER);
	BEGIN
		(*
		f.SetOffset^(x, y);
		(*Adapt(f, f.i)*)
		*)
	END SetOffset;

	PROCEDURE (f: DateField) Close;
	BEGIN
		(* TODO *)
	END Close;

	PROCEDURE (f: DateField) Update;
	BEGIN
	END Update;

	PROCEDURE (f: DateField) Restore (l, t, r, b: INTEGER);
	BEGIN
		DummyRestore(f, "DateField", l, t, r, b)
	END Restore;

	PROCEDURE (f: DateField) MouseDown (x, y: INTEGER; buttons: SET);
	BEGIN
		(* TODO *)
	END MouseDown;

	PROCEDURE (f: DateField) KeyDown (ch: CHAR);
	BEGIN
		(* TODO *)
	END KeyDown;

	PROCEDURE (f: DateField) Edit (op: INTEGER; VAR v: Views.View; VAR w, h: INTEGER;
										VAR singleton, clipboard: BOOLEAN);
	BEGIN
		(* TODO *)
	END Edit;

	PROCEDURE (f: DateField) Mark (on, focus: BOOLEAN);
	BEGIN
		(* TODO *)
	END Mark;

	PROCEDURE (f: DateField) GetCursor (x, y: INTEGER; modifiers: SET; VAR cursor: INTEGER);
	BEGIN
		(* TODO *)
	END GetCursor;



	(* ColorField *)

	PROCEDURE (f: ColorField) SetOffset (x, y: INTEGER);
	BEGIN
		(*
		f.SetOffset^(x, y);
		(*Adapt(f, f.i)*)
		*)
	END SetOffset;

	PROCEDURE (f: ColorField) Close;
	BEGIN
		(* TODO *)
	END Close;

	PROCEDURE (f: ColorField) Update;
	BEGIN
		(* TODO *)
	END Update;

	PROCEDURE (f: ColorField) Restore (l, t, r, b: INTEGER);
	BEGIN
		DummyRestore(f, "ColorField", l, t, r, b)
	END Restore;

	PROCEDURE (f: ColorField) MouseDown (x, y: INTEGER; buttons: SET);
	BEGIN
		(* TODO *)
	END MouseDown;

	PROCEDURE (f: ColorField) KeyDown (ch: CHAR);
	BEGIN
		(* TODO *)
	END KeyDown;

	PROCEDURE (f: ColorField) Mark (on, focus: BOOLEAN);
	BEGIN

	END Mark;



	(* ListBox *)

	PROCEDURE [ccall] ListSelect (clist: Gtk.GtkCList; row, column: INTEGER; event: Gdk.GdkEventButton; user_data: INTEGER);
		VAR f: ListBox; cur: INTEGER;
	BEGIN
		f := SYSTEM.VAL(ListBox, user_data);
		IF f.list # NIL THEN
			f.Get(f, cur);
			row := Gtk.gtk_clist_get_row_data(clist, row);
			IF row # cur THEN f.Set(f, row) END
		END
	END ListSelect;

	PROCEDURE (f: ListBox) SetOffset (x, y: INTEGER);
	BEGIN
		f.SetOffset^(x, y);
		(*Adapt(f, f.i)*)
	END SetOffset;

	PROCEDURE (c: ListBox) Close;
	BEGIN
		IF c.scrlw # NIL THEN
			Gtk.gtk_widget_unref(c.list);
			Gtk.gtk_widget_unref(c.scrlw);
			Gtk.gtk_container_remove(c.rider(HostPorts.Rider).port.fixed, c.scrlw);
		END;
		c.i := NIL; c.list := NIL; c.scrlw := NIL
	END Close;

	PROCEDURE (f: ListBox) Update;
		VAR i, r: INTEGER;
	BEGIN
		IF f.disabled OR f.readOnly THEN
			Gtk.gtk_widget_set_sensitive(f.list, off);
			IF f.disabled THEN
				Gtk.gtk_clist_unselect_all(f.list)
			END
		ELSE
			Gtk.gtk_widget_set_sensitive(f.list, on)
		END;
		IF ~f.disabled THEN
			IF f.undef THEN i := -1 ELSE f.Get(f, i) END;
			IF i >= 0 THEN
				r := Gtk.gtk_clist_find_row_from_data(f.list, i);
				IF r >= 0 THEN
					Gtk.gtk_clist_select_row(f.list, r, 0)
				END
			END;
			Gtk.gtk_widget_set_sensitive(f.scrlw, on)
		ELSE
			Gtk.gtk_widget_set_sensitive(f.scrlw, off)
		END;
		Gtk.gtk_widget_show_all(f.scrlw)
	END Update;

	PROCEDURE (c: ListBox) UpdateList;
		VAR  i, res: INTEGER; s: Dialog.String; li: Gtk.GtkWidget;
			ss: ARRAY (LEN(Dialog.String) - 1) * Utf8.maxShortCharsPerChar + 1 OF SHORTCHAR; sa: ARRAY [untagged] 1 OF Gtk.PString;
	BEGIN
		ASSERT(c.list # NIL, 20);
		(* make sure that no selection changes are propagated when rebuilding the list *)
		GtkU.gtk_signal_handler_block_by_func(c.list,
				SYSTEM.ADR(ListSelect),
				SYSTEM.VAL(INTEGER, c));
		Gtk.gtk_clist_unselect_all(c.list);
		Gtk.gtk_clist_clear(c.list);
		i := 0; c.GetName(c, i, s);
		WHILE s # "" DO
			Dialog.MapString(s, s);
			Utf8.Short(s, ss);
			sa[0] := ss;
			Gtk.gtk_clist_append(c.list, sa);
			Gtk.gtk_clist_set_row_data(c.list, i, i);
			INC(i); c.GetName(c, i, s)
		END;
		IF c.sorted THEN
			Gtk.gtk_clist_sort(c.list)
		END;
		GtkU.gtk_signal_handler_unblock_by_func(c.list, SYSTEM.ADR(ListSelect), SYSTEM.VAL(INTEGER, c));
		Gtk.gtk_clist_set_column_width(c.list, 0, Gtk.gtk_clist_optimal_column_width(c.list, 0));
		c.Update;
	END UpdateList;

	PROCEDURE (c: ListBox) Restore (l, t, r, b: INTEGER);
		VAR cx, cy, cw, ch, res: INTEGER;
	BEGIN
		IF c.list = NIL THEN
			c.noRedraw := TRUE;
			c.list := Gtk.gtk_clist_new(1);
			Gtk.gtk_widget_ref(c.list);
			Gtk.gtk_widget_modify_style(c.list, NewStyle(c.font));
(*			Gtk.gtk_clist_set_selection_mode(c.list, Gtk.GTK_SELECTION_BROWSE);*)
			Gtk.gtk_clist_set_selection_mode(c.list, Gtk.GTK_SELECTION_SINGLE);
			res := GtkU.gtk_signal_connect(c.list, "select-row", SYSTEM.ADR(ListSelect),SYSTEM.VAL(INTEGER, c));
			c.scrlw := Gtk.gtk_scrolled_window_new(NIL, NIL);
			Gtk.gtk_widget_ref(c.scrlw);
			c.i := NewInfo(c, c.scrlw, c.list);
			c.rider(HostPorts.Rider).GetRect(cx, cy, cw, ch); cw := cw - cx; ch := ch - cy;
			Gtk.gtk_widget_set_usize(c.scrlw, cw, ch);
			Gtk.gtk_container_add(c.scrlw, c.list);
			Gtk.gtk_scrolled_window_set_policy(c.scrlw, Gtk.GTK_POLICY_AUTOMATIC, Gtk.GTK_POLICY_AUTOMATIC);
			Gtk.gtk_fixed_put(c.rider(HostPorts.Rider).port.fixed, c.scrlw, cx, cy);
			c.UpdateList
		ELSE
			c.Update
		END;
		Paint(c.i)
	END Restore;

	PROCEDURE (f: ListBox) DblClickOk (x, y: INTEGER): BOOLEAN;
	BEGIN
		(* TODO *)
		RETURN FALSE
	END DblClickOk;

	PROCEDURE (f: ListBox) MouseDown (x, y: INTEGER; buttons: SET);
	BEGIN
		ASSERT(~f.disabled, 100);
		IF f.rider # NIL THEN
			HandleMouse(f.i, x DIV f.unit, y DIV f.unit, buttons)
		END
	END MouseDown;

	PROCEDURE (f: ListBox) WheelMove (x, y: INTEGER; op, nofLines: INTEGER; VAR done: BOOLEAN);
	BEGIN
		(* TODO *)
	END WheelMove;

	PROCEDURE (f: ListBox) KeyDown (ch: CHAR);
	BEGIN
		ASSERT(~f.disabled, 100);
		HandleKey(f.i, ch)
	END KeyDown;

	PROCEDURE (f: ListBox) Mark (on, focus: BOOLEAN);
	BEGIN
		Mark(on, f.front, f.i)
	END Mark;



	(* SelectionBox *)

	PROCEDURE [ccall] SelectionBoxSelect (clist: Gtk.GtkCList; row, column: INTEGER;
									event: Gdk.GdkEventButton; user_data: INTEGER);
		VAR f: SelectionBox; cur: INTEGER; in: BOOLEAN;
	BEGIN
		f := SYSTEM.VAL(SelectionBox, user_data);
		IF f.list # NIL THEN
			row := Gtk.gtk_clist_get_row_data(clist, row);
			f.Get(f, row, in);
			IF ~in THEN f.Incl(f, row, row) END
		END
	END SelectionBoxSelect;

	PROCEDURE [ccall] SelectionBoxUnSelect (clist: Gtk.GtkCList; row, column: INTEGER;
									event: Gdk.GdkEventButton; user_data: INTEGER);
		VAR f: SelectionBox; cur: INTEGER; in: BOOLEAN;
	BEGIN
		f := SYSTEM.VAL(SelectionBox, user_data);
		IF f.list # NIL THEN
			row := Gtk.gtk_clist_get_row_data(clist, row);
			f.Get(f, row, in);
			IF in THEN f.Excl(f, row, row)
				;Log.String("HostCFrames.SelectionBoxUnSelect: "); Log.Int(row); Log.Ln;
			END
		END
	END SelectionBoxUnSelect;

	PROCEDURE (f: SelectionBox) SetOffset (x, y: INTEGER);
	BEGIN
		f.SetOffset^(x, y);
		(*Adapt(f, f.i)*)
	END SetOffset;

	PROCEDURE (c: SelectionBox) Close;
	BEGIN
		IF c.scrlw # NIL THEN
			Gtk.gtk_widget_unref(c.list);
			Gtk.gtk_widget_unref(c.scrlw);
			Gtk.gtk_container_remove(c.rider(HostPorts.Rider).port.fixed, c.scrlw)
		END;
		c.i := NIL; c.list := NIL; c.scrlw := NIL
	END Close;

	PROCEDURE (f: SelectionBox) DblClickOk (x, y: INTEGER): BOOLEAN;
	BEGIN
		(* ... *)
		RETURN FALSE
	END DblClickOk;

	PROCEDURE (c: SelectionBox) Update;
		VAR i, r: INTEGER; in: BOOLEAN;
	BEGIN
		IF c.disabled OR c.readOnly THEN
			Gtk.gtk_widget_set_sensitive(c.list, off);
			IF c.disabled THEN
				Gtk.gtk_clist_unselect_all(c.list)
			END
		ELSE
			Gtk.gtk_widget_set_sensitive(c.list, on)
		END;
		IF ~c.disabled THEN
			IF ~c.undef THEN
				i := 0;
				WHILE i < c.num DO
					c.Get(c, i, in);
					r := Gtk.gtk_clist_find_row_from_data(c.list, i);
					IF in THEN
						Gtk.gtk_clist_select_row(c.list, r, 0)
					ELSE
						Gtk.gtk_clist_unselect_row(c.list, r, 0)
					END;
					INC(i)
				END
			END;
			Gtk.gtk_widget_set_sensitive(c.scrlw, on)
		ELSE
			Gtk.gtk_widget_set_sensitive(c.scrlw, off)
		END;
		Gtk.gtk_widget_show_all(c.scrlw)
	END Update;

	PROCEDURE (f: SelectionBox) UpdateRange (op, from, to: INTEGER);
	BEGIN
		(* TODO *)
	END UpdateRange;

	PROCEDURE (c: SelectionBox) UpdateList;
		VAR  i, res: INTEGER; s: Dialog.String; li: Gtk.GtkWidget;
			ss: ARRAY (LEN(Dialog.String) - 1) * Utf8.maxShortCharsPerChar + 1 OF SHORTCHAR;
			sa: ARRAY [untagged] 1 OF Gtk.PString;
	BEGIN
		ASSERT(c.list # NIL, 20);
		(* make sure that no selection changes are propagated when rebuilding the list *)
		GtkU.gtk_signal_handler_block_by_func(c.list, SYSTEM.ADR(SelectionBoxSelect), SYSTEM.VAL(INTEGER, c));
		Gtk.gtk_clist_unselect_all(c.list);
		Gtk.gtk_clist_clear(c.list);
		i := 0; c.GetName(c, i, s);
		WHILE s # "" DO
			Dialog.MapString(s, s);
			Utf8.Short(s, ss);
			sa[0] := ss;
			Gtk.gtk_clist_append(c.list, sa);
			Gtk.gtk_clist_set_row_data(c.list, i, i);
			INC(i); c.GetName(c, i, s)
		END;
		c.num := i;
		IF c.sorted THEN
			Gtk.gtk_clist_sort(c.list)
		END;
		GtkU.gtk_signal_handler_unblock_by_func(c.list, SYSTEM.ADR(SelectionBoxSelect), SYSTEM.VAL(INTEGER, c));
		Gtk.gtk_clist_set_column_width(c.list, 0, Gtk.gtk_clist_optimal_column_width(c.list, 0));
		c.Update
	END UpdateList;

	PROCEDURE (c: SelectionBox) Restore (l, t, r, b: INTEGER);
		VAR cx, cy, cw, ch, res: INTEGER;
	BEGIN
		IF c.list = NIL THEN
			c.noRedraw := TRUE;
			c.list := Gtk.gtk_clist_new(1); 			Gtk.gtk_widget_ref(c.list);
			Gtk.gtk_widget_modify_style(c.list, NewStyle(c.font));
			c.scrlw := Gtk.gtk_scrolled_window_new(NIL, NIL);			Gtk.gtk_widget_ref(c.scrlw);
			Gtk.gtk_clist_set_selection_mode(c.list, Gtk.GTK_SELECTION_EXTENDED);
			res := GtkU.gtk_signal_connect(c.list, "select-row",SYSTEM.ADR(SelectionBoxSelect),SYSTEM.VAL(INTEGER, c));
			res := GtkU.gtk_signal_connect(c.list, "unselect-row", SYSTEM.ADR(SelectionBoxUnSelect), SYSTEM.VAL(INTEGER, c));
			c.i := NewInfo(c, c.scrlw, c.list);
			c.rider(HostPorts.Rider).GetRect(cx, cy, cw, ch); cw := cw - cx; ch := ch - cy;
			Gtk.gtk_widget_set_usize(c.scrlw, cw, ch);
			Gtk.gtk_container_add(c.scrlw, c.list);
			Gtk.gtk_scrolled_window_set_policy(c.scrlw,
															Gtk.GTK_POLICY_AUTOMATIC, Gtk.GTK_POLICY_AUTOMATIC);
			Gtk.gtk_fixed_put(c.rider(HostPorts.Rider).port.fixed, c.scrlw, cx, cy);
			c.UpdateList
		ELSE
			c.Update;
		END;
		Paint(c.i)
	END Restore;

	PROCEDURE (f: SelectionBox) MouseDown (x, y: INTEGER; buttons: SET);
	BEGIN
		ASSERT(~f.disabled, 100);
		IF f.rider # NIL THEN
			HandleMouse(f.i, x DIV f.unit, y DIV f.unit, buttons)
		END
	END MouseDown;

	PROCEDURE (f: SelectionBox) WheelMove (x, y: INTEGER; op, nofLines: INTEGER; VAR done: BOOLEAN);
	BEGIN
		(* TODO *)
	END WheelMove;

	PROCEDURE (f: SelectionBox) KeyDown (ch: CHAR);
	BEGIN
		ASSERT(~f.disabled, 100);
		HandleKey(f.i, ch)
	END KeyDown;

	PROCEDURE (f: SelectionBox) Select (from, to: INTEGER), EMPTY;

	PROCEDURE (f: SelectionBox) GetSelection (OUT from, to: INTEGER);
	BEGIN
		from := 0; to := MAX(INTEGER)
	END GetSelection;

	PROCEDURE (f: SelectionBox) Mark (on, focus: BOOLEAN);
	BEGIN
		Mark(on, f.front, f.i)
	END Mark;



	(* ComboBox *)

	PROCEDURE [ccall] ComboChanged (editable: Gtk.GtkWidget; user_data: INTEGER);
		VAR s: Gtk.PString; c: ComboBox;
	BEGIN
		c := SYSTEM.VAL(ComboBox, user_data);
		IF ~c.isUpdate THEN
			s := Gtk.gtk_editable_get_chars(editable, 0, -1);
			IF s= NIL THEN
				c.Set(c, "");
			ELSE
				c.Set(c, s$);
				GLib.g_free(SYSTEM.VAL(GLib.gpointer, s))
			END
		END
	END ComboChanged;

	PROCEDURE (f: ComboBox) SetOffset (x, y: INTEGER);
	BEGIN
		f.SetOffset^(x, y);
		(*Adapt(f, f.i)*)
	END SetOffset;

	PROCEDURE (c: ComboBox) Close;
	BEGIN
		IF c.combo # NIL THEN
			Gtk.gtk_widget_unref(c.combo);
			Gtk.gtk_container_remove(c.rider(HostPorts.Rider).port.fixed, c.combo)
		END;
		c.i := NIL; c.combo := NIL
	END Close;

	PROCEDURE (c: ComboBox) Update;
		VAR s: Dialog.String; us: GLib.PString;
	BEGIN
		IF c.combo = NIL THEN RETURN END;
		IF c.combo.entry = NIL THEN RETURN END;
		c.isUpdate := TRUE;
		IF ~c.disabled THEN
			c.Get(c, s); Dialog.MapString(s, s);
			us := GLib.g_utf16_to_utf8(s, -1, NIL, NIL, NIL);
			Gtk.gtk_entry_set_text(c.combo.entry, us);
			GLib.g_free(SYSTEM.VAL(GLib.gpointer, us));
			IF ~c.readOnly THEN
				Gtk.gtk_widget_set_sensitive(c.combo, on)
			ELSE
				Gtk.gtk_widget_set_sensitive(c.combo, off)
			END
		ELSE
			Gtk.gtk_entry_set_text(c.combo.entry, "");
			Gtk.gtk_widget_set_sensitive(c.combo, off)
		END;
		c.isUpdate := FALSE;
		Gtk.gtk_widget_show_all(c.combo)
(*
		IF ~c.disabled THEN
			c.Get(c, value);
			IF c.undef THEN
			ELSIF value THEN Gtk.gtk_toggle_button_set_active(c.checkButton, 1)
			ELSE Gtk.gtk_toggle_button_set_active(c.checkButton, 0)
			END;
			IF c.readOnly THEN Gtk.gtk_widget_set_sensitive(c.checkButton, 0)
			ELSE Gtk.gtk_widget_set_sensitive(c.checkButton, 1)
			END
		ELSE
			Gtk.gtk_toggle_button_set_active(c.checkButton, 0);
			Gtk.gtk_widget_set_sensitive(c.checkButton, 0)
		END;
		UpdateEventMask(c.i);
		IF c.lbl.window # NIL THEN
			mask := Gdk.gdk_window_get_events(c.lbl.window);
			mask := mask - Gdk.GDK_BUTTON_PRESS_MASK;
			Gdk.gdk_window_set_events(c.lbl.window, mask)
		END
*)
	END Update;

	PROCEDURE (c: ComboBox) UpdateList;
		VAR
			strs: POINTER TO ARRAY OF ARRAY [untagged] (LEN(Dialog.String) - 1) * Utf8.maxShortCharsPerChar + 1 OF SHORTCHAR;
			gl: GLib.GList; l, i: INTEGER; s: Dialog.String;
	BEGIN
		l := 0;
		REPEAT c.GetName(c, l, s); INC(l) UNTIL s = "";
		DEC(l);
		IF l > 0 THEN
			NEW(strs, l);
			gl := NIL;
			i := 0;
			WHILE i < l DO
				c.GetName(c, i, s); Dialog.MapString(s, s);
				Utf8.Short(s, strs[i]);
				gl := GLib.g_list_append(gl, SYSTEM.VAL(GLib.gpointer, SYSTEM.ADR(strs[i])));
				INC(i);
			END;
			Gtk.gtk_combo_set_popdown_strings(c.combo, gl)
		END;
		c.Update
	END UpdateList;

	PROCEDURE (c: ComboBox) Restore (l, t, r, b: INTEGER);
		VAR w, h, cx, cy, cw, ch, res: INTEGER;
	BEGIN
		IF c.combo = NIL THEN
			c.noRedraw := TRUE;
			c.combo := Gtk.gtk_combo_new(); 	Gtk.gtk_widget_ref(c.combo);
			res := GtkU.gtk_signal_connect(c.combo.entry, "changed", SYSTEM.ADR(ComboChanged),SYSTEM.VAL(INTEGER, c));
			c.i := NewInfo(c, c.combo, c.combo);

			(* make sure all contained widgets also have the right signals connected *)
			res := GtkU.gtk_signal_connect(c.combo.entry, "button_press_event", SYSTEM.ADR(MouseHandler),SYSTEM.VAL(INTEGER, c.i));
			res := GtkU.gtk_signal_connect_after(c.combo.entry, "button-release-event", SYSTEM.ADR(MouseRelease),SYSTEM.VAL(INTEGER, c.i));
			res := GtkU.gtk_signal_connect(c.combo.entry, "key-press-event", SYSTEM.ADR(KeyHandler),SYSTEM.VAL(INTEGER, c.i));

			c.rider(HostPorts.Rider).GetRect(cx, cy, cw, ch); cw := cw - cx; ch := ch - cy;
			Gtk.gtk_widget_set_usize(c.combo, cw, ch);
			Gtk.gtk_fixed_put(c.rider(HostPorts.Rider).port.fixed, c.combo, cx, cy);
			c.UpdateList
		END;
		c.Update;
		Paint(c.i)
	END Restore;

	PROCEDURE (f: ComboBox) MouseDown (x, y: INTEGER; buttons: SET);
	BEGIN
		ASSERT(~f.disabled, 100);
		IF f.rider # NIL THEN
			HandleMouse(f.i, x DIV f.unit, y DIV f.unit, buttons)
		END
	END MouseDown;

	PROCEDURE (f: ComboBox) KeyDown (ch: CHAR);
	BEGIN
		ASSERT(~f.disabled, 100);
		HandleKey(f.i, ch)
	END KeyDown;

	PROCEDURE (f: ComboBox) Edit (op: INTEGER;
		VAR v: Views.View; VAR w, h: INTEGER; VAR singleton, clipboard: BOOLEAN
	);
	BEGIN
		(* TODO *)
	END Edit;

	PROCEDURE (f: ComboBox) Idle, EMPTY;

	PROCEDURE (f: ComboBox) Select (from, to: INTEGER);
	BEGIN
		IF to = MAX(INTEGER) THEN to := -1 END;
		Gtk.gtk_editable_select_region(f.combo.entry, from, to)
	END Select;

	PROCEDURE (f: ComboBox) GetSelection (OUT from, to: INTEGER);
		VAR e: Gtk.GtkOldEditable;
	BEGIN
		(*
		e := f.combo.entry(Gtk.GtkOldEditable);
		IF 0 IN e.has_selection (* testing bit field *) THEN
			from := e.selection_start_pos; to := e.selection_end_pos
		ELSE
		*)
			from := -1; to := -1
		(*
		END
		*)
	END GetSelection;

	PROCEDURE (f: ComboBox) Mark (on, focus: BOOLEAN);
	BEGIN
		Mark(on, f.front, f.i)
	END Mark;

	PROCEDURE (f: ComboBox) Length (): INTEGER;
	BEGIN
		RETURN EditableTextLen(f.combo)
	END Length;

(*
	PROCEDURE (f: ComboBox) GetCursor (x, y: INTEGER; modifiers: SET; VAR cursor: INTEGER);
	BEGIN
	END GetCursor;
*)



	(* Caption *)

	PROCEDURE SetAlign(w: Gtk.GtkMisc; isLeft, isRight: BOOLEAN);
		VAR align: INTEGER;
	BEGIN
		(*	TODO: Justification does not seem to work.
				Is it necessary to pack the label into a GtkHBox to align it correctly? *)
		IF isRight & ~isLeft THEN
			Gtk.gtk_misc_set_alignment(w, 1, 1);
			(*	Gtk.gtk_label_set_justify(l, Gtk.GTK_JUSTIFY_RIGHT)	*)
		ELSIF ~isLeft THEN
			(*	Gtk.gtk_misc_set_alignment(l, 0.5, 0.5);*)
			Gtk.gtk_misc_set_alignment(w, 0.5, 0.5);
		ELSE
			(*	Gtk.gtk_label_set_justify(l, Gtk.GTK_JUSTIFY_LEFT) *)
			Gtk.gtk_misc_set_alignment(w, 0, 0);
		END
	END SetAlign;

	PROCEDURE (f: Caption) SetOffset (x, y: INTEGER);
	BEGIN
		f.SetOffset^(x, y);
		(*Adapt(f, f.i)*)
	END SetOffset;

	PROCEDURE (c: Caption) Close;
	BEGIN
		IF c.labelWidget # NIL THEN
			Gtk.gtk_widget_unref(c.labelWidget);
			Gtk.gtk_widget_unref(c.ebox);
			Gtk.gtk_container_remove(c.rider(HostPorts.Rider).port.fixed, c.ebox)
		END;
		c.i := NIL; c.labelWidget := NIL; c.ebox := NIL
	END Close;

	PROCEDURE (c: Caption) Update;
	BEGIN
		IF c.disabled THEN Gtk.gtk_widget_set_sensitive(c.labelWidget, off)
		ELSE Gtk.gtk_widget_set_sensitive(c.labelWidget, on)
		END
	END Update;

	PROCEDURE (c: Caption) Restore (l, t, r, b: INTEGER);
		VAR w, h, cx, cy, cw, ch, res: INTEGER;
	BEGIN
		IF c.labelWidget = NIL THEN
			c.noRedraw := TRUE;
			c.labelWidget := NewLabel(c.label,NewStyle(c.font));
			SetAlign(c.labelWidget, c.left, c.right);
			c.ebox := Gtk.gtk_event_box_new();
			Gtk.gtk_widget_ref(c.ebox);
			c.i := NewInfo(c, c.ebox, c.ebox);
			c.rider(HostPorts.Rider).GetRect(cx, cy, cw, ch); cw := cw - cx; ch := ch - cy;
			Gtk.gtk_widget_set_usize(c.ebox, cw, ch);
			Gtk.gtk_container_add(c.ebox, c.labelWidget);
			Gtk.gtk_label_set_line_wrap(c.labelWidget, 0);
			Gtk.gtk_fixed_put(c.rider(HostPorts.Rider).port.fixed, c.ebox, cx, cy);
		END;
		c.Update;
		Paint(c.i)
	END Restore;



	(* Group *)

	PROCEDURE (f: Group) SetOffset (x, y: INTEGER);
	BEGIN
		(*
		f.SetOffset^(x, y);
		(*Adapt(f, f.i)*)
		*)
	END SetOffset;

	PROCEDURE (f: Group) Close;
	BEGIN
		(* TODO *)
	END Close;

	PROCEDURE (f: Group) Update;
	BEGIN
		(* TODO *)
	END Update;

	PROCEDURE (c: Group) Restore (l, t, r, b: INTEGER);
		VAR w, h, asc, dsc, sw: INTEGER; s: Dialog.String; col: Ports.Color;
	BEGIN
		c.view.context.GetSize(w, h);
		Dialog.MapString(c.label, s); AmpersandToUline(s);
		HostFonts.dlgFont.GetBounds(asc, dsc, sw);
		sw := HostFonts.dlgFont.StringWidth(s);
		IF c.disabled THEN col := Ports.grey50 ELSE col := Ports.black END;
		c.DrawRect(Ports.point, asc DIV 2, w - Ports.point, h - Ports.point, Ports.point, col);
		c.DrawRect(Ports.mm, 0, Ports.mm + sw, asc + dsc, Ports.fill, Ports.dialogBackground);
		c.DrawString(Ports.mm, asc + dsc, col, s, HostFonts.dlgFont)
	END Restore;


	(* TreeFrame *)

	PROCEDURE [ccall] TreeSelect (ctree: Gtk.GtkCTree; node: Gtk.GtkCTreeNode; column: INTEGER; user_data: INTEGER);
		VAR tn: Dialog.TreeNode; c: TreeFrame;
	BEGIN
		tn := SYSTEM.VAL(Dialog.TreeNode, Gtk.gtk_ctree_node_get_row_data(ctree, node));
		c := SYSTEM.VAL(TreeFrame, user_data);
		IF tn # c.Selected(c) THEN c.Select(c, tn) END
	END TreeSelect;

	PROCEDURE LoadTreeIcons (window: Gdk.GdkWindow);
		VAR loc: HostFiles.Locator; s: ARRAY LEN(HostFiles.FullName) OF SHORTCHAR;
	BEGIN
		IF (tree_closed = NIL) & (window # NIL) THEN
			loc := Files.dir.This("")(HostFiles.Locator);
			(* TODO: UTF-8 ? *)
			s := SHORT(loc.path) + "/Win/Rsrc/folder.xpm";
			tree_closed := Gdk.gdk_pixmap_create_from_xpm (window, tree_closed_mask, NIL, s);
			s := SHORT(loc.path) + "/Win/Rsrc/openfold.xpm";
			tree_open := Gdk.gdk_pixmap_create_from_xpm (window, tree_open_mask, NIL, s);
			s := SHORT(loc.path) + "/Win/Rsrc/file.xpm";
			tree_leaf := Gdk.gdk_pixmap_create_from_xpm (window, tree_leaf_mask, NIL, s)
		END
	END LoadTreeIcons;

	PROCEDURE (f: TreeFrame) SetOffset (x, y: INTEGER);
	BEGIN
		(*
		f.SetOffset^(x, y);
		(*Adapt(f, f.i)*)
		*)
	END SetOffset;

	PROCEDURE (c: TreeFrame) Close;
	BEGIN
		IF c.ctree # NIL THEN
			Gtk.gtk_widget_unref(c.ctree);
			Gtk.gtk_widget_unref(c.scrlw);
			Gtk.gtk_container_remove(c.rider(HostPorts.Rider).port.fixed, c.scrlw)
		END;
		c.ctree := NIL; c.scrlw := NIL; c.i := NIL;
	END Close;

	PROCEDURE (f: TreeFrame) Update;
		VAR tn: Dialog.TreeNode; r: INTEGER;
	BEGIN
		UpdateEventMask(f.i);
		IF f.disabled OR f.readOnly THEN
			Gtk.gtk_widget_set_sensitive(f.ctree, off);
			IF f.disabled THEN
				Gtk.gtk_clist_unselect_all(f.ctree)
			END
		ELSE
			Gtk.gtk_widget_set_sensitive(f.ctree, on)
		END;
		IF ~f.disabled THEN
			tn := f.Selected(f);
			IF tn # NIL THEN
				r := Gtk.gtk_clist_find_row_from_data(f.ctree, SYSTEM.VAL(INTEGER, tn));
				IF r >= 0 THEN
					Gtk.gtk_clist_select_row(f.ctree, r, 0)
				END
			END;
			Gtk.gtk_widget_set_sensitive(f.scrlw, on)
		ELSE
			Gtk.gtk_widget_set_sensitive(f.scrlw, off)
		END;
		Gtk.gtk_widget_show_all(f.scrlw)
	END Update;

	PROCEDURE (f: TreeFrame) RealInsert (tn: Dialog.TreeNode; parent: Gtk.GtkCTreeNode;
														OUT newNode: Gtk.GtkCTreeNode), NEW;
		VAR
			name: Dialog.String; leaf, expanded: INTEGER;
			ss: ARRAY (LEN(Dialog.String) - 1) * Utf8.maxShortCharsPerChar + 1 OF SHORTCHAR; sa: ARRAY [untagged] 1 OF Gtk.PString;
	BEGIN
		ASSERT(tn # NIL, 20);
		tn.GetName(name); Utf8.Short(name, ss);
		sa[0] := ss;
		IF tn.IsFolder() THEN leaf := 0 ELSE leaf := 1 END;
		IF tn.IsExpanded() THEN expanded := 1 ELSE expanded := 0 END;
		IF f.foldericons THEN
			IF tn.IsFolder() THEN
				newNode:= Gtk.gtk_ctree_insert_node(f.ctree, parent, NIL,
								sa, 0,
								tree_closed, tree_closed_mask, tree_open, tree_open_mask, leaf, expanded);
			ELSE
				newNode:= Gtk.gtk_ctree_insert_node(f.ctree, parent, NIL,
								sa, 0,
								tree_leaf, tree_leaf_mask, NIL, NIL, leaf, expanded);
			END
		ELSE
			newNode:= Gtk.gtk_ctree_insert_node(f.ctree, parent, NIL, sa, 0,
							NIL, NIL, NIL, NIL, leaf, expanded);
		END;
		Gtk.gtk_ctree_node_set_row_data(f.ctree, newNode, SYSTEM.VAL(INTEGER, tn))
	END RealInsert;

	PROCEDURE (f: TreeFrame) InsertNode (tn: Dialog.TreeNode; parent: Gtk.GtkCTreeNode), NEW;
		VAR newNode: Gtk.GtkCTreeNode;
	BEGIN
		IF tn # NIL THEN
			f.RealInsert(tn, parent, newNode);
			f.InsertNode(f.Child(f, tn), newNode);
			f.InsertNode(f.Next(f, tn), parent)
		END
	END InsertNode;

	PROCEDURE (f: TreeFrame) UpdateList;
		VAR
			node: Dialog.TreeNode; name: Dialog.String; cnode: Gtk.GtkWidget;
			ss: ARRAY LEN(Dialog.String) OF SHORTCHAR; sa: ARRAY [untagged] 1 OF Gtk.PString;
	BEGIN
		LoadTreeIcons(f.ctree.window);
		Gtk.gtk_clist_clear(f.ctree);
		Gtk.gtk_clist_freeze(f.ctree);
		f.InsertNode(f.Child(f, NIL), NIL);
		Gtk.gtk_clist_thaw(f.ctree);
		IF f.sorted THEN
			Gtk.gtk_clist_sort(f.ctree)
		END;
		f.Update
	END UpdateList;
(**)
	PROCEDURE (f: TreeFrame) ExpandCollapse(index, action: INTEGER), NEW;
	BEGIN
	END ExpandCollapse;
(**)
	PROCEDURE (f: TreeFrame) DblClickOk (x, y: INTEGER): BOOLEAN;
	BEGIN
		(* TODO *)
		RETURN FALSE
	END DblClickOk;

	PROCEDURE (f: TreeFrame) MouseDown (x, y: INTEGER; buttons: SET);
	BEGIN
		ASSERT(~f.disabled, 100);
		IF f.rider # NIL THEN
			HandleMouse(f.i, x DIV f.unit, y DIV f.unit, buttons)
		END
	END MouseDown;

	PROCEDURE (f: TreeFrame) WheelMove (x, y: INTEGER; op, nofLines: INTEGER; VAR done: BOOLEAN);
	BEGIN
		(* TODO *)
	END WheelMove;

	PROCEDURE (c: TreeFrame) Restore (l, t, r, b: INTEGER);
		VAR w, h, cx, cy, cw, ch, res: INTEGER;
	BEGIN
		IF c.ctree = NIL THEN
			c.noRedraw := TRUE;
			c.rider(HostPorts.Rider).GetRect(cx, cy, cw, ch);
			cw := cw - cx; ch := ch - cy;
			c.ctree := Gtk.gtk_ctree_new(1, 0);			Gtk.gtk_widget_ref(c.ctree);
			Gtk.gtk_widget_modify_style(c.ctree, NewStyle(c.font));
			res := GtkU.gtk_signal_connect(c.ctree, "tree-select-row", SYSTEM.ADR(TreeSelect), SYSTEM.VAL(INTEGER, c));
			c.scrlw := Gtk.gtk_scrolled_window_new(NIL, NIL);
			Gtk.gtk_widget_ref(c.scrlw);
			c.i := NewInfo(c, c.scrlw, c.ctree);
			Gtk.gtk_widget_set_usize(c.scrlw, cw, ch);
			Gtk.gtk_container_add(c.scrlw, c.ctree);
			Gtk.gtk_scrolled_window_set_policy(c.scrlw, Gtk.GTK_POLICY_AUTOMATIC, Gtk.GTK_POLICY_AUTOMATIC);
			Gtk.gtk_fixed_put(c.rider(HostPorts.Rider).port.fixed, c.scrlw, cx, cy);
			c.UpdateList
		ELSE
			c.Update
		END;
		Paint(c.i)
	END Restore;

	PROCEDURE (f: TreeFrame) KeyDown (ch: CHAR);
	BEGIN
		ASSERT(~f.disabled, 100);
		HandleKey(f.i, ch)
	END KeyDown;

	PROCEDURE (f: TreeFrame) Mark (on, focus: BOOLEAN);
	BEGIN
		Mark(on, f.front, f.i)
	END Mark;

	PROCEDURE (f: TreeFrame) GetSize (OUT w, h: INTEGER);
	BEGIN
		w := 0; h := 0 (* FIXME *)
	END GetSize;


	(* Directory *)

	PROCEDURE (d: Directory) GetPushButtonSize (VAR w, h: INTEGER);
	BEGIN
		IF w = Views.undefined THEN w := 56 * Ports.point END;
		IF h = Views.undefined THEN h := 28 * Ports.point END; (* 18 points in Windows OS *)
	END GetPushButtonSize;

	PROCEDURE (d: Directory) GetCheckBoxSize (VAR w, h: INTEGER);
	BEGIN
		IF w = Views.undefined THEN w := 60 * Ports.point END;
		IF h = Views.undefined THEN h := 12 * Ports.point END
	END GetCheckBoxSize;

	PROCEDURE (d: Directory) GetRadioButtonSize (VAR w, h: INTEGER);
	BEGIN
		IF w = Views.undefined THEN w := 60 * Ports.point END;
		IF h = Views.undefined THEN h := 12 * Ports.point END
	END GetRadioButtonSize;

	PROCEDURE (d: Directory) GetScrollBarSize (VAR w, h: INTEGER);
	BEGIN
		IF w = Views.undefined THEN w := 120 * Ports.point END;
		IF h = Views.undefined THEN h := 12 * Ports.point END
	END GetScrollBarSize;

	PROCEDURE (d: Directory) GetFieldSize (max: INTEGER; VAR w, h: INTEGER);
	BEGIN
		IF w = Views.undefined THEN
			IF max = 0 THEN w := 80 * Ports.point
			ELSIF max < 10 THEN w := 32 * Ports.point
			ELSIF max < 15 THEN w := 56 * Ports.point
			ELSIF max < 30 THEN w := 80 * Ports.point
			ELSIF max < 100 THEN w := 120 * Ports.point
			ELSE w := 150 * Ports.point
			END
		END;
		IF h = Views.undefined THEN h := 17 * Ports.point END
	END GetFieldSize;

	PROCEDURE (d: Directory) GetUpDownFieldSize (max: INTEGER; VAR w, h: INTEGER);
	BEGIN
		IF w = Views.undefined THEN w := 56 * Ports.point END;
		IF h = Views.undefined THEN h := 17 * Ports.point END
	END GetUpDownFieldSize;

	PROCEDURE (d: Directory) GetDateFieldSize (VAR w, h: INTEGER);
	BEGIN
		IF w = Views.undefined THEN w := 72 * Ports.point END;
		IF h = Views.undefined THEN h := 17 * Ports.point END
	END GetDateFieldSize;

	PROCEDURE (d: Directory) GetTimeFieldSize (VAR w, h: INTEGER);
	BEGIN
		IF w = Views.undefined THEN w := 72 * Ports.point END;
		IF h = Views.undefined THEN h := 17 * Ports.point END
	END GetTimeFieldSize;

	PROCEDURE (d: Directory) GetColorFieldSize (VAR w, h: INTEGER);
	BEGIN
		IF w = Views.undefined THEN w := 36 * Ports.point END;
		IF h = Views.undefined THEN h := 18 * Ports.point END
	END GetColorFieldSize;

	PROCEDURE (d: Directory) GetListBoxSize (VAR w, h: INTEGER);
	BEGIN
		IF w = Views.undefined THEN w := 100 * Ports.point END;
		IF h = Views.undefined THEN h := 18 * Ports.point END
	END GetListBoxSize;

	PROCEDURE (d: Directory) GetSelectionBoxSize (VAR w, h: INTEGER);
	BEGIN
		IF w = Views.undefined THEN w := 100 * Ports.point END;
		IF h = Views.undefined THEN h := 54 * Ports.point END
	END GetSelectionBoxSize;

	PROCEDURE (d: Directory) GetComboBoxSize (VAR w, h: INTEGER);
	BEGIN
		IF w = Views.undefined THEN w := 100 * Ports.point END;
		IF h = Views.undefined THEN h := 18 * Ports.point END
	END GetComboBoxSize;

	PROCEDURE (d: Directory) GetCaptionSize (VAR w, h: INTEGER);
	BEGIN
		IF w = Views.undefined THEN w := 50 * Ports.point END;
		IF h = Views.undefined THEN h := 12 * Ports.point END
	END GetCaptionSize;

	PROCEDURE (d: Directory) GetGroupSize (VAR w, h: INTEGER);
	BEGIN
		IF w = Views.undefined THEN w := 100 * Ports.point END;
		IF h = Views.undefined THEN h := 100 * Ports.point END
	END GetGroupSize;

	PROCEDURE (d: Directory) GetTreeFrameSize (VAR w, h: INTEGER);
	BEGIN
		IF w = Views.undefined THEN w := 100 * Ports.point END;
		IF h = Views.undefined THEN h := 100 * Ports.point END
	END GetTreeFrameSize;

	PROCEDURE (d: Directory) NewPushButton (): StdCFrames.PushButton;
		VAR f: PushButton;
	BEGIN
		NEW(f); RETURN f
	END NewPushButton;

	PROCEDURE (d: Directory) NewCheckBox (): StdCFrames.CheckBox;
		VAR f: CheckBox;
	BEGIN
		NEW(f); RETURN f
	END NewCheckBox;

	PROCEDURE (d: Directory) NewRadioButton (): StdCFrames.RadioButton;
		VAR f: RadioButton;
	BEGIN
		NEW(f); RETURN f
	END NewRadioButton;

	PROCEDURE (d: Directory) NewScrollBar (): StdCFrames.ScrollBar;
		VAR f: ScrollBar;
	BEGIN
		NEW(f); RETURN f
	END NewScrollBar;

	PROCEDURE (d: Directory) NewField (): StdCFrames.Field;
		VAR f: Field;
	BEGIN
		NEW(f); RETURN f
	END NewField;

	PROCEDURE (d: Directory) NewUpDownField (): StdCFrames.UpDownField;
		VAR f: UpDownField;
	BEGIN
		NEW(f); RETURN f
	END NewUpDownField;

	PROCEDURE (d: Directory) NewDateField (): StdCFrames.DateField;
		VAR f: DateField;
	BEGIN
		NEW(f); RETURN f
	END NewDateField;

	PROCEDURE (d: Directory) NewTimeField (): StdCFrames.TimeField;
		VAR f: TimeField;
	BEGIN
		NEW(f); RETURN f
	END NewTimeField;

	PROCEDURE (d: Directory) NewColorField (): StdCFrames.ColorField;
		VAR f: ColorField;
	BEGIN
		NEW(f); RETURN f
	END NewColorField;

	PROCEDURE (d: Directory) NewListBox (): StdCFrames.ListBox;
		VAR f: ListBox;
	BEGIN
		NEW(f); RETURN f
	END NewListBox;

	PROCEDURE (d: Directory) NewSelectionBox (): StdCFrames.SelectionBox;
		VAR f: SelectionBox;
	BEGIN
		NEW(f); RETURN f
	END NewSelectionBox;

	PROCEDURE (d: Directory) NewComboBox (): StdCFrames.ComboBox;
		VAR f: ComboBox;
	BEGIN
		NEW(f); RETURN f
	END NewComboBox;

	PROCEDURE (d: Directory) NewCaption (): StdCFrames.Caption;
		VAR f: Caption;
	BEGIN
		NEW(f); RETURN f
	END NewCaption;

	PROCEDURE (d: Directory) NewGroup (): StdCFrames.Group;
		VAR f: Group;
	BEGIN
		NEW(f); RETURN f
	END NewGroup;

	PROCEDURE (d: Directory) NewTreeFrame (): StdCFrames.TreeFrame;
		VAR f: TreeFrame;
	BEGIN
		NEW(f); RETURN f
	END NewTreeFrame;



	PROCEDURE SetDefFonts*;
	BEGIN
		StdCFrames.defaultFont := HostFonts.dlgFont;
		StdCFrames.defaultLightFont := Fonts.dir.This(
			HostFonts.dlgFont.typeface, HostFonts.dlgFont.size, HostFonts.dlgFont.style, Fonts.normal)
	END SetDefFonts;

	PROCEDURE Install;
		VAR dir: Directory;
	BEGIN
		NEW(dir); StdCFrames.SetDir(dir)
	END Install;

(*
	PROCEDURE InitNationalInfo;
	BEGIN
	END InitNationalInfo;
*)

	PROCEDURE Init;
		VAR dir: Directory;
	BEGIN
		StdCFrames.setFocus := TRUE;
		SetDefFonts;
		(*InitNationalInfo;*)
		NEW(mouseDelayer);
		Install
	END Init;

BEGIN
	Init
END HostCFrames.
