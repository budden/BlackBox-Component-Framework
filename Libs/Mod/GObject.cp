MODULE LibsGObject ["libgobject-2.0-0.dll"];
(*  GObject - GLib Type, Object, Parameter and Signal Library   P_*)
IMPORT SYSTEM, GLib:=LibsGlib;

(* gtype.h *)

CONST 
  G_TYPE_FUNDAMENTAL_SHIFT = 2;
  G_TYPE_RESERVED_GLIB_FIRST = 21;
  G_TYPE_RESERVED_GLIB_LAST = 31;
  G_TYPE_RESERVED_BSE_FIRST = 32;
  G_TYPE_RESERVED_BSE_LAST = 48;
  G_TYPE_RESERVED_USER_FIRST = 49;


TYPE 
		PString = GLib.PString;
		
TYPE 
		GType* = INTEGER;

		GTypeClass* = POINTER TO ABSTRACT RECORD [untagged]
			g_type*: GType;
		END;

		GTypeInstance* = POINTER TO ABSTRACT RECORD [untagged]
			g_class* :GTypeClass
		END;
  
  	GTypeInterface = POINTER TO ABSTRACT RECORD [untagged]
     g_type         : GType;   (*  iface type  *)
     g_instance_type: GType;
   END;
	
		GTypeQuery = POINTER TO RECORD [untagged]
			type         : GType;
			type_name    : PString;
			class_size   : INTEGER;
			instance_size: INTEGER;
		END;
	
		GBaseInitFunc = PROCEDURE (g_class:GLib.gpointer);
		GBaseFinalizeFunc = GBaseInitFunc;
		GClassInitFunc = PROCEDURE ( g_class,class_data :GLib.gpointer);
		GClassFinalizeFunc = GClassInitFunc;
		GInstanceInitFunc = PROCEDURE ( instance:GTypeInstance;g_class : GLib.gpointer );
		GInterfaceInitFunc = GClassInitFunc;
		GInterfaceFinalizeFunc = GClassInitFunc;

	TYPE 
   GTypeFundamentalFlags = INTEGER; (* enum  GTypeFundamentalFlags *)
	CONST 
   G_TYPE_FLAG_CLASSED = 1;
   G_TYPE_FLAG_INSTANTIATABLE = 2;
   G_TYPE_FLAG_DERIVABLE = 4;
   G_TYPE_FLAG_DEEP_DERIVABLE = 8;
	(* / GTypeFundamentalFlags *)

	TYPE 
		GTypeInfo = RECORD [untagged]
			class_size    : GLib.guint16;
			base_init     : GBaseInitFunc;
			base_finalize : GBaseFinalizeFunc;
			class_init    : GClassInitFunc;
			class_finalize: GClassFinalizeFunc;
			class_data    : GLib.gpointer;
			instance_size : GLib.guint16;
			n_preallocs   : GLib.guint16;
			instance_init : GInstanceInitFunc;
			value_table   : POINTER TO GTypeValueTable;
		END;

		GTypeFundamentalInfo = RECORD [untagged]
			type_flags: GTypeFundamentalFlags;
		END;

		GInterfaceInfo = RECORD [untagged]
			interface_init    : GInterfaceInitFunc;
			interface_finalize: GInterfaceFinalizeFunc;
			interface_data    : GLib.gpointer;
		END;

		GTypePlugin = POINTER TO RECORD  [untagged]
		END;

(* gvalue.h *)	

		GValue* = POINTER [untagged] TO GValueDesc;
		
		GValueDesc = RECORD [untagged]
			g_type: GType;
			data  : ARRAY 2 OF RECORD [union]
				v_int    : GLib.gint;
				v_int64  : GLib.gint64;
				v_float  : GLib.gfloat;
				v_double : GLib.gdouble;
				v_pointer: GLib.gpointer;
			END;
		END;

		GValueArray = POINTER TO RECORD  [untagged]
			n_values    : GLib.guint;
			values      : GValue;
			n_prealloced: GLib.guint;
		END;


		_GTypeCValue = RECORD  [union]
				v_int    : GLib.gint;
				v_int64  : GLib.gint64;
				v_float  : GLib.gfloat;
				v_double : GLib.gdouble;
				v_pointer: GLib.gpointer;
		END;
	  GTypeCValue = POINTER TO _GTypeCValue;



  GTypeValueTable = RECORD  [untagged]
    value_init        : PROCEDURE (value : GValue );
    value_free        : PROCEDURE (value : GValue );
    value_copy        : PROCEDURE (src_value, dest_value : GValue);
    value_peek_pointer: PROCEDURE (value : GValue): GLib.gpointer;
    collect_format    : PString;
    collect_value     : PROCEDURE (value : GValue; n_collect_values: INTEGER; collect_values: GTypeCValue;collect_flags: INTEGER): PString;;
    lcopy_format      : PString;
    lcopy_value       : PROCEDURE (value : GValue; n_collect_values: INTEGER; collect_values: GTypeCValue;collect_flags: INTEGER): PString;;
  END;
	
  

