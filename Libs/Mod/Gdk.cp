MODULE LibsGdk ["libgdk-win32-2.0-0.dll"];
IMPORT SYSTEM, GLib:=LibsGlib, GObject:=LibsGObject, Pango := LibsPango;
(* deprecated *)
TYPE 
	gpointer = GLib.gpointer;
	gboolean = GLib.gboolean;
	gunichar = GLib.gunichar;
	GSList = GLib.GSList;

	String = ARRAY [untagged] OF SHORTCHAR;
	PString* = POINTER TO String;
	UString = ARRAY [untagged] OF gunichar;
	PUString = POINTER TO UString;
	APString = ARRAY [untagged] OF PString;
	PBYTES = POINTER TO ARRAY [untagged] OF BYTE;

CONST 
  GDK_CURRENT_TIME* = 0;
  GDK_PARENT_RELATIVE = 1;

TYPE	GdkDragAction = INTEGER;
TYPE	GdkDragProtocol = INTEGER;
TYPE	GdkEventFunc= INTEGER;
TYPE	GdkVisibilityState = INTEGER;
TYPE	GdkScrollDirection = INTEGER;
TYPE	GdkFilterReturn = INTEGER;
TYPE	GdkSettingAction = INTEGER;
TYPE	GdkNotifyType = INTEGER;
TYPE	GdkCrossingMode = INTEGER;
TYPE	GdkPropertyState = INTEGER;
TYPE	GdkExtensionMode= INTEGER;
TYPE	GdkFill= INTEGER;
TYPE	GdkFillRule= INTEGER;
TYPE	GdkFilterFunc= INTEGER;
TYPE	GdkFunction= INTEGER;
TYPE	GdkOverlapType= INTEGER;
TYPE	GdkPixbufAlphaMode= INTEGER;
TYPE	GdkPropMode= INTEGER;
TYPE	GdkRgbDither= INTEGER;
TYPE	GdkSpanFunc= INTEGER;
TYPE	GdkSubwindowMode= INTEGER;
TYPE	GdkWMDecoration= INTEGER;
TYPE	GdkWMFunction= INTEGER;
TYPE	GdkWindowEdge= INTEGER;
TYPE	GdkWindowHints= INTEGER;
TYPE	GdkWindowState= INTEGER;
TYPE	GdkWindowType= INTEGER;
TYPE	GdkWindowTypeHint= INTEGER;
TYPE	GdkAxisUse = INTEGER; CONST (* enu: GdkAxisUse *)
  GDK_AXIS_IGNORE = 0;
  GDK_AXIS_X = 1;
  GDK_AXIS_Y = 2;
  GDK_AXIS_PRESSURE = 3;
  GDK_AXIS_XTILT = 4;
  GDK_AXIS_YTILT = 5;
  GDK_AXIS_WHEEL = 6;
  GDK_AXIS_LAST = 7;
(* end: GdkAxisUse *)


TYPE	GdkWindowClass= INTEGER;
(* enu: GdkWindowClass *)
CONST
  GDK_INPUT_OUTPUT=0;
  GDK_INPUT_ONLY=1;


TYPE	GdkInputCondition = INTEGER; (* enu: GdkInputCondition *)
CONST
  GDK_INPUT_READ = 1;
  GDK_INPUT_WRITE = 2;
  GDK_INPUT_EXCEPTION = 4;


TYPE	GdkStatus = INTEGER;	(* enu: GdkStatus *)
CONST
  GDK_OK = 0;
  GDK_ERROR = -1;
  GDK_ERROR_PARAM = -2;
  GDK_ERROR_FILE = -3;
  GDK_ERROR_MEM = -4;

TYPE	GdkGrabStatus = INTEGER; (* enu: GdkGrabStatus *)
CONST 
  GDK_GRAB_SUCCESS = 0;
  GDK_GRAB_ALREADY_GRABBED = 1;
  GDK_GRAB_INVALID_TIME = 2;
  GDK_GRAB_NOT_VIEWABLE = 3;
  GDK_GRAB_FROZEN = 4;

TYPE	GdkVisualType = INTEGER; (* enu: GdkVisualType *)
CONST 
  GDK_VISUAL_STATIC_GRAY = 0;
  GDK_VISUAL_GRAYSCALE = 1;
  GDK_VISUAL_STATIC_COLOR = 2;
  GDK_VISUAL_PSEUDO_COLOR = 3;
  GDK_VISUAL_TRUE_COLOR = 4;
  GDK_VISUAL_DIRECT_COLOR = 5;(* end: GdkVisualType *)
TYPE	GdkByteOrder = INTEGER;	(* enu: GdkByteOrder *)
CONST GDK_LSB_FIRST = 0; GDK_MSB_FIRST = 1;

TYPE	GdkCursorType = INTEGER;	(* enu: GdkCursorType *)
CONST
  GDK_X_CURSOR = 0;
  GDK_ARROW = 2;
  GDK_BASED_ARROW_DOWN = 4;
  GDK_BASED_ARROW_UP = 6;
  GDK_BOAT = 8;
  GDK_BOGOSITY = 10;
  GDK_BOTTOM_LEFT_CORNER = 12;
  GDK_BOTTOM_RIGHT_CORNER = 14;
  GDK_BOTTOM_SIDE = 16;
  GDK_BOTTOM_TEE = 18;
  GDK_BOX_SPIRAL = 20;
  GDK_CENTER_PTR = 22;
  GDK_CIRCLE* = 24;
  GDK_CLOCK = 26;
  GDK_COFFEE_MUG = 28;
  GDK_CROSS* = 30;
  GDK_CROSS_REVERSE = 32;
  GDK_CROSSHAIR* = 34;
  GDK_DIAMOND_CROSS = 36;
  GDK_DOT = 38;
  GDK_DOTBOX = 40;
  GDK_DOUBLE_ARROW = 42;
  GDK_DRAFT_LARGE = 44;
  GDK_DRAFT_SMALL = 46;
  GDK_DRAPED_BOX = 48;
  GDK_EXCHANGE = 50;
  GDK_FLEUR* = 52;
  GDK_GOBBLER = 54;
  GDK_GUMBY = 56;
  GDK_HAND1* = 58;
  GDK_HAND2* = 60;
  GDK_HEART = 62;
  GDK_ICON = 64;
  GDK_IRON_CROSS = 66;
  GDK_LEFT_PTR* = 68;
  GDK_LEFT_SIDE = 70;
  GDK_LEFT_TEE = 72;
  GDK_LEFTBUTTON = 74;
  GDK_LL_ANGLE = 76;
  GDK_LR_ANGLE = 78;
  GDK_MAN = 80;
  GDK_MIDDLEBUTTON = 82;
  GDK_MOUSE = 84;
  GDK_PENCIL = 86;
  GDK_PIRATE = 88;
  GDK_PLUS = 90;
  GDK_QUESTION_ARROW = 92;
  GDK_RIGHT_PTR = 94;
  GDK_RIGHT_SIDE = 96;
  GDK_RIGHT_TEE = 98;
  GDK_RIGHTBUTTON = 100;
  GDK_RTL_LOGO = 102;
  GDK_SAILBOAT* = 104;
  GDK_SB_DOWN_ARROW = 106;
  GDK_SB_H_DOUBLE_ARROW* = 108;
  GDK_SB_LEFT_ARROW = 110;
  GDK_SB_RIGHT_ARROW = 112;
  GDK_SB_UP_ARROW = 114;
  GDK_SB_V_DOUBLE_ARROW* = 116;
  GDK_SHUTTLE = 118;
  GDK_SIZING = 120;
  GDK_SPIDER = 122;
  GDK_SPRAYCAN = 124;
  GDK_STAR = 126;
  GDK_TARGET = 128;
  GDK_TCROSS = 130;
  GDK_TOP_LEFT_ARROW = 132;
  GDK_TOP_LEFT_CORNER* = 134;
  GDK_TOP_RIGHT_CORNER* = 136;
  GDK_TOP_SIDE = 138;
  GDK_TOP_TEE = 140;
  GDK_TREK = 142;
  GDK_UL_ANGLE = 144;
  GDK_UMBRELLA = 146;
  GDK_UR_ANGLE = 148;
  GDK_WATCH* = 150;
  GDK_XTERM* = 152;
  GDK_LAST_CURSOR = 153;
  GDK_CURSOR_IS_PIXMAP = -1;
(* end: GdkCursorType *)
TYPE	GdkInputSource = INTEGER;	(* enu: GdkInputSource *)
CONST
  GDK_SOURCE_MOUSE = 0;
  GDK_SOURCE_PEN = 1;
  GDK_SOURCE_ERASER = 2;
  GDK_SOURCE_CURSOR = 3;
(* end: GdkInputSource *)

TYPE	GdkInputMode = INTEGER;	(* enu: GdkInputMode *)
CONST
  GDK_MODE_DISABLED = 0;  GDK_MODE_SCREEN = 1;  GDK_MODE_WINDOW = 2;(* end: GdkInputMode *)
TYPE	GdkImageType = INTEGER;	(* enu: GdkImageType *)
CONST
  GDK_IMAGE_NORMAL = 0;
  GDK_IMAGE_SHARED = 1;
  GDK_IMAGE_FASTEST = 2;(* end: GdkImageType *)
TYPE	GdkJoinStyle= INTEGER;		(* Possible values for JoinStyle *)
CONST
		GDK_JOIN_MITER* = 0;
		GDK_JOIN_ROUND* = 1;
		GDK_JOIN_BEVEL* = 2;

TYPE	GdkLineStyle= INTEGER;	(* Possible values for LineStyle *)
CONST
		GDK_LINE_SOLID* = 0;
		GDK_LINE_ON_OFF_DASH* = 1;
		GDK_LINE_DOUBLE_DASH* = 2;
		
TYPE	GdkCapStyle= INTEGER;		(* Possible values for CapStyle *)
CONST
		GDK_CAP_NOT_LAST* = 0;
		GDK_CAP_BUTT* = 1;
		GDK_CAP_ROUND* = 2;
		GDK_CAP_PROJECTING* = 3;
		
TYPE  GdkEventMask* = SET; 		(* possible values for EventMask *)
CONST
       GDK_EXPOSURE_MASK* = {1};
       GDK_POINTER_MOTION_MASK* = {2};
       GDK_POINTER_MOTION_HINT_MASK* = {3};
       GDK_BUTTON_MOTION_MASK* = {4};
       GDK_BUTTON1_MOTION_MASK* = {5};
       GDK_BUTTON2_MOTION_MASK* = {6};
       GDK_BUTTON3_MOTION_MASK* = {7};
       GDK_BUTTON_PRESS_MASK* = {8};
       GDK_BUTTON_RELEASE_MASK* = {9};
       GDK_KEY_PRESS_MASK* = {10};
       GDK_KEY_RELEASE_MASK* = {11};
       GDK_ENTER_NOTIFY_MASK* = {12};
       GDK_LEAVE_NOTIFY_MASK* = {13};
       GDK_FOCUS_CHANGE_MASK* = {14};
       GDK_STRUCTURE_MASK* = {15};
       GDK_PROPERTY_CHANGE_MASK* = {16};
       GDK_VISIBILITY_NOTIFY_MASK* = {17};
       GDK_PROXIMITY_IN_MASK* = {18};
       GDK_PROXIMITY_OUT_MASK* = {19};
       GDK_SUBSTRUCTURE_MASK* = {20};
		GDK_SCROLL_MASK* = {21};
       GDK_ALL_EVENTS_MASK* = {0 .. 21};  (* 0x3FFFFE *)

TYPE	GdkGCValuesMask= SET;		(* Possible values for GCValuesMask *)
CONST
		GDK_GC_FOREGROUND* = {0};  
		GDK_GC_BACKGROUND* = {1};
		GDK_GC_FONT* = {2};
		GDK_GC_FUNCTION* = {3};
		GDK_GC_FILL* = {4};
		GDK_GC_TILE* = {5};
		GDK_GC_STIPPLE* = {6};
		GDK_GC_CLIP_MASK* = {7};
		GDK_GC_SUBWINDOW* = {8};
		GDK_GC_TS_X_ORIGIN* = {9};
		GDK_GC_TS_Y_ORIGIN* = {10};
		GDK_GC_CLIP_X_ORIGIN* = {11};
		GDK_GC_CLIP_Y_ORIGIN* = {12};
		GDK_GC_EXPOSURES* = {13};
		GDK_GC_LINE_WIDTH* = {14};
		GDK_GC_LINE_STYLE* = {15};
		GDK_GC_CAP_STYLE* = {16};
		GDK_GC_JOIN_STYLE* = {17};

