(* This file is part of Cairo library binding. See https://bitbucket.org/Romiras/bindings for obtaining a complete source code package *)

(* Win32 platform specific functions of Cairo *)

MODULE LibsCairoWin32 ["libcairo-2.dll"];

		IMPORT Lib := LibsCairo, WinApi;

		TYPE
			Handle* = WinApi.HDC;
		
		PROCEDURE [ccall] cairo_win32_surface_create* (HDC: Handle): Lib.cairo_surface_t;
		PROCEDURE [ccall] cairo_win32_surface_create_with_ddb* (HDC: Handle; format: Lib.cairo_format_t; width, height: Lib.Integer): Lib.cairo_surface_t;
		PROCEDURE [ccall] cairo_win32_surface_create_with_dib* (format: Lib.cairo_format_t; width, height: Lib.Integer): Lib.cairo_surface_t;
		PROCEDURE [ccall] cairo_win32_surface_get_dc* (surface: Lib.cairo_surface_t): Handle;
		PROCEDURE [ccall] cairo_win32_surface_get_image* (surface: Lib.cairo_surface_t): Lib.cairo_surface_t;
		PROCEDURE [ccall] cairo_win32_font_face_create_for_logfontw* (logfont: WinApi.PtrLOGFONTW): Lib.cairo_font_face_t;
		PROCEDURE [ccall] cairo_win32_font_face_create_for_hfont* (font: WinApi.HFONT): Lib.cairo_font_face_t;
		PROCEDURE [ccall] cairo_win32_scaled_font_select_font* (scaled_font: Lib.cairo_scaled_font_t; HDC: Handle): Lib.cairo_status_t;
		PROCEDURE [ccall] cairo_win32_scaled_font_done_font* (scaled_font: Lib.cairo_scaled_font_t);
		PROCEDURE [ccall] cairo_win32_scaled_font_get_metrics_factor* (scaled_font: Lib.cairo_scaled_font_t): Lib.Real;
		PROCEDURE [ccall] cairo_win32_scaled_font_get_logical_to_device* (scaled_font: Lib.cairo_scaled_font_t; VAR logical_to_device: Lib.cairo_matrix_t);
		PROCEDURE [ccall] cairo_win32_scaled_font_get_device_to_logical* (scaled_font: Lib.cairo_scaled_font_t; VAR device_to_logical: Lib.cairo_matrix_t);

END LibsCairoWin32.