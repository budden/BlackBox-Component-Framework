MODULE HostCmds;
(**
	project	= "BlackBox"
	organization	= "www.oberon.ch"
	contributors	= "Oberon microsystems"
	version	= "System/Rsrc/About"
	copyright	= "System/Rsrc/About"
	license	= "Docu/BB-License"
	changes	= ""
	issues	= ""

	(*
		TODO: Many commands are commented out for the Linux port. Make them available. 
	*)

	(* bj	06.03.01	Changed the gap for headers in PrintDoc from 10*Ports.mm to 5*Ports.mm *)
	(* bj	26.02.01	Changed Open and SaveWindow to handle file names which are longer than Views.Title *)
*)
	(* bj	31.10.00	Moved OpenStationary here from StdCmds. Renamed it to OpenCopyOf *)
	(* ww	07.02.00	added SaveAll command *)
	(* dg	06.05.99	changes due to interface change of BeginModification/EndModification *)

	IMPORT
		Kernel, Ports, Printers, Files,
		Stores, Views, Controllers, Dialog, (*Printing,*)
		Converters, Sequencers, Documents, Windows,
		StdDialog, StdCmds,
		HostFiles, HostWindows, HostDialog, HostClipboard;

	CONST
		(* hints *)
		impAll = 0;	(* can import all file types *)
		expDoc = 1;	(* imported view should be stored as document *)

	VAR quit*: BOOLEAN;


	(** File menu **)

	PROCEDURE Open*;
	(** open an existing document or view **)
		VAR loc: Files.Locator; name: Files.Name; v: Views.View;
			s: Stores.Store; done: BOOLEAN; w: Windows.Window; conv: Converters.Converter;
	BEGIN
		HostDialog.GetIntSpec(loc, name, conv);
		IF loc # NIL THEN
			w := Windows.dir.First();
			WHILE (w # NIL)
				& ((w.loc = NIL) OR (w.name = "") OR (w.loc.res = 77)
					OR	~Files.dir.SameFile(loc, name, w.loc, w.name)
					OR (w.conv # conv)) DO
				w := Windows.dir.Next(w)
			END;
			IF w # NIL THEN
				s := w.doc
			ELSE
				Converters.Import(loc, name, conv, s);
				IF s # NIL THEN StdDialog.RecalcView(s(Views.View)) END
			END;
			IF s # NIL THEN
				v := s(Views.View);
				IF (conv # NIL) & (expDoc IN conv.opts) THEN conv := NIL END;
				Views.Open(v, loc, name, conv)
			ELSE
				Dialog.ShowParamMsg("#System:FailedToOpen", name, "", "")
			END
		END
	END Open;

	PROCEDURE OpenCopyOf*;
		VAR loc: Files.Locator; name: Files.Name; conv: Converters.Converter;
				v: Views.View;
	BEGIN
		v := Views.Old(TRUE, loc, name, conv);
		IF v # NIL THEN
			IF v.context # NIL THEN
				v := Views.CopyOf(v.context(Documents.Context).ThisDoc(), Views.deep);
				Stores.InitDomain(v)
			ELSE v := Views.CopyOf(v, Views.deep)
			END;
			Views.Open(v, NIL, "", conv)
		END
	END OpenCopyOf;

	PROCEDURE SaveWindow (w: Windows.Window; rename: BOOLEAN);
		VAR title: Views.Title; loc: Files.Locator; name: Files.Name; v: Views.View;
			conv: Converters.Converter; stat: BOOLEAN; i: INTEGER;
	BEGIN
		IF (w # NIL) & (rename OR ~(Windows.neverDirty IN w.flags)) THEN
			v := w.doc.OriginalView();
			loc := w.loc; name := w.name$; conv := w.conv;
			IF name = "" THEN Dialog.MapString("#System:untitled", name) END;
			IF (loc = NIL) OR (loc.res = 77) OR (conv # NIL) & (conv.exp = "") THEN
				rename := TRUE 
			END;
			IF rename THEN HostDialog.GetExtSpec(v, loc, name, conv) END;
			IF loc # NIL THEN
				Dialog.ShowStatus("#Host:Saving");
				Converters.Export(loc, name, conv, v);
				IF loc.res = 0 THEN
					IF w.seq.Dirty() THEN
						w.seq.BeginModification(Sequencers.notUndoable, NIL);
						w.seq.EndModification(Sequencers.notUndoable, NIL);	(* clear sequencer *)
						w.seq.SetDirty(FALSE)
					END;
					IF rename THEN
						i := 0;
						WHILE (i < LEN(title) - 1) & (name[i] # 0X) DO title[i] := name[i]; INC(i) END;
						title[i] := 0X;
						w.SetTitle(title); w.SetSpec(loc, name, conv)
					END;
					Dialog.ShowStatus("#Host:Saved")
				ELSE
					Dialog.ShowStatus("#Host:Failed")
				END
			END;
			IF ~quit THEN Kernel.Cleanup END
		END
	END SaveWindow;

	PROCEDURE Save*;
	(** save document shown in target window under old name **)
		VAR w: Windows.Window;
	BEGIN
		w := Windows.dir.Focus(Controllers.targetPath);
		SaveWindow(w, FALSE)
	END Save;

	PROCEDURE SaveAs*;
	(** save document shown in target window under new name **)
		VAR w: Windows.Window;
	BEGIN
		w := Windows.dir.Focus(Controllers.targetPath);
		SaveWindow(w, TRUE)
	END SaveAs;

	PROCEDURE SaveCopyAs*;
	(** save copy of document shown in target window under new name **)
		VAR w: Windows.Window; loc: Files.Locator; name: Files.Name; v: Views.View;
			conv: Converters.Converter;
	BEGIN
		w := Windows.dir.Focus(Controllers.targetPath);
		IF (w # NIL) THEN
			v := w.doc.OriginalView();
			loc := w.loc; name := w.name$; conv := w.conv;
			IF name = "" THEN Dialog.MapString("#System:untitled", name) END;
			HostDialog.GetExtSpec(v, loc, name, conv);
			IF loc # NIL THEN
				Dialog.ShowStatus("#Host:Saving");
				Converters.Export(loc, name, conv, v);
				IF loc.res = 0 THEN Dialog.ShowStatus("#Host:Saved")
				ELSE Dialog.ShowStatus("#Host:Failed")
				END
			END;
			IF ~quit THEN Kernel.Cleanup END
		END
	END SaveCopyAs;

	PROCEDURE CloseWindow (w: Windows.Window);
		VAR res: INTEGER; msg: Sequencers.CloseMsg;
	BEGIN
		IF w # NIL THEN
			IF ~w.sub THEN
				msg.sticky := FALSE; w.seq.Notify(msg);
				IF ~msg.sticky THEN
					IF w.seq.Dirty() & ~(Windows.neverDirty IN w.flags) THEN
						HostDialog.CloseDialog(w, quit, res);
						IF res = HostDialog.save THEN
							SaveWindow(w, FALSE);	(* if saving is canceled, document remains dirty *)
							IF w.seq.Dirty() THEN quit := FALSE
							ELSE Windows.dir.Close(w)
							END
						ELSIF res = HostDialog.cancel THEN quit := FALSE
						ELSE Windows.dir.Close(w)
						END
					ELSE Windows.dir.Close(w)
					END
				ELSE quit := FALSE
				END
			ELSE Windows.dir.Close(w)
			END;
			IF ~quit THEN Kernel.Cleanup END
		END
	END CloseWindow;

	PROCEDURE Close*;
	(** close top window **)
	BEGIN
		CloseWindow(Windows.dir.First())
	END Close;


	PROCEDURE Print*;
	BEGIN
		(* empty dummy implementation *)
	END Print;


	PROCEDURE Quit*;
	(** stop if all windows can be closed **)
		VAR w: Windows.Window;
	BEGIN
		quit := TRUE;
		w := Windows.dir.First();
(*		WHILE (w # NIL) & (HostWindows.inPlace IN w.flags) DO w := Windows.dir.Next(w) END;*)
		WHILE (w # NIL) & quit DO
			CloseWindow(w);
			w := Windows.dir.First();
(*			WHILE (w # NIL) & (HostWindows.inPlace IN w.flags) DO w := Windows.dir.Next(w) END*)
		END
	END Quit;

(*
	PROCEDURE SaveAll*;
		VAR w: Windows.Window; res: INTEGER;
	BEGIN
		quit := FALSE;
		w := Windows.dir.First();
		WHILE (w # NIL) & (HostWindows.inPlace IN w.flags) DO w := Windows.dir.Next(w) END;
		res := HostDialog.save;
		WHILE (w # NIL) & (res # HostDialog.cancel) DO
			IF ~w.sub & w.seq.Dirty() & ~(Windows.neverDirty IN w.flags) THEN
				HostDialog.CloseDialog(w, FALSE, res);
				IF res = HostDialog.save THEN
					SaveWindow(w, FALSE)	(* if saving is canceled, document remains dirty *)
				END;
				Kernel.Cleanup
			END;
			w := Windows.dir.Next(w)
		END
	END SaveAll;
*)
	(** Edit menu **)

	PROCEDURE Cut*;
	(** move the focus document's selection into the clipboard **)
		VAR msg: Controllers.EditMsg;
	BEGIN
		msg.op := Controllers.cut;
		msg.clipboard := TRUE;
		msg.view := NIL; msg.w := Views.undefined; msg.h := Views.undefined;
		Controllers.Forward(msg);
		IF msg.view # NIL THEN HostClipboard.Register(msg.view, msg.w, msg.h, msg.isSingle) END
	END Cut;

	PROCEDURE Copy*;
	(** move a copy of the focus document's selection into the clipboard **)
		VAR msg: Controllers.EditMsg;
	BEGIN
		msg.op := Controllers.copy;
		msg.clipboard := TRUE;
		msg.view := NIL; msg.w := Views.undefined; msg.h := Views.undefined;
		Controllers.Forward(msg);
		IF msg.view # NIL THEN HostClipboard.Register(msg.view, msg.w, msg.h, msg.isSingle) END
	END Copy;

	PROCEDURE Paste*;
	(** let focus document insert a copy of the clipboard's contents **)
		VAR ops: Controllers.PollOpsMsg; msg: Controllers.EditMsg;
			res: INTEGER;
	BEGIN
		HostClipboard.InitPaste;

		(*
		HostClipboard.cloneAttributes := TRUE;
		HostClipboard.isText := TRUE;
		Controllers.PollOps(ops);
		HostClipboard.isText := ops.type = "TextViews.View";
		IF Controllers.paste IN ops.valid THEN
			msg.clipboard := TRUE;
			HostClipboard.GetClipView(ops.pasteType, msg.view, msg.w, msg.h, msg.isSingle);
			IF msg.view # NIL THEN
				msg.op := Controllers.paste; Controllers.Forward(msg)
			END
		END;
		HostClipboard.cloneAttributes := FALSE;
		HostClipboard.isText := TRUE;
		*)
	END Paste;

	PROCEDURE PasteObject*;
	(** let focus document insert a copy of the clipboard's contents **)
		VAR ops: Controllers.PollOpsMsg; v: Views.View; w, h: INTEGER; s: BOOLEAN;
	BEGIN
		Dialog.ShowMsg("PasteObject not implemented yet");
		(*
		HostClipboard.cloneAttributes := FALSE;
		Controllers.PollOps(ops);
		IF Controllers.paste IN ops.valid THEN
			HostClipboard.GetClipView(ops.pasteType, v, w, h, s);
			IF v # NIL THEN
				Controllers.PasteView(v, w, h, TRUE)
			END
		END
		*)
	END PasteObject;

	PROCEDURE PasteToWindow*;
		VAR v: Views.View; w, h: INTEGER; d: Documents.Document; s: BOOLEAN;
	BEGIN
		Dialog.ShowMsg("PasteToWindow not implemented yet");
		(*
		HostClipboard.cloneAttributes := FALSE;
		HostClipboard.GetClipView("", v, w, h, s);
		IF v # NIL THEN
			d := Documents.dir.New(v, w, h);
			Views.OpenView(d)
		END
		*)
	END PasteToWindow;



	PROCEDURE OpenDoc* (file: ARRAY OF CHAR);
		VAR w: Windows.Window;
	BEGIN
		w := Windows.dir.Focus(FALSE);
		IF (w.loc # NIL) & (w.loc IS HostFiles.Locator) & (w.loc(HostFiles.Locator).path # "") THEN
			StdCmds.OpenDoc(w.loc(HostFiles.Locator).path + "\" + file)
		ELSE
			StdCmds.OpenDoc(file)
		END
	END OpenDoc;


	(* Guards *)

	PROCEDURE SaveGuard* (VAR par: Dialog.Par);
		VAR w: Windows.Window;
	BEGIN
		w := Windows.dir.Focus(Controllers.targetPath);
		IF (w = NIL) OR (Windows.neverDirty IN w.flags) OR ~w.seq.Dirty() THEN par.disabled := TRUE END
	END SaveGuard;

	PROCEDURE PrintGuard* (VAR par: Dialog.Par);
		VAR w: Windows.Window;
	BEGIN
		w := Windows.dir.Focus(Controllers.targetPath);
		IF (w = NIL) OR (Printers.dir = NIL) OR ~Printers.dir.Available() THEN 
			par.disabled := TRUE
		END
	END PrintGuard;

	PROCEDURE PrinterGuard* (VAR par: Dialog.Par);
	BEGIN
		IF ~Printers.dir.Available() THEN par.disabled := TRUE END
	END PrinterGuard;

	PROCEDURE CutGuard* (VAR par: Dialog.Par);
		VAR ops: Controllers.PollOpsMsg;
	BEGIN
		Controllers.PollOps(ops);
		IF ~(Controllers.cut IN ops.valid) THEN par.disabled := TRUE END
	END CutGuard;

	PROCEDURE CopyGuard* (VAR par: Dialog.Par);
		VAR ops: Controllers.PollOpsMsg;
	BEGIN
		Controllers.PollOps(ops);
		IF ~(Controllers.copy IN ops.valid) THEN par.disabled := TRUE END
	END CopyGuard;

	PROCEDURE PasteGuard* (VAR par: Dialog.Par);
	BEGIN
	(*
		VAR ops: Controllers.PollOpsMsg;
	BEGIN
		Controllers.PollOps(ops);
		IF ~(Controllers.paste IN ops.valid)
			OR ~HostClipboard.ConvertibleTo(ops.pasteType) THEN par.disabled := TRUE END
	*)
	END PasteGuard;

(*
	PROCEDURE PasteObjectGuard* (VAR par: Dialog.Par);
		VAR ops: Controllers.PollOpsMsg;
	BEGIN
		Controllers.PollOps(ops);
		IF ~(Controllers.paste IN ops.valid)
			OR ~HostClipboard.ConvertibleTo("") THEN par.disabled := TRUE END
	END PasteObjectGuard;

	PROCEDURE PasteToWindowGuard* (VAR par: Dialog.Par);
	BEGIN
		IF ~HostClipboard.ConvertibleTo("") THEN par.disabled := TRUE END
	END PasteToWindowGuard;
*)
BEGIN
	quit := FALSE
END HostCmds.