(* gparam.h *)
  GParameter = POINTER TO GParameterDesc;
  GParameterDesc = RECORD [untagged]
    name : PString;
    value: GValueDesc;
  END;
  


TYPE   GParamFlags = INTEGER; (* enum  GParamFlags *)
CONST
  G_PARAM_READABLE = 1;
  G_PARAM_WRITABLE = 2;
  G_PARAM_CONSTRUCT = 4;
  G_PARAM_CONSTRUCT_ONLY = 8;
  G_PARAM_LAX_VALIDATION = 16;
  G_PARAM_STATIC_NAME = 32;
  G_PARAM_STATIC_NICK = 64;
  G_PARAM_STATIC_BLURB = 128;
(* / GParamFlags *)

TYPE   GTypeDebugFlags = INTEGER;(* enum  GTypeDebugFlags *)
CONST 
  G_TYPE_DEBUG_NONE = 0;
  G_TYPE_DEBUG_OBJECTS = 1;
  G_TYPE_DEBUG_SIGNALS = 2;
  G_TYPE_DEBUG_MASK = 3H;
(* / GTypeDebugFlags *)

(* gsignal.h *)
TYPE
  GSignalFlags = INTEGER;

(* gclosure.h *)
TYPE
  GClosureNotify = PROCEDURE ( data: GLib.gpointer; closure:GClosure );
  GClosureNotifyData = POINTER TO GClosureNotifyDataDesc;
  GClosureNotifyDataDesc = RECORD [untagged]
    data  : GLib.gpointer;
    notify: GClosureNotify;
  END;

  GClosure = POINTER TO RECORD [untagged]
    ref_count      : SET ;    (* bit fields: ref_count:15, meta_marshal:1, n_guards:1, n_fnotifiers:2, n_inotifiers:8, in_inotify:1, floating:1, derivative_flag:1, in_marshal:1, is_invalid:1. *)
    marshal        : PROCEDURE  (value : GValue );
    data           : GLib.gpointer;
    notifiers      : GClosureNotifyData;
  END;

  GCallback = PROCEDURE  (  );

TYPE
  GParamSpec = POINTER TO ABSTRACT RECORD (GTypeInstance)
    name           : PString;
    flags          : GParamFlags;
    value_type     : GType;
    owner_type     : GType;          (*  class or interface using this property  *)
    _nick          : PString;
    _blurb         : PString;
    qdata          : GLib.GData;
    ref_count      : INTEGER;
    param_id       : INTEGER;         (*  sort-criteria  *)
  END;

(* gobject.h *)
TYPE
	GObject* = POINTER TO  ABSTRACT RECORD (GTypeInstance)
		ref_count      : INTEGER;
		qdata          : GLib.GData;
	END;
  GObjectConstructParam =   POINTER TO ABSTRACT RECORD (GParamSpec)
    value: GValue;
  END;

  GObjectConstructor = PROCEDURE (type:GType; n_construct_properties:GLib.guint; construct_properties:GObjectConstructParam ): GObject;
  GObjectGetPropertyFunc = PROCEDURE (object :GObject; property_id: GLib.guint; value :GValue;pspec:GParamSpec );
	GObjectSetPropertyFunc = GObjectGetPropertyFunc;
	GObjectFinalizeFunc = PROCEDURE ( object :GObject );
  GObjectDispatchFunc = PROCEDURE (object :GObject;  n_pspecs:GLib.guint; VAR pspecs: (*aPtr*) GParamSpec );
	GObjectNotifyFunc = PROCEDURE (object :GObject;  pspec :GParamSpec );
  GWeakNotify = PROCEDURE ( data: GLib.gpointer; where_the_object_was: GObject);
	
  GObjectClass = POINTER TO ABSTRACT RECORD (GTypeClass)
    construct_properties       : GLib.GData;
    constructor                : GObjectConstructor;
    set_property               : GObjectGetPropertyFunc;
    get_property               : GObjectGetPropertyFunc;
    dispose                    : GObjectFinalizeFunc;
    finalize                   : GObjectFinalizeFunc;
    dispatch_properties_changed: GObjectDispatchFunc;
    notify                     : GObjectNotifyFunc;
    pdummy                     : ARRAY 8 OF GLib.gpointer;
  END;