TYPE	GdkEventType= INTEGER;	(* enu: GdkEventType *)
CONST  
  GDK_NOTHING * = -1;
  GDK_DELETE * = 0;
  GDK_DESTROY * = 1;
  GDK_EXPOSE * = 2;
  GDK_MOTION_NOTIFY * = 3;
  GDK_BUTTON_PRESS * = 4;
  GDK_2BUTTON_PRESS * = 5;
  GDK_3BUTTON_PRESS * = 6;
  GDK_BUTTON_RELEASE * = 7;
  GDK_KEY_PRESS * = 8;
  GDK_KEY_RELEASE * = 9;
  GDK_ENTER_NOTIFY * = 10;
  GDK_LEAVE_NOTIFY * = 11;
  GDK_FOCUS_CHANGE * = 12;
  GDK_CONFIGURE * = 13;
  GDK_MAP * = 14;
  GDK_UNMAP * = 15;
  GDK_PROPERTY_NOTIFY * = 16;
  GDK_SELECTION_CLEAR * = 17;
  GDK_SELECTION_REQUEST * = 18;
  GDK_SELECTION_NOTIFY * = 19;
  GDK_PROXIMITY_IN * = 20;
  GDK_PROXIMITY_OUT * = 21;
  GDK_DRAG_ENTER * = 22;
  GDK_DRAG_LEAVE * = 23;
  GDK_DRAG_MOTION * = 24;
  GDK_DRAG_STATUS * = 25;
  GDK_DROP_START * = 26;
  GDK_DROP_FINISHED * = 27;
  GDK_CLIENT_EVENT * = 28;
  GDK_VISIBILITY_NOTIFY * = 29;
  GDK_NO_EXPOSE * = 30;
  GDK_SCROLL * = 31;
  GDK_WINDOW_STATE * = 32;
  GDK_SETTING * = 33;

(* end: GdkEventType *)
TYPE	GdkFontType = INTEGER; (* enu: GdkFontType *)
CONST
		GDK_FONT_FONT = 0;    (*  XFontStruct.  *)
		GDK_FONT_FONTSET = 1; (*  XFontSet used for I18N.  *)
 (* end: GdkFontType *)

CONST
		(* selections *)
		GDK_SELECTION_PRIMARY* = 1;
		GDK_SELECTION_SECONDARY* = 2;
		
		(* targets (Target) *)
		GDK_TARGET_BITMAP* = 5;
		GDK_TARGET_COLORMAP* = 7;
		GDK_TARGET_DRAWABLE* = 17;
		GDK_TARGET_PIXMAP* = 20;
		GDK_TARGET_STRING* = 31;
		(* selection types (SelectionType) *)
		GDK_SELECTION_TYPE_ATOM* = 4;
		GDK_SELECTION_TYPE_BITMAP* = 5;
		GDK_SELECTION_TYPE_COLORMAP* = 7;
		GDK_SELECTION_TYPE_DRAWABLE* = 17;
		GDK_SELECTION_TYPE_INTEGER* = 19;
		GDK_SELECTION_TYPE_PIXMAP* = 20;
		GDK_SELECTION_TYPE_WINDOW* = 33;
		GDK_SELECTION_TYPE_STRING* = 31;
		

		(* cursor types *)		
		(* Possible values for SubwindowMode *)  
		GDK_CLIP_BY_CHILDREN* = 0;
		GDK_INCLUDE_INFERIORS* = 1;
		
		(* Possible values for Function *)
		GDK_COPY* = 0;
		GDK_INVERT* = 1;
		GDK_XOR* = 2;
		GDK_CLEAR* = 3;
		GDK_AND* = 4;
		GDK_AND_REVERSE* = 5;
		GDK_AND_INVERT* = 6;
		GDK_NOOP* = 7;
		GDK_OR* = 8;
		GDK_EQUIV* = 9;
		GDK_OR_REVERSE* = 10;
		GDK_COPY_INVERT* = 11;
		GDK_OR_INVERT* = 12;
		GDK_NAND* = 13;
		GDK_SET* = 14;
		(* Possible values for Fill *)  
		GDK_SOLID*= 0;
		GDK_TILED* = 1;
		GDK_STIPPLED* = 2;
		GDK_OPAQUE_STIPPLED* = 3;
		
TYPE  GdkModifierType* = SET; (*  GdkModifierType *)
CONST
       GDK_SHIFT_BIT* = 0;
       GDK_LOCK_BIT* = 1;
       GDK_CONTROL_BIT* = 2;
       GDK_MOD1_BIT* = 3;
       GDK_MOD2_BIT* = 4;
       GDK_MOD3_BIT*  = 5;
       GDK_MOD4_BIT* = 6;
       GDK_MOD5_BIT* = 7;
       GDK_BUTTON1_BIT* = 8;
       GDK_BUTTON2_BIT* = 9;
       GDK_BUTTON3_BIT* = 10;
       GDK_BUTTON4_BIT* = 11;
       GDK_BUTTON5_BIT* = 12;
       GDK_RELEASE_BIT* =  13;
		GDK_MODIFIER_MASK* = 03FFFH; (* end: GdkModifierType *)
		(* GdkModifierType _MASK
  GDK_SHIFT_MASK * = 1;
  GDK_LOCK_MASK * = 2;
  GDK_CONTROL_MASK * = 4;
  GDK_MOD1_MASK * = 8;
  GDK_MOD2_MASK * = 16;
  GDK_MOD3_MASK * = 32;
  GDK_MOD4_MASK * = 64;
  GDK_MOD5_MASK * = 128;
  GDK_BUTTON1_MASK * = 256;
  GDK_BUTTON2_MASK * = 512;
  GDK_BUTTON3_MASK * = 1024;
  GDK_BUTTON4_MASK * = 2048;
  GDK_BUTTON5_MASK * = 4096;
  GDK_RELEASE_MASK * = 1073741824; *)

	
TYPE	GdkNativeWindow = INTEGER;
TYPE	GdkAtom* = INTEGER; 
CONST  GDK_NONE=0;  


TYPE	GdkPoint* = RECORD [untagged]
        x*: INTEGER;
        y*: INTEGER;
      END;
			GdkPoints* = ARRAY [untagged] OF GdkPoint;

TYPE	GdkRectangle* = RECORD [untagged]
				x*     : INTEGER;
				y*     : INTEGER;
				width* : INTEGER;
				height*: INTEGER;
			END;
			pGdkRectangle* = POINTER TO GdkRectangle;
			GdkRectangles* = ARRAY [untagged] OF GdkRectangle;

TYPE	GdkSegment = RECORD[untagged]
				x1: INTEGER;
				y1: INTEGER;
				x2: INTEGER;
				y2: INTEGER;
			END;
			pGdkSegment = POINTER TO GdkSegment;

			GdkSpan = RECORD[untagged]
				x, y, width: INTEGER;
			END;

TYPE  GdkColor* = RECORD [untagged]
				pixel*: INTEGER;
				red*, green*, blue*: SHORTINT;
			END;
			GdkColors = ARRAY [untagged] OF  GdkColor;
	
TYPE	GdkPixbuf*= POINTER TO   RECORD (GObject.GObject)  END;
TYPE	GdkColormap* = POINTER TO LIMITED RECORD (GObject.GObject)
				size*    : INTEGER;
				colors*  : POINTER TO GdkColors;
			END;


TYPE	GdkDrawable* = POINTER TO LIMITED RECORD (GObject.GObject) END;
TYPE	GdkBitmap* = POINTER TO LIMITED RECORD (GdkDrawable) END;
TYPE	GdkPixmap* = POINTER TO LIMITED RECORD (GdkDrawable) END;
TYPE	GdkWindow* = POINTER TO LIMITED RECORD (GdkDrawable) END;
TYPE	GdkImage = POINTER TO LIMITED RECORD  (GObject.GObject);
				type-           : GdkImageType;         
				visual-         : GdkVisual;     (*  visual used to create the image  *)
				byte_order-     : GdkByteOrder;         
				width -         : INTEGER;          
				height-         : INTEGER;          
				depth  -        : SHORTINT;       
				bpp -           : SHORTINT;       (*   bytes per pixel  *)
				bpl -           : SHORTINT;       (*   bytes per line  *)
				bits_per_pixel- : SHORTINT;       (*   bits per pixel  *)
				mem*            : gpointer;
				colormap-       : GdkColormap;   
				windowing_data- : gpointer;      
				END;

TYPE	GdkFont* = POINTER TO LIMITED RECORD [untagged]
				type   : GdkFontType;
				ascent- : INTEGER;
				descent-: INTEGER;
			END;

TYPE	GdkGC* = POINTER TO LIMITED RECORD (GObject.GObject)		END;
TYPE	GdkGCValues* = RECORD [untagged]
				foreground* , 
				background* : GdkColor;
				font*       : GdkFont;
				function*   : GdkFunction;
				fill*       : GdkFill;
				tile*       : GdkPixmap;
				stipple*    : GdkPixmap;
				clip_mask*  : GdkPixmap;
				subwindow_mode*: GdkSubwindowMode;
				ts_x_origin*,   
				ts_y_origin*   : INTEGER;
				clip_x_origin*, 
				clip_y_origin* : INTEGER;
				graphics_exposures: INTEGER;
				line_width*    : INTEGER;
				line_style*    : GdkLineStyle;
				cap_style*     : GdkCapStyle;
				join_style*    : GdkJoinStyle;
			END;

TYPE	GdkKeymap = POINTER TO LIMITED RECORD (GObject.GObject) END;

TYPE	GdkKeymapKey = RECORD [untagged] 
				keycode:INTEGER;
				group:INTEGER;
				level:INTEGER;
			END;

TYPE	GdkVisual* = POINTER TO LIMITED RECORD (GObject.GObject)
				type           : GdkVisualType;
				depth          : INTEGER;
				byte_order     : GdkByteOrder;
				colormap_size  : INTEGER;
				bits_per_rgb   : INTEGER;
				red_mask       : INTEGER;
				red_shift      : INTEGER;
				red_prec       : INTEGER;
				green_mask     : INTEGER;
				green_shift    : INTEGER;
				green_prec     : INTEGER;
				blue_mask      : INTEGER;
				blue_shift     : INTEGER;
				blue_prec      : INTEGER;
			END;

TYPE  GdkDeviceKey = RECORD [untagged]
				keyval   : INTEGER;
				modifiers: GdkModifierType;
			END;

TYPE	GdkDeviceAxis = RECORD [untagged]
				use: GdkAxisUse;
				min,  max: REAL;
			END;

TYPE  GdkDevice = POINTER TO LIMITED RECORD (GObject.GObject)
				name-       : PString;
				source-     : GdkInputSource;
				mode-       : GdkInputMode;
				has_cursor- : gboolean;       (*  TRUE if the X pointer follows device motion  *)
				num_axes-   : INTEGER;
				axes-       : POINTER TO ARRAY [untagged] OF GdkDeviceAxis;
				num_keys-   : INTEGER;
				keys-       : POINTER TO ARRAY [untagged] OF GdkDeviceKey;
			END;
			
TYPE  GdkScreen* = POINTER TO LIMITED RECORD (GObject.GObject)
				closed         : SET ;              (* bit field. closed:1. *)
				normal_gcs     : ARRAY 32 OF GdkGC;
				exposure_gcs   : ARRAY 32 OF GdkGC;
				END;

TYPE	GdkRegion = POINTER TO RECORD [untagged] END;
TYPE	GdkDisplayManager= POINTER TO RECORD [untagged] END;



