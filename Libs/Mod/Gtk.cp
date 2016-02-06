MODULE LibsGtk ["libgtk-win32-2.0-0.dll"];
IMPORT SYSTEM, GLib:=LibsGlib, GObject:=LibsGObject, Pango:=LibsPango, Gdk:=LibsGdk;

	TYPE
		ADDRESS = INTEGER;
		String = ARRAY [untagged] OF SHORTCHAR;
		PString* = POINTER TO ARRAY [untagged] OF SHORTCHAR;
		StrArray =  ARRAY [untagged] OF PString;
		
		gboolean = GLib.gboolean;
		gpointer = GLib.gpointer;
		GArray = GLib.GArray;
		GSList = GLib.GSList;

		
	CONST
		(* types for calls to gtk_window_new, these are the possible values of GtkWindowType *)
		GTK_WINDOW_TOPLEVEL* =0;
		GTK_WINDOW_DIALOG* = 1;
		GTK_WINDOW_POPUP* = 2;
		
		(* GTK_STATE_TYPE enum *)
		GTK_STATE_NORMAL* = 0;
		GTK_STATE_ACTIVE* = 1;
		GTK_STATE_PRELIGHT* = 2;
		GTK_STATE_SELECTED* = 3;
		GTK_STATE_INSENSITIVE* = 4;

		(* GtkAccelFlags enum *)
		GTK_ACCEL_VISIBLE* = 0; (* should the accelerator appear in the widget's display? *)
		GTK_ACCEL_SIGNAL_VISIBLE* = 1; (* should the signal associated with this accelerator be also visible? *)
		GTK_ACCEL_LOCKED* = 2; (* may the accelerator be removed again? *)
		GTK_ACCEL_MASK* = {0, 1, 2};
		
		(* Values for GtkWindowPosition *)
		GTK_WIN_POS_NONE* = 0;
		GTK_WIN_POS_CENTER* = 1;
		GTK_WIN_POS_MOUSE* = 2;
		
		(* Values for GtkSelectionMode *) 
		GTK_SELECTION_SINGLE* = 0;
		GTK_SELECTION_BROWSE* = 1;
		GTK_SELECTION_MULTIPLE* = 2;
		GTK_SELECTION_EXTENDED* = 3;

		(* Values for GtkPolicyType *)
		GTK_POLICY_ALWAYS* = 0;
		GTK_POLICY_AUTOMATIC* = 1;
		GTK_POLICY_NEVER* = 2;

		(* Values for GtkJustification *)
		GTK_JUSTIFY_LEFT* = 0;
		GTK_JUSTIFY_RIGHT* = 1;
		GTK_JUSTIFY_CENTER* = 2;
		GTK_JUSTIFY_FILL* = 3;
		
		(* Values for GtkRcFlags *)
		GTK_RC_FG* = 0;
		GTK_RC_BG* = 1;
		GTK_RC_TEXT* = 2;
		GTK_RC_BASE* = 3;
		
		(* Values for GtkShadowType *) 
		GTK_SHADOW_NONE* = 0;
		GTK_SHADOW_IN* = 1;
		GTK_SHADOW_OUT* = 2;
		GTK_SHADOW_ETCHED_IN* = 3;
		GTK_SHADOW_ETCHED_OUT* = 4;

	CONST		
		GTK_DIALOG_MODAL ={0};
		GTK_DIALOG_DESTROY_WITH_PARENT = {1};
		GTK_DIALOG_NO_SEPARATOR = {2} ;
	CONST
		GTK_MESSAGE_INFO*=0;
		GTK_MESSAGE_WARNING*=1;
		GTK_MESSAGE_QUESTION*=2;
		GTK_MESSAGE_ERROR*=3;
		
	CONST		
		GTK_BUTTONS_NONE* =0;
		GTK_BUTTONS_OK*=1;
		GTK_BUTTONS_CLOSE*=2;
		GTK_BUTTONS_CANCEL*=3;
		GTK_BUTTONS_YES_NO*=4;
		GTK_BUTTONS_OK_CANCEL*=5;
		
	CONST		
     GTK_RESPONSE_NONE* = -1;
     GTK_RESPONSE_REJECT* = -2;
     GTK_RESPONSE_ACCEPT* = -3;
     GTK_RESPONSE_DELETE_EVENT* = -4;
     GTK_RESPONSE_OK*     = -5;
     GTK_RESPONSE_CANCEL* = -6;
     GTK_RESPONSE_CLOSE*  = -7;
     GTK_RESPONSE_YES*    = -8;
     GTK_RESPONSE_NO*     = -9;
     GTK_RESPONSE_APPLY  = -10;
     GTK_RESPONSE_HELP   = -11;

	TYPE 
		GtkMessageType*=INTEGER;
		GtkButtonsType*=INTEGER;
		GtkPolicyType* = INTEGER;
		GtkSelectionMode* = INTEGER;
		GtkShadowType* = INTEGER;
		GtkStateType* = INTEGER;
		GtkWindowPosition* = INTEGER;
		GtkWindowType* = INTEGER;
		GtkAccelFlags* = SET;
		GtkJustification* = INTEGER;
		GtkRcFlags* = SET;

		TextLenProc* = PROCEDURE [ccall]  (text: GtkWidget): INTEGER;
		
	TYPE 
		GtkColors* = ARRAY [untagged] 4 OF REAL;
		
	TYPE 
		GtkSelectionData* = RECORD [untagged]
			selection*, target*, type*: Gdk.GdkAtom;
			format*: INTEGER;
			data*: PString;
			length*: INTEGER;
			display: Gdk.GdkDisplay;
		END; 

		GtkRequisition* = RECORD [noalign] width*, height*: INTEGER END;
		GtkAllocation* =  RECORD [noalign] x*, y*, width*, height*: INTEGER END;

		GtkCTreeNode* = POINTER TO RECORD [untagged] list: GLib.GList END;
		
		
		GtkAccelGroup* = POINTER TO  RECORD (GObject.GObject)
			lock_count*: INTEGER;
			modifier_mask*: Gdk.GdkModifierType;
			acceleratables*: GSList;
			n_accels:INTEGER;
			priv_accels: INTEGER (*^GtkAccelGroupEntry*)
		END;

		GtkRcStyle* = POINTER TO RECORD (GObject.GObject)
			name* : PString;
			bg_pixmap_name*: ARRAY [untagged] 5 OF PString;
			font_desc*: Pango.PangoFontDescription;
			
			color_flags*: ARRAY [untagged] 5 OF GtkRcFlags;
			fg*, bg*, text*, base*: ARRAY [untagged] 5 OF Gdk.GdkColor;
			xthickness,ythickness :INTEGER;
  			rc_properties:  GArray;
			rc_style_lists:GSList;
			icon_factories:GSList;
			engine_specified:INTEGER
		END;


		GtkStyle* = POINTER TO RECORD (GObject.GObject)
			fg*, bg*, light*, dark*, mid*, text*, base*, text_aa : ARRAY 5 OF Gdk.GdkColor;
			black*, white*: Gdk.GdkColor;
			font_desc*: Pango.PangoFontDescription;
			xthickness,ythickness :INTEGER;
			fg_gc*, bg_gc*, light_gc*, dark_gc*, mid_gc*, text_gc*, base_gc*, text_aa_gc: ARRAY [untagged] 5 OF Gdk.GdkGC;
			black_gc*, white_gc*: Gdk.GdkGC;
			bg_pixmap*: POINTER TO ARRAY [untagged] 5 OF Gdk.GdkPixmap;
			
			attach_count:INTEGER;
			depth:INTEGER;
			colormap:Gdk.GdkColormap; 
			private_font: Gdk.GdkFont;
			private_font_desc: Pango.PangoFontDescription; 
			rc_style: GtkRcStyle;
			styles:GSList;
			property_cache:  GArray;
			icon_factories:  GSList;
		END;
		GtkWindowGroup = POINTER TO RECORD (GObject.GObject) END;

		GtkSignalFunc* = ADDRESS; (* PROCEDURE [ccall] (object: GtkObject; ... ; func_data: Ptr); *)
		GtkCallBack* = PROCEDURE [ccall] (widget: GtkWidget; data: gpointer);
		GtkFunction* = PROCEDURE [ccall]  (data: gpointer): INTEGER;
		GtkKeySnoopFunc* = PROCEDURE [ccall] (widget: GtkWidget; event: Gdk.GdkEventKey; user_data: gpointer): INTEGER;
		GtkMenuDetachFunc* = ADDRESS;
		GtkCallbackMarshal  = ADDRESS;
		GtkDestroyNotify  = ADDRESS;

(* Object hierarchy *)
		GtkObject* = POINTER TO LIMITED RECORD (GObject.GObject)
			flags*: SET; (* GtkObject only uses 4 of these bits and GtkWidget uses the rest. *)
		END;
		
		GtkAdjustment* = POINTER TO LIMITED RECORD (GtkObject)
			lower*, upper*, value*, 
			step_increment*, page_increment*, page_size*: REAL 
		END;
		
		GtkWidget* = POINTER TO LIMITED RECORD (GtkObject)
			private_flags*: SHORTINT;    
			state*: BYTE;     (* see GTK_STATE_TYPE enum above *)
			saved_state*: BYTE;   
			name*: PString;  
			style*: GtkStyle;    
			requisition*: GtkRequisition;    
			allocation*: GtkAllocation;    
			window*: Gdk.GdkWindow;  
			parent*: GtkWidget
		END;
		
			GtkDrawingArea* = POINTER TO LIMITED RECORD (GtkWidget) 
				draw_data: gpointer 
			END;
				GtkCurve = POINTER TO LIMITED RECORD (GtkDrawingArea) END;
				
			GtkEntry* = POINTER TO LIMITED RECORD (GtkWidget) 
					text*:PString;
					editable*: SET; (* editable : 1; visible  : 1; overwrite_mode : 1; in_drag : 1 *)
					(* ... *)
			END;
				GtkSpinButton* = POINTER TO LIMITED RECORD (GtkEntry) 
				
				END;
				
			GtkRange* = POINTER TO LIMITED RECORD (GtkWidget) END;
				GtkScrollbar* = POINTER TO LIMITED RECORD (GtkRange) END;
					GtkHScrollbar* = POINTER TO LIMITED RECORD (GtkScrollbar) END;
					GtkVScrollbar* = POINTER TO LIMITED RECORD (GtkScrollbar) END;
				GtkScale = POINTER TO LIMITED RECORD (GtkRange) END;

			GtkRuler = POINTER TO LIMITED RECORD (GtkWidget) END;
					GtkHRuler= POINTER TO LIMITED RECORD (GtkRuler) END;
					GtkVRuler= POINTER TO LIMITED RECORD (GtkRuler) END;
			
			GtkSeparator = POINTER TO LIMITED RECORD (GtkWidget) END;
				GtkHSeparator = POINTER TO LIMITED RECORD (GtkSeparator) END;
				GtkVSeparator = POINTER TO LIMITED RECORD (GtkSeparator) END;
				
			GtkInvisible = POINTER TO LIMITED RECORD (GtkWidget) END;
			GtkPreview = POINTER TO LIMITED RECORD (GtkWidget) END;
			GtkProgress = POINTER TO LIMITED RECORD (GtkWidget) END;
				GtkProgressBar = POINTER TO LIMITED RECORD (GtkProgress) END;
					
			GtkOldEditable* = POINTER TO LIMITED RECORD (GtkWidget) 
				current_pos*, 
				selection_start_pos*, 
				selection_end_pos*: INTEGER;
				has_selection-: SET; (*  has_selection:1; editable : 1; visible : 1*)
				clipboard_text:PString;
			END;
				GtkText*= POINTER TO LIMITED RECORD (GtkOldEditable) 
					text_area:Gdk.GdkWindow;
					hadj,vadj:GtkAdjustment;
					gc:Gdk.GdkGC ;
					line_wrap_bitmap,
					line_arrow_bitmap:Gdk.GdkPixmap;
					(* ... *)
				END;
			GtkCalendar = POINTER TO LIMITED RECORD (GtkWidget) END;
			
			GtkMisc* = POINTER TO LIMITED RECORD (GtkWidget) END;
					GtkLabel* = POINTER TO LIMITED RECORD (GtkMisc) END;
						GtkAccelLabel = POINTER TO LIMITED RECORD (GtkLabel) END;
						GtkTipsQuery = POINTER TO LIMITED RECORD (GtkLabel) END;
					GtkArrow = POINTER TO LIMITED RECORD (GtkMisc) END;
					GtkImage = POINTER TO LIMITED RECORD (GtkMisc) END;
					GtkPixmap = POINTER TO LIMITED RECORD (GtkMisc) END;

			GtkContainer* = POINTER TO LIMITED RECORD (GtkWidget)
				focus_child*: GtkWidget;
				bitFieldValues: INTEGER; 
					(* border_width: 16; need_resize: 1; 
						resize_mode: 2; reallocate_redraws: 1; *)
			END;

				GtkFixed* = POINTER TO LIMITED RECORD (GtkContainer)
				END;

				GtkBox= POINTER TO LIMITED RECORD (GtkContainer)
					chlidren: GLib.GList;
					spacing: SHORTINT;
					homogeneous: SHORTINT (*:1*)
				END;
					GtkButtonBox = POINTER TO LIMITED RECORD (GtkBox) END;
						GtkHButtonBox = POINTER TO LIMITED RECORD (GtkButtonBox) END;
						GtkVButtonBox = POINTER TO LIMITED RECORD (GtkButtonBox) END;
					GtkVBox = POINTER TO LIMITED RECORD (GtkBox) END;
						GtkColorSelection = POINTER TO LIMITED RECORD (GtkVBox) END;
						GtkFileChooserWidget = POINTER TO LIMITED RECORD (GtkVBox) END;
						GtkFontSelection = POINTER TO LIMITED RECORD (GtkVBox) END;
						GtkGammaCurve = POINTER TO LIMITED RECORD (GtkVBox) END;
					GtkHBox* = POINTER TO LIMITED RECORD (GtkBox) END;
							GtkCombo* = POINTER TO LIMITED RECORD (GtkHBox)
								entry*, 
								button, popup, popwin, 
								list*: GtkWidget;
								entry_change_id, list_change_id: INTEGER;
								someBits: SHORTINT; (**)
								current_button: SHORTINT;
								activate_id: INTEGER;
							END;
							GtkStatusbar* = POINTER TO LIMITED RECORD (GtkHBox) END;
							
				GtkPaned= POINTER TO LIMITED RECORD (GtkContainer)END;
						GtkHPaned = POINTER TO LIMITED RECORD (GtkPaned)END;
						GtkVPaned = POINTER TO LIMITED RECORD (GtkPaned)END;
				GtkLayout = POINTER TO LIMITED RECORD (GtkContainer)END;
				GtkMenuShell* = POINTER TO LIMITED RECORD (GtkContainer)END;
					GtkMenu* = POINTER TO LIMITED RECORD (GtkMenuShell) END;
					GtkMenuBar* = POINTER TO LIMITED RECORD (GtkMenuShell) END;
				GtkNotebook= POINTER TO LIMITED RECORD (GtkContainer)END;
				GtkSocket= POINTER TO LIMITED RECORD (GtkContainer)END;
				GtkTable= POINTER TO LIMITED RECORD (GtkContainer)END;
				GtkTextView= POINTER TO LIMITED RECORD (GtkContainer)END;
				GtkToolbar= POINTER TO LIMITED RECORD (GtkContainer)END;
				GtkTreeView= POINTER TO LIMITED RECORD (GtkContainer)END;
				GtkList = POINTER TO LIMITED RECORD (GtkContainer)END;
				GtkTree= POINTER TO LIMITED RECORD (GtkContainer)END;
				GtkCList* = POINTER TO LIMITED RECORD (GtkContainer)END;
					GtkCTree* = POINTER TO LIMITED RECORD (GtkCList)END;
					
				GtkBin* = POINTER TO LIMITED RECORD (GtkContainer)
					child*: GtkWidget;
				END;

					GtkItem* = POINTER TO LIMITED RECORD (GtkBin) END;

						GtkMenuItem* = POINTER TO LIMITED RECORD (GtkItem) 
							submenu:  GtkWidget ;
							event_window*:  Gdk.GdkWindow ;
							toggle_size:SHORTINT;
							accelerator_width:SHORTINT;
							accel_path:PString;
							mi_flags: SET; (*  show_submenu_indicator : 1;   submenu_placement : 1;   
										submenu_direction : 1;   right_justify: 1;   timer_from_keypress : 1;
							*)
  							timer:INTEGER;
						END;
							GtkCheckMenuItem* = POINTER TO LIMITED RECORD (GtkMenuItem) 
								cmi_flags*: SET;
								(*  active : 1;   always_show_toggle : 1;   inconsistent : 1;   draw_as_radio : 1;*)
							END;
								GtkRadioMenuItem= POINTER TO LIMITED RECORD (GtkCheckMenuItem) END;
							GtkImageMenuItem= POINTER TO LIMITED RECORD (GtkMenuItem) END;
							GtkSeparatorMenuItem= POINTER TO LIMITED RECORD (GtkMenuItem) END;
							GtkTearoffMenuItem= POINTER TO LIMITED RECORD (GtkMenuItem) END;
						GtkListItem= POINTER TO LIMITED RECORD (GtkItem) END;
						GtkTreeItem= POINTER TO LIMITED RECORD (GtkItem) END;
						
					GtkToolItem= POINTER TO LIMITED RECORD (GtkBin) END;
							GtkToolButton = POINTER TO LIMITED RECORD (GtkToolItem) END;
								GtkToggleToolButton = POINTER TO LIMITED RECORD (GtkToolButton) END;
									GtkRadioToolButton = POINTER TO LIMITED RECORD (GtkToggleToolButton) END;
							GtkSeparatorToolItem = POINTER TO LIMITED RECORD (GtkToolItem) END;
							
					GtkButton* = POINTER TO LIMITED RECORD (GtkBin) END;
						GtkToggleButton = POINTER TO LIMITED RECORD (GtkButton) END;
							GtkCheckButton* = POINTER TO LIMITED RECORD (GtkToggleButton) END;
								GtkRadioButton* = POINTER TO LIMITED RECORD (GtkCheckButton) END;
						GtkColorButton = POINTER TO LIMITED RECORD (GtkButton) END;
						GtkFontButton = POINTER TO LIMITED RECORD (GtkButton) END;
						GtkOptionMenu = POINTER TO LIMITED RECORD (GtkButton) END;

					GtkAlignment = POINTER TO LIMITED RECORD (GtkBin) END;
					GtkViewport = POINTER TO LIMITED RECORD (GtkBin) END;
					GtkComboBox= POINTER TO LIMITED RECORD (GtkBin) END;
							GtkComboBoxEntry = POINTER TO LIMITED RECORD (GtkComboBox) END;
					GtkEventBox* = POINTER TO LIMITED RECORD (GtkBin) END;
					GtkHandleBox = POINTER TO LIMITED RECORD (GtkBin) END;
					GtkExpander = POINTER TO LIMITED RECORD (GtkBin) END;
					GtkFrame= POINTER TO LIMITED RECORD (GtkBin) END;
							GtkAspectFrame = POINTER TO LIMITED RECORD (GtkFrame) END;
					GtkScrolledWindow* = POINTER TO LIMITED RECORD (GtkBin) END;

					GtkWindow* = POINTER TO LIMITED RECORD (GtkBin)
						title*: PString;
						wmclass_name*: PString;
						wmclass_class*: PString;
						wm_role*: PString;

						focus_widget*: GtkWidget;
						default_widget*: GtkWidget;
						transient_parent*: GtkWindow;

						geometry_info: gpointer;
						frame: Gdk.GdkWindow;
						group: GtkWindowGroup;
						
						configure_request_count*: INTEGER; (*???*)
						bitFieldValues2: INTEGER; 
						(*	allow_shrink : 1;   
								allow_grow : 1;  
								configure_notify_received : 1;
								need_default_position : 1;   
								need_default_size : 1;
								position : 3;
								
								type : 4; /* GtkWindowType */ 
								has_user_ref_count : 1;   
								has_focus : 1;   
								modal : 1;   
								destroy_with_parent : 1;
								
								has_frame : 1; 
								iconify_initially : 1;   
								stick_initially : 1;   
								maximize_initially : 1;
								decorated : 1;   
								type_hint : 3; /* GdkWindowTypeHint */ 
								   
								gravity : 5; /* GdkGravity */
								is_active : 1;   
								has_toplevel_focus : 1;
							*)
						frame_left: INTEGER;
						frame_top: INTEGER;
						frame_right: INTEGER;
						frame_bottom: INTEGER;
						keys_changed_handler: INTEGER;
						mnemonic_modifier: Gdk.GdkModifierType;
						screen: Gdk.GdkScreen;
					END;


						GtkDialog* = POINTER TO LIMITED RECORD (GtkWindow)
							vbox*:GtkWidget;
							action_area_*:GtkWidget;
							separator:GtkWidget;
						END;

							GtkColorSelectionDialog* = POINTER TO LIMITED RECORD (GtkDialog)
								colorsel*: GtkColorSelection;
								ok_button*: GtkWidget;
								cancel_button*: GtkWidget;
								help_button*: GtkWidget;
							END;

							GtkFontSelectionDialog* = POINTER TO LIMITED RECORD (GtkDialog)
								fontsel*: GtkWidget;
								main_vbox*: GtkWidget;
								action_area*: GtkWidget;
  							ok_button*: GtkWidget;
								apply_button*: GtkWidget; (* is not shown by default but you can show/hide it. *)
								cancel_button*: GtkWidget;
								(*  *)
								dialog_width: INTEGER;
								auto_resize: INTEGER;
							END;

							GtkFileSelection* = POINTER TO LIMITED RECORD (GtkDialog)
								dir_list*, 
								file_list*, 
								selection_entry*, 
								selection_text*, 
								main_vbox*: GtkWidget;
								ok_button*, 
								cancel_button*, 
								help_button*, 
								history_pulldown*, 
								history_menu*: GtkWidget;
								history_list*: GLib.GList;
								fileop_dialog*, 
								fileop_entry*: GtkWidget;
								fileop_file*: PString;
								cmpl_state*: gpointer;
								fileop_c_dir*, 
								fileop_del_file*, 
								fileop_ren_file*, 
								button_area*, 
								action_area*: GtkWidget;
								selected_names:GLib.GPtrArray;
								last_selected: PString;
							END;


							GtkFileChooserDialog* = POINTER TO LIMITED RECORD (GtkDialog)END;
							
							GtkMessageDialog* = POINTER TO LIMITED RECORD (GtkDialog)
								image:GtkWidget;
								label:GtkWidget;
							END;
							GtkInputDialog = POINTER TO LIMITED RECORD (GtkDialog)END;

						GtkPlug = POINTER TO LIMITED RECORD (GtkWindow) END;
						


(*	*)
	PROCEDURE [ccall] gtk_init* (VAR [nil] argc: INTEGER; VAR [nil] argv: ARRAY [untagged] OF PString);
	
	PROCEDURE [ccall] gtk_grab_add* (widget: GtkWidget);
	PROCEDURE [ccall] gtk_grab_get_current* (): GtkWidget;
	PROCEDURE [ccall] gtk_grab_remove* (widget: GtkWidget);
	PROCEDURE [ccall] gtk_handler_block_by_func* (object: GtkObject; func: GtkSignalFunc; data: gpointer);
	PROCEDURE [ccall] gtk_main* ();
	PROCEDURE [ccall] gtk_main_do_event* (event: Gdk.GdkEvent);
	PROCEDURE [ccall] gtk_main_iteration* (): INTEGER;
	PROCEDURE [ccall] gtk_main_iteration_do* (blocking: INTEGER): INTEGER;
	PROCEDURE [ccall] gtk_main_quit* ();

	PROCEDURE [ccall] gtk_key_snooper_install* (snooper: GtkKeySnoopFunc; user_data: gpointer): INTEGER;
	PROCEDURE [ccall] gtk_get_current_event* (): Gdk.GdkEvent;
	PROCEDURE [ccall] gtk_get_event_widget* (event: Gdk.GdkEvent): GtkWidget;
	
(* GtkEditable — Interface for text-editing widgets.	
		GtkEditable is implemented by GtkEntry, GtkSpinButton, GtkOldEditable and GtkText.*)
	
	PROCEDURE [ccall] gtk_editable_get_chars* (editable: GtkWidget; start, end: INTEGER): PString;
	PROCEDURE [ccall] gtk_editable_get_position* (editable: GtkWidget): INTEGER;
	PROCEDURE [ccall] gtk_editable_delete_text* (editable: GtkWidget; start, end: INTEGER);
	PROCEDURE [ccall] gtk_editable_insert_text* (editable: GtkWidget; new_text: PString; length: INTEGER; VAR pos: INTEGER);
	PROCEDURE [ccall] gtk_editable_select_region* (editable: GtkWidget; start, end: INTEGER);
	PROCEDURE [ccall] gtk_editable_set_editable* (text: GtkWidget; editable: gboolean);
	
(*	*)
	

	PROCEDURE [ccall] gtk_misc_set_alignment* (misc: GtkMisc; xalign, yalign: REAL);
	PROCEDURE [ccall] gtk_misc_set_padding* (misc: GtkMisc; xpad, ypad: INTEGER);
	PROCEDURE [ccall] gtk_object_get_user_data* (object: GtkObject): gpointer;
	PROCEDURE [ccall] gtk_object_ref* (object: GtkObject);
	PROCEDURE [ccall] gtk_object_set_user_data* (object: GtkObject; data: gpointer);
	PROCEDURE [ccall] gtk_object_unref* (object: GtkObject);
	PROCEDURE [ccall] gtk_range_get_adjustment* (range: GtkRange): GtkAdjustment;
	PROCEDURE [ccall] gtk_range_set_adjustment* (range: GtkRange; adjustment: GtkAdjustment);
	PROCEDURE [ccall] gtk_range_slider_update* (range: GtkRange);
	
	PROCEDURE [ccall] gtk_selection_add_target* (widget: GtkWidget; selection, target: Gdk.GdkAtom; info: INTEGER);
	PROCEDURE [ccall] gtk_selection_convert* (widget: GtkWidget; selection, target: Gdk.GdkAtom; time: INTEGER): BOOLEAN;
	PROCEDURE [ccall] gtk_selection_data_set* (VAR selection_data: GtkSelectionData; type: Gdk.GdkAtom; format: INTEGER; 
														IN data: String; length: INTEGER);
	PROCEDURE [ccall] gtk_selection_owner_set* (widget: GtkWidget; selection: Gdk.GdkAtom; time: INTEGER): INTEGER;


	PROCEDURE [ccall] gtk_signal_connect_full*(object: GtkObject; name: PString; func: GtkSignalFunc; 
			marshal:GtkCallbackMarshal; 
			func_data: gpointer;
			destroy_func:GtkDestroyNotify; object_signal: INTEGER; after:INTEGER): INTEGER;
	
	PROCEDURE [ccall] gtk_signal_compat_matched*(object: GtkObject; func: GtkSignalFunc; data:gpointer;match:SET; action: INTEGER);

	PROCEDURE [ccall] gtk_signal_emit_by_name* (object: GtkObject; name: PString);
	PROCEDURE [ccall] gtk_signal_emit_stop_by_name* (object: GtkObject; name: PString);
	
	PROCEDURE [ccall] gtk_style_copy* (style: GtkStyle): GtkStyle;
	PROCEDURE [ccall] gtk_style_ref* (style: GtkStyle);
	PROCEDURE [ccall] gtk_style_unref* (style: GtkStyle);
(* ShiryaevAV: deprecated
	PROCEDURE [ccall] gtk_timeout_add* (interval: INTEGER; function: GtkFunction; data: gpointer): INTEGER;
*)
	PROCEDURE [ccall] gtk_toggle_button_get_active* (toggle_button: GtkWidget): INTEGER;
	PROCEDURE [ccall] gtk_toggle_button_set_active* (toggle_button: GtkWidget; is_active: INTEGER);
	PROCEDURE [ccall] gtk_toggle_button_toggled* (toggle_button: GtkWidget);

	PROCEDURE [ccall] gtk_widget_queue_draw_area* (widget: GtkWidget; x, y, width, height: INTEGER); 
	PROCEDURE [ccall] gtk_widget_destroy* (object: GtkWidget);
	PROCEDURE [ccall] gtk_widget_draw* (widget: GtkWidget; IN area: Gdk.GdkRectangle); 

(* ShiryaevAV
	PROCEDURE [ccall] gtk_widget_draw_focus* (widget: GtkWidget);
	PROCEDURE [ccall] gtk_widget_draw_default *(widget: GtkWidget);
*)

	PROCEDURE [ccall] gtk_widget_event* (widget: GtkWidget; IN event: Gdk.GdkEventDesc);
	PROCEDURE [ccall] gtk_widget_get_events* (widget: GtkWidget): Gdk.GdkEventMask;
	PROCEDURE [ccall] gtk_widget_get_parent_window* (widget: GtkWidget): Gdk.GdkWindow;
	PROCEDURE [ccall] gtk_widget_get_style* (widget: GtkWidget): GtkStyle;
	PROCEDURE [ccall] gtk_widget_get_visual* (widget: GtkWidget): Gdk.GdkVisual;
	PROCEDURE [ccall] gtk_widget_grab_default* (widget: GtkWidget);
	PROCEDURE [ccall] gtk_widget_grab_focus* (widget: GtkWidget);
	PROCEDURE [ccall] gtk_widget_hide* (widget: GtkWidget);
	PROCEDURE [ccall] gtk_widget_hide_all* (widget: GtkWidget);
	PROCEDURE [ccall] gtk_widget_modify_style* (widget: GtkWidget; style: GtkRcStyle);
	PROCEDURE [ccall] gtk_widget_ref* (widget: GtkWidget);
	PROCEDURE [ccall] gtk_widget_set_events* (widget: GtkWidget; events: Gdk.GdkEventMask);
	PROCEDURE [ccall] gtk_widget_set_sensitive* (widget: GtkWidget; sensitive: gboolean);
	PROCEDURE [ccall] gtk_widget_set_state* (widget: GtkWidget; state: INTEGER);
	PROCEDURE [ccall] gtk_widget_set_uposition* (widget: GtkWidget; x, y: INTEGER);
	PROCEDURE [ccall] gtk_widget_set_usize* (widget: GtkWidget; width, height: INTEGER);
	PROCEDURE [ccall] gtk_widget_show* (widget: GtkWidget);
	PROCEDURE [ccall] gtk_widget_show_all* (widget: GtkWidget);
	PROCEDURE [ccall] gtk_widget_show_now* (widget: GtkWidget);
	PROCEDURE [ccall] gtk_widget_size_allocate* (widget: GtkWidget; IN allocation: GtkAllocation);
	PROCEDURE [ccall] gtk_widget_size_request* (widget: GtkWidget; OUT requisition: GtkRequisition);
	PROCEDURE [ccall] gtk_widget_unparent* (widget: GtkWidget);
	PROCEDURE [ccall] gtk_widget_unref* (widget: GtkWidget);
	PROCEDURE [ccall] gtk_widget_queue_clear* (widget: GtkWidget);
	PROCEDURE [ccall] gtk_widget_queue_draw* (widget: GtkWidget);
	PROCEDURE [ccall] gtk_widget_add_events* (widget: GtkWidget; events: INTEGER);
	PROCEDURE [ccall] gtk_widget_add_accelerator* (widget: GtkWidget; accel_signal: PString; 
															accel_group: GtkAccelGroup; accel_key: INTEGER; 
															accel_mods: Gdk.GdkModifierType; accel_flags: GtkAccelFlags);
															

	PROCEDURE [ccall] gtk_container_add* (container: GtkContainer; widget: GtkWidget);
	PROCEDURE [ccall] gtk_container_children* (container: GtkContainer): GLib.GList;
	PROCEDURE [ccall] gtk_container_foreach* (container: GtkContainer;callback: GtkCallBack; callback_data:gpointer);
	PROCEDURE [ccall] gtk_container_remove* (container: GtkContainer; widget: GtkWidget);
	PROCEDURE [ccall] gtk_container_set_focus_child* (container: GtkContainer; child: GtkWidget);

	PROCEDURE [ccall] gtk_dialog_run *(dialog:GtkDialog ):INTEGER;
	PROCEDURE [ccall] gtk_dialog_add_buttons *(dialog:GtkDialog; name: PString);
(*	*)
	PROCEDURE [ccall] gtk_accel_group_new* (): GtkAccelGroup;
	PROCEDURE [ccall] gtk_accel_group_add* (accel_group: GtkAccelGroup; accel_key: INTEGER; accel_mods: Gdk.GdkModifierType;
                                              accel_flags: GtkAccelFlags; object: GtkObject; accel_signal: PString);
	PROCEDURE [ccall] gtk_accel_group_attach* (accel_group: GtkAccelGroup; object: GtkObject);
	PROCEDURE [ccall] gtk_accel_group_lock* (accel_group: GtkAccelGroup);
	PROCEDURE [ccall] gtk_accel_group_unlock* (accel_group: GtkAccelGroup);
	PROCEDURE [ccall] gtk_accelerator_valid* (keyval: INTEGER; modifier: Gdk.GdkModifierType): INTEGER; 

	PROCEDURE [ccall] gtk_adjustment_new* (value, lower, upper, step_increment, page_increment, page_size: REAL): GtkAdjustment; 
	PROCEDURE [ccall] gtk_adjustment_changed* (adjustment: GtkAdjustment);
	PROCEDURE [ccall] gtk_box_reorder_child* (box: GtkWidget; widget: GtkWidget; position: INTEGER);

	PROCEDURE [ccall] gtk_button_new* (): GtkButton;
	PROCEDURE [ccall] gtk_button_new_with_label* (IN label: String): GtkButton;
	PROCEDURE [ccall] gtk_button_new_with_mnemonic* (IN label: String): GtkButton;
	PROCEDURE [ccall] gtk_button_new_from_stock*(IN stock: String): GtkButton; 
	PROCEDURE [ccall] gtk_button_pressed* (button: GtkButton);
	PROCEDURE [ccall] gtk_button_clicked* (button: GtkButton); 
	
	PROCEDURE [ccall] gtk_check_button_new* (): GtkCheckButton;
	PROCEDURE [ccall] gtk_check_button_new_with_label* (IN label: String): GtkCheckButton;
	PROCEDURE [ccall] gtk_check_button_new_with_mnemonic* (IN label: String): GtkCheckButton;
	
	
	PROCEDURE [ccall] gtk_radio_button_new* (group: GLib.GList): GtkRadioButton; 
	PROCEDURE [ccall] gtk_radio_button_new_from_widget* (radioButton: GtkWidget): GtkRadioButton;
PROCEDURE [ccall] 	gtk_radio_button_new_with_mnemonic*(group: GLib.GList;IN label: String):GtkRadioButton;
	
	PROCEDURE [ccall] gtk_combo_new* (): GtkCombo;
	PROCEDURE [ccall] gtk_combo_set_popdown_strings* (combo: GtkCombo; strings: GLib.GList);
	
	
	PROCEDURE [ccall] gtk_clist_new* (columns: INTEGER): GtkCList;
	PROCEDURE [ccall] gtk_clist_append* (clist: GtkCList; IN text: StrArray);
	PROCEDURE [ccall] gtk_clist_clear* (clist: GtkCList);
	PROCEDURE [ccall] gtk_clist_find_row_from_data* (clist: GtkCList; data: gpointer): INTEGER;
	PROCEDURE [ccall] gtk_clist_freeze* (clist: GtkCList);
	PROCEDURE [ccall] gtk_clist_get_row_data* (clist: GtkCList; row: INTEGER): gpointer;
	PROCEDURE [ccall] gtk_clist_get_text* (clist: GtkCList; row, column: INTEGER; VAR text: PString);
	PROCEDURE [ccall] gtk_clist_insert* (clist: GtkCList; row: INTEGER; IN text: StrArray);
	PROCEDURE [ccall] gtk_clist_moveto* (clist: GtkCList; row, column: INTEGER; row_align, col_align: REAL);
	PROCEDURE [ccall] gtk_clist_optimal_column_width* (clist: GtkCList; column: INTEGER): INTEGER;
	PROCEDURE [ccall] gtk_clist_prepend* (clist: GtkCList; IN text: StrArray);
	PROCEDURE [ccall] gtk_clist_select_row* (clist: GtkCList; row, column: INTEGER);
	PROCEDURE [ccall] gtk_clist_set_background* (clist: GtkCList; row: INTEGER; ΙΝ color: Gdk.GdkColor);
	PROCEDURE [ccall] gtk_clist_set_column_justification* (clist: GtkCList; column: INTEGER; justification: GtkJustification);
	PROCEDURE [ccall] gtk_clist_set_column_width* (clist: GtkCList; column, width: INTEGER);
	PROCEDURE [ccall] gtk_clist_set_row_data* (clist: GtkCList; row: INTEGER; data: gpointer);
	PROCEDURE [ccall] gtk_clist_set_selection_mode* (clist: GtkCList; mode: GtkSelectionMode);
	PROCEDURE [ccall] gtk_clist_set_text* (clist: GtkCList; row, column: INTEGER; text: PString);
	PROCEDURE [ccall] gtk_clist_sort* (clist: GtkCList);
	PROCEDURE [ccall] gtk_clist_thaw* (clist: GtkCList);
	PROCEDURE [ccall] gtk_clist_unselect_all* (clist: GtkCList);
	PROCEDURE [ccall] gtk_clist_unselect_row* (clist: GtkCList; row, column: INTEGER);
	
	PROCEDURE [ccall] gtk_ctree_new* (columns, tree_column: INTEGER):GtkCTree ;
	PROCEDURE [ccall] gtk_ctree_insert_node* (ctree: GtkCTree; parent, sibling: GtkCTreeNode; IN  text: StrArray; 
		spacing: INTEGER;
		pixmap_closed: Gdk.GdkPixmap;   mask_closed: Gdk.GdkBitmap; 
		pixmap_open: Gdk.GdkPixmap;  mask_open: Gdk.GdkBitmap;
		is_leaf, expanded: INTEGER
		): GtkCTreeNode; (* returns the inserted GtkCTreeNode *)
		
	PROCEDURE [ccall] gtk_ctree_node_get_row_data* (ctree: GtkCTree; node: GtkCTreeNode): gpointer;
	PROCEDURE [ccall] gtk_ctree_node_set_row_data* (ctree: GtkCTree; node: GtkCTreeNode; data: gpointer);
	PROCEDURE [ccall] gtk_ctree_node_nth* (ctree: GtkCTree; row: INTEGER): GtkCTreeNode;
	PROCEDURE [ccall] gtk_ctree_sort_recursive* (ctree: GtkCTree; tree_node: GtkCTreeNode);
	

											
	PROCEDURE [ccall] gtk_drawing_area_new* (): GtkDrawingArea;
	PROCEDURE [ccall] gtk_drawing_area_size* (darea: GtkDrawingArea; width, height: INTEGER);
	
	
	PROCEDURE [ccall] gtk_entry_new* (): GtkEntry;
	PROCEDURE [ccall] gtk_entry_set_text* (entry: GtkWidget; text: PString);
	

	PROCEDURE [ccall] gtk_event_box_new* (): GtkEventBox;
	PROCEDURE [ccall] gtk_events_pending* (): BOOLEAN;
	
	PROCEDURE [ccall] gtk_fixed_new* (): GtkFixed;
	PROCEDURE [ccall] gtk_fixed_put* (fixed:GtkFixed; widget: GtkWidget; x, y: INTEGER);
	PROCEDURE [ccall] gtk_fixed_move* (fixed, widget: GtkWidget; x, y: INTEGER);
PROCEDURE [ccall] gtk_fixed_set_has_window*(fixed:GtkFixed; f: gboolean);
	
	
	
	PROCEDURE [ccall] gtk_label_new* (str: PString): GtkLabel;
	PROCEDURE [ccall] gtk_label_new_with_mnemonic* (str: PString): GtkLabel;
	
	PROCEDURE [ccall] gtk_label_get* (label: GtkLabel; VAR str: PString);
	PROCEDURE [ccall] gtk_label_set_justify* (label: GtkLabel; jtype: GtkJustification);
	PROCEDURE [ccall] gtk_label_set_line_wrap* (label: GtkLabel; wrap: INTEGER);
	PROCEDURE [ccall] gtk_label_set_text* (label: GtkLabel; IN str: String);
	PROCEDURE [ccall] gtk_label_parse_uline* (label: GtkLabel; string: PString): INTEGER;
	
	PROCEDURE [ccall] gtk_list_new* (): GtkWidget;
	PROCEDURE [ccall] gtk_list_child_position* (list, child: GtkWidget): INTEGER;
	PROCEDURE [ccall] gtk_list_clear_items* (list: GtkWidget; start, end: INTEGER);
	PROCEDURE [ccall] gtk_list_item_new* (): GtkWidget;
	PROCEDURE [ccall] gtk_list_item_new_with_label* (label: PString): GtkWidget;
	PROCEDURE [ccall] gtk_list_select_item* (list: GtkWidget; item: INTEGER);
	PROCEDURE [ccall] gtk_list_set_selection_mode* (list: GtkWidget; mode: GtkSelectionMode);
	PROCEDURE [ccall] gtk_list_unselect_all* (list: GtkWidget);

	PROCEDURE [ccall] gtk_menu_new* (): GtkMenu;
	PROCEDURE [ccall] gtk_menu_bar_new* (): GtkMenu;
	PROCEDURE [ccall] gtk_menu_popup* (menu: GtkMenu; parent_menu_shell, parent_menu_item: GtkWidget;
															func, data: gpointer; 
															button: INTEGER; activate_time: INTEGER);
	PROCEDURE [ccall] gtk_menu_prepend* (menu: GtkMenu; child: GtkWidget);

	PROCEDURE [ccall] gtk_menu_set_title* (menu: GtkMenu; title: PString);
	PROCEDURE [ccall] gtk_menu_shell_activate_item* (menu_shell: GtkMenuShell; menu_item: GtkWidget; 
																					force_deactivate: gboolean);
	PROCEDURE [ccall] gtk_menu_shell_select_item* (menu_shell: GtkMenuShell; menu_item: GtkWidget);

	PROCEDURE [ccall] gtk_menu_detach* (menu: GtkMenu);
	PROCEDURE [ccall] gtk_menu_get_attach_widget* (menu: GtkMenu): GtkWidget;

	PROCEDURE [ccall] gtk_menu_item_new* (): GtkMenuItem;
	PROCEDURE [ccall] gtk_menu_item_new_with_label* (label: PString): GtkMenuItem;
	PROCEDURE [ccall] gtk_menu_item_new_with_mnemonic* (label: PString): GtkMenuItem;

	PROCEDURE [ccall] gtk_menu_item_activate* (menu_item: GtkWidget);
	PROCEDURE [ccall] gtk_menu_item_deselect* (menu_item: GtkMenuItem);

	PROCEDURE [ccall] gtk_menu_item_remove_submenu* (menu_item: GtkMenuItem);
	PROCEDURE [ccall] gtk_menu_item_select* (menu_item: GtkMenuItem);
	PROCEDURE [ccall] gtk_menu_item_set_submenu* (menu_item: GtkMenuItem; submenu: GtkWidget);
	
	PROCEDURE [ccall] gtk_menu_shell_append* (menu: GtkMenu; child: GtkMenuItem);

	PROCEDURE [ccall] gtk_menu_attach_to_widget* (menu: GtkMenu; attatch_widget: GtkWidget; detacher: GtkMenuDetachFunc);

	PROCEDURE [ccall] gtk_separator_menu_item_new* (): GtkSeparatorMenuItem;

	PROCEDURE [ccall] gtk_check_menu_item_new_with_mnemonic* (label: PString): GtkCheckMenuItem;
	PROCEDURE [ccall] gtk_check_menu_item_new_with_label* (label: PString): GtkWidget;
	PROCEDURE [ccall] gtk_check_menu_item_set_active* (check_menu_item: GtkMenuItem; is_active: gboolean);
	PROCEDURE [ccall] gtk_check_menu_item_set_inconsistent* (check_menu_item: GtkMenuItem; setting: gboolean);
	PROCEDURE [ccall] gtk_check_menu_item_set_show_toggle* (check_menu_item: GtkMenuItem; always: gboolean);


	PROCEDURE [ccall] gtk_image_menu_item_new_with_mnemonic* (label: PString): GtkMenuItem;
	PROCEDURE [ccall] gtk_image_menu_item_set_image* (menu_item: GtkMenuItem; image: GtkWidget);


	PROCEDURE [ccall] gtk_notebook_new* (): GtkWidget;
	PROCEDURE [ccall] gtk_notebook_append_page* (notebook, child, tab_label: GtkWidget);

	PROCEDURE [ccall] gtk_pixmap_new* (pixmap: Gdk.GdkPixmap; mask: Gdk.GdkBitmap): GtkWidget;
	PROCEDURE [ccall] gtk_propagate_event* (widget: GtkWidget; event: Gdk.GdkEvent);

	
	PROCEDURE [ccall] gtk_rc_style_new* (): GtkRcStyle; 
	PROCEDURE [ccall] gtk_rc_style_ref* (style: GtkRcStyle);
	PROCEDURE [ccall] gtk_rc_style_unref* (style: GtkRcStyle);
	PROCEDURE [ccall] gtk_rc_get_style* (widget: GtkWidget): GtkStyle;
	

	PROCEDURE [ccall] gtk_scrolled_window_new* (hadjustment, vadjustment: GtkAdjustment): GtkScrolledWindow;
	PROCEDURE [ccall] gtk_scrolled_window_add_with_viewport* (scrolled_window: GtkScrolledWindow; child: GtkWidget);
	PROCEDURE [ccall] gtk_scrolled_window_get_vadjustment* (scrolled_window: GtkScrolledWindow): GtkAdjustment;
	PROCEDURE [ccall] gtk_scrolled_window_set_hadjustment* (scrolled_window: GtkScrolledWindow; hadjustment: GtkAdjustment);
	PROCEDURE [ccall] gtk_scrolled_window_set_policy* (scrolled_window: GtkScrolledWindow; 
																hscrollbar_policy, vscrollbar_policy: GtkPolicyType);
	PROCEDURE [ccall] gtk_scrolled_window_set_vadjustment* (scrolled_window: GtkScrolledWindow; vadjustment: GtkAdjustment);

	PROCEDURE [ccall] gtk_spin_button_new* (adjustment: GtkAdjustment; climb_rate: REAL; digits: INTEGER): 	GtkSpinButton;
	PROCEDURE [ccall] gtk_spin_button_get_value_as_int* (spin_button: GtkWidget): INTEGER;

	PROCEDURE [ccall] gtk_spin_button_set_numeric* (spin_button: GtkWidget; numeric: INTEGER);
	PROCEDURE [ccall] gtk_spin_button_set_shadow_type (spin_button: GtkWidget; shadow_type: GtkShadowType);
	PROCEDURE [ccall] gtk_spin_button_set_snap_to_ticks* (spin_button: GtkWidget; snap_to_ticks: INTEGER);
	PROCEDURE [ccall] gtk_spin_button_set_value* (spin_button: GtkWidget; value: REAL);
	PROCEDURE [ccall] gtk_spin_button_set_wrap* (spin_button: GtkWidget; wrap: INTEGER);
	PROCEDURE [ccall] gtk_spin_button_update* (spin_button: GtkWidget);
	PROCEDURE [ccall] gtk_statusbar_get_context_id* (statusbar: GtkStatusbar; context_description: PString): INTEGER;

	PROCEDURE [ccall] gtk_statusbar_new* (): GtkStatusbar;
	PROCEDURE [ccall] gtk_statusbar_pop* (statusbar: GtkStatusbar; context_id: INTEGER);
	PROCEDURE [ccall] gtk_statusbar_push* (statusbar: GtkStatusbar; context_id: INTEGER; text: PString): INTEGER;

	PROCEDURE [ccall] gtk_text_new* (hadjustment, vadjustment: GtkAdjustment): GtkText;
	PROCEDURE [ccall] gtk_text_get_length* (text: GtkWidget): INTEGER;
	PROCEDURE [ccall] gtk_text_insert* (text: GtkWidget; font: Gdk.GdkFont; ΙΝ fore, back: Gdk.GdkColor; 
											chars: PString; length: INTEGER);
	PROCEDURE [ccall] gtk_text_set_editable* (text: GtkWidget; editable: INTEGER);
	PROCEDURE [ccall] gtk_text_set_line_wrap* (text: GtkWidget; line_wrap: INTEGER);
	PROCEDURE [ccall] gtk_text_set_word_wrap* (text: GtkWidget; word_wrap: INTEGER);


	PROCEDURE [ccall] gtk_tree_new* (): GtkTree;
	PROCEDURE [ccall] gtk_tree_append* (tree: GtkTree; tree_item: GtkWidget);
	PROCEDURE [ccall] gtk_tree_insert* (tree: GtkTree; tree_item: GtkWidget; position: INTEGER);
	
	PROCEDURE [ccall] gtk_tree_item_new* (): GtkWidget;
	PROCEDURE [ccall] gtk_tree_item_new_with_label* (label: PString): GtkWidget;
	PROCEDURE [ccall] gtk_tree_item_set_subtree* (tree_item, subtree: GtkWidget);
	PROCEDURE [ccall] gtk_tree_prepend* (tree, tree_item: GtkWidget);
	PROCEDURE [ccall] gtk_tree_select_child* (tree, item: GtkWidget);
	PROCEDURE [ccall] gtk_tree_select_item* (tree: GtkWidget; item: INTEGER);
	PROCEDURE [ccall] gtk_tree_unselect_child* (tree, item: GtkWidget);
	PROCEDURE [ccall] gtk_tree_unselect_item* (tree: GtkWidget; item: INTEGER);
	
	PROCEDURE [ccall] gtk_hbox_new* (homogeneous:gboolean; spacing: INTEGER): GtkHBox;
	PROCEDURE [ccall] gtk_vbox_new* (homogeneous:gboolean; spacing: INTEGER): GtkVBox;

	PROCEDURE [ccall] gtk_hscrollbar_new* (adjustment: GtkAdjustment): GtkHScrollbar;
	PROCEDURE [ccall] gtk_vscrollbar_new* (adjustment: GtkAdjustment): GtkVScrollbar;

	PROCEDURE [ccall] gtk_hseparator_new* (): GtkWidget;
	PROCEDURE [ccall] gtk_vseparator_new* (): GtkWidget;
	
	PROCEDURE [ccall] gtk_viewport_new* (hadjustment, vadjustmen: GtkAdjustment): GtkWidget;
	
	PROCEDURE [ccall] gtk_window_new* (type: INTEGER): GtkWindow;
	PROCEDURE [ccall] gtk_window_activate_focus* (window: GtkWindow): INTEGER;
	PROCEDURE [ccall] gtk_window_add_accel_group* (window: GtkWindow; accel_group: GtkAccelGroup);
	PROCEDURE [ccall] gtk_window_set_default_size* (window: GtkWindow; width, height: INTEGER); 
	PROCEDURE [ccall] gtk_window_resize* (window: GtkWindow; width, height: INTEGER); 
	PROCEDURE [ccall] gtk_window_set_focus* (window: GtkWindow; focus: GtkWidget);
	PROCEDURE [ccall] gtk_window_set_modal* (window: GtkWindow; modal: gboolean);
	PROCEDURE [ccall] gtk_window_set_policy* (window: GtkWindow; allow_shrink, allow_grow, auto_shrink: INTEGER);
	PROCEDURE [ccall] gtk_window_set_position* (window: GtkWindow; position: GtkWindowPosition);
	PROCEDURE [ccall] gtk_window_set_title* (window: GtkWindow; title: PString);
	PROCEDURE [ccall] gtk_window_get_title* (window: GtkWindow): PString;

	PROCEDURE [ccall] gtk_color_selection_new* (): GtkColorSelection;
	PROCEDURE [ccall] gtk_color_selection_get_color* (colorsel: GtkColorSelection; VAR color: GtkColors);
	PROCEDURE [ccall] gtk_color_selection_set_color* (colorsel: GtkColorSelection; VAR color: GtkColors);
	PROCEDURE [ccall] gtk_color_selection_dialog_new* (title: PString): GtkColorSelectionDialog;

	PROCEDURE [ccall] gtk_message_dialog_new* (parent:GtkWindow; flags:SET;  type:GtkMessageType;buttons:GtkButtonsType;IN message_format:String(*...*)): GtkMessageDialog;

	PROCEDURE [ccall] gtk_font_selection_dialog_new* (title: PString): GtkFontSelectionDialog;
	PROCEDURE [ccall] gtk_font_selection_dialog_get_font_name* (fsd: GtkFontSelectionDialog): PString;
	PROCEDURE [ccall] gtk_font_selection_dialog_set_font_name* (fsd: GtkFontSelectionDialog; IN fontname: String): INTEGER;

	PROCEDURE [ccall] gtk_file_selection_new* (title: PString): GtkFileSelection;
	PROCEDURE [ccall] gtk_file_selection_get_filename* (filesel: GtkFileSelection): PString;
	PROCEDURE [ccall] gtk_file_selection_set_filename* (filesel: GtkFileSelection;IN filename: String );
	PROCEDURE [ccall] gtk_file_selection_hide_fileop_buttons*(filesel: GtkFileSelection);

(* GtkFileChooser — File chooser interface used by GtkFileChooserWidget and GtkFileChooserDialog. *)		
	PROCEDURE [ccall] gtk_file_chooser_get_filename* (fc: GtkDialog): PString;
	PROCEDURE [ccall] gtk_file_chooser_set_filename* (fc: GtkDialog; IN filename: String);

	PROCEDURE [ccall] gtk_file_chooser_dialog_new*(title: PString; parent:GtkWindow; action:INTEGER; 
		  b1:PString;r1:INTEGER;
		  b2:PString; r2:INTEGER;
		  b3:PString; r3:INTEGER;
		  terminator:INTEGER
		):GtkFileChooserDialog;


	PROCEDURE [ccall] gtk_widget_create_pango_context  (widget:GtkWidget):Pango.PangoContext;
	PROCEDURE [ccall] gtk_widget_get_pango_context    (widget:GtkWidget):Pango.PangoContext;
	PROCEDURE [ccall] gtk_widget_create_pango_layout * (widget: GtkWidget;IN text:String): Pango.PangoLayout;
		
END LibsGtk.