TYPE  GTypeFlags = INTEGER; (* enum  GTypeFlags *)
CONST 
  G_TYPE_FLAG_ABSTRACT = 16;
  G_TYPE_FLAG_VALUE_ABSTRACT = 32;
(* / GTypeFlags *)

  

(*  --- prototypes ---  *)


PROCEDURE [ccall] g_type_init (  );
PROCEDURE [ccall] g_type_init_with_debug_flags ( debug_flags: GTypeDebugFlags );
PROCEDURE [ccall] g_type_name ( type: GType ): PString;
PROCEDURE [ccall] g_type_qname ( type: GType ): GLib.GQuark;
PROCEDURE [ccall] g_type_from_name ( name: PString ): GType;
PROCEDURE [ccall] g_type_parent ( type: GType ): GType;
PROCEDURE [ccall] g_type_depth ( type: GType ): INTEGER;
PROCEDURE [ccall] g_type_next_base ( leaf_type: GType; root_type: GType ): GType;
PROCEDURE [ccall] g_type_is_a ( type: GType; is_a_type: GType ): GLib.gboolean;
PROCEDURE [ccall] g_type_class_ref ( type: GType ): GLib.gpointer;
PROCEDURE [ccall] g_type_class_peek ( type: GType ): GLib.gpointer;
PROCEDURE [ccall] g_type_class_peek_static ( type: GType ): GLib.gpointer;
PROCEDURE [ccall] g_type_class_unref ( g_class: GLib.gpointer );
PROCEDURE [ccall] g_type_class_peek_parent ( g_class: GLib.gpointer ): GLib.gpointer;
PROCEDURE [ccall] g_type_interface_peek ( instance_class: GLib.gpointer; iface_type: GType ): GLib.gpointer;
PROCEDURE [ccall] g_type_interface_peek_parent ( g_iface: GLib.gpointer ): GLib.gpointer;
PROCEDURE [ccall] g_type_default_interface_ref ( g_type: GType ): GLib.gpointer;
PROCEDURE [ccall] g_type_default_interface_peek ( g_type: GType ): GLib.gpointer;
PROCEDURE [ccall] g_type_default_interface_unref ( g_iface: GLib.gpointer );
PROCEDURE [ccall] g_type_children ( type: GType; OUT n_children: INTEGER ): GType;
PROCEDURE [ccall] g_type_interfaces ( type: GType; OUT n_interfaces: INTEGER ): GType;
PROCEDURE [ccall] g_type_set_qdata ( type: GType; quark: GLib.GQuark; data: GLib.gpointer );
PROCEDURE [ccall] g_type_get_qdata ( type: GType; quark: GLib.GQuark ): GLib.gpointer;
PROCEDURE [ccall] g_type_query ( type: GType; query: GTypeQuery);

PROCEDURE [ccall] g_type_register_static (parent_type: GType; type_name: PString; IN info: GTypeInfo; flags: GTypeFlags ): GType;
PROCEDURE [ccall] g_type_register_dynamic (parent_type: GType; type_name: PString;plugin: GTypePlugin; flags: GTypeFlags ): GType;
PROCEDURE [ccall] g_type_register_fundamental(type_id: GType; type_name: PString; IN info: GTypeInfo; IN finfo: GTypeFundamentalInfo; flags: GTypeFlags ): GType;
PROCEDURE [ccall] g_type_add_interface_static ( instance_type: GType; interface_type: GType; IN info: GInterfaceInfo);
PROCEDURE [ccall] g_type_add_interface_dynamic ( instance_type: GType; interface_type: GType; plugin: GTypePlugin );
PROCEDURE [ccall] g_type_interface_add_prerequisite ( interface_type: GType; prerequisite_type: GType );
PROCEDURE [ccall] g_type_interface_prerequisites ( interface_type: GType; n_prerequisites: INTEGER ): GType;
PROCEDURE [ccall] g_type_class_add_private ( g_class: GLib.gpointer; private_size: INTEGER );
PROCEDURE [ccall] g_type_instance_get_private ( instance: GTypeInstance; private_type: GType ): GLib.gpointer;