TYPE	GdkGravity = INTEGER; (* enu: GdkGravity *)
CONST
(*
  GDK_GRAVITY_NORTH_WEST = 1;
  GDK_GRAVITY_NORTH,
  GDK_GRAVITY_NORTH_EAST,
  GDK_GRAVITY_WEST,
  GDK_GRAVITY_CENTER,
  GDK_GRAVITY_EAST,
  GDK_GRAVITY_SOUTH_WEST,
  GDK_GRAVITY_SOUTH,
  GDK_GRAVITY_SOUTH_EAST,
  GDK_GRAVITY_STATIC
*)
 


TYPE	GdkCursor* = POINTER TO LIMITED RECORD [untagged]	END;

TYPE	GdkDragContext = POINTER TO LIMITED RECORD (GObject.GObject) END;
TYPE	GdkRgbCmap = POINTER TO LIMITED RECORD [untagged] 
				colors*:ARRAY 256 OF INTEGER;
				n_colors*:INTEGER;
				info_list:GSList;
			END;
			
TYPE	GdkWindowAttr = POINTER TO RECORD [untagged] 
         title:PString;
         event_mask:INTEGER;
         x, y:INTEGER;
         width:INTEGER;
         height:INTEGER;
         wclass:GdkWindowClass;
         visual:GdkVisual;
         colormap:GdkColormap ;
         window_type:GdkWindowType ;
         cursor:GdkCursor ;
         wmclass_name:PString;
         wmclass_class:PString;
         override_redirect:gboolean;
			END;

TYPE	GdkGeometry = POINTER TO RECORD [untagged] 
					min_width:  INTEGER; 
					min_height: INTEGER; 
					max_width:  INTEGER; 
					max_height: INTEGER; 
					base_width: INTEGER; 
					base_height:INTEGER; 
					width_inc:  INTEGER; 
					height_inc: INTEGER; 
					min_aspect: REAL;
					max_aspect: REAL;
					win_gravity:GdkGravity;
				END;

	

TYPE	GdkDisplay* = POINTER TO LIMITED RECORD (GObject.GObject)	END;
 

TYPE	GdkEvent*  = POINTER TO GdkEventDesc;
			GdkEventDesc* = ABSTRACT RECORD [untagged]
				type*      : GdkEventType;
				window*    : GdkWindow;
				send_event*: BYTE;
			END;

TYPE	GdkEventKey*=POINTER  TO GdkEventKeyDesc;
			GdkEventKeyDesc* = RECORD   (GdkEvent)
				time*: INTEGER;
				state*: SET;
				keyval*: INTEGER;
				length*: INTEGER;
				string*: PString;
				hardware_keycode: SHORTINT;
				group: BYTE;
			END;

TYPE	GdkEventButton*= POINTER  TO GdkEventButtonDesc;
  GdkEventButtonDesc* = RECORD    (GdkEvent)
			time*: INTEGER;
			x*, y*: REAL;
			axes*: POINTER[untagged] TO ARRAY OF REAL;
			state*: SET;
			button*: INTEGER;
			deviceid*: INTEGER;
			x_root*, y_root*: REAL
  END;

TYPE	GdkEventConfigure* = POINTER  TO  RECORD    (GdkEvent)
				x*, y*, width*, height*: INTEGER
			END;

TYPE	GdkEventExpose*  = POINTER TO RECORD (GdkEvent)
    area    * : GdkRectangle;
    region  * : GdkRegion;
    count   * : INTEGER;            (*  If non-zero, how many more events follow.  *)
  END;

TYPE	GdkEventFocus* = POINTER TO RECORD   (GdkEvent)
    in      * : SHORTINT;
  END;

TYPE	GdkEventMotion* = POINTER  TO RECORD  (GdkEvent)
				time    * : INTEGER;
				x*, y*: REAL;
				axes*: POINTER TO ARRAY [untagged] OF REAL;
				state * : SET;
				is_hint * : SHORTINT;
				device  * : GdkDevice;
				x_root*, y_root*: REAL
			END;

TYPE	GdkEventNoExpose = RECORD  (GdkEvent)  END;

TYPE	GdkEventVisibility = RECORD    (GdkEvent)
				state   * : GdkVisibilityState;
			END;

TYPE
			(* DIA: Scroll *)
			GdkEventScroll* = POINTER TO GdkEventScrollDesc;
			GdkEventScrollDesc* = RECORD    (GdkEvent)
				time    * :  INTEGER;
				x       * : REAL;
				y       * : REAL;
				state * : SET;
				direction- : GdkScrollDirection;
				device  * : GdkDevice;
				x_root  * : REAL;
				y_root  * : REAL;
			END;

TYPE	GdkEventProperty = RECORD    (GdkEvent)
    atom    * : GdkAtom;
    time    * :  INTEGER;
    state * : SET;
  END;

TYPE	GdkEventProximity = RECORD     (GdkEvent)
    time    * :  INTEGER;
    device  * : GdkDevice;
  END;

TYPE	GdkEventClient = RECORD (GdkEvent)
    message_type: GdkAtom;
    data_format : BYTE;
    data * : RECORD [union]
      b: ARRAY [untagged] 20 OF BYTE;
      s: ARRAY [untagged] 10 OF SHORTINT;
      l: ARRAY [untagged]  5 OF INTEGER;
    END;
  END;

TYPE	GdkEventCrossing = RECORD    (GdkEvent)
    subwindow : GdkAtom;
    time    * :  INTEGER;
    x       * : REAL;
    y       * : REAL;
    x_root  * : REAL;
    y_root  * : REAL;
    mode    * : GdkCrossingMode;
    detail  * : GdkNotifyType;
    focus   * : gboolean;
    state * : SET;
  END;


TYPE	GdkEventSelection = RECORD    (GdkEvent)
    selection : GdkAtom;
    target  * : GdkAtom;
    property* : GdkAtom;
    time    * :  INTEGER;
    requestor : GdkNativeWindow;
  END;

TYPE	GdkEventDND = RECORD     (GdkEvent)
    context * : GdkDragContext;
    time    * :  INTEGER;
    x_root  * : SHORTINT;
    y_root  * : SHORTINT;
  END;

TYPE	GdkEventWindowState = RECORD  (GdkEvent)
    changed_mask  * : GdkWindowState;
    new_window_state: GdkWindowState;
  END;

TYPE	GdkEventSetting = RECORD    (GdkEvent)
    action  * : GdkSettingAction;
    name    * : PString;
  END;


TYPE 
  GdkInputFunction = PROCEDURE (data :gpointer; source: INTEGER; condition: GdkInputCondition);
  GdkDestroyNotify = PROCEDURE (data: gpointer);
	ChildFunc = PROCEDURE(w:GdkWindow; p:gpointer): gboolean;



(* General — Library initialization and miscellaneous functions *)

PROCEDURE [ccall] gdk_init* (VAR [nil] argc: INTEGER; VAR [nil] argv: APString);
PROCEDURE [ccall] gdk_init_check(VAR argc: INTEGER; VAR argv: APString): BOOLEAN;
PROCEDURE [ccall] gdk_parse_args(VAR argc: INTEGER; VAR argv: APString);
PROCEDURE [ccall] gdk_set_locale(): PString;
PROCEDURE [ccall] gdk_get_program_class(): PString;
PROCEDURE [ccall] gdk_set_program_class(program_class: PString);
PROCEDURE [ccall] gdk_exit(error_code: INTEGER);

PROCEDURE [ccall] gdk_error_trap_push();
PROCEDURE [ccall] gdk_error_trap_pop(): INTEGER;
PROCEDURE [ccall] gdk_get_display(): PString;
PROCEDURE [ccall] gdk_get_display_arg_name(): PString;
PROCEDURE [ccall] gdk_set_use_xshm (use_xshm: gboolean);
PROCEDURE [ccall] gdk_get_use_xshm (): gboolean;

PROCEDURE [ccall] gdk_pointer_grab(window: GdkWindow; owner_events: gboolean; event_mask: GdkEventMask; confine_to: GdkWindow; cursor: GdkCursor; time:  INTEGER): GdkGrabStatus;
PROCEDURE [ccall] gdk_keyboard_grab(window: GdkWindow; owner_events: gboolean; time:  INTEGER): GdkGrabStatus;
PROCEDURE [ccall] gdk_pointer_ungrab(time:  INTEGER);
PROCEDURE [ccall] gdk_keyboard_ungrab(time:  INTEGER);
PROCEDURE [ccall] gdk_pointer_is_grabbed(): BOOLEAN;

PROCEDURE [ccall] gdk_screen_width*(): INTEGER; (*  in pixels *)
PROCEDURE [ccall] gdk_screen_height*(): INTEGER;
PROCEDURE [ccall] gdk_screen_width_mm*(): INTEGER;
PROCEDURE [ccall] gdk_screen_height_mm*(): INTEGER;
PROCEDURE [ccall] gdk_beep*;
PROCEDURE [ccall] gdk_flush;
PROCEDURE [ccall] gdk_set_double_click_time(msec: INTEGER);


(*  Conversion functions between wide char and multibyte strings.   *)
PROCEDURE [ccall] gdk_wcstombs*(IN src: UString): PString;
PROCEDURE [ccall] gdk_mbstowcs*(OUT dest: UString; IN src: String; dest_max: INTEGER): INTEGER;
(*  Miscellaneous  *)
PROCEDURE [ccall] gdk_event_send_client_message(event: GdkEvent; winid: GdkNativeWindow): BOOLEAN;
PROCEDURE [ccall] gdk_event_send_clientmessage_toall(event: GdkEvent);
PROCEDURE [ccall] gdk_event_send_client_message_for_display(display: GdkDisplay; event: GdkEvent; winid: GdkNativeWindow): BOOLEAN;
PROCEDURE [ccall] gdk_notify_startup_complete();

(* display  *)
	PROCEDURE [ccall] gdk_display_open(IN display_name: String): GdkDisplay;
	PROCEDURE [ccall] gdk_display_get_name(display: GdkDisplay): PString;
	PROCEDURE [ccall] gdk_display_get_n_screens(display: GdkDisplay):  INTEGER;
	PROCEDURE [ccall] gdk_display_get_screen(display: GdkDisplay; screen_num: INTEGER): GdkScreen;
	PROCEDURE [ccall] gdk_display_get_default_screen(display: GdkDisplay): GdkScreen;
	PROCEDURE [ccall] gdk_display_pointer_ungrab(display: GdkDisplay; time:  INTEGER);
	PROCEDURE [ccall] gdk_display_keyboard_ungrab(display: GdkDisplay; time:  INTEGER);
	PROCEDURE [ccall] gdk_display_pointer_is_grabbed(display: GdkDisplay): BOOLEAN;
	PROCEDURE [ccall] gdk_display_beep(display: GdkDisplay);
	PROCEDURE [ccall] gdk_display_sync(display: GdkDisplay);
	PROCEDURE [ccall] gdk_display_flush(display: GdkDisplay);
	PROCEDURE [ccall] gdk_display_close(display: GdkDisplay);
	PROCEDURE [ccall] gdk_display_list_devices(display: GdkDisplay): GLib.GList;
	PROCEDURE [ccall] gdk_display_get_event(display: GdkDisplay): GdkEvent;
	PROCEDURE [ccall] gdk_display_peek_event(display: GdkDisplay): GdkEvent;
	PROCEDURE [ccall] gdk_display_put_event(display: GdkDisplay; event: GdkEvent);
	PROCEDURE [ccall] gdk_display_add_client_message_filter(display: GdkDisplay; message_type: GdkAtom; func: GdkFilterFunc; data: gpointer);
	PROCEDURE [ccall] gdk_display_set_double_click_time(display: GdkDisplay; msec: INTEGER);
	PROCEDURE [ccall] gdk_display_set_double_click_distance(display: GdkDisplay; distance: INTEGER);
	PROCEDURE [ccall] gdk_display_get_default(): GdkDisplay;
	PROCEDURE [ccall] gdk_display_get_core_pointer(display: GdkDisplay): GdkDevice;
	PROCEDURE [ccall] gdk_display_get_pointer(display: GdkDisplay; OUT screen: GdkScreen; OUT x, y: INTEGER; OUT mask: GdkModifierType);
	PROCEDURE [ccall] gdk_display_get_window_at_pointer(display: GdkDisplay; OUT win_x, win_y: INTEGER): GdkWindow ;
	PROCEDURE [ccall] gdk_display_open_default_libgtk_only(): GdkDisplay;
	PROCEDURE [ccall] gdk_display_supports_cursor_alpha(display: GdkDisplay): BOOLEAN;
	PROCEDURE [ccall] gdk_display_supports_cursor_color(display: GdkDisplay): BOOLEAN;
	PROCEDURE [ccall] gdk_display_get_default_cursor_size(display: GdkDisplay): INTEGER;
	PROCEDURE [ccall] gdk_display_get_maximal_cursor_size(display: GdkDisplay; OUT width, height: INTEGER);
	PROCEDURE [ccall] gdk_display_get_default_group(display: GdkDisplay): GdkWindow;
