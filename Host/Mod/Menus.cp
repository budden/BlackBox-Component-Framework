MODULE HostMenus;

	IMPORT SYSTEM, Kernel,    
		GLib := LibsGlib, Gdk := LibsGdk, Gtk := LibsGtk,
		GtkU := Gtk2Util, Key := Gtk2Keysyms,
		StdInterpreter,
		HostPorts, HostWindows, HostClipboard,
		Files, Dialog, Strings, Services, Properties, 
		Controllers, Views, Stores, Containers, Windows, Documents, Converters, 
		StdDialog, Log, StdCmds, HostCmds,
		HostCFrames, HostMechanisms, (*HostDates,*) HostTabFrames, HostTextConv (*, HostConsole*);

	CONST
		idlePeriod = 50; (* ms *)
		gcCycle = 100;

		(* hints *)
		impAll = 0;	(* can import all file types *)

		iClose = 100; (* known in HostWindows *)
		iOpen = 102; 
		iExit = 110; iUpdateMenus = 111; iUndo = 112; iCut = 114; iCopy = 115; iPaste = 116;	
		iObject = 120; 
		iPopup = 160; iObjEdit = 161; iObjOpen = 162; iProperties = 163;
		(* custom menus *)
		firstId = 300;

		(* File for specifying command line options *)
		cmdLinePath = "System/Rsrc"; 
		cmdLineFile = "CommandLine.txt";

		off=0; on=1;			

	TYPE
		Item* = POINTER TO RECORD (StdDialog.Item)
			code-: INTEGER; 
			shift-, ctrl-, alt: BOOLEAN;
			del: BOOLEAN;
			mi: Gtk.GtkCheckMenuItem
		END;

		Menu* = POINTER TO RECORD
			next-: Menu;
			menu-, type-: Dialog.String;
			firstItem-, lastItem: Item;
			submenu-: Gtk.GtkMenu;
			mi: Gtk.GtkMenuItem;
			isWinMenu-: BOOLEAN;
			isPopup-: BOOLEAN;
			class-: INTEGER;
			maxId: INTEGER;
		END;

	VAR
		(* active menu bar state *)
		menus-: Menu;
		menuBar-: Gtk.GtkMenu;
		lastId-: INTEGER;	(* last custom menu id *)

		currType: Stores.TypeName;

		(* new menu bar state *)
		newMenuBar, newWinMenu: Gtk.GtkMenu;
		popMenu, newPopMenu: Gtk.GtkMenu;
		nextId: INTEGER;	(* id of next menu item *)
		firstMenu, lastMenu, curMenu: Menu;

		quit: BOOLEAN;
		gc: INTEGER;	(* how many events must be executed before next GC *)
		num: INTEGER;
		sourcesAdded: BOOLEAN;

	(* menu dispatching *)

	PROCEDURE PrepareMenu (menua, notUsed1, notUsed2: INTEGER);
	(* this procedure is called after the user has clicked into the menu bar, but before showing the menu; 
			to prepare item enabling/disabling, check marks, etc. *)
	CONST miMark=0;
	VAR res: INTEGER; failed, ok: BOOLEAN; 
			m: Menu; 	par: Dialog.Par; i: StdDialog.Item; str: Dialog.String; 
			us: GLib.PString;
	BEGIN
		m := menus;
		WHILE (m # NIL) & (m.submenu # SYSTEM.VAL(Gtk.GtkMenu, menua)) DO m := m.next END;
		IF m # NIL THEN
			i := m.firstItem; 
			WHILE i # NIL DO
				IF i.item^ # "" THEN
					WITH i: Item DO
						IF i.filter^ = "" THEN 
							EXCL(i.mi.cmi_flags,miMark)
						ELSE (* custom menu item with custom guard *)
							StdDialog.CheckFilter(i, failed, ok, par);
							IF ~failed THEN
								IF par.label = "-" THEN
									IF ~i.del THEN i.del := TRUE END;
								ELSE
									IF i.del THEN i.del := FALSE END;
									IF par.label # i.item$ THEN
										Dialog.MapString(par.label, str);
										us := GLib.g_utf16_to_utf8(str, -1, NIL, NIL, NIL);
										Gtk.gtk_label_set_text(i.mi.child(Gtk.GtkLabel), us);
										GLib.g_free(SYSTEM.VAL(GLib.gpointer, us))
									END;
									IF par.disabled THEN
										Gtk.gtk_widget_set_sensitive(i.mi, off);
									ELSE
										Gtk.gtk_widget_set_sensitive(i.mi, on)
									END;
									IF par.checked THEN
										INCL(i.mi.cmi_flags,miMark)
									ELSE
										EXCL(i.mi.cmi_flags,miMark)
									END;
									(* Gtk.gtk_check_menu_item_set_active(i.mi, on/off) cause activation *)
									IF ~ok THEN
										Gtk.gtk_widget_set_sensitive(i.mi, off);
									END
								END
							END
						END
					END
				END;
				i := i.next; 
			END
		END
	END PrepareMenu;


	(* Menu command handler *)

	PROCEDURE [ccall] MenuActivate (item: Gtk.GtkItem; menua: INTEGER);
	BEGIN
		Controllers.SetCurrentPath(Controllers.frontPath);
		Kernel.Try(PrepareMenu, menua, 0, 0); 
		Controllers.ResetCurrentPath()
	END MenuActivate;

	PROCEDURE [ccall] MenuSelect (item: Gtk.GtkItem; itema: INTEGER);
		VAR i: Item; 
	BEGIN
		DEC(gc);
		i := SYSTEM.VAL(Item, itema);
		IF i # NIL THEN StdDialog.HandleItem(i) END;
		Properties.IncEra;
	END MenuSelect;


	(* shortcut support *)

	PROCEDURE SetShortcut (VAR item: Item);
		VAR j, n: INTEGER; ch, nch: CHAR;
	BEGIN
		item.code := 0; item.shift := FALSE; item.ctrl := FALSE; item.alt := FALSE;  
		j := 0; 
		ch := item.shortcut[0];
		WHILE (ch # 0X) & (item.code = 0) DO 
			INC(j);
			IF (ch >= "a") & (ch <= "z") THEN ch := CAP(ch) END;
			nch := item.shortcut[j];
			IF ch = "*" THEN item.shift := TRUE
			ELSIF ch = "^" THEN item.ctrl := TRUE
			ELSIF ch = "@" THEN item.alt := TRUE
			ELSIF (ch >= "A") & (ch <= "Z") OR (ch >= "0") & (ch <= "9") OR (ch = " ") OR (ch = "-") THEN
				IF (nch >= "a") & (nch <= "z") THEN nch := CAP(nch) END;
				IF nch = 0X THEN item.code := ORD(ch); item.ctrl := TRUE
				ELSIF ch = "F" THEN
					n := 0;
					WHILE (nch >= "0") & (nch <= "9") DO
						n := 10 * n + ORD(nch) - ORD("0"); INC(j); nch := item.shortcut[j]
					END;
					IF (n >= 1) & (n <= 16) THEN item.code := Key.GDK_F1 - 1 + n END
				END
			END;
			ch := nch
		END;
	END SetShortcut;


	(* Menus *)

	PROCEDURE FirstMenu* (): Menu;
	BEGIN
		RETURN menus
	END FirstMenu;

	PROCEDURE DeleteAll*;
	BEGIN
(*		WHILE USER32.RemoveMenu(menuBar, 0, {USER32.MFByPosition}) # 0 DO END;*)
		firstMenu := NIL; lastMenu := NIL; curMenu := NIL;
 	newWinMenu := NIL;
		newPopMenu := NIL;
		nextId := firstId
	END DeleteAll;

	PROCEDURE Open* (menu, type: ARRAY OF CHAR);
	BEGIN
		ASSERT(curMenu = NIL, 20); ASSERT(menu # "", 21);
		NEW(curMenu); curMenu.next := NIL;
		curMenu.submenu := Gtk.gtk_menu_new();
		Dialog.MapString(menu, curMenu.menu);
		curMenu.type := type$;
		curMenu.firstItem := NIL
	END Open;

	PROCEDURE AddItem* (item, string, shortcut, filter: Dialog.String);
		VAR i: Item; id: INTEGER;
	BEGIN
		ASSERT(curMenu # NIL, 20); ASSERT(item # "", 21); ASSERT(string # "", 22);
		IF string = "HostMenus.WindowList" THEN
			curMenu.isWinMenu := TRUE
		ELSE
			NEW(i); i.next := NIL;
			IF curMenu.lastItem = NIL THEN curMenu.firstItem := i ELSE curMenu.lastItem.next := i END;
			curMenu.lastItem := i;
			StdDialog.AddItem(i, item, string, filter, shortcut);
			IF string = "HostMenus.ObjectMenu" THEN id := iObject
			ELSE id := nextId; INC(nextId)
			END;
		(*			i.id := id;*)
			IF id > curMenu.maxId THEN curMenu.maxId := id END
		END
	END AddItem;

	PROCEDURE AddSeparator*;
		VAR i: Item;
	BEGIN
		ASSERT(curMenu # NIL, 20);
		NEW(i); i.next := NIL;
		IF curMenu.lastItem = NIL THEN curMenu.firstItem := i ELSE curMenu.lastItem.next := i END;
		curMenu.lastItem := i;
		StdDialog.AddItem(i, "", "", "", "");
		(*		i.id := 0*)
	END AddSeparator;

	(* On Windows & is used to mark a Alt-shortcut. On Linux underscore is used. (&& should be interpreted as &) *)
	PROCEDURE AmpersandToUline(VAR ss: ARRAY OF CHAR);
		VAR i, j: INTEGER;
	BEGIN
		i := 0; j := 0;
		WHILE ss[i] # 0X DO
			IF (ss[i] = "&") & (ss[i + 1] = "&") THEN ss[j] := "&";  INC(i)
			ELSIF ss[i] = "&" THEN ss[j] := "_"
			ELSE ss[j] := ss[i]
			END;
			INC(i); INC(j);
		END;
		ss[j] := 0X
	END AmpersandToUline;

	PROCEDURE NewMenuItem (title: ARRAY OF CHAR; OUT mi: Gtk.GtkMenuItem);
	VAR us: GLib.PString; 
	BEGIN
		AmpersandToUline(title);
		us:=GLib.g_utf16_to_utf8(title,-1,NIL,NIL,NIL);
		mi := Gtk.gtk_menu_item_new_with_mnemonic(us);
		GLib.g_free(SYSTEM.VAL(GLib.gpointer, us)) 
	END NewMenuItem;

	PROCEDURE NewMenuCheckItem (title: ARRAY OF CHAR; OUT mi: Gtk.GtkCheckMenuItem);
	VAR us: GLib.PString; 
	BEGIN
		AmpersandToUline(title);
		us:=GLib.g_utf16_to_utf8(title,-1,NIL,NIL,NIL);
		mi := Gtk.gtk_check_menu_item_new_with_mnemonic(us);
		GLib.g_free(SYSTEM.VAL(GLib.gpointer, us)) 
		(* Gtk.gtk_check_menu_item_set_active(mi, off);
		Gtk.gtk_check_menu_item_set_show_toggle(mi,off); *)
	END NewMenuCheckItem;

	PROCEDURE Close*;
		VAR 
			res: INTEGER; item: StdDialog.Item; 
			title: Dialog.String; 
			mask: SET; 
			accGroup: Gtk.GtkAccelGroup; 
	BEGIN
		ASSERT(curMenu # NIL, 20);
		item := curMenu.firstItem; 
		accGroup := Gtk.gtk_accel_group_new();
		Gtk.gtk_window_add_accel_group(HostWindows.main, accGroup);
		(*	Gtk.gtk_accel_group_unlock(accGroup);  *)
		WHILE item # NIL DO
			WITH item: Item DO
				IF item.item^ # "" THEN
					SetShortcut(item); 
					Dialog.MapString(item.item, title); 
					NewMenuCheckItem(title, item.mi);
					Gtk.gtk_menu_shell_append(curMenu.submenu, item.mi);
					res := GtkU.gtk_signal_connect(item.mi, "activate", SYSTEM.ADR(MenuSelect),SYSTEM.VAL(INTEGER, item));
					IF item.code #  0 THEN
						mask := {};
						IF item.ctrl THEN INCL(mask, Gdk.GDK_CONTROL_BIT) END;
						IF item.shift THEN INCL(mask, Gdk.GDK_SHIFT_BIT) END;
						IF item.alt THEN INCL(mask, Gdk.GDK_MOD1_BIT) END;
						Gtk.gtk_widget_add_accelerator(item.mi, "activate", accGroup, item.code, mask, {Gtk.GTK_ACCEL_VISIBLE}); 
					END
				ELSIF item.next # NIL THEN
					Gtk.gtk_menu_shell_append(curMenu.submenu, Gtk.gtk_separator_menu_item_new())
				END
			END;
			item := item.next; 
		END;
		IF curMenu.menu = "*" THEN curMenu.isPopup := TRUE END;
		IF curMenu.type = "WindowMenu" THEN 
			curMenu.isWinMenu := TRUE; 
			curMenu.type := "" 
		END;
		IF curMenu.isWinMenu THEN newWinMenu := curMenu.submenu END; 
		IF curMenu.type = "PopupMenu" THEN newPopMenu := curMenu.submenu END;
		IF lastMenu = NIL THEN firstMenu := curMenu ELSE lastMenu.next := curMenu END;
		lastMenu := curMenu; curMenu := NIL;
		(*Gtk.gtk_accel_group_lock(accGroup) *)
	END Close;

	PROCEDURE InitMenus*;
		VAR m,  old: Menu; 
			res, i: INTEGER; 
			used: SET; oldBar: Gtk.GtkWidget;
	BEGIN
		ASSERT(curMenu = NIL, 20);
		IF firstMenu # NIL THEN
			used := {}; m := firstMenu;
			m := firstMenu; i := 0;
			WHILE m # NIL DO
				IF m.isWinMenu THEN m.class := 4; i := 100
				ELSIF m.isPopup THEN m.class := 10
				ELSIF i = 0 THEN m.class := 0
				ELSIF i < 3 THEN m.class := 1
				ELSIF i < 100 THEN m.class := 3
				ELSE m.class := 5
				END;
				m := m.next; INC(i)
			END;
			newMenuBar := Gtk.gtk_menu_bar_new();
			m := firstMenu;
			WHILE m # NIL DO
				IF ((m.type = "") OR (m.type = currType)) & ~m.isPopup THEN
					NewMenuItem(m.menu$, m.mi);
					res := GtkU.gtk_signal_connect(m.mi, "activate",SYSTEM.ADR(MenuActivate), SYSTEM.VAL(INTEGER, m.submenu));
					Gtk.gtk_menu_item_set_submenu(m.mi, m.submenu);
					Gtk.gtk_menu_shell_append(newMenuBar, m.mi)
				END;
				m := m.next
			END;
			oldBar := menuBar; menuBar := newMenuBar;
			popMenu := newPopMenu;
			old := menus; menus := firstMenu; lastId := nextId;

			(*IF oldBar # NIL THEN Gtk.gtk_container_remove(HostWindows.main, oldBar) END;*)
			(*Gtk.gtk_container_add(HostWindows.main, menuBar);*)
			HostWindows.SetMenu(menuBar);
			Gtk.gtk_widget_show_all(menuBar);
		(**)
		END
	END InitMenus;

	PROCEDURE UpdateMenus;
		VAR res: INTEGER; m: Menu;
		oldBar: Gtk.GtkWidget;
	BEGIN
		oldBar := menuBar; menuBar := Gtk.gtk_menu_bar_new(); 
		m := menus;
		WHILE m # NIL DO
			IF ((m.type = "") OR (m.type = currType)) & ~m.isPopup THEN
				m.mi := Gtk.gtk_menu_get_attach_widget(m.submenu)(Gtk.GtkMenuItem);
				IF m.mi = NIL THEN
					NewMenuItem(m.menu, m.mi);
					res := GtkU.gtk_signal_connect(m.mi, "activate", SYSTEM.ADR(MenuActivate),SYSTEM.VAL(INTEGER,m.submenu));
					Gtk.gtk_menu_item_set_submenu(m.mi,  m.submenu);
				ELSE
					IF m.mi.parent # NIL THEN
						Gtk.gtk_container_remove(m.mi.parent(Gtk.GtkContainer), m.mi)
					END;
				END;
				Gtk.gtk_menu_shell_append(menuBar, m.mi)
			ELSE
				Gtk.gtk_object_ref( m.submenu); 
				(* TODO: Where should the unref be done? *)
				m.mi := Gtk.gtk_menu_get_attach_widget(m.submenu)(Gtk.GtkMenuItem);
				IF m.mi # NIL THEN Gtk.gtk_menu_detach(m.submenu) END
			END;
			m := m.next
		END;
		(*
		IF oldBar # NIL THEN Gtk.gtk_container_remove(HostWindows.main, oldBar) END;
		Gtk.gtk_container_add(HostWindows.main, menuBar);
		*)
		HostWindows.SetMenu(menuBar);
		Gtk.gtk_widget_show_all(menuBar);
	END UpdateMenus;

	PROCEDURE TimerTick (notUsed0, notUsed1, notUsed2: INTEGER);
		VAR ops: Controllers.PollOpsMsg;
	BEGIN
		IF ~Log.synch THEN Log.FlushBuf END;
		HostWindows.Idle;
		Controllers.SetCurrentPath(Controllers.targetPath);
		Controllers.PollOps(ops);
		IF (ops.type # currType) & (menus # NIL) THEN
			currType := ops.type$;
			UpdateMenus
		END;
		Controllers.ResetCurrentPath()
	END TimerTick;		

	PROCEDURE [ccall] DoTimerTick (data: INTEGER): INTEGER;
		VAR ops: Controllers.PollOpsMsg;
	BEGIN
		Controllers.SetCurrentPath(Controllers.targetPath);
		Kernel.Try(TimerTick, 0, 0, 0);
		Controllers.ResetCurrentPath();
		RETURN 1
	END DoTimerTick;		

	PROCEDURE HandleVerb (n: INTEGER);
		VAR v: Views.View; dvm: Properties.DoVerbMsg;
	BEGIN
		v := Containers.FocusSingleton();
		IF v # NIL THEN
			dvm.frame := Views.ThisFrame(Controllers.FocusFrame(), v);
			dvm.verb := n;
			Views.HandlePropMsg(v, dvm)
		END
	END HandleVerb;

	PROCEDURE CheckVerb (v: Views.View; n: INTEGER; VAR pvm: Properties.PollVerbMsg);
	BEGIN
		pvm.verb := n;
		pvm.label := "";
		pvm.disabled := FALSE; pvm.checked := FALSE;
		Views.HandlePropMsg(v, pvm)
	END CheckVerb;

	PROCEDURE Exit*;
	BEGIN
		Gtk.gtk_signal_emit_by_name(HostWindows.main, "delete-event");
		IF HostCmds.quit THEN
			quit := TRUE
		END
(*		Gtk.gtk_widget_destroy(HostWindows.main)*)
	END Exit;

	PROCEDURE PopupMenu*;
		VAR f: Views.Frame;
			menu: Menu; gmenu:Gtk.GtkMenu; 
	BEGIN
		f := Controllers.FocusFrame();
		IF (f # NIL) & f.front THEN
			menu := menus;
			WHILE (menu # NIL) & (~menu.isPopup OR (menu.type # "") & (menu.type # currType)) DO 
				menu := menu.next 
			END;
			IF menu # NIL THEN 
				gmenu := menu.submenu 
			ELSE 
				gmenu := popMenu; Dialog.Beep 
			END;
			IF gmenu # NIL THEN 
				Kernel.Try(PrepareMenu, SYSTEM.ADR(gmenu^), 0, 0);
				Gtk.gtk_menu_popup(gmenu, NIL, NIL, 0, 0, 0, Gdk.GDK_CURRENT_TIME); 
				Gtk.gtk_widget_show_all(gmenu)
			END; 
		END
	END PopupMenu;

	PROCEDURE SetFocus;
		VAR c: Containers.Controller; f: Views.Frame; v, s: Views.View;
	BEGIN
		f := Controllers.FocusFrame(); v := f.view;
		WITH v: Containers.View DO
			c := v.ThisController();
			s := c.Singleton();
			IF s # NIL THEN c.SetFocus(s) END
		ELSE
		END
	END SetFocus;

	PROCEDURE OpenWindow;
		VAR c: Containers.Controller; f: Views.Frame; v, s: Views.View; doc: Documents.Document;
			win: Windows.Window; title: Views.Title;
	BEGIN
		f := Controllers.FocusFrame(); v := f.view;
		WITH v: Containers.View DO
			c := v.ThisController();
			s := c.Singleton();
			IF (s # NIL) & (s.ThisModel() # NIL) THEN
				win := Windows.dir.Focus(Controllers.frontPath); ASSERT(win # NIL, 100);
				doc := win.doc.DocCopyOf(s);
				c := doc.ThisController();
				c.SetOpts(c.opts - {Documents.pageWidth, Documents.pageHeight}
										+ {Documents.winWidth, Documents.winHeight});
				(* Stores.InitDomain(doc, v.domain); done by DocCopyOf *)
				win.GetTitle(title);
				Windows.dir.OpenSubWindow(Windows.dir.New(), doc, {Windows.isAux}, title)
			END
		ELSE
		END
	END OpenWindow;		

	PROCEDURE DispatchSpecialShortCuts (id: INTEGER);
		VAR res: INTEGER; 
	BEGIN
		Dialog.ShowStatus("");
		DEC(gc);
		CASE id OF
		| iClose: HostCmds.Close
		| iOpen: HostCmds.Open
		| iUndo: StdCmds.Undo
		| iCut: HostCmds.Cut
		| iCopy: HostCmds.Copy
		| iPaste: HostCmds.Paste
		| iProperties: StdCmds.ShowProp
		| iExit: Exit
		| iUpdateMenus: Dialog.Call("StdMenuTool.UpdateAllMenus", "", res)
		| iPopup: PopupMenu
		| iObjEdit: SetFocus
		| iObjOpen: OpenWindow
		ELSE
(*	
			(* TODO: Can this ELSE be removed? *)
			IF id < firstId THEN HandleVerb(id - iVerb0)
			ELSE
				HandleCustomMenu(id)
			END		
*)		
		END;
		Properties.IncEra;
	END DispatchSpecialShortCuts;

	(* Gtk.GtkKeySnoopFunc *)
	(* RETURN TRUE -> remove event, RETURN FALSE -> let Gtk handle the event *)

	PROCEDURE [ccall] TranslateAccelerators  (widget: Gtk.GtkWidget; event: Gdk.GdkEventKey; 
																	user_data: INTEGER): INTEGER;
		VAR m: Menu; item: Item; id, code: INTEGER; 
				ctrl, shift, alt, done: BOOLEAN; ch: CHAR;
				failed, ok: BOOLEAN; par: Dialog.Par; 
				filter: SET;
	BEGIN
		done := FALSE;
		filter := {0..5};
		IF event.type # Gdk.GDK_KEY_PRESS THEN RETURN 0 END;
		code := event.keyval; 
		id := 0; ch := 0X;
		shift := Gdk.GDK_SHIFT_BIT IN event.state; 
		ctrl := Gdk.GDK_CONTROL_BIT IN event.state;
		alt := Gdk.GDK_MOD1_BIT IN event.state;   

		IF shift & (code = Key.GDK_F10 ) THEN id := iPopup	(* shift F10 *)
		ELSIF alt & (code = Key.GDK_BackSpace)  THEN id := iUndo	(* alt bs *)
		ELSIF ctrl & (code = Key.GDK_Insert) THEN	 id := iCopy	(* ctrl insert *)
		ELSIF shift & (code = Key.GDK_Insert) THEN	id := iPaste	(* shift insert *)
		ELSIF shift & (code = Key.GDK_Delete) THEN	id := iCut	(* shift delete *)
		ELSIF alt &  (code = Key.GDK_Return)  THEN id := iProperties	(* alt enter *)
	(*			
		ELSIF ctrl & shift & (code = Key.GDK_space) THEN event.keyval := Key.GDK_nobreakspace
		ELSIF alt  & shift & (code = Key.GDK_space)  THEN event.keyval := Key.GDK_digitspace
		ELSIF ctrl & ~shift & (code = Key.GDK_minus) THEN event.keyval := Key.GDK_hyphen
	*)		
	(*	
		ELSIF alt & shift & (code = Key.GDK_minus)  event.keyval := NBHYPHEN
		ELSIF ctrl & shift & (code = Key.GDK_minus)  event.keyval := SOFTHYPHEN
	 *)
(*	ELSIF shift & (code = Key.GDK_Escape)  THEN done := TRUE	 TODO: why? *)
		END;

		(*TODO: If ch was change a new event needs to be sent, or can you change the current event? *)
		IF id # 0 THEN
			DispatchSpecialShortCuts(id);
			done := TRUE
		END;
		IF ~done THEN
			ch := CHR(code); 
			IF (ch >= "a") & (ch <= "z") THEN ch := CAP(ch) END;
			code := ORD(ch);
			IF ~alt & (ctrl OR (code >= Key.GDK_F1) & (code <= Key.GDK_F12)) THEN
				m := menus;
				WHILE (m # NIL) & ~done DO
					IF ((m.type = "") OR (m.type = currType)) & ~m.isPopup & (m.class IN filter) THEN
						item := m.firstItem;
						LOOP 
							IF item = NIL THEN EXIT END;
							IF (item.code = code) & (item.ctrl = ctrl) & (item.shift = shift) & (item.alt = alt) THEN 
								IF item.filter^ # "" THEN StdDialog.CheckFilter(item, failed, ok , par) END;
								IF (item.filter^ = "") OR ~failed & ~par.disabled THEN 
									Gtk.gtk_menu_item_activate(item.mi)
								END;
								done := TRUE;
								EXIT 
							END;
							IF item.next = NIL THEN EXIT END;
							item := item.next(Item);
						END
					END;
					m := m.next
				END
			(*  *)
			END
		END;
		IF done THEN RETURN 1 ELSE RETURN 0 END
	END TranslateAccelerators;

(*
	PROCEDURE PathToSpec (VAR path: ARRAY OF CHAR; VAR loc: Files.Locator; VAR name: Files.Name);
		VAR i, j: INTEGER; ch: CHAR;
	BEGIN
		i := 0; j := 0; loc := Files.dir.This("");
		WHILE (loc.res = 0) & (i < LEN(path) - 1) & (j < LEN(name) - 1) & (path[i] # 0X) DO
			ch := path[i]; INC(i);
			IF (j > 0) & ((ch = "/") OR (ch = "\")) THEN
				name[j] := 0X; j := 0;
				IF name = "*" THEN
					IF Dialog.language # "" THEN loc := loc.This(Dialog.language) END
				ELSE loc := loc.This(name)
				END
			ELSE
				name[j] := ch; INC(j)
			END
		END;
		IF path[i] = 0X THEN name[j] := 0X
		ELSE loc.res := 1; name := ""
		END
	END PathToSpec;

	PROCEDURE OpenFile (VAR name: ARRAY OF CHAR; l, t, r, b: INTEGER; VAR ok: BOOLEAN);
		VAR res: INTEGER; loc: Files.Locator; 
			file: Files.Name; v: Views.View; 
			conv: Converters.Converter; f: Files.File;
	BEGIN
		ok := FALSE;
		PathToSpec(name, loc, file);
		IF file # "" THEN
			f := Files.dir.Old(loc, file, Files.shared);
			IF f # NIL THEN
				conv := Converters.list;
				WHILE (conv # NIL) & (conv.fileType # f.type) DO conv := conv.next END;
				IF conv = NIL THEN
					conv := Converters.list;
					WHILE (conv # NIL) & ~(impAll IN conv.opts) DO conv := conv.next END
				END;
				IF f.type = "" THEN file := file + "." END;
				v := Views.Old(Views.dontAsk, loc, file, conv);
				IF v # NIL THEN
					Windows.dir.l := l; Windows.dir.t := t; Windows.dir.r := r; Windows.dir.b := b;
					Views.Open(v, loc, file, conv); ok := TRUE;
					Windows.dir.l := 0; Windows.dir.t := 0; Windows.dir.r := 0; Windows.dir.b := 0
				END
			END
		END
	END OpenFile;

	PROCEDURE IncludingFileCommandLine(IN line: ARRAY OF CHAR): POINTER TO ARRAY OF CHAR;
		VAR f: Files.File; r: Files.Reader; i, len: INTEGER; 
			header: ARRAY 12 OF BYTE; keyword: ARRAY 12 OF CHAR;
			b: POINTER TO ARRAY OF BYTE;
			l2: POINTER TO ARRAY OF CHAR;
	BEGIN
		len := LEN(line$);
		f := Files.dir.Old(Files.dir.This(cmdLinePath), cmdLineFile, Files.shared);
		IF (f # NIL) & (f.Length() > LEN(header)) THEN
			r := f.NewReader(NIL); r.ReadBytes(header, 0, LEN(header));
			FOR i := 0 TO LEN(header) - 1 DO keyword[i] := CHR(header[i]) END;
			keyword[LEN(keyword) - 1] := 0X;
			IF keyword = 'COMMANDLINE' THEN
				NEW(b, f.Length() - LEN(header)); NEW(l2, LEN(b) + len + 1);
				r.ReadBytes(b, 0, LEN(b));
				FOR i := 0 TO len - 1 DO l2[i] := line[i] END; l2[i] := " ";
				FOR i := 0 TO LEN(b) - 1 DO l2[i + len + 1] := SHORT(CHR(b[i])) END;
				RETURN l2
			END
		END;
		NEW(l2, len); 
		FOR i := 0 TO len - 1 DO l2[i] := line[i] END;
		RETURN l2
	END IncludingFileCommandLine;

	PROCEDURE ReadCommandLine (IN line: ARRAY OF CHAR; open: BOOLEAN);
		VAR name, opt: ARRAY 260 OF CHAR; i, l, t, r, b, res: INTEGER;
			ok: BOOLEAN; ln: ARRAY 260 OF CHAR;

		PROCEDURE CopyName;
			VAR ch, tch: CHAR; j: INTEGER;
		BEGIN
			j := 0; ch := line[i]; tch := " ";
			WHILE ch = " " DO INC(i); ch := line[i] END;
			IF (ch = "'") OR (ch = '"') THEN tch := ch; INC(i); ch := line[i] END;
			WHILE (ch >= " ") & (ch # tch) DO
				name[j] := ch;
				IF (ch >= "a") & (ch <= "z") OR (ch >= "à") & (ch <= "ö") OR (ch >= "ø") & (ch <= "þ") THEN ch := CAP(ch)
				ELSIF ch = "-" THEN ch := "/"
				END;
				opt[j] := ch; INC(j); INC(i); ch := line[i]
			END;
			IF ch > " " THEN INC(i); ch := line[i] END;
			WHILE (ch # 0X) & (ch <= " ") DO INC(i); ch := line[i] END;
			name[j] := 0X; opt[j] := 0X
		END CopyName;

	BEGIN
		l := 0; t := 0; r := 0; b := 0; i := 0;
		CopyName;	(* skip program name *)
		WHILE line[i] > " " DO
			CopyName;
			IF opt = "/LOAD" THEN	(* load module *)
				CopyName; ln := name$;
				IF open THEN Kernel.LoadMod(ln) END
			ELSIF opt = "/USE" THEN	(* use directory *)
				CopyName	(* working directory: handled in HostFiles *)

			ELSIF opt = "/PT" THEN	(* print file to printer *)
				CopyName; CopyName; CopyName; CopyName	(* to be completed !!! *)
	(*
			ELSIF opt = "/EMBEDDING" THEN	(* start as server *)
				IF ~open THEN state := embedded END
			ELSIF opt = "/NOAPPWIN" THEN	(* start without application window *)
				IF ~open THEN state := noAppWin; HostWindows.noAppWin := TRUE END
			ELSIF opt = "/NOSCROLL" THEN	(* no scroll bars in  application window *)
				HostWindows.noClientScroll := TRUE
			ELSIF opt = "/FULLSIZE" THEN
				HostWindows.fullSize := TRUE
	*)	
			ELSIF opt = "/LTRB" THEN	(* window position *)
				CopyName; ln := name$; Strings.StringToInt(ln, l, res);
				CopyName; ln := name$; Strings.StringToInt(ln, t, res);
				CopyName; ln := name$; Strings.StringToInt(ln, r, res);
				CopyName; ln := name$; Strings.StringToInt(ln, b, res)
			ELSIF opt = "/LANG" THEN
				CopyName; ln := name$;
				IF LEN(ln$) = 2 THEN 
					Strings.ToLower(ln, ln); 
					Dialog.SetLanguage(ln$, Dialog.nonPersistent) 
				END
			ELSIF opt = "/O" THEN	(* open file *)
				CopyName; (*openUsed := TRUE;*)
				IF open THEN OpenFile(name, l, t, r, b, ok) END;
				l := 0; t := 0; r := 0; b := 0
			ELSIF opt = "/PAR" THEN
				CopyName;
				Dialog.commandLinePars := name$
			ELSE	(* open file *)
				IF open THEN OpenFile(name, l, t, r, b, ok) END;
				l := 0; t := 0; r := 0; b := 0
			END
		END
	END ReadCommandLine;
*)

	PROCEDURE [ccall] Quit (object: Gtk.GtkObject; func_data: INTEGER);
	BEGIN
		quit := TRUE
	END Quit;

	PROCEDURE TryQuit (notUsed0, notUsed1, notUsed2: INTEGER);
	BEGIN
		HostCmds.Quit
	END TryQuit;

	(* retrun 0 -> close ok. return 1 -> don't close *)
	PROCEDURE [ccall] DeleteHandler(widget: Gtk.GtkWidget; event: Gdk.GdkEvent; user_data: INTEGER): INTEGER;
	BEGIN
		Controllers.SetCurrentPath(Controllers.targetPath);
		Kernel.Try(TryQuit, 0, 0, 0);
		IF HostCmds.quit THEN
			(*	HostWindows.SaveWindowState; TODO: implement*)
			Controllers.ResetCurrentPath();
			RETURN 0
		ELSE 
			gc := 0;
			Controllers.ResetCurrentPath();
			RETURN 1
		END
	END DeleteHandler;

	PROCEDURE [ccall] StyleSet (widget: Gtk.GtkWidget; previous_style: Gtk.GtkStyle; user_data: INTEGER);
	BEGIN
		HostPorts.ResetColors(widget)
	END StyleSet;

	PROCEDURE OpenApp*;
		VAR res: INTEGER; 
	BEGIN
		HostWindows.CreateMainWindows;
		res := GtkU.gtk_signal_connect(HostWindows.main, "destroy", SYSTEM.ADR(Quit), 0);
		res := GtkU.gtk_signal_connect(HostWindows.main, "delete-event",SYSTEM.ADR(DeleteHandler), 0);
		res := GtkU.gtk_signal_connect(HostWindows.main, "style-set", SYSTEM.ADR(StyleSet), 0);

		(* copy/paste functionallity (selections) *)
		Gtk.gtk_selection_add_target(HostWindows.main, Gdk.GDK_SELECTION_PRIMARY, Gdk.GDK_TARGET_STRING, Gdk.GDK_SELECTION_TYPE_STRING);
		Gtk.gtk_selection_add_target(HostWindows.main, Gdk.GDK_SELECTION_PRIMARY, HostClipboard.bbAtom, HostClipboard.bbAtom);

		res := GtkU.gtk_signal_connect(HostWindows.main, "selection-received", SYSTEM.ADR(HostClipboard.DoPaste), 0);
		res := GtkU.gtk_signal_connect(HostWindows.main, "selection-get", SYSTEM.ADR(HostClipboard.ConvertCopy), 0);
		res := GtkU.gtk_signal_connect(HostWindows.main, "selection-clear-event", SYSTEM.ADR(HostClipboard.Clear), 0);

		(* timer ticks *)
		res := GLib.g_timeout_add_full(GLib.G_PRIORITY_DEFAULT, idlePeriod, DoTimerTick, 0, NIL);
		(* install a keysnooper to handle shortcuts *)
		res := Gtk.gtk_key_snooper_install(TranslateAccelerators, 0)
	END OpenApp;

(*
	PROCEDURE Loop;
		VAR res, n: INTEGER;
	BEGIN
		HostWindows.ShowMain;
		quit := FALSE;
		gc := gcCycle;
		n := 0;
		WHILE ~quit DO
			Services.actionHook.Loop;
			INC(n);
			IF (n > num) OR (~Gtk.gtk_events_pending()) THEN 
				Windows.dir.Update(NIL); n := 0;
			ELSE
				res := Gtk.gtk_main_iteration()
			END;
			IF gc <= 0 THEN
				Kernel.Collect;
				gc := gcCycle
			END;
		END;
		Kernel.Quit(0)
	END Loop;
*)

(*
	PROCEDURE Loop;
		VAR res, n: INTEGER;
	BEGIN
		HostWindows.ShowMain;
		quit := FALSE;
		gc := gcCycle;
		n := 0;
		WHILE ~quit DO
			Services.actionHook.Loop;
			INC(n);
			IF (n > num) OR (~Gtk.gtk_events_pending()) THEN 
				Windows.dir.Update(NIL); n := 0;
			ELSE
				res := Gtk.gtk_main_iteration()
			END;
			IF gc <= 0 THEN
				(* Kernel.Collect; *)
				gc := gcCycle
			END;
		END;
		Kernel.Quit(0)
	END Loop;
*)

	PROCEDURE [ccall] F (data: GLib.gpointer): GLib.gboolean;
	BEGIN
		Services.actionHook.Loop; (* immediates *)

		Windows.dir.Update(NIL);

		IF quit THEN
			Gtk.gtk_main_quit;
			RETURN 0 (* FALSE: terminate *)
		ELSE
			RETURN 1 (* TRUE: continue *)
		END
	END F;

	PROCEDURE Loop;
		VAR res: INTEGER;
	BEGIN
		IF ~sourcesAdded THEN (* do not add sources after TRAP *)
			HostWindows.ShowMain;
			quit := FALSE;

			(* too high CPU load *)
			(* res := GLib.g_idle_add_full(
				GLib.G_PRIORITY_LOW + 100, F, 0 (* data *), NIL); *)

			(* seems OK if Kernel.Time based on clock_gettime() instead of clock() *)
			res := GLib.g_timeout_add_full(
				GLib.G_PRIORITY_HIGH, 5 (* ms *), F, 0 (* data *), NIL);

			sourcesAdded := TRUE
		END;
		IF ~quit THEN
			Gtk.gtk_main
		END
	END Loop;

	PROCEDURE Run*;
	BEGIN
(*	ReadCommandLine(IncludingFileCommandLine(Kernel.cmdLine), TRUE);
		ReadCommandLine(Kernel.cmdLine, TRUE); *)
		Kernel.Start(Loop)
	END Run;

	PROCEDURE SetNum* (n: INTEGER);
	BEGIN
		num := n
	END SetNum;

	PROCEDURE PrimaryVerb*;
		VAR v: Views.View; pvm: Properties.PollVerbMsg;
	BEGIN
		v := Containers.FocusSingleton();
		IF v # NIL THEN
			CheckVerb(v, 0, pvm);
			IF pvm.label # "" THEN HandleVerb(0)
			ELSE  SetFocus
			END
		END
	END PrimaryVerb;

	PROCEDURE Init;
	BEGIN
		sourcesAdded := FALSE;
		num := 10;
		popMenu := NIL;
	END Init;

BEGIN
	Init
(* TODO: do we need a close section to clean up the menus? *)
END HostMenus.