(* gvalue.h *)
	PROCEDURE [ccall] g_value_init ( value: GValue; g_type: GType ): GValue;
	PROCEDURE [ccall] g_value_copy ( src_value: GValue; dest_value: GValue );
	PROCEDURE [ccall] g_value_reset ( value: GValue ): GValue;
	PROCEDURE [ccall] g_value_unset ( value: GValue );
	PROCEDURE [ccall] g_value_set_instance ( value: GValue; instance: GLib.gpointer );

(* gvaluetypes.h *)
	PROCEDURE [ccall] g_value_set_char ( value: GValue; v_char: GLib.gchar );
	PROCEDURE [ccall] g_value_get_char ( value: GValue ): GLib.gchar;
	PROCEDURE [ccall] g_value_set_uchar ( value: GValue; v_uchar: GLib.guchar );
	PROCEDURE [ccall] g_value_get_uchar ( value: GValue ): GLib.guchar;
	PROCEDURE [ccall] g_value_set_boolean ( value: GValue; v_boolean: GLib.gboolean );
	PROCEDURE [ccall] g_value_get_boolean ( value: GValue ): GLib.gboolean;
	PROCEDURE [ccall] g_value_set_int ( value: GValue; v_int: GLib.gint );
	PROCEDURE [ccall] g_value_get_int ( value: GValue ): GLib.gint;
	PROCEDURE [ccall] g_value_set_uint ( value: GValue; v_uint: GLib.guint );
	PROCEDURE [ccall] g_value_get_uint ( value: GValue ): GLib.guint;
	PROCEDURE [ccall] g_value_set_long ( value: GValue; v_long: GLib.glong );
	PROCEDURE [ccall] g_value_get_long ( value: GValue ): GLib.glong;
	PROCEDURE [ccall] g_value_set_ulong ( value: GValue; v_ulong: GLib.gulong );
	PROCEDURE [ccall] g_value_get_ulong ( value: GValue ): GLib.gulong;
	PROCEDURE [ccall] g_value_set_int64 ( value: GValue; v_int64: GLib.gint64 );
	PROCEDURE [ccall] g_value_get_int64 ( value: GValue ): GLib.gint64;
	PROCEDURE [ccall] g_value_set_uint64 ( value: GValue; v_uint64: GLib.guint64 );
	PROCEDURE [ccall] g_value_get_uint64 ( value: GValue ): GLib.guint64;
	PROCEDURE [ccall] g_value_set_float ( value: GValue; v_float: GLib.gfloat );
	PROCEDURE [ccall] g_value_get_float ( value: GValue ): GLib.gfloat;
	PROCEDURE [ccall] g_value_set_double ( value: GValue; v_double: GLib.gdouble );
	PROCEDURE [ccall] g_value_get_double ( value: GValue ): GLib.gdouble;
	PROCEDURE [ccall] g_value_set_string ( value: GValue; v_string: PString );
	PROCEDURE [ccall] g_value_set_static_string ( value: GValue; v_string: PString );
	PROCEDURE [ccall] g_value_get_string ( value: GValue ): PString;
	PROCEDURE [ccall] g_value_dup_string ( value: GValue ): PString;
	PROCEDURE [ccall] g_value_set_pointer ( value: GValue; v_pointer: GLib.gpointer );
	PROCEDURE [ccall] g_value_get_pointer ( value: GValue ): GLib.gpointer;
	PROCEDURE [ccall] g_value_take_string ( value: GValue; v_string: PString );
	PROCEDURE [ccall] g_value_set_string_take_ownership ( value: GValue; v_string: PString );
	PROCEDURE [ccall] g_strdup_value_contents ( value: GValue ): PString;

	PROCEDURE [ccall] g_pointer_type_register_static ( name: PString ): GType;
	