(* display manager *)
	PROCEDURE [ccall] gdk_display_manager_get():  GdkDisplayManager;
	PROCEDURE [ccall] gdk_display_manager_get_default_display(display_manager: GdkDisplayManager): GdkDisplay;
	PROCEDURE [ccall] gdk_display_manager_set_default_display(display_manager: GdkDisplayManager; display: GdkDisplay);
	PROCEDURE [ccall] gdk_display_manager_list_displays(display_manager: GdkDisplayManager): GSList;
(* Screen  *)
	PROCEDURE [ccall] gdk_screen_get_default_colormap(screen: GdkScreen): GdkColormap;
	PROCEDURE [ccall] gdk_screen_set_default_colormap(screen: GdkScreen; colormap: GdkColormap);
	PROCEDURE [ccall] gdk_screen_get_system_colormap(screen: GdkScreen): GdkColormap;
	PROCEDURE [ccall] gdk_screen_get_system_visual(screen: GdkScreen): GdkVisual;
	PROCEDURE [ccall] gdk_screen_get_rgb_colormap(screen: GdkScreen): GdkColormap;
	PROCEDURE [ccall] gdk_screen_get_rgb_visual(screen: GdkScreen): GdkVisual;
	PROCEDURE [ccall] gdk_screen_get_root_window(screen: GdkScreen): GdkWindow;
	PROCEDURE [ccall] gdk_screen_get_toplevel_windows(screen: GdkScreen):GLib.GList;
	PROCEDURE [ccall] gdk_screen_get_display(screen: GdkScreen): GdkDisplay;
	PROCEDURE [ccall] gdk_screen_get_number(screen: GdkScreen): INTEGER;
	PROCEDURE [ccall] gdk_screen_get_width(screen: GdkScreen): INTEGER;
	PROCEDURE [ccall] gdk_screen_get_height(screen: GdkScreen): INTEGER;
	PROCEDURE [ccall] gdk_screen_get_width_mm(screen: GdkScreen): INTEGER;
	PROCEDURE [ccall] gdk_screen_get_height_mm(screen: GdkScreen): INTEGER;
	PROCEDURE [ccall] gdk_screen_list_visuals(screen: GdkScreen): GdkVisual;
	PROCEDURE [ccall] gdk_screen_make_display_name(IN screen: GdkScreen): PString;
	PROCEDURE [ccall] gdk_screen_get_n_monitors(screen: GdkScreen): INTEGER;
	PROCEDURE [ccall] gdk_screen_get_monitor_geometry(screen: GdkScreen; monitor_num: INTEGER; dest: pGdkRectangle);
	PROCEDURE [ccall] gdk_screen_get_monitor_at_point(screen: GdkScreen; x: INTEGER; y: INTEGER): INTEGER;
	PROCEDURE [ccall] gdk_screen_get_monitor_at_window(screen: GdkScreen; window:GdkWindow ): INTEGER;
	PROCEDURE [ccall] gdk_screen_broadcast_client_message(screen: GdkScreen; event: GdkEvent);
	PROCEDURE [ccall] gdk_screen_get_default(): GdkScreen;
	PROCEDURE [ccall] gdk_screen_get_setting (screen: GdkScreen; IN name: String; value: GdkAtom): BOOLEAN;

	PROCEDURE [ccall] gdk_spawn_on_screen(screen: GdkScreen; IN working_directory: String; IN argv, envp: APString; flags: GLib.GSpawnFlags; child_setup: GLib.GSpawnChildSetupFunc; user_data: gpointer; 
	OUT child_pid: INTEGER; 
	OUT error: GLib.GError): BOOLEAN;
	
	PROCEDURE [ccall] gdk_spawn_on_screen_with_pipes(screen: GdkScreen; IN working_directory: String; IN argv, envp: APString; flags: GLib.GSpawnFlags; child_setup: GLib.GSpawnChildSetupFunc; user_data: gpointer; OUT child_pid, standard_input, standard_output, standard_error:INTEGER; OUT error: GLib.GError): BOOLEAN;
	PROCEDURE [ccall] gdk_spawn_command_line_on_screen(screen: GdkScreen; IN command_line: String; OUT error: GLib.GError): BOOLEAN;

(*  Points, Rectangles and Regions   *)
	PROCEDURE [ccall] gdk_region_new(): GdkRegion;
	PROCEDURE [ccall] gdk_region_polygon(IN points: GdkPoints; npoints: INTEGER; fill_rule: GdkFillRule): GdkRegion;
	PROCEDURE [ccall] gdk_region_copy(region: GdkRegion): GdkRegion;
	PROCEDURE [ccall] gdk_region_rectangle(rectangle: pGdkRectangle): GdkRegion;
	PROCEDURE [ccall] gdk_region_destroy(region: GdkRegion);
	PROCEDURE [ccall] gdk_region_get_clipbox(region: GdkRegion; rectangle: pGdkRectangle);
	PROCEDURE [ccall] gdk_region_get_rectangles(region: GdkRegion; OUT rectangles: POINTER TO GdkRectangles; OUT n_rectangles: INTEGER);
	PROCEDURE [ccall] gdk_region_empty(region: GdkRegion): BOOLEAN;
	PROCEDURE [ccall] gdk_region_equal(region1: GdkRegion; region2: GdkRegion): BOOLEAN;
	PROCEDURE [ccall] gdk_region_point_in(region: GdkRegion; x: INTEGER; y: INTEGER): BOOLEAN;
	PROCEDURE [ccall] gdk_region_rect_in(region: GdkRegion; rect: pGdkRectangle): GdkOverlapType;
	PROCEDURE [ccall] gdk_region_offset(region: GdkRegion; dx: INTEGER; dy: INTEGER);
	PROCEDURE [ccall] gdk_region_shrink(region: GdkRegion; dx: INTEGER; dy: INTEGER);
	PROCEDURE [ccall] gdk_region_union_with_rect(region: GdkRegion; rect: pGdkRectangle);
	PROCEDURE [ccall] gdk_region_intersect(source1: GdkRegion; source2: GdkRegion);
	PROCEDURE [ccall] gdk_region_union(source1: GdkRegion; source2: GdkRegion);
	PROCEDURE [ccall] gdk_region_subtract(source1: GdkRegion; source2: GdkRegion);
	PROCEDURE [ccall] gdk_region_xor(source1: GdkRegion; source2: GdkRegion);
	PROCEDURE [ccall] gdk_region_spans_intersect_foreach(region: GdkRegion; IN spans: ARRAY [untagged] OF GdkSpan; n_spans: INTEGER; sorted: gboolean; function: GdkSpanFunc; data: gpointer);
	(*  Rectangle utilities  *)
	PROCEDURE [ccall] gdk_rectangle_intersect(IN src1,src2: GdkRectangle; OUT dest: GdkRectangle): BOOLEAN;
	PROCEDURE [ccall] gdk_rectangle_union(IN src1,src2: GdkRectangle; OUT dest: GdkRectangle);
(* Graphics Contexts *)
PROCEDURE [ccall] gdk_gc_new*(drawable: GdkDrawable): GdkGC;
PROCEDURE [ccall] gdk_gc_new_with_values*(drawable: GdkDrawable; VAR values: GdkGCValues; values_mask: GdkGCValuesMask): GdkGC;
PROCEDURE [ccall] gdk_gc_ref*(gc: GdkGC) (*: GdkGC*);
PROCEDURE [ccall] gdk_gc_unref*(gc: GdkGC);

PROCEDURE [ccall] gdk_gc_get_values(gc: GdkGC; VAR values: GdkGCValues);
PROCEDURE [ccall] gdk_gc_set_values(gc: GdkGC; VAR values: GdkGCValues; values_mask: GdkGCValuesMask);
PROCEDURE [ccall] gdk_gc_set_foreground*(gc: GdkGC; IN color: GdkColor);
PROCEDURE [ccall] gdk_gc_set_background*(gc: GdkGC; IN color: GdkColor);

PROCEDURE [ccall] gdk_gc_set_function(gc: GdkGC; function: GdkFunction);
PROCEDURE [ccall] gdk_gc_set_fill(gc: GdkGC; fill: GdkFill);
PROCEDURE [ccall] gdk_gc_set_tile(gc: GdkGC; tile: GdkPixmap);
PROCEDURE [ccall] gdk_gc_set_stipple(gc: GdkGC; stipple: GdkPixmap);
PROCEDURE [ccall] gdk_gc_set_ts_origin(gc: GdkGC; x: INTEGER; y: INTEGER);
PROCEDURE [ccall] gdk_gc_set_clip_origin*(gc: GdkGC; x: INTEGER; y: INTEGER);
PROCEDURE [ccall] gdk_gc_set_clip_rectangle*(gc: GdkGC; rectangle: pGdkRectangle);
PROCEDURE [ccall] gdk_gc_set_clip_mask(gc: GdkGC; mask: GdkPixmap);
PROCEDURE [ccall] gdk_gc_set_clip_region(gc: GdkGC; region: GdkRegion);
PROCEDURE [ccall] gdk_gc_set_subwindow(gc: GdkGC; mode: GdkSubwindowMode);
PROCEDURE [ccall] gdk_gc_set_exposures*(gc: GdkGC; exposures: BOOLEAN);
PROCEDURE [ccall] gdk_gc_set_line_attributes*(gc: GdkGC; line_width: INTEGER; line_style: GdkLineStyle; cap_style: GdkCapStyle; join_style: GdkJoinStyle);
PROCEDURE [ccall] gdk_gc_set_dashes*(gc: GdkGC; dash_offset: INTEGER; dash_list: ARRAY OF BYTE; n: INTEGER);
PROCEDURE [ccall] gdk_gc_set_font(gc: GdkGC; font: GdkFont);
PROCEDURE [ccall] gdk_gc_offset(gc: GdkGC; x_offset, y_offset: INTEGER);
PROCEDURE [ccall] gdk_gc_copy(dst_gc: GdkGC; src_gc: GdkGC);
PROCEDURE [ccall] gdk_gc_set_colormap(gc: GdkGC; colormap: GdkColormap);
PROCEDURE [ccall] gdk_gc_get_colormap(gc: GdkGC): GdkColormap;
PROCEDURE [ccall] gdk_gc_set_rgb_fg_color(gc: GdkGC; IN color: GdkColor);
PROCEDURE [ccall] gdk_gc_set_rgb_bg_color(gc: GdkGC; IN color: GdkColor);
PROCEDURE [ccall] gdk_gc_get_screen(gc: GdkGC):GdkScreen ;


