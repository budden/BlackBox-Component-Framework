(*
	Component Pascal binding for Cairo graphics library
	 Ported from Free Pascal binding by Romiras

	- Updated to Cairo version 1.6
	See https://bitbucket.org/Romiras/bindings for obtaining a complete source code package
*)

MODULE LibsCairo ["libcairo-2.dll"];

	IMPORT SYSTEM;

	TYPE
		(* C types aliases *)
		int32* = INTEGER;
		uint32* = INTEGER;
		double* = REAL;
		
		(* other Pascal-like type aliases *)
		Enum* =  int32;
		Integer* =  int32;
		LongWord* =  uint32;
		Real* =  double;
		Pointer* = POINTER TO RECORD [untagged] END; (* unsafe *)
		String* =  POINTER TO ARRAY [untagged] OF SHORTCHAR; (* UTF-8 encoded string *) (* unsafe *)
		ByteArrPtr* =  POINTER TO ARRAY [untagged] OF BYTE; (* unsafe *)
		RealArrPtr* =  POINTER TO ARRAY [untagged] OF Real; (* unsafe *)
		Data* = Pointer;
		
		(* Cairo types *)

		cairo_status_t* =  Enum;
		cairo_operator_t* =  Enum;
		cairo_antialias_t* =  Enum;
		cairo_fill_rule_t* =  Enum;
		cairo_line_cap_t* =  Enum;
		cairo_line_join_t* =  Enum;
		cairo_font_slant_t* =  Enum;
		cairo_font_weight_t* =  Enum;
		cairo_subpixel_order_t* =  Enum;
		cairo_hint_style_t* =  Enum;
		cairo_hint_metrics_t* =  Enum;
		cairo_path_data_type_t* =  Enum;
		cairo_content_t* =  Enum;
		cairo_format_t* =  Enum;
		cairo_extend_t* =  Enum;
		cairo_filter_t* =  Enum;
		cairo_font_type_t* =  Enum;
		cairo_pattern_type_t* =  Enum;
		cairo_surface_type_t* =  Enum;
		cairo_svg_version_t* =  Enum;
		
		cairo_matrix_t* =  RECORD [noalign] (* unsafe *)
			xx* : Real;
			yx* : Real;
			xy* : Real;
			yy* : Real;
			x0* : Real;
			y0* : Real;
		END;

		cairo_user_data_key_t* =  RECORD [noalign] (* unsafe *)
			unused : Integer;
		END;

		cairo_glyph_t* =  RECORD [noalign] (* unsafe *)
			index* : LongWord;
			x* : Real;
			y* : Real;
		END;

		cairo_text_extents_t* =  RECORD [noalign] (* unsafe *)
			x_bearing* : Real;
			y_bearing* : Real;
			width* : Real;
			height* : Real;
			x_advance* : Real;
			y_advance* : Real;
		END;

		cairo_font_extents_t* =  RECORD [noalign] (* unsafe *)
			ascent* : Real;
			descent* : Real;
			height* : Real;
			max_x_advance* : Real;
			max_y_advance* : Real;
		END;

		cairo_path_data_t* =  RECORD [union] (* unsafe *)
			header* : RECORD [untagged]
				type* : cairo_path_data_type_t;
				length* : Integer
			END;
			point* : RECORD [untagged]
				x* : Real;
				y* : Real
			END
		END;

		cairo_path_t* =  RECORD [noalign] (* unsafe *)
			status- : cairo_status_t;
			data- : cairo_path_data_t;
			num_data- : Integer;
		END;
		cairo_path_tPtr* = POINTER TO cairo_path_t;

		cairo_rectangle_t* =  RECORD [noalign] (* unsafe *)
			x*, y*, width*, height*: Real;
		END;

		cairo_rectangle_list_t* =  RECORD [noalign] (* unsafe *)
			status*: cairo_status_t;
			rectangles*: cairo_rectangle_t;
			num_rectangles*: Integer;
		END;
		cairo_rectangle_list_tPtr* = POINTER TO cairo_rectangle_list_t;
		
		cairo_bool_t* =  Integer;

		cairo_t* =  Pointer;
		cairo_surface_t* =  Pointer;
		cairo_pattern_t* =  Pointer;
		cairo_font_options_t* =  Pointer;
		cairo_font_face_t* =  Pointer;
		cairo_scaled_font_t* =  Pointer;

		cairo_destroy_func_t* =  PROCEDURE [ccall]  (data: Pointer);
		cairo_write_func_t* =  PROCEDURE [ccall]  (closure: Pointer; data: ByteArrPtr;
			length: LongWord): cairo_status_t;
		cairo_read_func_t* =  PROCEDURE [ccall]  (closure: Pointer; data: ByteArrPtr;
			length: LongWord): cairo_status_t;

	CONST

		CAIRO_STATUS_SUCCESS* = 0;
		CAIRO_STATUS_NO_MEMORY* = 1;
		CAIRO_STATUS_INVALID_RESTORE* = 2;
		CAIRO_STATUS_INVALID_POP_GROUP* = 3;
		CAIRO_STATUS_NO_CURRENT_POINT* = 4;
		CAIRO_STATUS_INVALID_MATRIX* = 5;
		CAIRO_STATUS_INVALID_STATUS* = 6;
		CAIRO_STATUS_NULL_POINTER* = 7;
		CAIRO_STATUS_INVALID_STRING* = 8;
		CAIRO_STATUS_INVALID_PATH_DATA* = 9;
		CAIRO_STATUS_READ_ERROR* = 10;
		CAIRO_STATUS_WRITE_ERROR* = 11;
		CAIRO_STATUS_SURFACE_FINISHED* = 12;
		CAIRO_STATUS_SURFACE_TYPE_MISMATCH* = 13;
		CAIRO_STATUS_PATTERN_TYPE_MISMATCH* = 14;
		CAIRO_STATUS_INVALID_CONTENT* = 15;
		CAIRO_STATUS_INVALID_FORMAT* = 16;
		CAIRO_STATUS_INVALID_VISUAL* = 17;
		CAIRO_STATUS_FILE_NOT_FOUND* = 18;
		CAIRO_STATUS_INVALID_DASH* = 19;

		CAIRO_OPERATOR_CLEAR* = 0;
		CAIRO_OPERATOR_SOURCE* = 1;
		CAIRO_OPERATOR_OVER* = 2;
		CAIRO_OPERATOR_IN* = 3;
		CAIRO_OPERATOR_OUT* = 4;
		CAIRO_OPERATOR_ATOP* = 5;
		CAIRO_OPERATOR_DEST* = 6;
		CAIRO_OPERATOR_DEST_OVER* = 7;
		CAIRO_OPERATOR_DEST_IN* = 8;
		CAIRO_OPERATOR_DEST_OUT* = 9;
		CAIRO_OPERATOR_DEST_ATOP* = 10;
		CAIRO_OPERATOR_XOR* = 11;
		CAIRO_OPERATOR_ADD* = 12;
		CAIRO_OPERATOR_SATURATE* = 13;

		CAIRO_ANTIALIAS_DEFAULT* = 0;
		CAIRO_ANTIALIAS_NONE* = 1;
		CAIRO_ANTIALIAS_GRAY* = 2;
		CAIRO_ANTIALIAS_SUBPIXEL* = 3;

		CAIRO_FILL_RULE_WINDING* = 0;
		CAIRO_FILL_RULE_EVEN_ODD* = 1;

		CAIRO_LINE_CAP_BUTT* = 0;
		CAIRO_LINE_CAP_ROUND* = 1;
		CAIRO_LINE_CAP_SQUARE* = 2;

		CAIRO_LINE_JOIN_MITER* = 0;
		CAIRO_LINE_JOIN_ROUND* = 1;
		CAIRO_LINE_JOIN_BEVEL* = 2;

		CAIRO_FONT_SLANT_NORMAL* = 0;
		CAIRO_FONT_SLANT_ITALIC* = 1;
		CAIRO_FONT_SLANT_OBLIQUE* = 2;

		CAIRO_FONT_WEIGHT_NORMAL* = 0;
		CAIRO_FONT_WEIGHT_BOLD* = 1;

		CAIRO_SUBPIXEL_ORDER_DEFAULT* = 0;
		CAIRO_SUBPIXEL_ORDER_RGB* = 1;
		CAIRO_SUBPIXEL_ORDER_BGR* = 2;
		CAIRO_SUBPIXEL_ORDER_VRGB* = 3;
		CAIRO_SUBPIXEL_ORDER_VBGR* = 4;

		CAIRO_HINT_STYLE_DEFAULT* = 0;
		CAIRO_HINT_STYLE_NONE* = 1;
		CAIRO_HINT_STYLE_SLIGHT* = 2;
		CAIRO_HINT_STYLE_MEDIUM* = 3;
		CAIRO_HINT_STYLE_FULL* = 4;

		CAIRO_HINT_METRICS_DEFAULT* = 0;
		CAIRO_HINT_METRICS_OFF* = 1;
		CAIRO_HINT_METRICS_ON* = 2;

		CAIRO_PATH_MOVE_TO* = 0;
		CAIRO_PATH_LINE_TO* = 1;
		CAIRO_PATH_CURVE_TO* = 2;
		CAIRO_PATH_CLOSE_PATH* = 3;

		CAIRO_CONTENT_COLOR* = 1000H;
		CAIRO_CONTENT_ALPHA* = 2000H;
		CAIRO_CONTENT_COLOR_ALPHA* = 3000H;

		CAIRO_FORMAT_ARGB32* = 0;
		CAIRO_FORMAT_RGB24* = 1;
		CAIRO_FORMAT_A8* = 2;
		CAIRO_FORMAT_A1* = 3;

		CAIRO_EXTEND_NONE* = 0;
		CAIRO_EXTEND_REPEAT* = 1;
		CAIRO_EXTEND_REFLECT* = 2;
		CAIRO_EXTEND_PAD* = 3;

		CAIRO_FILTER_FAST* = 0;
		CAIRO_FILTER_GOOD* = 1;
		CAIRO_FILTER_BEST* = 2;
		CAIRO_FILTER_NEAREST* = 3;
		CAIRO_FILTER_BILINEAR* = 4;
		CAIRO_FILTER_GAUSSIAN* = 5;

		CAIRO_FONT_TYPE_TOY* = 0;
		CAIRO_FONT_TYPE_FT* = 1;
		CAIRO_FONT_TYPE_WIN32* = 2;
		CAIRO_FONT_TYPE_ATSUI* = 3;

		CAIRO_PATTERN_TYPE_SOLID* = 0;
		CAIRO_PATTERN_TYPE_SURFACE* = 1;
		CAIRO_PATTERN_TYPE_LINEAR* = 2;
		CAIRO_PATTERN_TYPE_RADIAL* = 3;

		CAIRO_SURFACE_TYPE_IMAGE* = 0;
		CAIRO_SURFACE_TYPE_PDF* = 1;
		CAIRO_SURFACE_TYPE_PS* = 2;
		CAIRO_SURFACE_TYPE_XLIB* = 3;
		CAIRO_SURFACE_TYPE_XCB* = 4;
		CAIRO_SURFACE_TYPE_GLITZ* = 5;
		CAIRO_SURFACE_TYPE_QUARTZ* = 6;
		CAIRO_SURFACE_TYPE_WIN32* = 7;
		CAIRO_SURFACE_TYPE_BEOS* = 8;
		CAIRO_SURFACE_TYPE_DIRECTFB* = 9;
		CAIRO_SURFACE_TYPE_SVG* = 10;
		CAIRO_SURFACE_TYPE_OS2* = 11;

		CAIRO_SVG_VERSION_1_1* = 0;
		CAIRO_SVG_VERSION_1_2* = 1;


	PROCEDURE [ccall] cairo_version* (): Integer;
	PROCEDURE [ccall] cairo_version_string* (): String;

	(* functions for manipulating state objects *)

	PROCEDURE [ccall] cairo_create* (target: cairo_surface_t): cairo_t;
	PROCEDURE [ccall] cairo_destroy* (cr: cairo_t);
	PROCEDURE [ccall] cairo_reference* (cr: cairo_t): cairo_t;
	PROCEDURE [ccall] cairo_get_reference_count* (cr: cairo_t): LongWord;

	PROCEDURE [ccall] cairo_get_user_data* (cr: cairo_t; key: cairo_user_data_key_t): Data;
	PROCEDURE [ccall] cairo_set_user_data* (cr: cairo_t; key: cairo_user_data_key_t;
         user_data: Data; destroy: cairo_destroy_func_t): cairo_status_t;

	PROCEDURE [ccall] cairo_save* (cr: cairo_t);
	PROCEDURE [ccall] cairo_restore* (cr: cairo_t);

	PROCEDURE [ccall] cairo_push_group* (cr: cairo_t);
	PROCEDURE [ccall] cairo_push_group_with_content* (cr: cairo_t; content: cairo_content_t);
	PROCEDURE [ccall] cairo_pop_group* (cr: cairo_t): cairo_pattern_t;
	PROCEDURE [ccall] cairo_pop_group_to_source* (cr: cairo_t);

	PROCEDURE [ccall] cairo_get_target* (cr: cairo_t): cairo_surface_t;
	PROCEDURE [ccall] cairo_get_group_target* (cr: cairo_t): cairo_surface_t;

	(* Modify state *)

	PROCEDURE [ccall] cairo_set_source* (cr: cairo_t; source: cairo_pattern_t);
	PROCEDURE [ccall] cairo_get_source* (cr: cairo_t): cairo_pattern_t;
	PROCEDURE [ccall] cairo_set_source_surface* (cr: cairo_t; surface: cairo_surface_t; x, y: Real);
	PROCEDURE [ccall] cairo_set_source_rgb* (cr: cairo_t; red, green, blue: Real);
	PROCEDURE [ccall] cairo_set_source_rgba* (cr: cairo_t; red, green, blue, alpha: Real);

	PROCEDURE [ccall] cairo_set_operator* (cr: cairo_t; op: cairo_operator_t);
	PROCEDURE [ccall] cairo_get_operator* (cr: cairo_t): cairo_operator_t;
	PROCEDURE [ccall] cairo_set_tolerance* (cr: cairo_t; tolerance: Real);
	PROCEDURE [ccall] cairo_get_tolerance* (cr: cairo_t): Real;

	(* Path procedures *)

	PROCEDURE [ccall] cairo_new_path* (cr: cairo_t);
	PROCEDURE [ccall] cairo_new_sub_path* (cr: cairo_t);
	PROCEDURE [ccall] cairo_close_path* (cr: cairo_t);
	PROCEDURE [ccall] cairo_copy_path* (cr: cairo_t): cairo_path_tPtr;
	PROCEDURE [ccall] cairo_copy_path_flat* (cr: cairo_t): cairo_path_tPtr;
	PROCEDURE [ccall] cairo_append_path* (cr: cairo_t; VAR path: cairo_path_t);
	PROCEDURE [ccall] cairo_path_destroy* (VAR path: cairo_path_t);
	(* PROCEDURE [ccall] cairo_path_extents* (cr: cairo_t; VAR x1, y1, x2, y2: Real); *)

	PROCEDURE [ccall] cairo_move_to* (cr: cairo_t; x, y: Real);
	PROCEDURE [ccall] cairo_rel_move_to* (cr: cairo_t; dx, dy: Real);
	PROCEDURE [ccall] cairo_get_current_point* (cr: cairo_t; VAR x, y: Real);

	PROCEDURE [ccall] cairo_line_to* (cr: cairo_t; x, y: Real);
	PROCEDURE [ccall] cairo_rel_line_to* (cr: cairo_t; dx, dy: Real);
	PROCEDURE [ccall] cairo_rectangle* (cr: cairo_t; x, y, width, height: Real);
	PROCEDURE [ccall] cairo_arc* (cr: cairo_t; xc, yc, radius, angle1, angle2: Real);
	PROCEDURE [ccall] cairo_arc_negative* (cr: cairo_t; xc, yc, radius, angle1, angle2: Real);
	PROCEDURE [ccall] cairo_curve_to* (cr: cairo_t; x1, y1, x2, y2, x3, y3: Real);
	PROCEDURE [ccall] cairo_rel_curve_to* (cr: cairo_t; dx1, dy1, dx2, dy2, dx3, dy3: Real);

	(* Clipping *)

	PROCEDURE [ccall] cairo_reset_clip* (cr: cairo_t);
	PROCEDURE [ccall] cairo_clip* (cr: cairo_t);
	PROCEDURE [ccall] cairo_clip_preserve* (cr: cairo_t);
	PROCEDURE [ccall] cairo_clip_extents* (cr: cairo_t; VAR x1, y1, x2, y2: Real);
	PROCEDURE [ccall] cairo_copy_clip_rectangle_list* (cr: cairo_t): cairo_rectangle_list_tPtr;
	PROCEDURE [ccall] cairo_rectangle_list_destroy* (rectangle_list: cairo_rectangle_list_t);

	(* CTM functions *)

	PROCEDURE [ccall] cairo_set_matrix* (cr: cairo_t; IN matrix: cairo_matrix_t);
	PROCEDURE [ccall] cairo_get_matrix* (cr: cairo_t; VAR matrix: cairo_matrix_t);
	PROCEDURE [ccall] cairo_identity_matrix* (cr: cairo_t);
	PROCEDURE [ccall] cairo_translate* (cr: cairo_t; tx, ty: Real);
	PROCEDURE [ccall] cairo_scale* (cr: cairo_t; sx, sy: Real);
	PROCEDURE [ccall] cairo_rotate* (cr: cairo_t; angle: Real);
	PROCEDURE [ccall] cairo_transform* (cr: cairo_t; VAR matrix: cairo_matrix_t);
	PROCEDURE [ccall] cairo_user_to_device* (cr: cairo_t; VAR x, y: Real);
	PROCEDURE [ccall] cairo_user_to_device_distance* (cr: cairo_t; VAR dx, dy: Real);
	PROCEDURE [ccall] cairo_device_to_user* (cr: cairo_t; VAR x, y: Real);
	PROCEDURE [ccall] cairo_device_to_user_distance* (cr: cairo_t; VAR dx, dy: Real);

	(* Painting procedures *)

	PROCEDURE [ccall] cairo_set_line_width* (cr: cairo_t; width: Real);
	PROCEDURE [ccall] cairo_get_line_width* (cr: cairo_t): Real;
	PROCEDURE [ccall] cairo_set_line_cap* (cr: cairo_t; line_cap: cairo_line_cap_t);
	PROCEDURE [ccall] cairo_get_line_cap* (cr: cairo_t): cairo_line_cap_t;
	PROCEDURE [ccall] cairo_set_line_join* (cr: cairo_t; line_join: cairo_line_join_t);
	PROCEDURE [ccall] cairo_get_line_join* (cr: cairo_t): cairo_line_join_t;
	PROCEDURE [ccall] cairo_set_miter_limit* (cr: cairo_t; limit: Real);
	PROCEDURE [ccall] cairo_get_miter_limit* (cr: cairo_t): Real;
	PROCEDURE [ccall] cairo_set_dash* (cr: cairo_t; dashes: RealArrPtr; num_dashes: Integer; offset: Real);
	PROCEDURE [ccall] cairo_get_dash_count* (cr: cairo_t): Integer;
	PROCEDURE [ccall] cairo_get_dash* (cr: cairo_t; VAR dashes: RealArrPtr; VAR offset: Real);

	PROCEDURE [ccall] cairo_set_fill_rule* (cr: cairo_t; fill_rule: cairo_fill_rule_t);
	PROCEDURE [ccall] cairo_get_fill_rule* (cr: cairo_t): cairo_fill_rule_t;

	PROCEDURE [ccall] cairo_paint* (cr: cairo_t);
	PROCEDURE [ccall] cairo_paint_with_alpha* (cr: cairo_t; alpha: Real);
	PROCEDURE [ccall] cairo_mask* (cr: cairo_t; pattern: cairo_pattern_t);
	PROCEDURE [ccall] cairo_mask_surface* (cr: cairo_t; surface: cairo_surface_t; surface_x, surface_y: Real);
	PROCEDURE [ccall] cairo_stroke* (cr: cairo_t);
	PROCEDURE [ccall] cairo_stroke_preserve* (cr: cairo_t);
	PROCEDURE [ccall] cairo_fill* (cr: cairo_t);
	PROCEDURE [ccall] cairo_fill_preserve* (cr: cairo_t);
	PROCEDURE [ccall] cairo_copy_page* (cr: cairo_t);
	PROCEDURE [ccall] cairo_show_page* (cr: cairo_t);

	(* Insideness testing *)

	PROCEDURE [ccall] cairo_in_stroke* (cr: cairo_t; x, y: Real): cairo_bool_t;
	PROCEDURE [ccall] cairo_in_fill* (cr: cairo_t; x, y: Real): cairo_bool_t;

	(* Rectangular extents *)

	PROCEDURE [ccall] cairo_stroke_extents* (cr: cairo_t; VAR x1, y1, x2, y2: Real);
	PROCEDURE [ccall] cairo_fill_extents* (cr: cairo_t; VAR x1, y1, x2, y2: Real);

	(* Font/Text functions *)

	PROCEDURE [ccall] cairo_font_options_create* (): cairo_font_options_t;
	PROCEDURE [ccall] cairo_font_options_copy* (original: cairo_font_options_t): cairo_font_options_t;
	PROCEDURE [ccall] cairo_font_options_destroy * (options: cairo_font_options_t);
	PROCEDURE [ccall] cairo_font_options_status* (options: cairo_font_options_t): cairo_status_t;
	PROCEDURE [ccall] cairo_font_options_merge* (options, other: cairo_font_options_t);
	PROCEDURE [ccall] cairo_font_options_equal* (options, other: cairo_font_options_t): cairo_bool_t;
	PROCEDURE [ccall] cairo_font_options_hash* (options: cairo_font_options_t): LongWord;
	PROCEDURE [ccall] cairo_font_options_set_antialias* (options: cairo_font_options_t; antialias: cairo_antialias_t);
	PROCEDURE [ccall] cairo_font_options_get_antialias* (options: cairo_font_options_t): cairo_antialias_t;
	PROCEDURE [ccall] cairo_font_options_set_subpixel_order* (options: cairo_font_options_t; subpixel_order: cairo_subpixel_order_t);
	PROCEDURE [ccall] cairo_font_options_get_subpixel_order* (options: cairo_font_options_t): cairo_subpixel_order_t;
	PROCEDURE [ccall] cairo_font_options_set_hint_style* (options: cairo_font_options_t; hint_style: cairo_hint_style_t);
	PROCEDURE [ccall] cairo_font_options_get_hint_style* (options: cairo_font_options_t): cairo_hint_style_t;
	PROCEDURE [ccall] cairo_font_options_set_hint_metrics* (options: cairo_font_options_t; hint_metrics: cairo_hint_metrics_t);
	PROCEDURE [ccall] cairo_font_options_get_hint_metrics* (options: cairo_font_options_t): cairo_hint_metrics_t;

	(* This interface is for dealing with text as text, not caring about the
    font object inside the the cairo_t. *)

	PROCEDURE [ccall] cairo_select_font_face* (cr: cairo_t; family: String; slant: cairo_font_slant_t; weight: cairo_font_weight_t);
	PROCEDURE [ccall] cairo_set_font_size* (cr: cairo_t; size: Real);
	PROCEDURE [ccall] cairo_set_font_matrix* (cr: cairo_t; VAR matrix: cairo_matrix_t);
	PROCEDURE [ccall] cairo_get_font_matrix* (cr: cairo_t; VAR matrix: cairo_matrix_t);
	PROCEDURE [ccall] cairo_set_font_options* (cr: cairo_t; options: cairo_font_options_t);
	PROCEDURE [ccall] cairo_get_font_options* (cr: cairo_t; options: cairo_font_options_t);
	PROCEDURE [ccall] cairo_set_font_face* (cr: cairo_t; font_face: cairo_font_face_t);
	PROCEDURE [ccall] cairo_get_font_face* (cr: cairo_t): cairo_font_face_t;
	PROCEDURE [ccall] cairo_set_scaled_font* (cr: cairo_t; scaled_font: cairo_scaled_font_t);
	PROCEDURE [ccall] cairo_get_scaled_font* (cr: cairo_t): cairo_scaled_font_t;
	PROCEDURE [ccall] cairo_show_text* (cr: cairo_t; utf8: String);
	PROCEDURE [ccall] cairo_show_glyphs* (cr: cairo_t; glyphs: cairo_glyph_t; num_glyphs: Integer);
	PROCEDURE [ccall] cairo_text_path* (cr: cairo_t; utf8: String);
	PROCEDURE [ccall] cairo_glyph_path* (cr: cairo_t; glyphs: cairo_glyph_t; num_glyphs: Integer);
	PROCEDURE [ccall] cairo_text_extents* (cr: cairo_t; utf8: String; extents: cairo_text_extents_t);
	PROCEDURE [ccall] cairo_glyph_extents* (cr: cairo_t; glyphs: cairo_glyph_t; num_glyphs: Integer; extents: cairo_text_extents_t);
	PROCEDURE [ccall] cairo_font_extents* (cr: cairo_t; extents: cairo_font_extents_t);

	(* Generic identifier for a font style *)

	PROCEDURE [ccall] cairo_font_face_reference* (font_face: cairo_font_face_t): cairo_font_face_t;
	PROCEDURE [ccall] cairo_font_face_destroy* (font_face: cairo_font_face_t);
	PROCEDURE [ccall] cairo_font_face_get_reference_count * (font_face: cairo_font_face_t):  LongWord;
	PROCEDURE [ccall] cairo_font_face_status* (font_face: cairo_font_face_t): cairo_status_t;
	PROCEDURE [ccall] cairo_font_face_get_type* (font_face: cairo_font_face_t): cairo_font_type_t;
	PROCEDURE [ccall] cairo_font_face_get_user_data* (font_face: cairo_font_face_t; key: cairo_user_data_key_t): Data;
	PROCEDURE [ccall] cairo_font_face_set_user_data* (font_face: cairo_font_face_t; key: cairo_user_data_key_t; user_data: Data; destroy: cairo_destroy_func_t): cairo_status_t;

	(* Portable interface to general font features *)

	PROCEDURE [ccall] cairo_scaled_font_create* (font_face: cairo_font_face_t; font_matrix: cairo_matrix_t; ctm: cairo_matrix_t; options: cairo_font_options_t): cairo_scaled_font_t;
	PROCEDURE [ccall] cairo_scaled_font_reference* (scaled_font: cairo_scaled_font_t): cairo_scaled_font_t;
	PROCEDURE [ccall] cairo_scaled_font_destroy* (scaled_font: cairo_scaled_font_t);
	PROCEDURE [ccall] cairo_scaled_font_get_reference_count * (scaled_font: cairo_scaled_font_t): LongWord;
	PROCEDURE [ccall] cairo_scaled_font_status* (scaled_font: cairo_scaled_font_t): cairo_status_t;
	PROCEDURE [ccall] cairo_scaled_font_get_type* (scaled_font: cairo_scaled_font_t): cairo_font_type_t;
	PROCEDURE [ccall] cairo_scaled_font_get_user_data * (scaled_font: cairo_scaled_font_t; key: cairo_user_data_key_t): Data;
	PROCEDURE [ccall] cairo_scaled_font_set_user_data * (scaled_font: cairo_scaled_font_t; key: cairo_user_data_key_t; user_data: Data; destroy: cairo_destroy_func_t): cairo_status_t;
	PROCEDURE [ccall] cairo_scaled_font_extents* (scaled_font: cairo_scaled_font_t; extents: cairo_font_extents_t);
	PROCEDURE [ccall] cairo_scaled_font_text_extents* (scaled_font: cairo_scaled_font_t; utf8: String; extents: cairo_text_extents_t);
	PROCEDURE [ccall] cairo_scaled_font_glyph_extents* (scaled_font: cairo_scaled_font_t; glyphs: cairo_glyph_t; num_glyphs: Integer; extents: cairo_text_extents_t);
	PROCEDURE [ccall] cairo_scaled_font_get_font_face* (scaled_font: cairo_scaled_font_t): cairo_font_face_t;
	PROCEDURE [ccall] cairo_scaled_font_get_font_matrix* (scaled_font: cairo_scaled_font_t;	font_matrix: cairo_matrix_t);
	PROCEDURE [ccall] cairo_scaled_font_get_ctm* (scaled_font: cairo_scaled_font_t;	ctm: cairo_matrix_t);
	PROCEDURE [ccall] cairo_scaled_font_get_font_options* (scaled_font: cairo_scaled_font_t; options: cairo_font_options_t);

	(* Pattern creation procedures *)

	PROCEDURE [ccall] cairo_pattern_create_rgb* (red, green, blue: Real): cairo_pattern_t;
	PROCEDURE [ccall] cairo_pattern_create_rgba* (red, green, blue, alpha: Real): cairo_pattern_t;
	PROCEDURE [ccall] cairo_pattern_create_for_surface* (surface: cairo_surface_t): cairo_pattern_t;
	PROCEDURE [ccall] cairo_pattern_create_linear* (x0, y0, x1, y1: Real): cairo_pattern_t;
	PROCEDURE [ccall] cairo_pattern_create_radial* (cx0, cy0, radius0, cx1, cy1, radius1: Real): cairo_pattern_t;
	PROCEDURE [ccall] cairo_pattern_reference* (pattern: cairo_pattern_t): cairo_pattern_t;
	PROCEDURE [ccall] cairo_pattern_destroy* (pattern: cairo_pattern_t);
	PROCEDURE [ccall] cairo_pattern_get_reference_count * (pattern: cairo_pattern_t): LongWord;
	PROCEDURE [ccall] cairo_pattern_status* (pattern: cairo_pattern_t): cairo_status_t;
	PROCEDURE [ccall] cairo_pattern_get_user_data * (pattern: cairo_pattern_t; key: cairo_user_data_key_t): Pointer;
	PROCEDURE [ccall] cairo_pattern_set_user_data * (pattern: cairo_pattern_t; key: cairo_user_data_key_t; user_data: Data; destroy: cairo_destroy_func_t): cairo_status_t;
	PROCEDURE [ccall] cairo_pattern_get_type* (pattern: cairo_pattern_t): cairo_pattern_type_t;
	PROCEDURE [ccall] cairo_pattern_add_color_stop_rgb* (pattern: cairo_pattern_t; offset, red, green, blue: Real);
	PROCEDURE [ccall] cairo_pattern_add_color_stop_rgba* (pattern: cairo_pattern_t; offset, red, green, blue, alpha: Real);
	PROCEDURE [ccall] cairo_pattern_set_matrix* (pattern: cairo_pattern_t; matrix: cairo_matrix_t);
	PROCEDURE [ccall] cairo_pattern_get_matrix* (pattern: cairo_pattern_t; VAR matrix: cairo_matrix_t);
	PROCEDURE [ccall] cairo_pattern_set_extend* (pattern: cairo_pattern_t; extend: cairo_extend_t);
	PROCEDURE [ccall] cairo_pattern_get_extend* (pattern: cairo_pattern_t): cairo_extend_t;
	PROCEDURE [ccall] cairo_pattern_set_filter* (pattern: cairo_pattern_t; filter: cairo_filter_t);
	PROCEDURE [ccall] cairo_pattern_get_filter* (pattern: cairo_pattern_t): cairo_filter_t;
	PROCEDURE [ccall] cairo_pattern_get_rgba * (pattern: cairo_pattern_t; VAR red, green, blue, alpha: Real): cairo_status_t;
	PROCEDURE [ccall] cairo_pattern_get_surface * (pattern: cairo_pattern_t; VAR surface: cairo_surface_t): cairo_status_t;
	PROCEDURE [ccall] cairo_pattern_get_color_stop_rgba* (pattern: cairo_pattern_t; index: Integer; VAR offset, red, green, blue, alpha: Real):cairo_status_t;
	PROCEDURE [ccall] cairo_pattern_get_color_stop_count* (pattern: cairo_pattern_t; VAR count: Integer):cairo_status_t;
	PROCEDURE [ccall] cairo_pattern_get_linear_points * (pattern: cairo_pattern_t; VAR x0, y0, x1, y1: Real): cairo_status_t;
	PROCEDURE [ccall] cairo_pattern_get_radial_circles * (pattern: cairo_pattern_t; VAR x0, y0, r0, x1, y1, r1: Real): cairo_status_t;

	(* Matrix functions *)

	PROCEDURE [ccall] cairo_matrix_init* (VAR matrix: cairo_matrix_t; xx, yx, xy, yy, x0, y0: Real);
	PROCEDURE [ccall] cairo_matrix_init_identity* (VAR matrix: cairo_matrix_t);
	PROCEDURE [ccall] cairo_matrix_init_translate* (VAR matrix: cairo_matrix_t; tx, ty: Real);
	PROCEDURE [ccall] cairo_matrix_init_scale* (VAR matrix: cairo_matrix_t; sx, sy: Real);
	PROCEDURE [ccall] cairo_matrix_init_rotate* (VAR matrix: cairo_matrix_t; radians: Real);
	PROCEDURE [ccall] cairo_matrix_translate* (VAR matrix: cairo_matrix_t; tx, ty: Real);
	PROCEDURE [ccall] cairo_matrix_scale* (VAR matrix: cairo_matrix_t; sx, sy: Real);
	PROCEDURE [ccall] cairo_matrix_rotate* (VAR matrix: cairo_matrix_t; radians: Real);
	PROCEDURE [ccall] cairo_matrix_invert* (VAR matrix: cairo_matrix_t): cairo_status_t;
	PROCEDURE [ccall] cairo_matrix_multiply* (OUT result: cairo_matrix_t; VAR a, b: cairo_matrix_t);
	PROCEDURE [ccall] cairo_matrix_transform_distance* (VAR matrix: cairo_matrix_t; VAR dx, dy: Real);
	PROCEDURE [ccall] cairo_matrix_transform_point* (VAR matrix: cairo_matrix_t; VAR x, y: Real);

	(* Error status queries *)

	PROCEDURE [ccall] cairo_status* (cr: cairo_t): cairo_status_t;
	PROCEDURE [ccall] cairo_status_to_string* (status: cairo_status_t): String;

	(* Antialias *)

	PROCEDURE [ccall] cairo_set_antialias* (cr: cairo_t; antialias: cairo_antialias_t);
	PROCEDURE [ccall] cairo_get_antialias* (cr: cairo_t): cairo_antialias_t;

	(* Surface manipulation *)

	PROCEDURE [ccall] cairo_surface_create_similar* (other: cairo_surface_t; content: cairo_content_t; width, height: Integer): cairo_surface_t;
	PROCEDURE [ccall] cairo_surface_reference* (surface: cairo_surface_t): cairo_surface_t;
	PROCEDURE [ccall] cairo_surface_finish* (surface: cairo_surface_t);
	PROCEDURE [ccall] cairo_surface_destroy* (surface: cairo_surface_t);
	PROCEDURE [ccall] cairo_surface_get_reference_count* (surface: cairo_surface_t): LongWord;
	PROCEDURE [ccall] cairo_surface_status* (surface: cairo_surface_t): cairo_status_t;
	PROCEDURE [ccall] cairo_surface_get_type* (surface: cairo_surface_t): cairo_surface_type_t;
	PROCEDURE [ccall] cairo_surface_get_content* (surface: cairo_surface_t): cairo_content_t;
	PROCEDURE [ccall] cairo_surface_get_user_data* (surface: cairo_surface_t; key: cairo_user_data_key_t): Pointer;
	PROCEDURE [ccall] cairo_surface_set_user_data* (surface: cairo_surface_t; key: cairo_user_data_key_t; user_data: Data; destroy: cairo_destroy_func_t): cairo_status_t;
	PROCEDURE [ccall] cairo_surface_get_font_options* (surface: cairo_surface_t; options: cairo_font_options_t);
	PROCEDURE [ccall] cairo_surface_flush* (surface: cairo_surface_t);
	PROCEDURE [ccall] cairo_surface_mark_dirty* (surface: cairo_surface_t);
	PROCEDURE [ccall] cairo_surface_mark_dirty_rectangle* (surface: cairo_surface_t; x, y, width, height: Integer);
	PROCEDURE [ccall] cairo_surface_set_device_offset* (surface: cairo_surface_t; x_offset, y_offset: Real);
	PROCEDURE [ccall] cairo_surface_get_device_offset* (surface: cairo_surface_t; VAR x_offset, y_offset: Real);
	PROCEDURE [ccall] cairo_surface_set_fallback_resolution* (surface: cairo_surface_t; x_pixels_per_inch, y_pixels_per_inch: Real);

	PROCEDURE [ccall] cairo_surface_write_to_png* (surface: cairo_surface_t; filename: String): cairo_status_t;
	PROCEDURE [ccall] cairo_surface_write_to_png_stream* (surface: cairo_surface_t; write_func: cairo_write_func_t; closure: Pointer): cairo_status_t;

	(* Image-surface functions *)

	PROCEDURE [ccall] cairo_image_surface_create* (format: cairo_format_t; width, height: Integer): cairo_surface_t;
	PROCEDURE [ccall] cairo_image_surface_create_for_data* (data: ByteArrPtr; format: cairo_format_t; width, height, stride: Integer): cairo_surface_t;
	PROCEDURE [ccall] cairo_image_surface_get_data* (surface: cairo_surface_t): ByteArrPtr;
	PROCEDURE [ccall] cairo_image_surface_get_format* (surface: cairo_surface_t): cairo_format_t;
	PROCEDURE [ccall] cairo_image_surface_get_width* (surface: cairo_surface_t): Integer;
	PROCEDURE [ccall] cairo_image_surface_get_height* (surface: cairo_surface_t): Integer;
	PROCEDURE [ccall] cairo_image_surface_get_stride* (surface: cairo_surface_t): Integer;
	PROCEDURE [ccall] cairo_image_surface_create_from_png* (filename: String): cairo_surface_t;
	PROCEDURE [ccall] cairo_image_surface_create_from_png_stream* (read_func: cairo_read_func_t; closure: Pointer): cairo_surface_t;

	(* PDF functions *)

	PROCEDURE [ccall] cairo_pdf_surface_create* (filename: String; width_in_points, height_in_points: Real): cairo_surface_t;
	PROCEDURE [ccall] cairo_pdf_surface_create_for_stream* (write_func: cairo_write_func_t; closure: Pointer; width_in_points, height_in_points: Real): cairo_surface_t;
	PROCEDURE [ccall] cairo_pdf_surface_set_size* (surface: cairo_surface_t; width_in_points, height_in_points: Real);

	(* PS functions *)

	PROCEDURE [ccall] cairo_ps_surface_create* (filename: String; width_in_points, height_in_points: Real): cairo_surface_t;
	PROCEDURE [ccall] cairo_ps_surface_create_for_stream* (write_func: cairo_write_func_t; closure: Pointer; width_in_points, height_in_points: Real): cairo_surface_t;
	PROCEDURE [ccall] cairo_ps_surface_set_size* (surface: cairo_surface_t; width_in_points, height_in_points: Real);
	PROCEDURE [ccall] cairo_ps_surface_dsc_comment* (surface: cairo_surface_t; comment: String);
	PROCEDURE [ccall] cairo_ps_surface_dsc_begin_setup* (surface: cairo_surface_t);
	PROCEDURE [ccall] cairo_ps_surface_dsc_begin_page_setup* (surface: cairo_surface_t);

	(* SVG functions *)

	PROCEDURE [ccall] cairo_svg_surface_create* (filename: String; width_in_points, height_in_points: Real): cairo_surface_t;
	PROCEDURE [ccall] cairo_svg_surface_create_for_stream* (write_func: cairo_write_func_t; closure: Pointer; width_in_points, height_in_points: Real): cairo_surface_t;
	PROCEDURE [ccall] cairo_svg_surface_restrict_to_version* (surface: cairo_surface_t; version: cairo_svg_version_t);

	PROCEDURE [ccall] cairo_svg_get_versions* (VAR versions: cairo_svg_version_t; VAR num_versions: Integer);
	PROCEDURE [ccall] cairo_svg_version_to_string* (version: cairo_svg_version_t): String;

	(* Miscellaneous functions *)

	PROCEDURE [ccall] cairo_debug_reset_static_data*;

END LibsCairo.
