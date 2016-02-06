MODULE HostDialog;
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

	(*	TODO:
			- Map titles for the dialogs
			- Preferences dialog: true type metric; visual scroll
	*)

	IMPORT
		WinApi, SYSTEM,
		Iconv := LibsIconv, GLib := LibsGlib, Gtk := LibsGtk, GtkU:=Gtk2Util,
		Kernel, Strings, Dates, Fonts, Ports, Files, Stores, Views, Controllers, Properties,
		Dialog, Windows, Converters,
		HostFonts, HostFiles, HostRegistry, HostWindows,
		StdCmds,
		HostCFrames (* don't remove *);

	CONST
		(** CloseDialog res **)
		save* = 1; cancel* = 2;

		dirtyString = "#Host:SaveChanges";

		sepChar = GLib.G_DIR_SEPARATOR;

	TYPE
		(*
		Preview = POINTER TO RECORD (Views.View) END;

		UpdateMsg = RECORD (Views.Message) END;
		*)

		DatesHook = POINTER TO RECORD (Dates.Hook) END;
		DialogHook = POINTER TO RECORD (Dialog.GetHook) END;
		ShowHook = POINTER TO RECORD (Dialog.ShowHook) END;
		GetSpecHook = POINTER TO RECORD (Views.GetSpecHook) END;
		LanguageHook = POINTER TO RECORD (Dialog.LanguageHook) END;

	VAR
		(*
		window-: Windows.Window;	(** window created/selected by most recent Old or Open **)
		oldWindow-: BOOLEAN;	(** most recent Old or Open selected existing window **)
		*)

		osVersion-: INTEGER;

		prefs*: RECORD
			useTTMetric-: BOOLEAN;
			visualScroll-: BOOLEAN;
			statusbar-: INTEGER;
			thickCaret*: BOOLEAN;
			caretPeriod*: INTEGER
		END;

		prefFName, prefDName: Fonts.Typeface;
		prefFSize, prefDSize, prefDWght: INTEGER;
		prefDStyle: SET;
		defConv: Converters.Converter;
		all: Converters.Converter;
		encoder, decoder: Iconv.iconv_t;
		hist:	HostFiles.FullName;
		response: INTEGER;

		dialogHook: DialogHook;

	(* Show Hook *)

	PROCEDURE^ RunDialog (dialog :Gtk.GtkDialog): INTEGER;

	PROCEDURE ShowParamStatus* (IN str, p0, p1, p2: ARRAY OF CHAR);
		VAR res: INTEGER; st: ARRAY 512 OF CHAR;
	BEGIN
		Dialog.MapParamString(str, p0, p1, p2, st);
		HostWindows.SetStatusText(st)
	END ShowParamStatus;

	PROCEDURE ShowParamMsg* (IN str, p0, p1, p2: ARRAY OF CHAR);
		VAR res: INTEGER;
			dlg: Gtk.GtkMessageDialog;
			st: ARRAY 512 OF CHAR;
			us: GLib.PString;
	BEGIN
		ASSERT(str # "", 20);
		(*Dialog.appName ?*);
		Dialog.MapParamString(str, p0, p1, p2, st);
		us := GLib.g_utf16_to_utf8(st, -1, NIL, NIL, NIL);
		dlg := Gtk.gtk_message_dialog_new (NIL,{0,1,2}, 0,1,us); (* TODO: NIL-> main_application_window*)
		GLib.g_free(SYSTEM.VAL(GLib.gpointer, us));
		res := RunDialog(dlg);
		Gtk.gtk_widget_destroy(dlg)
	END ShowParamMsg;

	PROCEDURE (h: ShowHook) ShowParamMsg (IN str, p0, p1, p2: ARRAY OF CHAR);
	BEGIN
		ShowParamMsg(str, p0, p1, p2)
	END ShowParamMsg;

	PROCEDURE (h: ShowHook) ShowParamStatus (IN str, p0, p1, p2: ARRAY OF CHAR);
	BEGIN
		ShowParamStatus(str, p0, p1, p2)
	END ShowParamStatus;

	(** general OK dialog **)

	PROCEDURE (hook: DialogHook) GetOK (IN str, p0, p1, p2: ARRAY OF CHAR; form: SET; OUT res: INTEGER);
		VAR r: INTEGER;
			dlg: Gtk.GtkMessageDialog;
			st: ARRAY 512 OF CHAR;
			us: GLib.PString;
			type:Gtk.GtkMessageType;
			buttons:Gtk.GtkButtonsType;
	BEGIN
		ASSERT(str # "", 20);
		Dialog.MapParamString(str, p0, p1, p2, st);
		IF Dialog.yes IN form THEN
			type:=Gtk.GTK_MESSAGE_QUESTION;
			buttons:= Gtk.GTK_BUTTONS_YES_NO;
			(* IF Dialog.cancel IN form THEN YES_NO_CANCEL	END *)
		ELSE (* ok *)
			type:=Gtk.GTK_MESSAGE_WARNING;
			IF Dialog.cancel IN form THEN buttons:= Gtk.GTK_BUTTONS_OK_CANCEL ELSE buttons := Gtk.GTK_BUTTONS_OK END;
		END;
		us := GLib.g_utf16_to_utf8(st, -1, NIL, NIL, NIL);
		dlg := Gtk.gtk_message_dialog_new(NIL,{0,1}, type, buttons, us);
		GLib.g_free(SYSTEM.VAL(GLib.gpointer, us));
		r := RunDialog(dlg);
		CASE r OF
		| Gtk.GTK_RESPONSE_YES : res := Dialog.yes
		| Gtk.GTK_RESPONSE_CANCEL: res := Dialog.cancel
		| Gtk.GTK_RESPONSE_OK : res := Dialog.ok
		| Gtk.GTK_RESPONSE_NO : res := Dialog.no
		ELSE res := 0
		END;
		Gtk.gtk_widget_destroy(dlg)
	END GetOK;


	(** time **)

	PROCEDURE (hook: DatesHook) GetTime (OUT d: Dates.Date; OUT t: Dates.Time);
		VAR dt: WinApi.SYSTEMTIME;
	BEGIN
		WinApi.GetLocalTime(dt);
		d.year := dt.wYear; d.month := dt.wMonth; d.day := dt.wDay;
		t.hour := dt.wHour; t.minute := dt.wMinute; t.second := dt.wSecond
	END GetTime;

	PROCEDURE (hook: DatesHook) GetUTCTime (OUT d: Dates.Date; OUT t: Dates.Time);
		VAR dt: WinApi.SYSTEMTIME;
	BEGIN
		WinApi.GetSystemTime(dt);
		d.year := dt.wYear; d.month := dt.wMonth; d.day := dt.wDay;
		t.hour := dt.wHour; t.minute := dt.wMinute; t.second := dt.wSecond
	END GetUTCTime;

	PROCEDURE (hook: DatesHook) GetUTCBias (OUT bias: INTEGER);
		VAR res: INTEGER; info: WinApi.TIME_ZONE_INFORMATION;
	BEGIN
		bias := 0;
		res := WinApi.GetTimeZoneInformation(info);
		IF res # -1 THEN
			IF BITS(res) = WinApi.TIME_ZONE_ID_DAYLIGHT THEN bias := info.Bias + info.DaylightBias
			ELSE bias := info.Bias + info.StandardBias
			END
		END
	END GetUTCBias;

(*
	(** import type dialog **)

	PROCEDURE ImpOk*;
	BEGIN
		impType.done := TRUE;
		StdCmds.CloseDialog
	END ImpOk;
*)

	PROCEDURE RunDialog (dialog :Gtk.GtkDialog): INTEGER;
	BEGIN
		RETURN Gtk.gtk_dialog_run(dialog)
	END RunDialog;

	(** file dialogs **)

	PROCEDURE^ GetFileSpec (mode:INTEGER; VAR loc: Files.Locator; VAR name: Files.Name);
	PROCEDURE^ FindConverter (IN name: Files.Name; VAR conv: Converters.Converter);

	PROCEDURE (hook: DialogHook) GetIntSpec (
		IN defType: Files.Type; VAR loc: Files.Locator; OUT name: Files.Name
	);
	(* asks user for a file name (for file internalization) *)
	BEGIN
		(* defType *)
		GetFileSpec(0, loc, name)
	END GetIntSpec;

	PROCEDURE GetIntSpec* (VAR loc: Files.Locator; VAR name: Files.Name; VAR conv: Converters.Converter);
	BEGIN
		GetFileSpec(0, loc, name);
		FindConverter(name,conv)
	END GetIntSpec;

	PROCEDURE (h: GetSpecHook) GetIntSpec (VAR loc: Files.Locator; VAR name: Files.Name;
															VAR conv: Converters.Converter);
	BEGIN
		GetIntSpec(loc, name, conv)
	END GetIntSpec;

	PROCEDURE (hook: DialogHook) GetExtSpec (IN default: Files.Name; IN defType: Files.Type; VAR loc: Files.Locator; OUT name: Files.Name);
	BEGIN
		(* defType *)
		name:=default;
		GetFileSpec(1, loc, name)
	END GetExtSpec;

	PROCEDURE GetExtSpec* (
		s: Stores.Store; VAR loc: Files.Locator; VAR name: Files.Name; VAR conv: Converters.Converter
	);
	(* ask user for a file name (for file externalization) *)
	BEGIN
		GetFileSpec(1, loc, name);
		FindConverter(name,conv)
	END GetExtSpec;

	PROCEDURE (h: GetSpecHook) GetExtSpec (
		s: Stores.Store; VAR loc: Files.Locator; VAR name: Files.Name; VAR conv: Converters.Converter
	);
	BEGIN
		GetExtSpec(s, loc, name, conv)
	END GetExtSpec;

(*
	(* printer dialogs *)

	(* page setup previewer view *)

	PROCEDURE (v: Preview) Restore (f: Views.Frame; l, t, r, b: INTEGER);
		CONST scale = 16; rmm = Ports.mm DIV scale; size = 460 * rmm;
		VAR u, w, h, x, y, uu: INTEGER;
	BEGIN
		u := f.unit;
		IF Dialog.metricSystem THEN uu := 10 * rmm ELSE uu := Ports.inch DIV scale END;
		w := setup.w DIV scale;
		h := setup.h DIV scale;
		x := (size - w) DIV 2;
		y := (size - h) DIV 2;
		l := SHORT(ENTIER(setup.left * uu));
		t := SHORT(ENTIER(setup.top * uu));
		r := SHORT(ENTIER(setup.right * uu));
		b := SHORT(ENTIER(setup.bottom * uu));
		f.DrawRect(x, y, x + w, y + h, Ports.fill, Ports.background);
		f.DrawRect(x - u, y - u, x + w + u, y + h + u, 0, Ports.defaultColor);
		IF setup.decorate THEN
			IF t < 14 * rmm THEN t := 14 * rmm END;
			f.DrawRect(x + l, y + 10 * rmm, x + l + 20 * rmm, y + 10 * rmm + u, Ports.fill, Ports.defaultColor);
			f.DrawRect(x + w - r - 8 * rmm, y + 10 * rmm, x + w - r, y + 10 * rmm + u, Ports.fill, Ports.defaultColor)
		END;
		IF (w - r > l) & (h - b > t) THEN
			f.DrawRect(x + l, y + t, x + w - r, y + h - b, 0, Ports.defaultColor)
		END
	END Restore;

	PROCEDURE (v: Preview) HandleViewMsg (f: Views.Frame; VAR msg: Views.Message);
	BEGIN
		WITH msg: UpdateMsg DO
			Views.Update(v, Views.keepFrames)
		ELSE
		END
	END HandleViewMsg;

	PROCEDURE Deposit*;
		VAR v: Preview;
	BEGIN
		NEW(v); Views.Deposit(v)
	END Deposit;

	(* page setup dialog *)

	PROCEDURE SetupNotify* (op, from, to: INTEGER);
		VAR msg: UpdateMsg; t: INTEGER;
	BEGIN
		IF op = Dialog.changed THEN
			IF setup.landscape # (setup.w > setup.h) THEN
				t := setup.w; setup.w := setup.h; setup.h := t
			END;
			Views.Omnicast(msg);
			Dialog.Update(setup)
		END
	END SetupNotify;

	PROCEDURE SetupOk*;
		VAR win: Windows.Window; w, h, l, t, r, b, uu: INTEGER;
	BEGIN
		win := Windows.dir.Focus(Controllers.targetPath);
		IF win # NIL THEN
			IF Dialog.metricSystem THEN uu := 10 * Ports.mm ELSE uu := Ports.inch END;
			w := setup.w; h := setup.h;
			l := SHORT(ENTIER(setup.left * uu));
			t := SHORT(ENTIER(setup.top * uu));
			r := w - SHORT(ENTIER(setup.right * uu));
			b := h - SHORT(ENTIER(setup.bottom * uu));
			IF (0 <= l) & (l < r) & (r <= w) & (0 <= t) & (t < b) & (b <= h) THEN
				win.doc.SetPage(w, h, l, t, r, b, setup.decorate);
				StdCmds.CloseDialog
			ELSE
				Dialog.Beep
			END
		END
	END SetupOk;

	PROCEDURE InitPageSetup*;
		VAR win: Windows.Window; w, h, pw, ph, l, t, r, b, uu: INTEGER; p: Printers.Printer;
	BEGIN
		win := Windows.dir.Focus(Controllers.targetPath);
		IF win # NIL THEN
			IF Dialog.metricSystem THEN uu := Ports.mm DIV 10 ELSE uu := Ports.inch DIV 100 END;
			win.doc.PollPage(w, h, l, t, r, b, setup.decorate);
			p := Printers.dir.Current();
			IF p # NIL THEN HostPrinters.GetPage(p, pw, ph);
				IF (pw > ph) = (w > h) THEN w := pw; h := ph ELSE w := ph; h := pw END
			END;
			r := w - r; b := h - b;
			setup.left := l DIV uu / 100;
			setup.right := r DIV uu / 100;
			setup.top := t DIV uu / 100;
			setup.bottom := b DIV uu / 100;
			setup.w := w; setup.h := h;
			setup.hs := setup.right + setup.left;
			setup.vs := setup.bottom + setup.top;
			setup.landscape := w > h
		END
	END InitPageSetup;


	PROCEDURE PrintDialog* (
		hasSelection: BOOLEAN; VAR from, to, copies: INTEGER; VAR selection: BOOLEAN
	);
		VAR res: INTEGER;
	BEGIN
		prt.Flags := {18 (*, 20 *)};	(* use dev mode copies, hide print to file *)
		IF ~hasSelection THEN INCL(prt.Flags, 2) END;	(* no selection *)
		prt.nCopies := 1;
		prt.hwndOwner := HostWindows.ActualWnd();
		prt.nFromPage := 1; prt.nToPage := 1;
		res := WinDlg.PrintDlgW(prt);
		IF (res = 0) & (WinDlg.CommDlgExtendedError() = 4106) THEN
			prt.hDevMode := 0;
			res := WinDlg.PrintDlgW(prt)
		END;
		IF (res = 0) & (WinDlg.CommDlgExtendedError() = 4108) THEN
			prt.hDevMode := 0; prt.hDevNames := 0;
			res := WinDlg.PrintDlgW(prt)
		END;
		HostPrinters.SetCurrent(prt.hDevNames, prt.hDevMode);
		IF res # 0 THEN
			IF 0 IN prt.Flags THEN selection := TRUE; from := 0; to := 0	(* print selection *)
			ELSIF 1 IN prt.Flags THEN selection := FALSE; from := prt.nFromPage - 1; to := prt.nToPage - 1 (* print pages *)
			ELSE selection := FALSE; from := 0; to := 32767	(* print all *)
			END;
			copies := prt.nCopies
		ELSE
			copies := 0;
			res := WinDlg.CommDlgExtendedError();
			ASSERT(res = 0, 100)
		END
	END PrintDialog;

	PROCEDURE PrintSetup*;
		VAR res: INTEGER; pt: Printers.Printer;
	BEGIN
		pt := Printers.dir.Current();
		IF pt # NIL THEN
			pt.SetOrientation(setup.landscape);
			HostPrinters.GetCurrent(prt.hDevNames, prt.hDevMode)
		END;
		prt.Flags := {6};	(* PrintSetup *)
		prt.hwndOwner := HostWindows.ActualWnd();
		res := WinDlg.PrintDlgW(prt);
		IF (res = 0) & (WinDlg.CommDlgExtendedError() = 4108) THEN
			prt.hDevMode := 0; prt.hDevNames := 0;
			res := WinDlg.PrintDlgW(prt)
		END;
		HostPrinters.SetCurrent(prt.hDevNames, prt.hDevMode);
		pt := Printers.dir.Current();
		IF pt # NIL THEN
			HostPrinters.GetPage(pt, setup.w, setup.h);
			setup.landscape := setup.w > setup.h
		END;
		SetupNotify(Dialog.changed, 0, 0)
	END PrintSetup;
*)

	PROCEDURE CloseDialog* (w: Windows.Window; quit: BOOLEAN; VAR res: INTEGER);
		VAR r: INTEGER;
			dlg: Gtk.GtkMessageDialog;
			title: Views.Title;
			text: ARRAY 256 OF CHAR;
			us: GLib.PString;
	BEGIN
		w.GetTitle(title);
		Dialog.MapParamString(dirtyString, title, 0DX, 0DX, text);
		us := GLib.g_utf16_to_utf8(text, -1, NIL, NIL, NIL);
		dlg := Gtk.gtk_message_dialog_new(NIL, {0, 1}, 2, Gtk.GTK_BUTTONS_YES_NO, us);
		GLib.g_free(SYSTEM.VAL(GLib.gpointer, us));
		r := RunDialog(dlg);
		Gtk.gtk_widget_destroy(dlg);

		IF r = Gtk.GTK_RESPONSE_YES THEN res := save
		ELSIF r = Gtk.GTK_RESPONSE_NO THEN res := 0
		ELSE res := cancel
		END;
	END CloseDialog;

	PROCEDURE GetColor(in: Ports.Color; OUT out: Ports.Color; OUT set: BOOLEAN);
		VAR colorDialog: Gtk.GtkColorSelectionDialog;
				color: Gtk.GtkColors;
				res: INTEGER;
	BEGIN
		set := FALSE;
		colorDialog := Gtk.gtk_color_selection_dialog_new("Color");
		color[0] := (in MOD 256) / 255.0;
		color[1] := ((in DIV 256) MOD 256) / 255.0;
		color[2] := ((in DIV 65536) MOD 256) / 255.0;
		color[3] := 0; (* opacity *)
		Gtk.gtk_color_selection_set_color(colorDialog.colorsel, color);
		(* Gtk.gtk_widget_hide(colorDialog.help_button); *)
		res := RunDialog(colorDialog);
		CASE res OF
		| Gtk.GTK_RESPONSE_OK:
			Gtk.gtk_color_selection_get_color(colorDialog.colorsel, color);
			out := Ports.RGBColor(
				SHORT(ENTIER((color[0]*255))),
				SHORT(ENTIER((color[1]*255))),
				SHORT(ENTIER((color[2]*255))));
			set := TRUE;
		| Gtk.GTK_RESPONSE_DELETE_EVENT:
		ELSE
		END;
		Gtk.gtk_widget_destroy(colorDialog)
	END GetColor;

	PROCEDURE (hook: DialogHook) GetColor (in: Ports.Color; OUT out: Ports.Color; OUT set: BOOLEAN);
	BEGIN
		GetColor(in, out, set);
	END GetColor;

	PROCEDURE ColorDialog*;
	(* open color dialog and set selection to choosen color *)
		VAR set: BOOLEAN; p: Properties.StdProp; col: Ports.Color;
	BEGIN
		Properties.CollectStdProp(p);
		IF ~(Properties.color IN p.known) THEN p.color.val := Ports.black END;
		GetColor(p.color.val, col, set);
		IF set THEN StdCmds.Color(col) END
	END ColorDialog;

	PROCEDURE GetFont (VAR typeface: Fonts.Typeface; VAR size: INTEGER; VAR weight: INTEGER; VAR style: SET; VAR set: BOOLEAN);
		VAR
			res: INTEGER;
			fsDialog: Gtk.GtkFontSelectionDialog;
			fn: GLib.PString;
			s: ARRAY 256 OF SHORTCHAR;
	BEGIN
		set := FALSE;
		fsDialog := Gtk.gtk_font_selection_dialog_new("Font");
		HostFonts.MakePangoString(typeface$, size, style, weight, s);
		res := Gtk.gtk_font_selection_dialog_set_font_name(fsDialog, s);
		res := RunDialog(fsDialog);
		CASE res OF
		| Gtk.GTK_RESPONSE_OK:
				fn := Gtk.gtk_font_selection_dialog_get_font_name(fsDialog);
				HostFonts.ParsePangoString(fn$, typeface,  size, style, weight);
				set := TRUE
		| Gtk.GTK_RESPONSE_DELETE_EVENT:
		ELSE
		END;
		Gtk.gtk_widget_destroy(fsDialog)
	END GetFont;

	PROCEDURE FontDialog*;
	(** open font dialog and set selection to choosen attributes **)
		VAR set: BOOLEAN; p, p0: Properties.StdProp;
	BEGIN
		Properties.CollectStdProp(p0);
		IF Properties.typeface IN p0.known THEN
			NEW(p);
			p.typeface := p0.typeface$;
			p.size := p0.size;
			p.weight := p0.weight;
			p.style := p0.style;
			GetFont(p.typeface, p.size, p.weight, p.style.val, set);
			IF set THEN
				p.valid := {Properties.typeface, Properties.style, Properties.weight, Properties.size};
				p.style.mask := {Fonts.italic, Fonts.underline, Fonts.strikeout};
				Properties.EmitProp(NIL, p)
			END
		END
	END FontDialog;

	PROCEDURE TypefaceDialog*;
	(** open font dialog and set selection to choosen typeface **)
		VAR set: BOOLEAN; p, p0: Properties.StdProp; s, w: INTEGER; st: SET;
	BEGIN
		Properties.CollectStdProp(p0);
		IF Properties.typeface IN p0.known THEN
			NEW(p);
			p.typeface := p0.typeface$;
			GetFont(p.typeface, s, w, st, set);
			IF set THEN
				p.valid := {Properties.typeface};
				Properties.EmitProp(NIL, p)
			END
		END
	END TypefaceDialog;


	(* preferences dialog *)

	PROCEDURE DefFont*;
		VAR tf: Fonts.Typeface; size: INTEGER; w: INTEGER; style: SET; set: BOOLEAN;
	BEGIN
		tf := prefFName;
		size := prefFSize;
		w := Fonts.normal;
		style := {};
		GetFont(tf, size, w, style, set);
		IF set THEN
			prefFName := tf; prefFSize := size
		END
	END DefFont;

	PROCEDURE DlgFont*;
		VAR tf: Fonts.Typeface; size: INTEGER; w: INTEGER; style: SET; set: BOOLEAN;
	BEGIN
		tf := prefDName;
		size := prefDSize;
		w := prefDWght;
		style := prefDStyle;
		GetFont(tf, size, w, style, set);
		IF set THEN
			prefDName := tf; prefDSize := size; prefDStyle := style; prefDWght := w
		END
	END DlgFont;

	PROCEDURE PrefOk*;
		VAR res: INTEGER;
	BEGIN
		HostFonts.SetDefaultFont(prefFName, prefFSize);
		HostFonts.SetDialogFont(prefDName, prefDSize, prefDStyle, prefDWght);

(*
		HostFonts.SetTTMetric(prefs.useTTMetric);
		HostWindows.SetVisualScroll(prefs.visualScroll);
		IF prefs.statusbar = 1 THEN Dialog.showsStatus := TRUE; HostWindows.memInStatus := FALSE
		ELSIF prefs.statusbar = 2 THEN Dialog.showsStatus := TRUE; HostWindows.memInStatus := TRUE
		ELSE Dialog.showsStatus := FALSE
		END;
*)
		Dialog.showsStatus := TRUE;

		Dialog.Call("StdCmds.UpdateAll", "", res);
		Dialog.Call("StdCmds.RecalcAllSizes", "", res);
		Dialog.Call("TextCmds.UpdateDefaultAttr", "", res);
		HostCFrames.SetDefFonts;
		HostRegistry.WriteBool("noStatus", ~Dialog.showsStatus);
(*
		HostRegistry.WriteBool("memStatus", HostWindows.memInStatus);
*)
(*
		res := WinApi.GetClientRect(HostWindows.main, rect);
		HostWindows.ResizeMainWindow(0, rect.right, rect.bottom);
*)
		Dialog.thickCaret := prefs.thickCaret;
		Dialog.caretPeriod := prefs.caretPeriod;
		HostRegistry.WriteBool("thickCaret", Dialog.thickCaret);
		HostRegistry.WriteInt("caretPeriod", Dialog.caretPeriod)
	END PrefOk;

	PROCEDURE InitPrefDialog*;
	BEGIN
		prefFName := HostFonts.defFont.alias;
		prefFSize := HostFonts.defFont.size;
		prefDName := HostFonts.dlgFont.typeface;
		prefDSize := HostFonts.dlgFont.size;
		prefDStyle := HostFonts.dlgFont.style;
		prefDWght := HostFonts.dlgFont.weight;

(*
		prefs.useTTMetric := HostFonts.useTTMetric;
		prefs.visualScroll := HostWindows.visualScroll;
*)
		prefs.visualScroll := TRUE;

(*
		IF ~Dialog.showsStatus THEN prefs.statusbar := 0
		ELSIF HostWindows.memInStatus THEN prefs.statusbar := 2
		ELSE prefs.statusbar := 1
		END;
*)
		prefs.statusbar := 2;

		prefs.thickCaret := Dialog.thickCaret;
		prefs.caretPeriod := Dialog.caretPeriod
	END InitPrefDialog;


	(* date / time *)

	PROCEDURE (hook: DatesHook) DateToString (d: Dates.Date; format: INTEGER; OUT str: ARRAY OF CHAR);
		VAR res, pos, i: INTEGER; time: WinApi.SYSTEMTIME; fmt: ARRAY 64 OF CHAR;
	BEGIN
		time.wYear := SHORT(d.year); time.wMonth := SHORT(d.month); time.wDay := SHORT(d.day);
		IF format = Dates.short THEN
			res := WinApi.GetDateFormatW(
				HostRegistry.localeId, WinApi.DATE_SHORTDATE, time, NIL, str, LEN(str))
		ELSIF format = Dates.long THEN
			res := WinApi.GetDateFormatW(HostRegistry.localeId, WinApi.DATE_LONGDATE, time, NIL, str, LEN(str))
		ELSE
			res := WinApi.GetLocaleInfoW(HostRegistry.localeId, WinApi.LOCALE_SLONGDATE, fmt, LEN(fmt));
			IF format # Dates.abbreviated THEN	(* remove weekday *)
				Strings.Find(fmt, "dddd", 0, pos); i := pos + 4;
				IF pos < 0 THEN Strings.Find(fmt, "ddd", 0, pos); i := pos + 3 END;
				IF pos >= 0 THEN
					WHILE (fmt[i] # 0X) & (CAP(fmt[i]) < "A") OR (CAP(fmt[i]) > "Z") DO INC(i) END;
					Strings.Replace(fmt, pos, i - pos, "")
				END
			END;
			IF format # Dates.plainLong THEN	(* abbreviated *)
				Strings.Find(fmt, "dddd", 0, pos);
				IF pos >= 0 THEN Strings.Replace(fmt, pos, 4, "ddd") END;
				Strings.Find(fmt, "MMMM", 0, pos);
				IF pos >= 0 THEN Strings.Replace(fmt, pos, 4, "MMM") END
			END;
			res := WinApi.GetDateFormatW(HostRegistry.localeId, {}, time, fmt, str, LEN(str))
		END;
		IF res = 0 THEN str := "?" END
	END DateToString;

	PROCEDURE (hook: DatesHook) TimeToString (t: Dates.Time; OUT str: ARRAY OF CHAR);
		VAR res: INTEGER; time: WinApi.SYSTEMTIME;
	BEGIN
		time.wHour := SHORT(t.hour); time.wMinute := SHORT(t.minute);
		time.wSecond := SHORT(t.second); time.wMilliseconds := 0;
		res := WinApi.GetTimeFormatW(HostRegistry.localeId, {}, time, NIL, str, LEN(str));
		IF res = 0 THEN str := "?" END
	END TimeToString;

	PROCEDURE (hook: LanguageHook) SetLanguage (
		lang: Dialog.Language; persistent: BOOLEAN; OUT ok: BOOLEAN
	);
	BEGIN
		ok := (lang = "") OR (LEN(lang$) = 2);
		IF ok & persistent THEN HostRegistry.WriteString("language", lang) END
	END SetLanguage;

	PROCEDURE (hook: LanguageHook) GetPersistentLanguage (OUT lang: Dialog.Language);
		VAR res: INTEGER; s: ARRAY 32 OF CHAR;
	BEGIN
		HostRegistry.ReadString("language", s, res);
		IF res = 0 THEN
			ASSERT((s = "") OR (LEN(s$) = 2), 100);
			lang := s$
		ELSE lang := ""
		END
	END GetPersistentLanguage;

	PROCEDURE Start* (name: ARRAY OF CHAR);
		VAR res: INTEGER; info: WinApi.STARTUPINFOW; process: WinApi.PROCESS_INFORMATION;
	BEGIN
		(* res := WinApi.WinExec(name, WinApi.SW_NORMAL) *)
		WinApi.GetStartupInfoW(info);
		info.wShowWindow := WinApi.SW_NORMAL;
		res := WinApi.CreateProcessW(NIL, name, NIL, NIL, WinApi.FALSE, {}, 0, NIL, info, process)
	END Start;


	(* initialization *)

	PROCEDURE ResetCodec (c: Iconv.iconv_t): BOOLEAN;
		VAR res, fLen, tLen: Iconv.size_t;
	BEGIN
		ASSERT(c # -1, 20);
		fLen := 0; tLen := 0;
		res := Iconv.iconv(c, NIL, fLen, NIL, tLen);
		RETURN res # -1
	END ResetCodec;

	(* decode filename from HostLang.enc encoding *)
	PROCEDURE Long (ss: Iconv.PtrSTR; OUT s: ARRAY OF CHAR);
		VAR res: Iconv.size_t;
			fLen, tLen: Iconv.size_t;
			to: Iconv.PtrLSTR;
			i: INTEGER;
	BEGIN
		i := 0;
		WHILE (i < LEN(s) - 1) & (ss[i] >= ' ') & (ss[i] <= '~') DO s[i] := ss[i]; INC(i) END;
		IF ss[i] = 0X THEN
			IF i < LEN(s) THEN s[i] := 0X
			ELSE s[0] := 0X
			END
		ELSIF (decoder # -1) & ResetCodec(decoder) THEN
			fLen := LEN(ss$); to := s; tLen := (LEN(s) - 1) * SIZE(CHAR);
			res := Iconv.iconv_decode(decoder, ss, fLen, to, tLen);
			IF (res >= 0) & (fLen = 0) & (tLen >= 0) THEN to[0] := 0X
			ELSE s[0] := 0X
			END
		ELSE s[0] := 0X
		END
	END Long;

	(* encode filename to HostLang.enc encoding *)
	PROCEDURE Short (IN s: ARRAY OF CHAR; OUT ss: ARRAY OF SHORTCHAR);
		VAR res: Iconv.size_t;
			fLen, tLen: Iconv.size_t;
			from: Iconv.PtrLSTR;
			to: Iconv.PtrSTR;
			i: INTEGER;
	BEGIN
		i := 0;
		WHILE (i < LEN(ss) - 1) & (s[i] >= ' ') & (s[i] <= '~') DO ss[i] := SHORT(s[i]); INC(i) END;
		IF s[i] = 0X THEN
			IF i < LEN(ss) THEN ss[i] := 0X
			ELSE ss[0] := 0X
			END
		ELSIF (encoder # -1) & ResetCodec(encoder) THEN
			from := s; fLen := LEN(s$) * SIZE(CHAR); to := ss; tLen := LEN(ss) - 1;
			res := Iconv.iconv_encode(encoder, from, fLen, to, tLen);
			IF (res >= 0) & (fLen = 0) & (tLen >= 0) THEN to[0] := 0X
			ELSE ss[0] := 0X
			END
		ELSE ss[0] := 0X
		END
	END Short;

	(* s -> [ locName sepChar ] name *)
	PROCEDURE SplitFileName (IN s: ARRAY OF CHAR; OUT name: Files.Name; OUT locName: HostFiles.FullName);
		VAR i, j, sepIdx, len: INTEGER;
	BEGIN
		len := LEN(s$);
		i := len - 1; WHILE (i >= 0) & (s[i] # sepChar) DO DEC(i) END;
		IF (i >= 0) & (s[i] = sepChar) THEN
			sepIdx := i;
			(* s(sepIdx;len) -> name *)
				INC(i);
				j := 0;
				WHILE (i < len) & (j < LEN(name) - 1) DO name[j] := s[i]; INC(j); INC(i) END;
				IF (i = len) & (j < LEN(name)) THEN name[j] := 0X
				ELSE name[0] := 0X
				END;
			(* s[0;sepIdx) -> locName *)
				i := 0;
				WHILE (i < sepIdx) & (i < LEN(locName) - 1) DO locName[i] := s[i]; INC(i) END;
				IF (i = sepIdx) & (i < LEN(locName)) THEN locName[i] := 0X
				ELSE locName[0] := 0X
				END
		ELSIF len > 0 THEN
			name := s$; locName[0] := 0X
		ELSE
			name[0] := 0X; locName[0] := 0X
		END
	END SplitFileName;

    PROCEDURE GetFileSpec (mode:INTEGER; VAR loc: Files.Locator; VAR name: Files.Name);
		VAR fs: Gtk.GtkFileSelection;
				res: INTEGER;
				locName: HostFiles.FullName;
				ss: GLib.PString;
				ss1: ARRAY LEN(HostFiles.FullName) * 4 OF SHORTCHAR;
				s: HostFiles.FullName;
	BEGIN
		CASE mode OF
		| 0 : fs := Gtk.gtk_file_selection_new("Open");    (*<--*)
		| 1 : fs := Gtk.gtk_file_selection_new("Save As"); (*<--*)
		END;
			IF loc # NIL THEN
				Short(loc(HostFiles.Locator).path + sepChar + name, ss1);
				Gtk.gtk_file_selection_set_filename(fs, ss1);
			ELSIF hist # "" THEN
				Short(hist + sepChar, ss1);
				Gtk.gtk_file_selection_set_filename(fs, ss1);
			END;
		(* Gtk.gtk_file_selection_hide_fileop_buttons(fs); *)
		res := RunDialog(fs);
		CASE res OF
		| Gtk.GTK_RESPONSE_OK:
			ss := Gtk.gtk_file_selection_get_filename(fs);
			Long(ss, s);
			SplitFileName(s,name,locName);
			loc := HostFiles.NewLocator(locName);
			hist:= loc(HostFiles.Locator).path$;
		| Gtk.GTK_RESPONSE_DELETE_EVENT:
		ELSE
		END;
		Gtk.gtk_widget_destroy(fs)
	END GetFileSpec;

	PROCEDURE FindConverter (IN name: Files.Name; VAR conv: Converters.Converter);
	VAR i, l: INTEGER;
			type: Files.Type;
	BEGIN
		l := LEN(name$);
		type:="";
		i := l;	WHILE (i > 0) & (name[i] # ".") DO DEC(i) END;
		IF i > 0 THEN
			Strings.Extract(name,i+1,l-i,type);
			conv := Converters.list;
			WHILE (conv # NIL) & (conv.fileType # type) DO conv := conv.next END
		ELSE
			conv := NIL
		END
	END FindConverter;

	PROCEDURE ConvInit;
	BEGIN
		(* NOTE: In case of Gtk for Windows, use UTF-8 encoding instead of HostLang.enc *)
		IF Kernel.littleEndian THEN
			decoder := Iconv.iconv_open("UCS-2LE", "UTF-8" (*HostLang.enc*));
			encoder := Iconv.iconv_open("UTF-8" (*HostLang.enc*), "UCS-2LE");
		ELSE
			decoder := Iconv.iconv_open("UCS-2BE", "UTF-8" (*HostLang.enc*));
			encoder := Iconv.iconv_open("UTF-8" (*HostLang.enc*), "UCS-2BE")
		END
	END ConvInit;

	PROCEDURE ConvClose;
		VAR res: INTEGER;
	BEGIN
		IF decoder # -1 THEN res := Iconv.iconv_close(decoder); decoder := -1 END;
		IF encoder # -1 THEN res := Iconv.iconv_close(encoder); encoder := -1 END
	END ConvClose;

	PROCEDURE Init*;
		VAR n, v, res: INTEGER; b: BOOLEAN;
			getSpecHook: GetSpecHook;
			datesHook: DatesHook;
			showHook: ShowHook;
			languageHook: LanguageHook;
	BEGIN
		NEW(all);
		v := WinApi.GetVersion();
		osVersion := v MOD 256 * 100 + v DIV 256 MOD 256;
		IF v >= 0 THEN
			IF osVersion < 400 THEN Dialog.platform := Dialog.windowsNT3
			ELSIF osVersion < 500 THEN Dialog.platform := Dialog.windowsNT4
			ELSIF osVersion = 500 THEN Dialog.platform := Dialog.windows2000
			ELSIF osVersion < 600 THEN Dialog.platform := Dialog.windowsXP
			ELSE Dialog.platform := Dialog.windowsVista
			END
		ELSE Dialog.platform := Dialog.windows98
		END;

		(*
		HostRegistry.ReadBool("noStatus", b, res); Dialog.showsStatus := (res # 0) OR ~b;
		HostRegistry.ReadBool("memStatus", b, res); HostWindows.memInStatus := (res = 0) & b;
		*)

		HostRegistry.ReadBool("thickCaret", b, res); IF res = 0 THEN Dialog.thickCaret := b END;
		HostRegistry.ReadInt("caretPeriod", n, res); IF res = 0 THEN Dialog.caretPeriod := n END;

		NEW(showHook); Dialog.SetShowHook(showHook);
(*
		Hooks.showParamMsg := ShowParamMsg;
		Hooks.showParamStatus := ShowParamStatus;
*)

		NEW(dialogHook); Dialog.SetGetHook(dialogHook);
(*
		Hooks.getOK := GetOK;
		Hooks.getIntSpec := GetIntSpec0;
		Hooks.getExtSpec := GetExtSpec0;
		Hooks.getColor := ColorDialog0;
*)

(*
		Sequencers.GetIntSpec := GetIntSpec;
		Sequencers.GetExtSpec := GetExtSpec;
*)
		NEW(getSpecHook); Views.SetGetSpecHook(getSpecHook);

		HostFiles.MapParamString := Dialog.MapParamString;

		NEW(datesHook); Dates.SetHook(datesHook);
(*
		Hooks.getTime := GetTime;
		Hooks.dateToString := DateToString;
		Hooks.timeToString := TimeToString
*)
		NEW(languageHook); Dialog.SetLanguageHook(languageHook); Dialog.ResetLanguage
	END Init;

BEGIN
	ConvInit;
	Init
CLOSE
	ConvClose
END HostDialog.