(* gboxed.h *)
	PROCEDURE [ccall] g_boxed_copy ( boxed_type: GType;src_boxed: GLib.gpointer ): GLib.gpointer;
	PROCEDURE [ccall] g_boxed_free ( boxed_type: GType; boxed: GLib.gpointer );
	PROCEDURE [ccall] g_value_set_boxed ( value: GValue; v_boxed: GLib.gconstpointer );
	PROCEDURE [ccall] g_value_set_static_boxed ( value: GValue; v_boxed: GLib.gconstpointer );
	PROCEDURE [ccall] g_value_get_boxed ( value: GValue ): GLib.gpointer;
	PROCEDURE [ccall] g_value_dup_boxed ( value: GValue ): GLib.gpointer;
	PROCEDURE [ccall] g_value_take_boxed ( value: GValue; v_boxed: GLib.gconstpointer );
	PROCEDURE [ccall] g_value_set_boxed_take_ownership ( value: GValue; v_boxed: GLib.gconstpointer );
	PROCEDURE [ccall] g_closure_get_type (  ): GType;
	PROCEDURE [ccall] g_value_get_type (  ): GType;
	PROCEDURE [ccall] g_value_array_get_type (  ): GType;
	PROCEDURE [ccall] g_date_get_type (  ): GType;
	PROCEDURE [ccall] g_strv_get_type (  ): GType;
	PROCEDURE [ccall] g_gstring_get_type (  ): GType;
(* gvaluearray.h *)

	PROCEDURE [ccall] g_value_array_get_nth ( value_array: GValueArray; index_: GLib.guint ): GValue;
	PROCEDURE [ccall] g_value_array_new ( n_prealloced: GLib.guint ): GValueArray;
	PROCEDURE [ccall] g_value_array_free ( value_array: GValueArray );
	PROCEDURE [ccall] g_value_array_copy ( value_array: GValueArray ): GValueArray;
	PROCEDURE [ccall] g_value_array_prepend ( value_array: GValueArray;value: GValue ): GValueArray;
	PROCEDURE [ccall] g_value_array_append ( value_array: GValueArray;value: GValue ): GValueArray;
	PROCEDURE [ccall] g_value_array_insert ( value_array: GValueArray;index: GLib.guint;value: GValue ): GValueArray;
	PROCEDURE [ccall] g_value_array_remove ( value_array: GValueArray;index_: GLib.guint ): GValueArray;
	PROCEDURE [ccall] g_value_array_sort ( value_array: GValueArray; compare_func: GLib.GCompareFunc ): GValueArray;
	PROCEDURE [ccall] g_value_array_sort_with_data ( value_array: GValueArray;compare_func: GLib.GCompareDataFunc;user_data: GLib.gpointer ): GValueArray;



(* gparam.h *)
	PROCEDURE [ccall] g_param_spec_ref ( pspec: GParamSpec ): GParamSpec;
	PROCEDURE [ccall] g_param_spec_unref ( pspec: GParamSpec );
	PROCEDURE [ccall] g_param_spec_sink ( pspec: GParamSpec );
	PROCEDURE [ccall] g_param_spec_get_qdata ( pspec: GParamSpec;quark: GLib.GQuark ): GLib.gpointer;
	PROCEDURE [ccall] g_param_spec_set_qdata ( pspec: GParamSpec; quark: GLib.GQuark;data: GLib.gpointer );
	PROCEDURE [ccall] g_param_spec_set_qdata_full ( pspec: GParamSpec;quark: GLib.GQuark; data: GLib.gpointer;destroy: GLib.GDestroyNotify );
	PROCEDURE [ccall] g_param_spec_steal_qdata ( pspec: GParamSpec;quark: GLib.GQuark ): GLib.gpointer;
	PROCEDURE [ccall] g_param_spec_get_redirect_target ( pspec: GParamSpec ): GParamSpec;
	PROCEDURE [ccall] g_param_value_set_default ( pspec: GParamSpec;value: GValue );
	PROCEDURE [ccall] g_param_value_defaults ( pspec: GParamSpec;value: GValue ): GLib.gboolean;
	PROCEDURE [ccall] g_param_value_validate ( pspec: GParamSpec;value: GValue ): GLib.gboolean;
	PROCEDURE [ccall] g_param_value_convert ( pspec: GParamSpec; src_value: GValue;dest_value: GValue;strict_validation: GLib.gboolean ): GLib.gboolean;
	PROCEDURE [ccall] g_param_values_cmp ( pspec: GParamSpec; value1: GValue; value2: GValue ): GLib.gint;
	PROCEDURE [ccall] g_param_spec_get_name ( pspec: GParamSpec ): PString;
	PROCEDURE [ccall] g_param_spec_get_nick ( pspec: GParamSpec ): PString;
	PROCEDURE [ccall] g_param_spec_get_blurb ( pspec: GParamSpec ): PString;
	PROCEDURE [ccall] g_value_set_param ( value: GValue; param: GParamSpec );
	PROCEDURE [ccall] g_value_get_param ( value: GValue ): GParamSpec;
	PROCEDURE [ccall] g_value_dup_param ( value: GValue ): GParamSpec;
	PROCEDURE [ccall] g_value_take_param ( value: GValue; param: GParamSpec );
	PROCEDURE [ccall] g_value_set_param_take_ownership ( value: GValue;param: GParamSpec );


