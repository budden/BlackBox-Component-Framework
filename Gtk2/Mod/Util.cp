MODULE Gtk2Util;

	IMPORT SYSTEM, Gtk := LibsGtk;
	
	TYPE
		String = ARRAY [untagged] OF SHORTCHAR;
			

	PROCEDURE gtk_signal_connect* (object: Gtk.GtkObject; IN name: String; 
														func: Gtk.GtkSignalFunc; func_data: INTEGER): INTEGER;
	BEGIN
		RETURN Gtk.gtk_signal_connect_full (object, name, func, 0, func_data, 0, 0, 0)
	END gtk_signal_connect;	
		
	PROCEDURE gtk_signal_connect_after* (object: Gtk.GtkObject; IN name: String; 
														func: Gtk.GtkSignalFunc; func_data: INTEGER): INTEGER;
	BEGIN
		RETURN Gtk.gtk_signal_connect_full (object, name, func, 0, func_data, 0, 0, 1)
	END gtk_signal_connect_after;	

	PROCEDURE [ccall] gtk_signal_handler_block_by_func* (object: Gtk.GtkObject; func: Gtk.GtkSignalFunc; data: INTEGER);
	BEGIN
		Gtk.gtk_signal_compat_matched (object, func, data, {3,4},1)
	END gtk_signal_handler_block_by_func;	

	PROCEDURE [ccall] gtk_signal_handler_unblock_by_func* (object: Gtk.GtkObject; func: Gtk.GtkSignalFunc; data: INTEGER);
	BEGIN
		Gtk.gtk_signal_compat_matched (object, func, data, {3,4},1)
	END gtk_signal_handler_unblock_by_func;	

BEGIN
	Gtk.gtk_init(NIL,NIL)
END Gtk2Util.