(* Drawing Primitives *)
	(*  Manipulation of drawables  *)
	PROCEDURE [ccall] gdk_drawable_set_data(drawable: GdkDrawable; IN key: String; data: gpointer; destroy_func: GLib.GDestroyNotify);
	PROCEDURE [ccall] gdk_drawable_get_data(drawable: GdkDrawable; IN key: String): gpointer;
	PROCEDURE [ccall] gdk_drawable_get_size(drawable: GdkDrawable; OUT width, height: INTEGER);
	PROCEDURE [ccall] gdk_drawable_set_colormap(drawable: GdkDrawable; colormap: GdkColormap);
	PROCEDURE [ccall] gdk_drawable_get_colormap(drawable: GdkDrawable): GdkColormap;
	PROCEDURE [ccall] gdk_drawable_get_visual(drawable: GdkDrawable): GdkVisual;
	PROCEDURE [ccall] gdk_drawable_get_depth(drawable: GdkDrawable): INTEGER;
	PROCEDURE [ccall] gdk_drawable_get_screen(drawable: GdkDrawable): GdkScreen;
	PROCEDURE [ccall] gdk_drawable_get_display(drawable: GdkDrawable): GdkDisplay;
	PROCEDURE [ccall] gdk_drawable_get_image(drawable: GdkDrawable; x, y: INTEGER; width: INTEGER; height: INTEGER): GdkImage;
	PROCEDURE [ccall] gdk_drawable_copy_to_image(drawable: GdkDrawable; image: GdkImage; src_x: INTEGER; src_y: INTEGER; dest_x: INTEGER; dest_y: INTEGER; width: INTEGER; height: INTEGER): GdkImage;
	PROCEDURE [ccall] gdk_drawable_get_clip_region(drawable: GdkDrawable): GdkRegion;
	PROCEDURE [ccall] gdk_drawable_get_visible_region(drawable: GdkDrawable): GdkRegion;
	PROCEDURE [ccall] gdk_drawable_ref(drawable: GdkDrawable): GdkDrawable;
	PROCEDURE [ccall] gdk_drawable_unref*(drawable: GdkDrawable);

	(*  Drawing  *)
	PROCEDURE [ccall] gdk_draw_point(drawable: GdkDrawable; gc: GdkGC; x: INTEGER; y: INTEGER);
	PROCEDURE [ccall] gdk_draw_points(drawable: GdkDrawable; gc: GdkGC; IN points: GdkPoints; npoints: INTEGER);
	PROCEDURE [ccall] gdk_draw_line*(drawable: GdkDrawable; gc: GdkGC; x1,y1, x2,y2: INTEGER);
	PROCEDURE [ccall] gdk_draw_rectangle*(drawable: GdkDrawable; gc: GdkGC; filled: gboolean; x,y, width,height: INTEGER);
	PROCEDURE [ccall] gdk_draw_arc*(drawable: GdkDrawable; gc: GdkGC; filled: gboolean; x,y, width,height,angle1,angle2: INTEGER);
	PROCEDURE [ccall] gdk_draw_polygon*(drawable: GdkDrawable; gc: GdkGC; filled: gboolean; IN points: GdkPoints; npoints: INTEGER);
	PROCEDURE [ccall] gdk_draw_lines*(drawable: GdkDrawable; gc: GdkGC; IN points: GdkPoints; npoints: INTEGER);
	PROCEDURE [ccall] gdk_draw_segments(drawable: GdkDrawable; gc: GdkGC; IN segs: ARRAY [untagged] OF GdkSegment; nsegs: INTEGER);
	

	PROCEDURE [ccall] gdk_draw_string*(drawable: GdkDrawable; font: GdkFont; gc: GdkGC; x, y: INTEGER; IN string: String);
	PROCEDURE [ccall] gdk_draw_text*(drawable: GdkDrawable; font: GdkFont; gc: GdkGC; x, y: INTEGER; IN text: String; text_length: INTEGER);
	PROCEDURE [ccall] gdk_draw_text_wc*(drawable: GdkDrawable; font: GdkFont; gc: GdkGC; x, y: INTEGER; IN text: UString; text_length: INTEGER);

	PROCEDURE [ccall] gdk_draw_drawable*(drawable: GdkDrawable; gc: GdkGC; src: GdkDrawable; xsrc: INTEGER; ysrc: INTEGER; xdest: INTEGER; ydest: INTEGER; width: INTEGER; height: INTEGER);
	PROCEDURE [ccall] gdk_draw_image(drawable: GdkDrawable; gc: GdkGC; image: GdkImage; xsrc: INTEGER; ysrc: INTEGER; xdest: INTEGER; ydest: INTEGER; width: INTEGER; height: INTEGER);
	PROCEDURE [ccall] gdk_draw_pixbuf(drawable: GdkDrawable; gc: GdkGC; pixbuf:GdkPixbuf ; src_x: INTEGER; src_y: INTEGER; dest_x: INTEGER; dest_y: INTEGER; width: INTEGER; height: INTEGER; dither: GdkRgbDither; x_dither: INTEGER; y_dither: INTEGER);

	PROCEDURE [ccall] gdk_draw_glyphs(drawable: GdkDrawable; gc: GdkGC; font:Pango.PangoFont ; x, y: INTEGER; glyphs:Pango.PangoGlyphString);
	PROCEDURE [ccall] gdk_draw_layout*(drawable: GdkDrawable; gc: GdkGC; x, y: INTEGER; layout:Pango.PangoLayout);
	PROCEDURE [ccall] gdk_draw_layout_with_colors(drawable: GdkDrawable; gc: GdkGC; x, y: INTEGER; layout:Pango.PangoLayout; IN foreground, background: GdkColor);
	PROCEDURE [ccall] gdk_draw_layout_line*(drawable: GdkDrawable; gc: GdkGC; x, y: INTEGER; line: Pango.PangoLayoutLine);
	PROCEDURE [ccall] gdk_draw_layout_line_with_colors(drawable: GdkDrawable; gc: GdkGC; x, y: INTEGER; line:Pango.PangoLayoutLine; IN foreground, background: GdkColor);


(* Bitmaps and Pixmaps - Offscreen drawables*) 
PROCEDURE [ccall] gdk_pixmap_new*(drawable: GdkDrawable; width: INTEGER; height: INTEGER; depth: INTEGER): GdkPixmap; 
PROCEDURE [ccall] gdk_bitmap_create_from_data(drawable: GdkDrawable; data: PBYTES; width:  INTEGER; height: INTEGER): GdkBitmap;
PROCEDURE [ccall] gdk_pixmap_create_from_data(drawable: GdkDrawable; data: PBYTES; width: INTEGER; height: INTEGER; depth: INTEGER; IN fg, bg: GdkColor): GdkPixmap;
PROCEDURE [ccall] gdk_pixmap_create_from_xpm*(drawable: GdkDrawable; VAR mask: GdkBitmap; IN [nil] transparent_color: GdkColor; IN filename: String): GdkPixmap;
PROCEDURE [ccall] gdk_pixmap_colormap_create_from_xpm(drawable: GdkDrawable; colormap: GdkColormap; VAR mask: GdkBitmap; transparent_color: GdkColor; IN filename: String): GdkPixmap;
PROCEDURE [ccall] gdk_pixmap_create_from_xpm_d(drawable: GdkDrawable; VAR mask: GdkBitmap; transparent_color: GdkColor; data: PBYTES): GdkPixmap;
PROCEDURE [ccall] gdk_pixmap_colormap_create_from_xpm_d(drawable: GdkDrawable; colormap: GdkColormap; VAR mask: GdkBitmap; transparent_color: GdkColor; data: PBYTES): GdkPixmap;
(*  Functions to create/lookup pixmaps from their native equivalents  *) 
PROCEDURE [ccall] gdk_pixmap_foreign_new(anid: GdkNativeWindow): GdkDrawable;
PROCEDURE [ccall] gdk_pixmap_lookup(anid: GdkNativeWindow): GdkDrawable;
PROCEDURE [ccall] gdk_pixmap_foreign_new_for_display(display: GdkDisplay; anid: GdkNativeWindow): GdkDrawable;
PROCEDURE [ccall] gdk_pixmap_lookup_for_display(display: GdkDisplay; anid: GdkNativeWindow): GdkDrawable;

(*  GdkRGB — Renders RGB, grayscale, or indexed image data to a GdkDrawable  *)
	PROCEDURE [ccall] gdk_rgb_init();
	PROCEDURE [ccall] gdk_rgb_xpixel_from_rgb(rgb:  INTEGER): INTEGER;
	PROCEDURE [ccall] gdk_rgb_gc_set_foreground(gc: GdkGC; rgb: INTEGER);
	PROCEDURE [ccall] gdk_rgb_gc_set_background(gc: GdkGC; rgb: INTEGER);
	PROCEDURE [ccall] gdk_rgb_find_color(colormap: GdkColormap; VAR color: GdkColor);

	PROCEDURE [ccall] gdk_draw_rgb_image(drawable: GdkDrawable; gc: GdkGC; x, y: INTEGER; width: INTEGER; height: INTEGER; dith: GdkRgbDither; rgb_buf: PBYTES; rowstride: INTEGER);
	PROCEDURE [ccall] gdk_draw_rgb_image_dithalign(drawable: GdkDrawable; gc: GdkGC; x, y: INTEGER; width: INTEGER; height: INTEGER; dith: GdkRgbDither; rgb_buf: PBYTES; rowstride: INTEGER; xdith: INTEGER; ydith: INTEGER);
	PROCEDURE [ccall] gdk_draw_rgb_32_image(drawable: GdkDrawable; gc: GdkGC; x, y: INTEGER; width: INTEGER; height: INTEGER; dith: GdkRgbDither; buf: PBYTES; rowstride: INTEGER);
	PROCEDURE [ccall] gdk_draw_rgb_32_image_dithalign(drawable: GdkDrawable; gc: GdkGC; x, y: INTEGER; width: INTEGER; height: INTEGER; dith: GdkRgbDither; buf: PBYTES; rowstride: INTEGER; xdith: INTEGER; ydith: INTEGER);
	PROCEDURE [ccall] gdk_draw_gray_image(drawable: GdkDrawable; gc: GdkGC; x, y: INTEGER; width: INTEGER; height: INTEGER; dith: GdkRgbDither; buf: PBYTES; rowstride: INTEGER);
	PROCEDURE [ccall] gdk_draw_indexed_image(drawable: GdkDrawable; gc: GdkGC; x, y: INTEGER; width: INTEGER; height: INTEGER; dith: GdkRgbDither; buf: PBYTES; rowstride: INTEGER; cmap: GdkRgbCmap);

	PROCEDURE [ccall] gdk_rgb_cmap_new(IN colors: ARRAY [untagged] OF INTEGER; n_colors: INTEGER): GdkRgbCmap;
	PROCEDURE [ccall] gdk_rgb_cmap_free(cmap: GdkRgbCmap);
	PROCEDURE [ccall] gdk_rgb_set_verbose(verbose: gboolean);
	(*  experimental colormap stuff  *)
	PROCEDURE [ccall] gdk_rgb_set_install(install: gboolean);
	PROCEDURE [ccall] gdk_rgb_set_min_colors(min_colors: INTEGER);
	PROCEDURE [ccall] gdk_rgb_get_colormap(): GdkColormap;
	PROCEDURE [ccall] gdk_rgb_get_visual(): GdkVisual;
	PROCEDURE [ccall] gdk_rgb_ditherable(): BOOLEAN;
	(* Images - A client-side area for bit-mapped graphics *)
	PROCEDURE [ccall] gdk_image_new(type: GdkImageType; visual: GdkVisual; width: INTEGER; height: INTEGER): GdkImage;
	PROCEDURE [ccall] gdk_image_put_pixel(image: GdkImage; x, y: INTEGER; pixel:  INTEGER);
	PROCEDURE [ccall] gdk_image_get_pixel(image: GdkImage; x: INTEGER; y: INTEGER): INTEGER;
	PROCEDURE [ccall] gdk_image_set_colormap(image: GdkImage; colormap: GdkColormap);
	PROCEDURE [ccall] gdk_image_get_colormap(image: GdkImage): GdkColormap;
	PROCEDURE [ccall] gdk_image_get(drawable: GdkDrawable; x, y: INTEGER; width: INTEGER; height: INTEGER): GdkImage;
	PROCEDURE [ccall] gdk_image_ref(image: GdkImage): GdkImage;
	PROCEDURE [ccall] gdk_image_unref(image: GdkImage);