(* gclosure.h *)
	PROCEDURE [ccall] g_cclosure_new ( callback_func: GCallback; user_data: GLib.gpointer; destroy_data: GClosureNotify ): GClosure;
	PROCEDURE [ccall] g_cclosure_new_swap ( callback_func: GCallback; user_data: GLib.gpointer; destroy_data: GClosureNotify ): GClosure;
	PROCEDURE [ccall] g_signal_type_cclosure_new ( itype: GType; struct_offset: INTEGER ): GClosure;
	PROCEDURE [ccall] g_closure_ref ( closure: GClosure ): GClosure;
	PROCEDURE [ccall] g_closure_sink ( closure: GClosure );
	PROCEDURE [ccall] g_closure_unref ( closure: GClosure );
	PROCEDURE [ccall] g_closure_new_simple ( sizeof_closure: INTEGER; data: GLib.gpointer ): GClosure;
	PROCEDURE [ccall] g_closure_add_finalize_notifier ( closure: GClosure;notify_data: GLib.gpointer;notify_func: GClosureNotify );
	PROCEDURE [ccall] g_closure_remove_finalize_notifier ( closure: GClosure;notify_data: GLib.gpointer;notify_func: GClosureNotify );
	PROCEDURE [ccall] g_closure_add_invalidate_notifier ( closure: GClosure;notify_data: GLib.gpointer;notify_func: GClosureNotify );
	PROCEDURE [ccall] g_closure_remove_invalidate_notifier ( closure: GClosure;notify_data: GLib.gpointer;notify_func: GClosureNotify );
	PROCEDURE [ccall] g_closure_add_marshal_guards ( closure: GClosure; pre_marshal_data: GLib.gpointer; pre_marshal_notify: GClosureNotify; post_marshal_data: GLib.gpointer;post_marshal_notify: GClosureNotify );
	PROCEDURE [ccall] g_closure_invalidate ( closure: GClosure );
	PROCEDURE [ccall] g_closure_invoke ( closure: GClosure;(* out? *) return_value: GValue; n_param_values: INTEGER; param_values: GValue;invocation_hint: GLib.gpointer );


(* gsignal.h *)

(* gobject.h *)
PROCEDURE [ccall] g_object_newv ( object_type: GType; n_parameters: INTEGER; parameters: GParameter ): GLib.gpointer;
PROCEDURE [ccall] g_object_set ( object: GLib.gpointer; first_property_name: PString; arg0: BYTE );
PROCEDURE [ccall] g_object_get ( object: GLib.gpointer; first_property_name: PString; arg0: BYTE );
PROCEDURE [ccall] g_object_connect ( object: GLib.gpointer; signal_spec: PString; arg0: BYTE ): GLib.gpointer;
PROCEDURE [ccall] g_object_disconnect ( object: GLib.gpointer; signal_spec: PString; arg0: BYTE );
PROCEDURE [ccall] g_object_set_property ( object: GObject; property_name: PString; value: GValue );
PROCEDURE [ccall] g_object_get_property ( object: GObject; property_name: PString; value: GValue );
PROCEDURE [ccall] g_object_freeze_notify ( object: GObject );
PROCEDURE [ccall] g_object_notify ( object: GObject; property_name: PString );
PROCEDURE [ccall] g_object_thaw_notify ( object: GObject );
PROCEDURE [ccall] g_object_ref* ( object: GObject ): GLib.gpointer;
PROCEDURE [ccall] g_object_unref* ( object: GObject );
PROCEDURE [ccall] g_object_weak_ref ( object: GObject; notify: GWeakNotify; data: GLib.gpointer );
PROCEDURE [ccall] g_object_weak_unref ( object: GObject; notify: GWeakNotify; data: GLib.gpointer );

