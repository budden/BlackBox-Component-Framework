MODULE HostFonts;

	IMPORT SYSTEM,
		Kernel, Fonts,  (*  HostRegistry, *)
		Pango := LibsPango, GLib := LibsGlib, Gdk := LibsGdk;

	CONST
		defSize = 10 * Fonts.point;	(* size of default font *)
		dlgSize =  8 * Fonts.point;	(* size of dialog font *)
		figureSpace = 8FX;


	TYPE
		Typeface = Fonts.Typeface;

		Font* = POINTER TO RECORD (Fonts.Font)
			asc*, dsc*, w: INTEGER;
			desc-: Pango.PangoFontDescription;
			alias-: Fonts.Typeface;	(* alias # typeface & typeface # "*" == alien font *)
		END;

		Directory = POINTER TO RECORD (Fonts.Directory) END;

		Identifier = RECORD (Kernel.Identifier)
			tface: Fonts.Typeface;
			size: INTEGER;
			style: SET;
			weight: INTEGER
		END;

		Counter = RECORD (Kernel.Identifier)
			count: INTEGER
		END;

		Traverser = RECORD (Kernel.Identifier)
		END;

	VAR
		dir: Directory;
		defFont-, dlgFont- : Font;
		DefFontName, DlgFontName: Fonts.Typeface;
		DefFontSize, DlgFontSize, DlgFontWght: INTEGER;
		DlgFontStyle: SET;
		pixel-: INTEGER;	(* screen resolution *)
		context-:Pango.PangoContext;
		layout-:Pango.PangoLayout;

	PROCEDURE IntToString (x: INTEGER; OUT s: ARRAY OF SHORTCHAR);
		VAR j, k: INTEGER; ch: SHORTCHAR; a: ARRAY 32 OF SHORTCHAR;
	BEGIN
		ASSERT(x >= 0, 20);
		k := 0;
		j := 0;
		REPEAT
			a[j] := SHORT(CHR(x MOD 10 + ORD("0")));
			x := x DIV 10;
			INC(j)
		UNTIL x = 0;
		ASSERT(k + j < LEN(s), 20);
		REPEAT DEC(j); ch := a[j]; s[k] := ch; INC(k) UNTIL j = 0;
		s[k] := 0X
	END IntToString;

	PROCEDURE ParsePangoString* (IN str: ARRAY OF SHORTCHAR;
										OUT typeface: Typeface; OUT size: INTEGER;
										OUT style: SET; OUT weight: INTEGER);
	VAR
			fdesc:Pango.PangoFontDescription;
			psize:INTEGER;
	BEGIN
		fdesc := Pango.pango_font_description_from_string(str);
		typeface := Pango.pango_font_description_get_family(fdesc)$;
		size := (Pango.pango_font_description_get_size(fdesc)+512) DIV 1024 * Fonts.point;
		weight := Pango.pango_font_description_get_weight(fdesc);
		style:={};
		CASE Pango.pango_font_description_get_style(fdesc) OF
		| Pango.PANGO_STYLE_NORMAL :
		| Pango.PANGO_STYLE_OBLIQUE,
		  Pango.PANGO_STYLE_ITALIC : INCL(style,Fonts.italic)
		ELSE
		END;
	END ParsePangoString;

	PROCEDURE MakePangoString* (typeface: Typeface; size: INTEGER; style: SET; weight: INTEGER;
															OUT str: ARRAY OF SHORTCHAR);
	VAR s_size, s_weight, s_slant: ARRAY 10 OF SHORTCHAR;
	BEGIN
		IF typeface = Fonts.default THEN	typeface := DefFontName$	END;
		IF weight = Fonts.bold THEN s_weight := "Bold" ELSE s_weight := "" END;
		IF Fonts.italic IN style THEN s_slant := "Italic" ELSE s_slant := "" END;
		IF size#0 THEN
			IntToString(size DIV Fonts.point, s_size)
		ELSE
			s_size := ""
		END;
		str := SHORT(typeface) + " " + s_weight + " " + s_slant + " " + s_size;
	END MakePangoString;

	PROCEDURE Cleanup (f: Font);
	BEGIN
		Pango.pango_font_description_free(f.desc);
		f.desc := NIL
	END Cleanup;

	(* width methods for unicode *)

	PROCEDURE (f: Font) wTab* (ch: CHAR): INTEGER, NEW;
	BEGIN
		RETURN 0 (* ??? *)
	END wTab;

	PROCEDURE (f: Font) fTab* (ch: CHAR): INTEGER, NEW;
	BEGIN
		RETURN 0
	END fTab;

	PROCEDURE (f: Font) tTab* (ch: CHAR): INTEGER, NEW;
	BEGIN
		RETURN 0
	END tTab;


	(** Font **)

	PROCEDURE (f: Font) GetBounds* (OUT asc, dsc, w: INTEGER);
	BEGIN
		asc := f.asc; dsc := f.dsc; w := f.w
	END GetBounds;


	PROCEDURE  ShapeUString (layout:Pango.PangoLayout; us: GLib.PString; font: Fonts.Font);
	VAR attrs: Pango.PangoAttrList; attr: Pango.PangoAttribute;
	BEGIN
		Pango.pango_layout_set_text(layout, us,-1);
		attrs:= Pango.pango_attr_list_new();
		IF font # NIL THEN
			Pango.pango_layout_set_font_description(layout, font(Font).desc);
			IF Fonts.underline IN font.style THEN
				attr:= Pango.pango_attr_underline_new(1);
			ELSE
				attr:= Pango.pango_attr_underline_new(0);
			END;
			attr.start_index:=0;
			attr.end_index:= LEN(us$);
			Pango.pango_attr_list_insert(attrs,attr);
		END;
		Pango.pango_layout_set_attributes(layout, attrs);
		Pango.pango_attr_list_unref(attrs);
	END ShapeUString;


	PROCEDURE ShapeString* (layout:Pango.PangoLayout; IN s: ARRAY OF CHAR; font: Fonts.Font);
	VAR us: GLib.PString;
	BEGIN
		us :=GLib.g_utf16_to_utf8(s,-1,NIL,NIL,NIL);
		IF us = NIL THEN (*!!!*)
			ShapeUString(layout,'',NIL);
		ELSE
			ShapeUString(layout,us,font);
			GLib.g_free(SYSTEM.VAL(GLib.gpointer, us));
		END;
	END ShapeString;


	PROCEDURE ShapeSString* (layout:Pango.PangoLayout; IN s: ARRAY OF SHORTCHAR; font: Fonts.Font);
	VAR us: GLib.PString;
	BEGIN
		us:=GLib.g_locale_to_utf8(s,-1,NIL,NIL,NIL);
		IF us = NIL THEN (*!!!*)
			ShapeUString(layout,'',NIL);
		ELSE
			ShapeUString(layout,us,font);
			GLib.g_free(SYSTEM.VAL(GLib.gpointer, us));
		END;
	END ShapeSString;

	PROCEDURE (font: Font) StringWidth* (IN s: ARRAY OF CHAR): INTEGER;
	VAR rect: Pango.PangoRectangle;
	BEGIN
		ShapeString(layout,s,font);
		Pango.pango_layout_line_get_pixel_extents(Pango.pango_layout_get_line(layout,0),NIL,rect);
		RETURN (rect.x + rect.width)*pixel;
	END StringWidth;

	PROCEDURE (font: Font) SStringWidth* (IN s: ARRAY OF SHORTCHAR): INTEGER;
	VAR rect: Pango.PangoRectangle;
	BEGIN
		ShapeUString(layout,s,font);
		Pango.pango_layout_line_get_pixel_extents(Pango.pango_layout_get_line(layout,0),NIL,rect);
		RETURN (rect.x + rect.width)*pixel;
	END SStringWidth;


	PROCEDURE (f: Font) IsAlien* (): BOOLEAN;
	BEGIN
		RETURN (f.typeface # Fonts.default) & (f.alias # f.typeface)
	END IsAlien;

	PROCEDURE (f: Font) FINALIZE-;
	BEGIN
		Cleanup(f)
	END FINALIZE;


	PROCEDURE SetupFont(font: Font);
	VAR  metrics:  Pango.PangoFontMetrics;
			language: Pango.PangoLanguage;
	BEGIN
		font.desc:=Pango.pango_font_description_new();
		Pango.pango_font_description_set_family(font.desc,SHORT(font.alias));
		IF Fonts.italic IN font.style THEN
			Pango.pango_font_description_set_style(font.desc, Pango.PANGO_STYLE_ITALIC);
		END;
		Pango.pango_font_description_set_weight(font.desc, font.weight);
		(* Pango.pango_font_description_set_absolute_size (font.desc, (font.size*Pango.PANGO_SCALE) DIV pixel); *)
		Pango.pango_font_description_set_size (font.desc, (font.size*Pango.PANGO_SCALE)  DIV Fonts.point);

		language :=Pango.pango_context_get_language(context);  (* !!! *)
		metrics := Pango.pango_context_get_metrics (context, font.desc, language);
		font.asc := Pango.pango_font_metrics_get_ascent(metrics) * pixel DIV 1024 ;
		font.dsc := Pango.pango_font_metrics_get_descent(metrics) * pixel DIV 1024 ;
		font.w := Pango.pango_font_metrics_get_approximate_char_width(metrics) * pixel DIV 1024;
		Pango.pango_font_metrics_unref(metrics);
	END SetupFont;


	PROCEDURE NewFont (typeface: Fonts.Typeface; size: INTEGER; style: SET; weight: INTEGER): Font;
	VAR font: Font;
	BEGIN
		NEW(font);
		IF typeface = Fonts.default THEN
			font.alias := DefFontName$
		ELSE
			font.alias := typeface$
		END;
		font.Init(typeface, size, style, weight);
		SetupFont(font);
		RETURN font
	END NewFont;


	PROCEDURE (VAR id: Identifier) Identified (): BOOLEAN;
		VAR f: Font;
	BEGIN
		f := id.obj(Font);
		RETURN (f.typeface = id.tface) & (f.size = id.size)
				 & (f.style = id.style) & (f.weight = id.weight)
	END Identified;

(* Directory *)
	PROCEDURE (d: Directory) This (typeface: Fonts.Typeface; size: INTEGER; style: SET; weight: INTEGER): Font;
		VAR f: Font; i: Identifier; p: ANYPTR;
	BEGIN
		ASSERT(size > 0, 20);
		style := style * {Fonts.italic, Fonts.underline, Fonts.strikeout};
		i.tface := typeface$;
		i.size := size; i.style := style; i.weight := weight;
		i.typ := SYSTEM.TYP(Font);
		p := Kernel.ThisFinObj(i);
		IF p # NIL THEN (* found in cache *)
			f := p(Font)
		ELSIF typeface = "" THEN
			f := defFont
		ELSE
			f := NewFont(typeface, size,  style, weight);
		END;
		RETURN f
	END This;

	PROCEDURE (d: Directory) Default (): Fonts.Font;
	BEGIN
		RETURN defFont
	END Default;

	PROCEDURE (d: Directory) TypefaceList* (): Fonts.TypefaceInfo;
	TYPE
		PtrSTR = POINTER TO ARRAY [untagged] OF SHORTCHAR;
		StrArray = POINTER TO ARRAY [untagged] OF PtrSTR;
	CONST maxFonts = 32767; (* taken from the Gtk FontPicker *)
	VAR xFontNames: StrArray;
				numFonts, fmLen, fndLen, i, j, k: INTEGER;
			typefaceInfo, t, q: Fonts.TypefaceInfo;
			familyName, foundryName: ARRAY 256 OF CHAR;
	BEGIN
		typefaceInfo := NIL;
		(* TODO: *)
		RETURN typefaceInfo
	END TypefaceList;


	(** miscellaneous **)

	PROCEDURE (VAR id: Counter) Identified (): BOOLEAN;
	BEGIN
		INC(id.count); RETURN FALSE
	END Identified;

	PROCEDURE NofFonts* (): INTEGER;
		VAR p: ANYPTR; cnt: Counter;
	BEGIN
		cnt.typ := SYSTEM.TYP(Font); cnt.count := 0; p := Kernel.ThisFinObj(cnt);
		RETURN cnt.count
	END NofFonts;

	PROCEDURE InstallDir*;
	BEGIN
		Fonts.SetDir(dir)
	END InstallDir;


	PROCEDURE (VAR id: Traverser) Identified (): BOOLEAN;
		VAR f: Font;
	BEGIN
		f := id.obj(Font);
		IF (f.typeface = Fonts.default) & (f.alias # DefFontName) THEN
			Cleanup(f);
			f.alias := DefFontName$;
			SetupFont(f);
		END;
		RETURN FALSE
	END Identified;

	PROCEDURE SetDefaultFont* (tf: Fonts.Typeface; size: INTEGER);
		VAR t: Traverser; p: ANYPTR;
	BEGIN
		ASSERT(tf # "", 20); ASSERT(size > 0, 21);
		IF tf = Fonts.default THEN tf := DefFontName$ END;
		IF (DefFontName # tf) OR (DefFontSize # size) THEN
			DefFontName := tf$; DefFontSize := size;
			t.typ := SYSTEM.TYP(Font); p := Kernel.ThisFinObj(t);
			defFont := dir.This(Fonts.default, DefFontSize, {}, Fonts.normal);
			(* TODO:*)
		END
	END SetDefaultFont;

	PROCEDURE SetDialogFont* (tf: Fonts.Typeface; size: INTEGER; style: SET; weight: INTEGER);
		VAR i: INTEGER;
	BEGIN
		ASSERT(tf # "", 20); ASSERT(size > 0, 21);
		IF (DlgFontName # tf) OR (DlgFontSize # size)
			OR (DlgFontStyle # style) OR (DlgFontWght # weight)
		THEN
			DlgFontName := tf$; DlgFontSize := size;
			DlgFontStyle := style; DlgFontWght := weight;
			dlgFont := dir.This(DlgFontName, DlgFontSize, DlgFontStyle, DlgFontWght);
			(* TODO: *)
		END
	END SetDialogFont;



	PROCEDURE Init;
		VAR i: INTEGER;
	BEGIN
		pixel := (Fonts.mm * Gdk.gdk_screen_height_mm()) DIV Gdk.gdk_screen_height();
		context :=Gdk.gdk_pango_context_get();
		layout :=Pango.pango_layout_new (context);
		NEW(dir); Fonts.SetDir(dir);
		DefFontName := "Verdana"; DefFontSize := defSize;
		DlgFontName := "Sans"; DlgFontSize := dlgSize;
		DlgFontStyle := {};
		DlgFontWght := Fonts.normal;
		(* TODO: HostRegistry *)
		defFont := dir.This(Fonts.default, DefFontSize, {}, Fonts.normal);
		dlgFont := dir.This(DlgFontName, DlgFontSize, DlgFontStyle, DlgFontWght);
	END Init;

BEGIN
	Init
END HostFonts.