(* Pixbufs - Functions for rendering pixbufs on drawables*)
	PROCEDURE [ccall] gdk_pixbuf_render_threshold_alpha(pixbuf:GdkPixbuf ; bitmap: GdkBitmap; src_x,src_y,dest_x,dest_y,width,height: INTEGER; alpha_threshold: INTEGER);

	PROCEDURE [ccall] gdk_pixbuf_render_to_drawable(pixbuf:GdkPixbuf ; drawable: GdkBitmap; gc: GdkGC; src_x,src_y,dest_x,dest_y,width,height,dither: GdkRgbDither; x_dither,y_dither: INTEGER);
	PROCEDURE [ccall] gdk_pixbuf_render_to_drawable_alpha(pixbuf:GdkPixbuf ; drawable: GdkBitmap; src_x,src_y,dest_x,dest_y,width,height: INTEGER; alpha_mode: GdkPixbufAlphaMode; alpha_threshold: INTEGER; dither: GdkRgbDither; x_dither, y_dither: INTEGER);
	PROCEDURE [ccall] gdk_pixbuf_render_pixmap_and_mask_for_colormap(pixbuf:GdkPixbuf ; colormap: GdkColormap; OUT pixmap_return: GdkPixmap; OUT mask_return: GdkPixmap; alpha_threshold: INTEGER);
	PROCEDURE [ccall] gdk_pixbuf_render_pixmap_and_mask(pixbuf:GdkPixbuf ; OUT pixmap_return: GdkPixmap; OUT mask_return: GdkPixmap; alpha_threshold: INTEGER);

	(*  Fetching a region from a drawable  *)
	PROCEDURE [ccall] gdk_pixbuf_get_from_drawable(dest:GdkPixbuf ; src: GdkBitmap; cmap: GdkColormap; src_x,src_y,dest_x,dest_y,width,height: INTEGER):GdkPixbuf ;
	PROCEDURE [ccall] gdk_pixbuf_get_from_image(dest:GdkPixbuf ; src: GdkImage; cmap: GdkColormap; src_x,src_y,dest_x,dest_y,width,height: INTEGER):GdkPixbuf ;

(*  Colormaps and Colors  *)
	PROCEDURE [ccall] gdk_color_copy(IN color: GdkColor):POINTER TO GdkColor;
	PROCEDURE [ccall] gdk_color_free(color: POINTER TO GdkColor);

	PROCEDURE [ccall] gdk_color_parse*(IN spec: String; OUT color: GdkColor): INTEGER;
	PROCEDURE [ccall] gdk_color_hash(IN colora: GdkColor): INTEGER;
	PROCEDURE [ccall] gdk_color_equal(IN colora, colorb: GdkColor): BOOLEAN;

	PROCEDURE [ccall] gdk_colormap_new(visual:GdkVisual ; allocate: gboolean): GdkColormap;
	PROCEDURE [ccall] gdk_colormap_ref(cmap: GdkColormap): GdkColormap;
	PROCEDURE [ccall] gdk_colormap_unref(cmap: GdkColormap);
	PROCEDURE [ccall] gdk_colormap_get_system*(): GdkColormap;

	PROCEDURE [ccall] gdk_colormap_get_screen(cmap: GdkColormap): GdkScreen;
	PROCEDURE [ccall] gdk_colormap_get_system_size(): INTEGER;


	PROCEDURE [ccall] gdk_colormap_alloc_color*(colormap: GdkColormap; VAR color: GdkColor; writeable, best_match: gboolean): BOOLEAN;
	PROCEDURE [ccall] gdk_colormap_alloc_colors(colormap: GdkColormap; IN colors: GdkColors; ncolors: INTEGER; writeable, best_match: gboolean; OUT success: gboolean):INTEGER;
	PROCEDURE [ccall] gdk_colormap_free_colors(colormap: GdkColormap; IN colors: GdkColors; ncolors: INTEGER);
	PROCEDURE [ccall] gdk_colormap_query_color(colormap: GdkColormap; pixel: INTEGER; VAR result: GdkColor);
	PROCEDURE [ccall] gdk_colormap_get_visual(colormap: GdkColormap):GdkVisual ;

	(*  The following functions are deprecated  *)
	PROCEDURE [ccall] gdk_colormap_change*(colormap: GdkColormap; ncolors: INTEGER);
	PROCEDURE [ccall] gdk_colors_store(colormap: GdkColormap; IN colors: GdkColors; ncolors: INTEGER); 
	PROCEDURE [ccall] gdk_color_white(colormap: GdkColormap; OUT color: GdkColor): INTEGER;
	PROCEDURE [ccall] gdk_color_black(colormap: GdkColormap; OUT color: GdkColor): INTEGER; 
	PROCEDURE [ccall] gdk_color_alloc(colormap: GdkColormap; OUT color: GdkColor): INTEGER;
	PROCEDURE [ccall] gdk_color_change(colormap: GdkColormap; VAR color: GdkColor): INTEGER;


(* Visuals - Low-level display hardware information *)
	PROCEDURE [ccall] gdk_visual_get_system*(): GdkVisual;
	PROCEDURE [ccall] gdk_visual_get_best(): GdkVisual;
	PROCEDURE [ccall] gdk_visual_get_best_depth(): INTEGER;
	PROCEDURE [ccall] gdk_visual_get_best_type(): GdkVisualType;
	PROCEDURE [ccall] gdk_visual_get_best_with_depth(depth: INTEGER): GdkVisual;
	PROCEDURE [ccall] gdk_visual_get_best_with_type(visual_type: GdkVisualType): GdkVisual;
	PROCEDURE [ccall] gdk_visual_get_best_with_both(depth: INTEGER; visual_type: GdkVisualType): GdkVisual;

	PROCEDURE [ccall] gdk_query_depths(OUT depths: POINTER TO ARRAY [untagged] OF INTEGER; OUT count: INTEGER);
	PROCEDURE [ccall] gdk_query_visual_types(OUT visual_types: POINTER TO ARRAY [untagged] OF GdkVisualType; OUT count: INTEGER);
	PROCEDURE [ccall] gdk_list_visuals(): GdkVisual;
	PROCEDURE [ccall] gdk_visual_get_screen(visual: GdkVisual):GdkScreen ;

(* Fonts — Loading and manipulating fonts *)
	PROCEDURE [ccall] gdk_font_load*(IN font_name: String): GdkFont;
	PROCEDURE [ccall] gdk_fontset_load*(IN fontset_name: String): GdkFont;
	PROCEDURE [ccall] gdk_font_load_for_display(display: GdkDisplay; IN font_name: String): GdkFont;
	PROCEDURE [ccall] gdk_fontset_load_for_display(display: GdkDisplay; IN fontset_name: String): GdkFont;
	PROCEDURE [ccall] gdk_font_from_description*(font_desc: Pango.PangoFontDescription): GdkFont;
	PROCEDURE [ccall] gdk_font_from_description_for_display(display: GdkDisplay; font_desc: Pango.PangoFontDescription): GdkFont;
	PROCEDURE [ccall] gdk_font_ref*(font: GdkFont): GdkFont;
	PROCEDURE [ccall] gdk_font_unref*(font: GdkFont);
	
	PROCEDURE [ccall] gdk_font_id(font: GdkFont): INTEGER;
	PROCEDURE [ccall] gdk_font_equal(fonta, fontb: GdkFont):  BOOLEAN;
	PROCEDURE [ccall] gdk_font_get_display(font: GdkFont): GdkDisplay;

	PROCEDURE [ccall] gdk_char_width*(font: GdkFont; character: SHORTCHAR): INTEGER;
	PROCEDURE [ccall] gdk_char_width_wc*(font: GdkFont; character: gunichar): INTEGER;
	PROCEDURE [ccall] gdk_string_width*(font: GdkFont; IN string: String): INTEGER;
	PROCEDURE [ccall] gdk_text_width*(font: GdkFont; IN text: String; text_length: INTEGER): INTEGER;
	PROCEDURE [ccall] gdk_text_width_wc*(font: GdkFont; IN text: UString; text_length: INTEGER): INTEGER;
	PROCEDURE [ccall] gdk_string_measure(font: GdkFont; IN string: String): INTEGER;
	PROCEDURE [ccall] gdk_text_measure(font: GdkFont; IN text: String; text_length: INTEGER): INTEGER;
	PROCEDURE [ccall] gdk_char_measure(font: GdkFont; character: SHORTCHAR): INTEGER;
	PROCEDURE [ccall] gdk_string_height(font: GdkFont; IN string: String): INTEGER;
	PROCEDURE [ccall] gdk_text_height(font: GdkFont; IN text: String; text_length: INTEGER): INTEGER;
	PROCEDURE [ccall] gdk_char_height(font: GdkFont; character: SHORTCHAR): INTEGER;

	PROCEDURE [ccall] gdk_string_extents*(font: GdkFont; IN string: String; OUT lbearing,rbearing,width,ascent, descent: INTEGER);
	PROCEDURE [ccall] gdk_text_extents(font: GdkFont; IN text: String; text_length: INTEGER; OUT lbearing,rbearing,width,ascent, descent: INTEGER);
	PROCEDURE [ccall] gdk_text_extents_wc(font: GdkFont; IN text: UString; text_length: INTEGER; OUT lbearing,rbearing,width,ascent, descent: INTEGER);


(*  Cursors - Standard and pixmap cursors  *)
	PROCEDURE [ccall] gdk_cursor_new*(cursor_type: GdkCursorType): GdkCursor;
	PROCEDURE [ccall] gdk_cursor_new_for_display(display: GdkDisplay; cursor_type: GdkCursorType): GdkCursor;
	PROCEDURE [ccall] gdk_cursor_new_from_pixmap(source: GdkPixmap; mask: GdkPixmap; IN fg,bg: GdkColor; x, y: INTEGER): GdkCursor;
	PROCEDURE [ccall] gdk_cursor_new_from_pixbuf(display: GdkDisplay; pixbuf:GdkPixbuf ; x: INTEGER; y: INTEGER): GdkCursor;
	PROCEDURE [ccall] gdk_cursor_get_display(cursor: GdkCursor): GdkDisplay;
	PROCEDURE [ccall] gdk_cursor_ref(cursor: GdkCursor): GdkCursor;
	PROCEDURE [ccall] gdk_cursor_unref(cursor: GdkCursor);