(* gparamspecs.h *)
	PROCEDURE [ccall] g_param_spec_char ( name: PString; nick: PString; blurb: PString; minimum: GLib.gint8;maximum: GLib.gint8; default_value: GLib.gint8;flags: GParamFlags ): GParamSpec;

	PROCEDURE [ccall] g_param_spec_uchar ( name: PString; nick: PString; blurb: PString; minimum,maximum,default_value: GLib.guint8;flags: GParamFlags ): GParamSpec;
	PROCEDURE [ccall] g_param_spec_int ( name: PString; nick: PString;blurb: PString; minimum,maximum: GLib.gint; default_value: GLib.gint;
	flags: GParamFlags ): GParamSpec;
	PROCEDURE [ccall] g_param_spec_uint ( name: PString; nick: PString;	blurb: PString; minimum,maximum: GLib.guint; default_value: GLib.guint;	flags: GParamFlags ): GParamSpec;
	PROCEDURE [ccall] g_param_spec_long ( name: PString; nick: PString;blurb: PString; minimum: GLib.glong;maximum: GLib.glong; default_value: GLib.glong;flags: GParamFlags ): GParamSpec;
	PROCEDURE [ccall] g_param_spec_ulong ( name: PString; nick: PString;blurb: PString; minimum: GLib.gulong;maximum: GLib.gulong; default_value: GLib.gulong;flags: GParamFlags ): GParamSpec;
	PROCEDURE [ccall] g_param_spec_int64 ( name: PString; nick: PString;blurb: PString; minimum: GLib.gint64;maximum: GLib.gint64; default_value: GLib.gint64;flags: GParamFlags ): GParamSpec;
	PROCEDURE [ccall] g_param_spec_uint64 ( name: PString; nick: PString;blurb: PString; minimum: GLib.guint64;maximum: GLib.guint64;
	default_value: GLib.guint64;flags: GParamFlags ): GParamSpec;
	PROCEDURE [ccall] g_param_spec_boolean ( name: PString; nick: PString;blurb: PString; default_value: GLib.gboolean;flags: GParamFlags ): GParamSpec;
	PROCEDURE [ccall] g_param_spec_unichar ( name: PString; nick: PString;blurb: PString; default_value: GLib.gunichar;flags: GParamFlags ): GParamSpec;
	PROCEDURE [ccall] g_param_spec_enum ( name: PString; nick: PString;blurb: PString; enum_type: GType;default_value: GLib.gint;flags: GParamFlags ): GParamSpec;
	PROCEDURE [ccall] g_param_spec_flags ( name: PString; nick: PString;blurb: PString; flags_type: GType;default_value: GLib.guint;flags: GParamFlags ): GParamSpec;
	PROCEDURE [ccall] g_param_spec_float ( name: PString; nick: PString;blurb: PString; minimum: GLib.gfloat;maximum: GLib.gfloat; default_value: GLib.gfloat;flags: GParamFlags ): GParamSpec;
	PROCEDURE [ccall] g_param_spec_double ( name: PString; nick: PString;blurb: PString; minimum: GLib.gdouble;maximum: GLib.gdouble;default_value: GLib.gdouble;flags: GParamFlags ): GParamSpec;
	PROCEDURE [ccall] g_param_spec_string ( name: PString; nick: PString;blurb: PString; default_value: PString;flags: GParamFlags ): GParamSpec;
	PROCEDURE [ccall] g_param_spec_param ( name: PString; nick: PString;blurb: PString; param_type: GType;flags: GParamFlags ): GParamSpec;
	PROCEDURE [ccall] g_param_spec_boxed ( name: PString; nick: PString;blurb: PString; boxed_type: GType;flags: GParamFlags ): GParamSpec;
	PROCEDURE [ccall] g_param_spec_pointer ( name: PString; nick: PString;blurb: PString;flags: GParamFlags ): GParamSpec;
	PROCEDURE [ccall] g_param_spec_value_array ( name: PString; nick: PString;blurb: PString; element_spec: GParamSpec;flags: GParamFlags): GParamSpec;
	PROCEDURE [ccall] g_param_spec_object ( name: PString; nick: PString;blurb: PString; object_type: GType;flags: GParamFlags ): GParamSpec;
	PROCEDURE [ccall] g_param_spec_override ( name: PString;overridden: GParamSpec ): GParamSpec;

	

END LibsGObject.