(* Windows - Onscreen display areas in the target window system *)
	PROCEDURE [ccall] gdk_get_default_root_window*(): GdkWindow;
	PROCEDURE [ccall] gdk_window_new*(parent: GdkWindow; attributes: GdkWindowAttr; attributes_mask: INTEGER): GdkWindow;
	PROCEDURE [ccall] gdk_window_destroy*(window: GdkWindow);
	PROCEDURE [ccall] gdk_window_get_window_type(window: GdkWindow): GdkWindowType;
	PROCEDURE [ccall] gdk_window_at_pointer*(OUT win_x,win_y: INTEGER): GdkWindow;
	PROCEDURE [ccall] gdk_window_show*(window: GdkWindow);
	PROCEDURE [ccall] gdk_window_hide(window: GdkWindow);
	PROCEDURE [ccall] gdk_window_withdraw(window: GdkWindow);
	PROCEDURE [ccall] gdk_window_show_unraised(window: GdkWindow);
	PROCEDURE [ccall] gdk_window_move(window: GdkWindow; x: INTEGER; y: INTEGER);
	PROCEDURE [ccall] gdk_window_resize(window: GdkWindow; width: INTEGER; height: INTEGER);
	PROCEDURE [ccall] gdk_window_move_resize(window: GdkWindow; x, y: INTEGER; width: INTEGER; height: INTEGER);
	PROCEDURE [ccall] gdk_window_reparent(window: GdkWindow; new_parent: GdkWindow; x: INTEGER; y: INTEGER);
	PROCEDURE [ccall] gdk_window_clear(window: GdkWindow);
	PROCEDURE [ccall] gdk_window_clear_area*(window: GdkWindow; x, y: INTEGER; width,height: INTEGER);
	PROCEDURE [ccall] gdk_window_clear_area_e*(window: GdkWindow; x, y: INTEGER; width,height: INTEGER);
	PROCEDURE [ccall] gdk_window_raise*(window: GdkWindow);
	PROCEDURE [ccall] gdk_window_lower(window: GdkWindow);
	PROCEDURE [ccall] gdk_window_focus(window: GdkWindow; timestamp:  INTEGER
);
	PROCEDURE [ccall] gdk_window_set_user_data(window: GdkWindow; user_data: gpointer);
	PROCEDURE [ccall] gdk_window_set_override_redirect(window: GdkWindow; override_redirect: gboolean);
	PROCEDURE [ccall] gdk_window_set_accept_focus(window: GdkWindow; accept_focus: gboolean);
	PROCEDURE [ccall] gdk_window_add_filter*(window: GdkWindow; function: GdkFilterFunc; data: gpointer);
	PROCEDURE [ccall] gdk_window_remove_filter(window: GdkWindow; function: GdkFilterFunc; data: gpointer);
	PROCEDURE [ccall] gdk_window_scroll(window: GdkWindow; dx: INTEGER; dy: INTEGER);
	PROCEDURE [ccall] gdk_window_shape_combine_mask(window: GdkWindow; mask: GdkWindow; x: INTEGER; y: INTEGER);
	PROCEDURE [ccall] gdk_window_shape_combine_region(window: GdkWindow; shape_region:GdkRegion ; offset_x: INTEGER; offset_y: INTEGER);
	PROCEDURE [ccall] gdk_window_set_child_shapes(window: GdkWindow);
	PROCEDURE [ccall] gdk_window_merge_child_shapes(window: GdkWindow);
	PROCEDURE [ccall] gdk_window_is_visible(window: GdkWindow): BOOLEAN;
	PROCEDURE [ccall] gdk_window_is_viewable(window: GdkWindow): BOOLEAN;
	PROCEDURE [ccall] gdk_window_get_state(window: GdkWindow): GdkWindowState;
	PROCEDURE [ccall] gdk_window_set_static_gravities(window: GdkWindow; use_static: gboolean): BOOLEAN;
	(*  Functions to create/lookup windows from their native equivalents  *)
	PROCEDURE [ccall] gdk_window_foreign_new(anid: GdkNativeWindow): GdkWindow;
	PROCEDURE [ccall] gdk_window_lookup(anid: GdkNativeWindow): GdkWindow;
	PROCEDURE [ccall] gdk_window_foreign_new_for_display(display: GdkDisplay; anid: GdkNativeWindow): GdkWindow;
	PROCEDURE [ccall] gdk_window_lookup_for_display(display: GdkDisplay; anid: GdkNativeWindow): GdkWindow;

	PROCEDURE [ccall] gdk_window_set_hints(window: GdkWindow; x, y: INTEGER; min_width, min_height, max_width, max_height: INTEGER; flags: INTEGER);
	PROCEDURE [ccall] gdk_window_set_type_hint(window: GdkWindow; hint: GdkWindowTypeHint);
	PROCEDURE [ccall] gdk_window_set_modal_hint(window: GdkWindow; modal: gboolean);
	PROCEDURE [ccall] gdk_window_set_skip_taskbar_hint(window: GdkWindow; skips_taskbar: gboolean);
	PROCEDURE [ccall] gdk_window_set_skip_pager_hint(window: GdkWindow; skips_pager: gboolean);
	PROCEDURE [ccall] gdk_window_set_geometry_hints(window: GdkWindow; geometry: GdkGeometry; geom_mask: GdkWindowHints);
	PROCEDURE [ccall] gdk_set_sm_client_id(IN sm_client_id: String);
	PROCEDURE [ccall] gdk_window_begin_paint_rect(window: GdkWindow; rectangle: pGdkRectangle);
	PROCEDURE [ccall] gdk_window_begin_paint_region(window: GdkWindow; region:GdkRegion );
	PROCEDURE [ccall] gdk_window_end_paint(window: GdkWindow);
	PROCEDURE [ccall] gdk_window_set_title(window: GdkWindow; IN title: String);
	PROCEDURE [ccall] gdk_window_set_role(window: GdkWindow; IN role: String);
	PROCEDURE [ccall] gdk_window_set_transient_for(window: GdkWindow; parent: GdkWindow);
	PROCEDURE [ccall] gdk_window_set_background*(window: GdkWindow; IN color: GdkColor);
	PROCEDURE [ccall] gdk_window_set_back_pixmap(window: GdkWindow; pixmap: GdkWindow; parent_relative: gboolean);
	PROCEDURE [ccall] gdk_window_set_cursor*(window: GdkWindow; cursor: GdkCursor);
	PROCEDURE [ccall] gdk_window_get_user_data(window: GdkWindow; VAR data: gpointer);
	PROCEDURE [ccall] gdk_window_get_geometry(window: GdkWindow; OUT x,y,width,height,depth:INTEGER);
	PROCEDURE [ccall] gdk_window_get_position*(window: GdkWindow; OUT x,y: INTEGER);
	PROCEDURE [ccall] gdk_window_get_origin(window: GdkWindow; OUT x,y:INTEGER): INTEGER;
	PROCEDURE [ccall] gdk_window_get_root_origin(window: GdkWindow; OUT x,y: INTEGER);
	PROCEDURE [ccall] gdk_window_get_frame_extents(window: GdkWindow; rect: pGdkRectangle);
	PROCEDURE [ccall] gdk_window_get_pointer(window: GdkWindow; OUT x,y: INTEGER; OUT mask: GdkModifierType): GdkWindow;
	PROCEDURE [ccall] gdk_window_get_parent(window: GdkWindow): GdkWindow;
	PROCEDURE [ccall] gdk_window_get_toplevel(window: GdkWindow): GdkWindow;
	PROCEDURE [ccall] gdk_window_get_children(window: GdkWindow):GdkWindow ;
	PROCEDURE [ccall] gdk_window_peek_children(window: GdkWindow): GdkWindow;
	PROCEDURE [ccall] gdk_window_get_events*(window: GdkWindow): GdkEventMask;
	PROCEDURE [ccall] gdk_window_set_events*(window: GdkWindow; event_mask: GdkEventMask);
	PROCEDURE [ccall] gdk_window_set_icon_list(window: GdkWindow; pixbufs:GdkPixbuf );
	PROCEDURE [ccall] gdk_window_set_icon(window: GdkWindow; icon_window: GdkWindow; pixmap: GdkWindow; mask: GdkWindow);
	PROCEDURE [ccall] gdk_window_set_icon_name(window: GdkWindow; IN name: String);
	PROCEDURE [ccall] gdk_window_set_group(window: GdkWindow; leader: GdkWindow);
	PROCEDURE [ccall] gdk_window_get_group(window: GdkWindow): GdkWindow;
	PROCEDURE [ccall] gdk_window_set_decorations(window: GdkWindow; decorations: GdkWMDecoration);
	PROCEDURE [ccall] gdk_window_get_decorations(window: GdkWindow; OUT decorations: GdkWMDecoration): BOOLEAN;
	PROCEDURE [ccall] gdk_window_set_functions(window: GdkWindow; functions: GdkWMFunction);
	PROCEDURE [ccall] gdk_window_get_toplevels():GdkWindow ;
	PROCEDURE [ccall] gdk_window_iconify(window: GdkWindow);
	PROCEDURE [ccall] gdk_window_deiconify(window: GdkWindow);
	PROCEDURE [ccall] gdk_window_stick(window: GdkWindow);
	PROCEDURE [ccall] gdk_window_unstick(window: GdkWindow);
	PROCEDURE [ccall] gdk_window_maximize(window: GdkWindow);
	PROCEDURE [ccall] gdk_window_unmaximize(window: GdkWindow);
	PROCEDURE [ccall] gdk_window_fullscreen(window: GdkWindow);
	PROCEDURE [ccall] gdk_window_unfullscreen(window: GdkWindow);
	PROCEDURE [ccall] gdk_window_set_keep_above(window: GdkWindow; setting: gboolean);
	PROCEDURE [ccall] gdk_window_set_keep_below(window: GdkWindow; setting: gboolean);
	PROCEDURE [ccall] gdk_window_register_dnd(window: GdkWindow);
	PROCEDURE [ccall] gdk_window_begin_resize_drag(window: GdkWindow; edge: GdkWindowEdge; button: INTEGER; root_x: INTEGER; root_y: INTEGER; timestamp:  INTEGER
);
	PROCEDURE [ccall] gdk_window_begin_move_drag(window: GdkWindow; button: INTEGER; root_x: INTEGER; root_y: INTEGER; timestamp:  INTEGER
);
	(*  Interface for dirty-region queueing  *)
	PROCEDURE [ccall] gdk_window_invalidate_rect(window: GdkWindow; rect: pGdkRectangle; invalidate_children: gboolean);
	PROCEDURE [ccall] gdk_window_invalidate_region(window: GdkWindow; region:GdkRegion ; invalidate_children: gboolean);
	PROCEDURE [ccall] gdk_window_invalidate_maybe_recurse(window: GdkWindow; region:GdkRegion; child_func: ChildFunc; user_data: gpointer);
	PROCEDURE [ccall] gdk_window_get_update_area(window: GdkWindow):GdkRegion;
	PROCEDURE [ccall] gdk_window_freeze_updates(window: GdkWindow);
	PROCEDURE [ccall] gdk_window_thaw_updates(window: GdkWindow);
	PROCEDURE [ccall] gdk_window_process_all_updates();
	PROCEDURE [ccall] gdk_window_process_updates(window: GdkWindow; update_children: gboolean);
	(*  Enable/disable flicker, so you can tell if your code is inefficient.  *)
	PROCEDURE [ccall] gdk_window_set_debug_updates(setting: gboolean);
	PROCEDURE [ccall] gdk_window_constrain_size(geometry: GdkGeometry; flags: INTEGER; width: INTEGER; height: INTEGER; OUT new_width, new_height: INTEGER);
	PROCEDURE [ccall] gdk_window_get_internal_paint_info(window: GdkWindow; OUT real_drawable: GdkDrawable; OUT x_offset,y_offset: INTEGER);

(* Events - Functions for handling events from the window system *)
	PROCEDURE [ccall] gdk_events_pending*(): BOOLEAN;
	PROCEDURE [ccall] gdk_event_get*(): GdkEvent;
	PROCEDURE [ccall] gdk_event_peek*(): GdkEvent;
	PROCEDURE [ccall] gdk_event_put*(event: GdkEvent);
	PROCEDURE [ccall] gdk_event_free*(event: GdkEvent);

	PROCEDURE [ccall] gdk_event_new(type: GdkEventType): GdkEvent;
	PROCEDURE [ccall] gdk_event_copy(event: GdkEvent): GdkEvent;

	PROCEDURE [ccall] gdk_event_get_time(event: GdkEvent):  INTEGER;
	PROCEDURE [ccall] gdk_event_get_state(event: GdkEvent; state: GdkModifierType): BOOLEAN;
	PROCEDURE [ccall] gdk_event_get_graphics_expose(window:GdkWindow ): GdkEvent;
	PROCEDURE [ccall] gdk_event_get_coords(event: GdkEvent; VAR x_win, y_win: REAL): BOOLEAN;
	PROCEDURE [ccall] gdk_event_get_root_coords(event: GdkEvent; VAR x_win, y_win: REAL): BOOLEAN;
	PROCEDURE [ccall] gdk_event_get_axis(event: GdkEvent; axis_use: GdkAxisUse; VAR value: REAL): BOOLEAN;
	PROCEDURE [ccall] gdk_event_handler_set(func: GdkEventFunc; data: gpointer; notify: GLib.GDestroyNotify);
	PROCEDURE [ccall] gdk_event_set_screen(event: GdkEvent; screen: GdkScreen);
	PROCEDURE [ccall] gdk_event_get_screen(event: GdkEvent): GdkScreen;
	PROCEDURE [ccall] gdk_add_client_message_filter*(message_type: GdkAtom; func:  GdkFilterFunc; data:  gpointer);
	PROCEDURE [ccall] gdk_setting_get(IN name: String; value: GObject.GValue):  BOOLEAN;
	PROCEDURE [ccall] gdk_get_show_events(): gboolean;
	PROCEDURE [ccall] gdk_set_show_events(show_events: gboolean);

	(*Event Structures *)


	(* Key Values - Functions for manipulating keyboard codes *)
	PROCEDURE [ccall] gdk_keymap_get_default(): GdkKeymap;
	PROCEDURE [ccall] gdk_keymap_get_for_display(display: GdkDisplay): GdkKeymap;
	PROCEDURE [ccall] gdk_keymap_lookup_key(keymap: GdkKeymap; key: GdkKeymapKey): INTEGER;
	PROCEDURE [ccall] gdk_keymap_translate_keyboard_state(keymap: GdkKeymap; hardware_keycode: INTEGER; state: GdkModifierType; group: INTEGER; OUT keyval: INTEGER; OUT effective_group: INTEGER; OUT level: INTEGER; OUT consumed_modifiers: GdkModifierType): BOOLEAN;
	PROCEDURE [ccall] gdk_keymap_get_entries_for_keyval(keymap: GdkKeymap; keyval: INTEGER; OUT keys: GdkKeymapKey; OUT n_keys: INTEGER): BOOLEAN;
	PROCEDURE [ccall] gdk_keymap_get_entries_for_keycode(keymap: GdkKeymap; hardware_keycode: INTEGER; OUT keys: GdkKeymapKey; OUT keyvals: POINTER TO ARRAY [untagged] OF INTEGER; OUT n_entries: INTEGER): BOOLEAN;
	PROCEDURE [ccall] gdk_keymap_get_direction(keymap: GdkKeymap): Pango.PangoDirection;
	
	(*  Key values  *)
	PROCEDURE [ccall] gdk_keyval_name(keyval: INTEGER): PString;
	PROCEDURE [ccall] gdk_keyval_from_name(IN keyval_name: String): INTEGER;
	PROCEDURE [ccall] gdk_keyval_convert_case(symbol: INTEGER; OUT lower, upper: INTEGER);
	PROCEDURE [ccall] gdk_keyval_to_upper(keyval: INTEGER): INTEGER;
	PROCEDURE [ccall] gdk_keyval_to_lower(keyval: INTEGER): INTEGER;
	PROCEDURE [ccall] gdk_keyval_is_upper(keyval: INTEGER): BOOLEAN;
	PROCEDURE [ccall] gdk_keyval_is_lower(keyval: INTEGER): BOOLEAN;
	PROCEDURE [ccall] gdk_keyval_to_unicode*(keyval: INTEGER): gunichar;
	PROCEDURE [ccall] gdk_unicode_to_keyval*(wc: gunichar): INTEGER;


	(* Selections - Functions for transfering data via the X selection mechanism *)
	PROCEDURE [ccall] gdk_selection_owner_set(owner: GdkWindow; selection:GdkAtom ; time: INTEGER; send_event:gboolean ): BOOLEAN;
	PROCEDURE [ccall] gdk_selection_owner_get(selection:GdkAtom): GdkWindow;
	PROCEDURE [ccall] gdk_selection_owner_set_for_display(display: GdkDisplay; owner: GdkWindow; selection:GdkAtom ; time: INTEGER; send_event: gboolean): BOOLEAN;
	PROCEDURE [ccall] gdk_selection_owner_get_for_display(display: GdkDisplay; selection:GdkAtom ): GdkWindow;
	PROCEDURE [ccall] gdk_selection_convert(requestor: GdkWindow; selection:GdkAtom ; target:GdkAtom ; time: INTEGER);
	PROCEDURE [ccall] gdk_selection_property_get(requestor: GdkWindow; OUT data: PBYTES; OUT prop_type: GdkAtom; OUT prop_format: INTEGER): BOOLEAN;
	PROCEDURE [ccall] gdk_selection_send_notify(requestor:  INTEGER; selection:GdkAtom ; target:GdkAtom ; property: GdkAtom; time:  INTEGER);

	(* Drag and Drop - Functions for controlling drag and drop handling *)
	PROCEDURE [ccall] gdk_drag_context_new(): GdkDragContext;
	PROCEDURE [ccall] gdk_drag_context_ref(context: GdkDragContext);
	PROCEDURE [ccall] gdk_drag_context_unref(context: GdkDragContext);

	(*  Destination side  *)
	PROCEDURE [ccall] gdk_drag_status(context: GdkDragContext; action: GdkDragAction; time:  INTEGER);
	PROCEDURE [ccall] gdk_drop_reply(context: GdkDragContext; ok: gboolean; time:  INTEGER);
	PROCEDURE [ccall] gdk_drop_finish(context: GdkDragContext; success: gboolean; time: INTEGER);
	PROCEDURE [ccall] gdk_drag_get_selection(context: GdkDragContext):GdkAtom ;

	(*  Source side  *)
	PROCEDURE [ccall] gdk_drag_begin(window:GdkWindow; targets: GdkAtom): GdkDragContext;
	PROCEDURE [ccall] gdk_drag_get_protocol_for_display(display: GdkDisplay; xid: INTEGER; OUT protocol: GdkDragProtocol):  INTEGER;
	PROCEDURE [ccall] gdk_drag_find_window_for_screen(context: GdkDragContext; drag_window:GdkWindow ; screen: GdkScreen; x_root: INTEGER; y_root: INTEGER; OUT dest_window: GdkWindow; OUT protocol: GdkDragProtocol);
	PROCEDURE [ccall] gdk_drag_get_protocol(xid:  INTEGER; OUT protocol: GdkDragProtocol):  INTEGER;
	PROCEDURE [ccall] gdk_drag_find_window(context: GdkDragContext; drag_window:GdkWindow ; x_root,y_root: INTEGER; OUT dest_window: GdkWindow; OUT protocol:  GdkDragProtocol);
	PROCEDURE [ccall] gdk_drag_motion(context: GdkDragContext; dest_window:GdkWindow ; protocol: GdkDragProtocol; x_root, y_root: INTEGER; suggested_action, possible_actions: GdkDragAction; time: INTEGER): BOOLEAN;
	PROCEDURE [ccall] gdk_drag_drop(context: GdkDragContext; time: INTEGER);
	PROCEDURE [ccall] gdk_drag_abort(context: GdkDragContext; time: INTEGER);


	(*  Properties and Atoms - Functions to manipulate properties on windows  *)
	PROCEDURE [ccall] gdk_atom_intern*(IN atom_name: String; only_if_exists: gboolean): GdkAtom;
	PROCEDURE [ccall] gdk_atom_name(atom: GdkAtom): PString;

	PROCEDURE [ccall] gdk_property_get(window: GdkWindow; property: GdkAtom; type: GdkAtom; offset,length: INTEGER; pdelete: INTEGER; OUT actual_property_type: GdkAtom; OUT actual_format, actual_length: INTEGER; OUT data: PString): BOOLEAN;
	
	PROCEDURE [ccall] gdk_property_change(window: GdkWindow; property: GdkAtom; type: GdkAtom; format: INTEGER; mode: GdkPropMode; data: PBYTES; nelements: INTEGER);
	PROCEDURE [ccall] gdk_property_delete(window: GdkWindow; property: GdkAtom);
	
	PROCEDURE [ccall] gdk_free_text_list(IN list: APString);
	PROCEDURE [ccall] gdk_free_compound_text(IN ctext: String);
	
	PROCEDURE [ccall] gdk_text_property_to_text_list(encoding: GdkAtom; format: INTEGER; IN text: String; length: INTEGER; OUT list: APString): INTEGER;
	PROCEDURE [ccall] gdk_text_property_to_text_list_for_display(display: GdkDisplay; encoding: GdkAtom; format: INTEGER; IN text: String; length: INTEGER; OUT list: APString): INTEGER;
	
	PROCEDURE [ccall] gdk_text_property_to_utf8_list(encoding: GdkAtom; format: INTEGER; IN text: String; length: INTEGER; OUT list: APString): INTEGER;
	PROCEDURE [ccall] gdk_text_property_to_utf8_list_for_display(display: GdkDisplay; encoding: GdkAtom; format: INTEGER; IN text: String; length: INTEGER; OUT list: APString): INTEGER;

	PROCEDURE [ccall] gdk_utf8_to_string_target(IN str: String): PString;
	PROCEDURE [ccall] gdk_utf8_to_compound_text(IN str: String; OUT encoding: GdkAtom; OUT format: INTEGER; OUT ctext: PString; OUT length: INTEGER): BOOLEAN;
	PROCEDURE [ccall] gdk_utf8_to_compound_text_for_display(display: GdkDisplay; IN str: String; OUT encoding: GdkAtom; OUT format: INTEGER; OUT ctext: PString; OUT length: INTEGER): BOOLEAN;

	PROCEDURE [ccall] gdk_string_to_compound_text(str: PString; OUTencoding: GdkAtom; OUT format: INTEGER; OUT ctext: PString; OUT length: INTEGER): INTEGER;
	PROCEDURE [ccall] gdk_string_to_compound_text_for_display(display: GdkDisplay; str: PString; OUT encoding: GdkAtom; OUT format: INTEGER; OUT ctext: PString; OUT length: INTEGER): INTEGER;


	(*  Threads - Functions for using GDK in multi-threaded programs  *)
	(*  GCallback gdk_threads_lock; *)
	(*  GCallback gdk_threads_unlock; *)
	PROCEDURE [ccall] gdk_threads_enter();
	PROCEDURE [ccall] gdk_threads_leave();
	PROCEDURE [ccall] gdk_threads_init();

	(*  Input - Callbacks on file descriptors  *)
	PROCEDURE [ccall] gdk_input_add(source: INTEGER; condition: GdkInputCondition; function: GdkInputFunction; data: gpointer): INTEGER;
	PROCEDURE [ccall] gdk_input_add_full(source: INTEGER; condition: GdkInputCondition; function: GdkInputFunction; data: gpointer; destroy: GdkDestroyNotify): INTEGER;
	PROCEDURE [ccall] gdk_input_remove(tag: INTEGER);


	(* Pango Interaction - Using Pango in GDK*)
	
	PROCEDURE [ccall] gdk_pango_context_get*(): Pango.PangoContext;
	PROCEDURE [ccall] gdk_pango_context_get_for_screen(screen: GdkScreen): Pango.PangoContext;
	PROCEDURE [ccall] gdk_pango_context_set_colormap(context: Pango.PangoContext; colormap: GdkColormap);
	PROCEDURE [ccall] gdk_pango_layout_line_get_clip_region(line: Pango.PangoLayoutLine; x_origin,y_origin: INTEGER; IN index_ranges: ARRAY  [untagged] OF INTEGER; n_ranges: INTEGER): GdkRegion;
	PROCEDURE [ccall] gdk_pango_layout_get_clip_region(layout:Pango.PangoLayout ; x_origin,y_origin: INTEGER; IN index_ranges: ARRAY  [untagged] OF INTEGER; n_ranges: INTEGER):GdkRegion;

	PROCEDURE [ccall] gdk_pango_attr_stipple_new(stipple: GdkBitmap): Pango.PangoAttribute;
	PROCEDURE [ccall] gdk_pango_attr_embossed_new(embossed: gboolean): Pango.PangoAttribute;

	(* Input Devices - Functions for handling extended input devices *)
	PROCEDURE [ccall] gdk_devices_list(): GLib.GList; 
	PROCEDURE [ccall] gdk_device_set_source(device: GdkDevice; source: GdkInputSource);
	PROCEDURE [ccall] gdk_device_set_mode(device: GdkDevice; mode: GdkInputMode): BOOLEAN;
	PROCEDURE [ccall] gdk_device_set_key(device: GdkDevice; index: INTEGER; keyval: INTEGER; modifiers: GdkModifierType);
	PROCEDURE [ccall] gdk_device_set_axis_use(device: GdkDevice; index: INTEGER; use: GdkAxisUse);
	PROCEDURE [ccall] gdk_device_get_state(device: GdkDevice; window: GdkWindow; OUT axes: ARRAY [untagged] OF REAL; mask: GdkModifierType);
	PROCEDURE [ccall] gdk_device_get_axis(device: GdkDevice; IN axes: ARRAY [untagged] OF REAL; use: GdkAxisUse; OUT value: REAL): BOOLEAN;
	PROCEDURE [ccall] gdk_input_set_extension_events(window: GdkWindow; mask: INTEGER; mode: GdkExtensionMode);
	PROCEDURE [ccall] gdk_device_get_core_pointer(): GdkDevice;

END LibsGdk.
