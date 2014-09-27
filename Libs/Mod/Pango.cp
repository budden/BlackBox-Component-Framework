MODULE LibsPango ["libpango-1.0-0.dll"];
IMPORT SYSTEM, GLib := LibsGlib,GObject:=LibsGObject;
TYPE
  gunichar = GLib.gunichar; 
	gpointer = GLib.gpointer;
	gboolean = GLib.gboolean;
	String = ARRAY [untagged] OF SHORTCHAR;
	PString = POINTER TO String;
  
TYPE
  GSList = GLib.GSList;
  GList = GLib.GList;

CONST 
  PANGO_SCALE* = 1024;

CONST
(*  CSS scale factors (1.2 factor between each size)  *)
  PANGO_SCALE_XX_SMALL = 0.578703703703700000000000000000;
  PANGO_SCALE_X_SMALL = 0.644444444444400000000000000000;
  PANGO_SCALE_SMALL = 0.833333333333300000000000000000;
  PANGO_SCALE_MEDIUM = 1.000000000000000000000000000000;
  PANGO_SCALE_LARGE = 1.200000000000000000000000000000;
  PANGO_SCALE_X_LARGE = 1.439999999999900000000000000000;
  PANGO_SCALE_XX_LARGE = 1.728000000000000000000000000000;

TYPE
  PangoRectangle* = RECORD [untagged]
    x*, y*, width*, height*: INTEGER;
  END;

TYPE  
  PangoColor* = RECORD [untagged]
    red*, green, blue*: SHORTINT;
  END;

TYPE
  PangoMatrix = RECORD [untagged]
    xx*, xy*,
    yx*, yy*,
    x0*, y0* : REAL;
  END;
  

TYPE  PangoLanguage* = PString ;
TYPE  PangoCoverage* = POINTER TO LIMITED RECORD [untagged] END;
TYPE  PangoCoverageLevel = INTEGER;(* \enum PangoCoverageLevel *)
CONST
  PANGO_COVERAGE_NONE = 0;
  PANGO_COVERAGE_FALLBACK = 1;
  PANGO_COVERAGE_APPROXIMATE = 2;
  PANGO_COVERAGE_EXACT = 3;
(* /enum PangoCoverageLevel *)
TYPE  PangoDirection* = INTEGER; (* \enum PangoDirection *)
CONST
  PANGO_DIRECTION_LTR = 0;
  PANGO_DIRECTION_RTL = 1;
  PANGO_DIRECTION_TTB_LTR = 2;
  PANGO_DIRECTION_TTB_RTL = 3;
  PANGO_DIRECTION_WEAK_LTR = 4;
  PANGO_DIRECTION_WEAK_RTL = 5;
  PANGO_DIRECTION_NEUTRAL = 6;
(* /enum PangoDirection *)
(*  PangoFontDescription  *)
TYPE  PangoFontDescription* = POINTER TO LIMITED RECORD [untagged] END;
TYPE  PangoStyle* = INTEGER; (* \enum PangoStyle *)
CONST PANGO_STYLE_NORMAL* = 0; PANGO_STYLE_OBLIQUE* = 1; PANGO_STYLE_ITALIC* = 2;
(* /enum PangoStyle *)
TYPE  PangoVariant = INTEGER; (* \enum PangoVariant *)
CONST  PANGO_VARIANT_NORMAL = 0;  PANGO_VARIANT_SMALL_CAPS = 1;
(* /enum PangoVariant *)
TYPE  PangoWeight = INTEGER; (* \enum PangoWeight *)
CONST  PANGO_WEIGHT_ULTRALIGHT = 200;  PANGO_WEIGHT_LIGHT = 300;  PANGO_WEIGHT_NORMAL = 400;
  PANGO_WEIGHT_SEMIBOLD = 600;  PANGO_WEIGHT_BOLD = 700;  PANGO_WEIGHT_ULTRABOLD = 800; PANGO_WEIGHT_HEAVY = 900;
(* /enum PangoWeight *)
TYPE  PangoStretch = INTEGER; (* \enum PangoStretch *)
CONST
  PANGO_STRETCH_ULTRA_CONDENSED = 0;  PANGO_STRETCH_EXTRA_CONDENSED = 1;  
	PANGO_STRETCH_CONDENSED = 2;  PANGO_STRETCH_SEMI_CONDENSED = 3;
  PANGO_STRETCH_NORMAL = 4;
  PANGO_STRETCH_SEMI_EXPANDED = 5;  PANGO_STRETCH_EXPANDED = 6;
  PANGO_STRETCH_EXTRA_EXPANDED = 7;  PANGO_STRETCH_ULTRA_EXPANDED = 8;
(* /enum PangoStretch *)
TYPE  PangoFontMask = SET; (* PangoFontMask *)
CONST
  PANGO_FONT_MASK_FAMILY = {0};  PANGO_FONT_MASK_STYLE = {1};  PANGO_FONT_MASK_VARIANT = {2};
  PANGO_FONT_MASK_WEIGHT = {3};  PANGO_FONT_MASK_STRETCH = {4};  PANGO_FONT_MASK_SIZE = {5};
(* /PangoFontMask *)

TYPE  PangoFontMetrics* = POINTER TO LIMITED RECORD [untagged] (*..*) END;
TYPE  PangoFontFace = POINTER TO LIMITED RECORD (GObject.GObject) END;
TYPE  PangoFontFamily = POINTER TO LIMITED RECORD (GObject.GObject) END;
TYPE  PangoFont* = POINTER TO LIMITED RECORD (GObject.GObject) END;
			PangoFontset = POINTER TO LIMITED RECORD (GObject.GObject) END;
			PangoFontsetForeachFunc = PROCEDURE (fontset:PangoFontset; font:PangoFont; data:gpointer):gboolean;
TYPE  PangoFontMap* = POINTER TO LIMITED RECORD (GObject.GObject) END;
  		PangoContext* = POINTER TO LIMITED RECORD [untagged] END;
	
(*  Attributes  *)
TYPE  PangoAttrType* = INTEGER;	(* \enum PangoAttrType *)
CONST
  PANGO_ATTR_INVALID = 0;   (*  invalid attribute type  *)
  PANGO_ATTR_LANGUAGE = 1;   (*  PangoAttrLanguage  *)
  PANGO_ATTR_FAMILY = 2;   (*  PangoAttrString  *)
  PANGO_ATTR_STYLE = 3;   (*  PangoAttrInt  *)
  PANGO_ATTR_WEIGHT = 4;   (*  PangoAttrInt  *)
  PANGO_ATTR_VARIANT = 5;   (*  PangoAttrInt  *)
  PANGO_ATTR_STRETCH = 6;   (*  PangoAttrInt  *)
  PANGO_ATTR_SIZE = 7;   (*  PangoAttrSize  *)
  PANGO_ATTR_FONT_DESC = 8;   (*  PangoAttrFontDesc  *)
  PANGO_ATTR_FOREGROUND = 9;   (*  PangoAttrColor  *)
  PANGO_ATTR_BACKGROUND = 10;   (*  PangoAttrColor  *)
  PANGO_ATTR_UNDERLINE = 11;   (*  PangoAttrInt  *)
  PANGO_ATTR_STRIKETHROUGH = 12;   (*  PangoAttrInt *)
  PANGO_ATTR_RISE = 13;   (*  PangoAttrInt  *)
  PANGO_ATTR_SHAPE = 14;  (*  PangoAttrShape  *)
  PANGO_ATTR_SCALE = 15;  (*  PangoAttrFloat  *)
  PANGO_ATTR_FALLBACK = 16;   (* PangoAttrInt  *)
  PANGO_ATTR_LETTER_SPACING = 17;  (* PangoAttrInt  *)
  PANGO_ATTR_UNDERLINE_COLOR = 18; (* PangoAttrColor *)
  PANGO_ATTR_STRIKETHROUGH_COLOR = 19;  (* PangoAttrColor  *)
  PANGO_ATTR_ABSOLUTE_SIZE = 20;        (* PangoAttrSize  *)
(* /enum PangoAttrType *)

TYPE  PangoUnderline* = INTEGER; (* \enum PangoUnderline *)
CONST
  PANGO_UNDERLINE_NONE = 0;
  PANGO_UNDERLINE_SINGLE = 1;
  PANGO_UNDERLINE_DOUBLE = 2;
  PANGO_UNDERLINE_LOW = 3;
  PANGO_UNDERLINE_ERROR = 4;
(* /enum PangoUnderline *)
TYPE  PangoAttrClass = POINTER TO RECORD [untagged] 
        type   : PangoAttrType;
        copy   : PROCEDURE (attr : PangoAttribute): PangoAttribute;
        destroy: PROCEDURE (attr : PangoAttribute);
        equal  : PROCEDURE (attr1, attr2 : PangoAttribute): gboolean;
      END;

      PangoAttribute* = POINTER TO ABSTRACT RECORD [untagged] 
        klass      : PangoAttrClass;
        start_index*: INTEGER;   (*  in bytes  *)
        end_index*  : INTEGER;   (*  in bytes. The character at this index is not included  *)
      END;

      PangoAttrString = POINTER TO LIMITED RECORD  (PangoAttribute)
        value: PString;
      END;

      PangoAttrInt = POINTER TO LIMITED RECORD (PangoAttribute)
        value: INTEGER;
      END;

      PangoAttrLanguage = POINTER TO LIMITED RECORD (PangoAttribute)
        value: PangoLanguage;
      END;

      PangoAttrFontDesc = POINTER TO LIMITED RECORD (PangoAttribute)
        value: PangoFontDescription;
      END;

      PangoAttrSize = POINTER TO LIMITED RECORD (PangoAttribute)
        size    : INTEGER;
        absolute: SET;  (* bit field. absolute:1. *)
      END;

      PangoAttrFloat = POINTER TO LIMITED RECORD (PangoAttribute)
        attr : PangoAttribute;
        value: REAL;
      END;

      PangoAttrColor = POINTER TO LIMITED RECORD (PangoAttribute)
        color: PangoColor;
      END;

      PangoAttrDataCopyFunc = PROCEDURE (data: gpointer): gpointer;
      PangoAttrShape = POINTER TO LIMITED RECORD (PangoAttribute)
        ink_rect    : PangoRectangle;
        logical_rect: PangoRectangle;
        data        : gpointer;
        copy_func   : PangoAttrDataCopyFunc;
        destroy_func: GLib.GDestroyNotify;
      END;


      PangoAttrList* = POINTER TO LIMITED RECORD [untagged] END;
      PangoAttrIterator = POINTER TO LIMITED RECORD [untagged] END;
      PangoAttrFilterFunc = PROCEDURE (attribute: PangoAttribute; data:gpointer): gboolean;

TYPE  PangoEngine = POINTER TO LIMITED RECORD (GObject.GObject) END;
      PangoEngineShape = POINTER TO LIMITED RECORD (PangoEngine) END;
			PangoEngineLang = POINTER TO LIMITED RECORD (PangoEngine) END;

TYPE  PangoAnalysis = RECORD [untagged]
        shape_engine: PangoEngine;
        lang_engine : PangoEngine;
        font        : PangoFont;
        level       : BYTE;
        language    : PangoLanguage ;
        extra_attrs : GSList;
      END;

TYPE  PangoItem = POINTER TO LIMITED RECORD [untagged]
        offset   : INTEGER;
        length   : INTEGER;
        num_chars: INTEGER;
        analysis : PangoAnalysis;
      END;

TYPE  PangoGlyphItem = POINTER TO LIMITED RECORD (PangoItem) 
	        glyphs: PangoGlyphString;
    	  END;

TYPE  PangoLogAttr = RECORD [untagged] (*  Logical attributes of a character  *)
        is_line_break: SET;
        (* bit field. 
							is_line_break:1, is_mandatory_break:1, is_char_break:1, 
							is_white:1, is_cursor_position:1, is_word_start:1, 
							is_word_end:1, is_sentence_boundary:1, is_sentence_start:1, 
							is_sentence_end:1, backspace_deletes_character:1. *)
      END;
			PangoLogAttrs = ARRAY [untagged] OF PangoLogAttr; 
			
			(*  Glyph storage *)
TYPE  PangoGlyph* = INTEGER;
TYPE  PangoGlyphUnit = INTEGER; (* in 1/PANGO_SCALE of a device unit *)

      PangoGlyphGeometry = RECORD [untagged]
        width   : PangoGlyphUnit;
        x_offset: PangoGlyphUnit;
        y_offset: PangoGlyphUnit;
      END;

      PangoGlyphVisAttr = RECORD [untagged]
        is_cluster_start: SET ;  (* bit field. is_cluster_start:1. *)
      END;

      PangoGlyphInfo = RECORD [untagged]
        glyph   : PangoGlyph;
        geometry: PangoGlyphGeometry;
        attr    : PangoGlyphVisAttr;
      END;
      
      PangoGlyphString * = POINTER TO LIMITED RECORD [untagged]
        num_glyphs  : INTEGER;
        glyphs      : POINTER TO ARRAY [untagged] OF PangoGlyphInfo;
        log_clusters: POINTER TO ARRAY [untagged] OF INTEGER;
        space       : INTEGER;
      END;
     

TYPE  PangoTabArray = POINTER TO LIMITED RECORD [untagged] END;
TYPE  PangoTabAlign = INTEGER;(* \enum PangoTabAlign *)
CONST
  PANGO_TAB_LEFT = 0;
 (* These are not supported now, but may be in the future:PANGO_TAB_RIGHT,PANGO_TAB_CENTER,PANGO_TAB_NUMERIC *)
(* /enum PangoTabAlign *)

TYPE  PangoAlignment = INTEGER; (* \enum PangoAlignment *)
CONST
  PANGO_ALIGN_LEFT = 0;
  PANGO_ALIGN_CENTER = 1;
  PANGO_ALIGN_RIGHT = 2;(* /enum PangoAlignment *)
TYPE  PangoWrapMode = INTEGER;(* \enum PangoWrapMode *)
CONST
  PANGO_WRAP_WORD = 0;
  PANGO_WRAP_CHAR = 1;
  PANGO_WRAP_WORD_CHAR = 2;(* /enum PangoWrapMode *)
TYPE  PangoEllipsizeMode = INTEGER; (* \enum PangoEllipsizeMode *)
CONST
  PANGO_ELLIPSIZE_NONE = 0;
  PANGO_ELLIPSIZE_START = 1;
  PANGO_ELLIPSIZE_MIDDLE = 2;
  PANGO_ELLIPSIZE_END = 3;(* /enum PangoEllipsizeMode *)

TYPE  PangoLayout* = POINTER TO LIMITED RECORD (GObject.GObject) END;
      PangoLayoutLine* = POINTER TO LIMITED RECORD [untagged] 
         layout            : PangoLayout;
         start_index       : INTEGER;                  (*  start of line as byte index into layout->text  *)
         length            : INTEGER;                  (*  length of line in bytes  *)
         runs              : GSList;
         is_paragraph_start: SET;   (* bit field. is_paragraph_start:1, resolved_dir:3. *)
      END;

TYPE  PangoLayoutIter = POINTER TO LIMITED RECORD [untagged] END;

(*  PangoScript *)
TYPE  PangoScript = INTEGER; (* \enum PangoScript *)CONST
(*  ISO 15924 code  *)
  PANGO_SCRIPT_INVALID_CODE = -1;
  PANGO_SCRIPT_COMMON = 0;   (*  Zyyy  *)
  PANGO_SCRIPT_INHERITED = 1;   (*  Qaai  *)
  PANGO_SCRIPT_ARABIC = 2;   (*  Arab  *)
  PANGO_SCRIPT_ARMENIAN = 3;   (*  Armn  *)
  PANGO_SCRIPT_BENGALI = 4;   (*  Beng  *)
  PANGO_SCRIPT_BOPOMOFO = 5;   (*  Bopo  *)
  PANGO_SCRIPT_CHEROKEE = 6;   (*  Cher  *)
  PANGO_SCRIPT_COPTIC = 7;   (*  Qaac  *)
  PANGO_SCRIPT_CYRILLIC = 8;   (*  Cyrl (Cyrs)  *)
  PANGO_SCRIPT_DESERET = 9;   (*  Dsrt  *)
  PANGO_SCRIPT_DEVANAGARI = 10;   (*  Deva  *)
  PANGO_SCRIPT_ETHIOPIC = 11;   (*  Ethi  *)
  PANGO_SCRIPT_GEORGIAN = 12;   (*  Geor (Geon, Geoa)  *)
  PANGO_SCRIPT_GOTHIC = 13;   (*  Goth  *)
  PANGO_SCRIPT_GREEK = 14;   (*  Grek  *)
  PANGO_SCRIPT_GUJARATI = 15;   (*  Gujr  *)
  PANGO_SCRIPT_GURMUKHI = 16;   (*  Guru  *)
  PANGO_SCRIPT_HAN = 17;   (*  Hani  *)
  PANGO_SCRIPT_HANGUL = 18;   (*  Hang  *)
  PANGO_SCRIPT_HEBREW = 19;   (*  Hebr  *)
  PANGO_SCRIPT_HIRAGANA = 20;   (*  Hira  *)
  PANGO_SCRIPT_KANNADA = 21;   (*  Knda  *)
  PANGO_SCRIPT_KATAKANA = 22;   (*  Kana  *)
  PANGO_SCRIPT_KHMER = 23;   (*  Khmr  *)
  PANGO_SCRIPT_LAO = 24;   (*  Laoo  *)
  PANGO_SCRIPT_LATIN = 25;   (*  Latn (Latf, Latg)  *)
  PANGO_SCRIPT_MALAYALAM = 26;   (*  Mlym  *)
  PANGO_SCRIPT_MONGOLIAN = 27;   (*  Mong  *)
  PANGO_SCRIPT_MYANMAR = 28;   (*  Mymr  *)
  PANGO_SCRIPT_OGHAM = 29;   (*  Ogam  *)
  PANGO_SCRIPT_OLD_ITALIC = 30;   (*  Ital  *)
  PANGO_SCRIPT_ORIYA = 31;   (*  Orya  *)
  PANGO_SCRIPT_RUNIC = 32;   (*  Runr  *)
  PANGO_SCRIPT_SINHALA = 33;   (*  Sinh  *)
  PANGO_SCRIPT_SYRIAC = 34;   (*  Syrc (Syrj, Syrn, Syre)  *)
  PANGO_SCRIPT_TAMIL = 35;   (*  Taml  *)
  PANGO_SCRIPT_TELUGU = 36;   (*  Telu  *)
  PANGO_SCRIPT_THAANA = 37;   (*  Thaa  *)
  PANGO_SCRIPT_THAI = 38;   (*  Thai  *)
  PANGO_SCRIPT_TIBETAN = 39;   (*  Tibt  *)
  PANGO_SCRIPT_CANADIAN_ABORIGINAL = 40;   (*  Cans  *)
  PANGO_SCRIPT_YI = 41;   (*  Yiii  *)
  PANGO_SCRIPT_TAGALOG = 42;   (*  Tglg  *)
  PANGO_SCRIPT_HANUNOO = 43;   (*  Hano  *)
  PANGO_SCRIPT_BUHID = 44;   (*  Buhd  *)
  PANGO_SCRIPT_TAGBANWA = 45;   (*  Tagb  *)
(*  Unicode-4.0 additions  *)
  PANGO_SCRIPT_BRAILLE = 46;   (*  Brai  *)
  PANGO_SCRIPT_CYPRIOT = 47;   (*  Cprt  *)
  PANGO_SCRIPT_LIMBU = 48;   (*  Limb  *)
  PANGO_SCRIPT_OSMANYA = 49;   (*  Osma  *)
  PANGO_SCRIPT_SHAVIAN = 50;   (*  Shaw  *)
  PANGO_SCRIPT_LINEAR_B = 51;   (*  Linb  *)
  PANGO_SCRIPT_TAI_LE = 52;   (*  Tale  *)
  PANGO_SCRIPT_UGARITIC = 53;   (*  Ugar  *)
(*  Unicode-4.1 additions  *)
  PANGO_SCRIPT_NEW_TAI_LUE = 54;   (*  Talu  *)
  PANGO_SCRIPT_BUGINESE = 55;   (*  Bugi  *)
  PANGO_SCRIPT_GLAGOLITIC = 56;   (*  Glag  *)
  PANGO_SCRIPT_TIFINAGH = 57;   (*  Tfng  *)
  PANGO_SCRIPT_SYLOTI_NAGRI = 58;   (*  Sylo  *)
  PANGO_SCRIPT_OLD_PERSIAN = 59;   (*  Xpeo  *)
  PANGO_SCRIPT_KHAROSHTHI = 60;   (*  Khar  *)
(* /enum PangoScript *)
TYPE  PangoScriptIter = POINTER TO LIMITED RECORD [untagged] END;

(*   *)
	PROCEDURE [ccall] pango_language_from_string* (IN language: String): PangoLanguage;
	PROCEDURE [ccall] pango_language_matches (language: PangoLanguage; range_list: PString): BOOLEAN;
(*   *)
	PROCEDURE [ccall] pango_coverage_new (): PangoCoverage;
	PROCEDURE [ccall] pango_coverage_ref (coverage: PangoCoverage): PangoCoverage;
	PROCEDURE [ccall] pango_coverage_unref (coverage: PangoCoverage);
	PROCEDURE [ccall] pango_coverage_copy (coverage: PangoCoverage): PangoCoverage;
	PROCEDURE [ccall] pango_coverage_get (coverage: PangoCoverage; index: INTEGER): PangoCoverageLevel;
	PROCEDURE [ccall] pango_coverage_set (coverage: PangoCoverage; index: INTEGER; level: PangoCoverageLevel);
	PROCEDURE [ccall] pango_coverage_max (coverage: PangoCoverage; other: PangoCoverage);
	PROCEDURE [ccall] pango_coverage_to_bytes (coverage: PangoCoverage; OUT bytes: POINTER TO ARRAY [untagged] OF BYTE; OUT n_bytes: INTEGER);
	PROCEDURE [ccall] pango_coverage_from_bytes (IN bytes: ARRAY [untagged] OF BYTE; n_bytes: INTEGER): PangoCoverage;

	PROCEDURE [ccall] pango_get_mirror_char (ch: gunichar; OUT mirrored_ch: gunichar): BOOLEAN;
	PROCEDURE [ccall] pango_unichar_direction (ch: gunichar): PangoDirection;
	PROCEDURE [ccall] pango_find_base_dir (text: PString; length: INTEGER): PangoDirection;

(*  PangoFontDescription  *)
	PROCEDURE [ccall] pango_font_description_new* (): PangoFontDescription;
	PROCEDURE [ccall] pango_font_description_copy (desc: PangoFontDescription): PangoFontDescription;
	PROCEDURE [ccall] pango_font_description_copy_static (desc: PangoFontDescription): PangoFontDescription;
	PROCEDURE [ccall] pango_font_description_from_string*(IN str: String): PangoFontDescription;
	PROCEDURE [ccall] pango_font_description_free* (desc: PangoFontDescription);
	PROCEDURE [ccall] pango_font_descriptions_free (IN descs: ARRAY [untagged] OF PangoFontDescription; n_descs: INTEGER); 

	PROCEDURE [ccall] pango_font_description_hash (desc: PangoFontDescription): INTEGER;
	PROCEDURE [ccall] pango_font_description_equal (desc1, desc2: PangoFontDescription): BOOLEAN;
	PROCEDURE [ccall] pango_font_description_get_family* (desc: PangoFontDescription): PString;
	PROCEDURE [ccall] pango_font_description_get_size* (desc: PangoFontDescription): INTEGER;
	PROCEDURE [ccall] pango_font_description_get_size_is_absolute*(desc: PangoFontDescription): BOOLEAN;
	PROCEDURE [ccall] pango_font_description_get_weight* (desc: PangoFontDescription): PangoWeight;
	PROCEDURE [ccall] pango_font_description_get_style* (desc: PangoFontDescription): PangoStyle;
	PROCEDURE [ccall] pango_font_description_get_variant (desc: PangoFontDescription): PangoVariant;
	PROCEDURE [ccall] pango_font_description_get_stretch (desc: PangoFontDescription): PangoStretch;
	PROCEDURE [ccall] pango_font_description_get_set_fields (desc: PangoFontDescription): PangoFontMask;
	
	PROCEDURE [ccall] pango_font_description_set_family* (desc: PangoFontDescription; IN family: String);
	PROCEDURE [ccall] pango_font_description_set_family_static (desc: PangoFontDescription; family: PString);
	PROCEDURE [ccall] pango_font_description_set_style*(desc: PangoFontDescription; style: PangoStyle);
	PROCEDURE [ccall] pango_font_description_set_variant*(desc: PangoFontDescription; variant: PangoVariant);
	PROCEDURE [ccall] pango_font_description_set_weight*(desc: PangoFontDescription; weight: PangoWeight);
	PROCEDURE [ccall] pango_font_description_set_stretch*(desc: PangoFontDescription; stretch: PangoStretch);
	PROCEDURE [ccall] pango_font_description_set_size*(desc: PangoFontDescription; size: INTEGER);
	PROCEDURE [ccall] pango_font_description_set_absolute_size*(desc: PangoFontDescription; size: REAL);
	
	PROCEDURE [ccall] pango_font_description_unset_fields (desc: PangoFontDescription; to_unset: PangoFontMask);
	PROCEDURE [ccall] pango_font_description_merge (desc,desc_to_merge: PangoFontDescription; replace_existing: gboolean);
	PROCEDURE [ccall] pango_font_description_merge_static (desc, desc_to_merge: PangoFontDescription; replace_existing: gboolean);
	PROCEDURE [ccall] pango_font_description_better_match (desc, old_match, new_match: PangoFontDescription): BOOLEAN;
	PROCEDURE [ccall] pango_font_description_to_string (desc: PangoFontDescription): PString;
	PROCEDURE [ccall] pango_font_description_to_filename (desc: PangoFontDescription): PString;

(* PangoFontMetrics  *)

	PROCEDURE [ccall] pango_font_metrics_ref (metrics: PangoFontMetrics): PangoFontMetrics;
	PROCEDURE [ccall] pango_font_metrics_unref *(metrics: PangoFontMetrics);
	PROCEDURE [ccall] pango_font_metrics_get_ascent* (metrics: PangoFontMetrics): INTEGER;
	PROCEDURE [ccall] pango_font_metrics_get_descent* (metrics: PangoFontMetrics): INTEGER;
	PROCEDURE [ccall] pango_font_metrics_get_approximate_char_width* (metrics: PangoFontMetrics): INTEGER;
	PROCEDURE [ccall] pango_font_metrics_get_approximate_digit_width (metrics: PangoFontMetrics): INTEGER;
	PROCEDURE [ccall] pango_font_metrics_get_underline_position (metrics: PangoFontMetrics): INTEGER;
	PROCEDURE [ccall] pango_font_metrics_get_underline_thickness (metrics: PangoFontMetrics): INTEGER;
	PROCEDURE [ccall] pango_font_metrics_get_strikethrough_position (metrics: PangoFontMetrics): INTEGER;
	PROCEDURE [ccall] pango_font_metrics_get_strikethrough_thickness (metrics: PangoFontMetrics): INTEGER;

(* PangoFontFace  *)
	PROCEDURE [ccall] pango_font_face_describe (face: PangoFontFace): PangoFontDescription;
	PROCEDURE [ccall] pango_font_face_get_face_name (face:PangoFontFace ): PString;
	PROCEDURE [ccall] pango_font_face_list_sizes (face: PangoFontFace; OUT sizes: POINTER TO ARRAY [untagged] OF INTEGER; OUT n_sizes: INTEGER);

(*  PangoFontFamily  *)

	PROCEDURE [ccall] pango_font_family_list_faces (family: PangoFontFamily; OUT faces: POINTER TO ARRAY [untagged] OF PangoFontFace; OUT n_faces: INTEGER);
	PROCEDURE [ccall] pango_font_family_get_name (family: PangoFontFamily): PString;
	PROCEDURE [ccall] pango_font_family_is_monospace (family: PangoFontFamily): BOOLEAN;

(*  PangoFont  *)
	
PROCEDURE [ccall] pango_font_describe* (font: PangoFont): PangoFontDescription;
PROCEDURE [ccall] pango_font_get_font_map (font: PangoFont): PangoFontMap;
PROCEDURE [ccall] pango_font_get_coverage (font: PangoFont; language: PangoLanguage): PangoCoverage;
PROCEDURE [ccall] pango_font_get_metrics (font: PangoFont; language: PangoLanguage):PangoFontMetrics ;
PROCEDURE [ccall] pango_font_get_glyph_extents (font: PangoFont; glyph: PangoGlyph; OUT ink_rect, logical_rect: PangoRectangle);

	PROCEDURE [ccall] pango_font_map_load_font (fontmap: PangoFontMap; context: PangoContext; desc: PangoFontDescription):PangoFont ;
	PROCEDURE [ccall] pango_font_map_load_fontset (fontmap: PangoFontMap; context: PangoContext; desc: PangoFontDescription; language: PangoLanguage):PangoFontset;
	PROCEDURE [ccall] pango_font_map_list_families (fontmap: PangoFontMap; OUT families: POINTER TO ARRAY [untagged] OF  PangoFontFamily ; OUT n_families: INTEGER);


(*  PangoFontset  *)
	
	PROCEDURE [ccall] pango_fontset_get_font (fontset: PangoFontset ; wc: gunichar): PangoFont;
	PROCEDURE [ccall] pango_fontset_get_metrics (fontset: PangoFontset): PangoFontMetrics;
	PROCEDURE [ccall] pango_fontset_foreach (fontset:PangoFontset; func: PangoFontsetForeachFunc; data: gpointer);

(*  PangoColor  *)
	PROCEDURE [ccall] pango_color_copy (IN src: PangoColor): POINTER TO PangoColor;
	PROCEDURE [ccall] pango_color_free (color: POINTER TO PangoColor);
	PROCEDURE [ccall] pango_color_parse (VAR color: PangoColor; IN spec: String): BOOLEAN;


	PROCEDURE [ccall] pango_matrix_copy (IN matrix: PangoMatrix): POINTER TO PangoMatrix;
	PROCEDURE [ccall] pango_matrix_free (matrix: POINTER TO PangoMatrix);
	PROCEDURE [ccall] pango_matrix_translate (VAR matrix: PangoMatrix; tx, ty: REAL);
	PROCEDURE [ccall] pango_matrix_scale (IN matrix: PangoMatrix; scale_x,scale_y: REAL);
	PROCEDURE [ccall] pango_matrix_rotate (VAR matrix: PangoMatrix; degrees: REAL);
	PROCEDURE [ccall] pango_matrix_concat (VAR matrix: PangoMatrix; IN new_matrix: PangoMatrix);
	PROCEDURE [ccall] pango_matrix_get_font_scale_factor (IN matrix: PangoMatrix): REAL;

PROCEDURE [ccall] pango_context_get_language* (context: PangoContext): PangoLanguage;
PROCEDURE [ccall] pango_context_set_language (context: PangoContext; language: PangoLanguage);
PROCEDURE [ccall] pango_context_set_font_description (context: PangoContext; desc: PangoFontDescription);
PROCEDURE [ccall] pango_context_get_font_description (context: PangoContext): PangoFontDescription;
PROCEDURE [ccall] pango_context_set_matrix (context: PangoContext; IN matrix: PangoMatrix);
PROCEDURE [ccall] pango_context_get_matrix (context: PangoContext): POINTER TO PangoMatrix;
PROCEDURE [ccall] pango_context_set_base_dir (context: PangoContext; direction: PangoDirection);
PROCEDURE [ccall] pango_context_get_base_dir (context: PangoContext): PangoDirection;

PROCEDURE [ccall] pango_context_get_font_map (context: PangoContext): PangoFontMap;
PROCEDURE [ccall] pango_context_list_families (context: PangoContext; OUT families: POINTER TO ARRAY [untagged] OF  PangoFontFamily; OUT n_families: INTEGER);
PROCEDURE [ccall] pango_context_load_font (context: PangoContext; desc: PangoFontDescription): PangoFont;
PROCEDURE [ccall] pango_context_load_fontset (context: PangoContext; desc: PangoFontDescription; language:PangoLanguage ): PangoFontset;
PROCEDURE [ccall] pango_context_get_metrics* (context: PangoContext; desc: PangoFontDescription; language:PangoLanguage ): PangoFontMetrics;


(*  Attributes  *)
PROCEDURE [ccall] pango_attr_type_register (name: PString): PangoAttrType;

PROCEDURE [ccall] pango_attribute_copy (attr: PangoAttribute): PangoAttribute;
PROCEDURE [ccall] pango_attribute_destroy* (attr: PangoAttribute);
PROCEDURE [ccall] pango_attribute_equal (attr1: PangoAttribute; attr2: PangoAttribute): BOOLEAN;
PROCEDURE [ccall] pango_attr_language_new (language:PangoLanguage ): PangoAttribute;
PROCEDURE [ccall] pango_attr_family_new (family: PString): PangoAttribute;
PROCEDURE [ccall] pango_attr_foreground_new (red, green, blue: SHORTINT): PangoAttribute;
PROCEDURE [ccall] pango_attr_background_new (red, green, blue: SHORTINT): PangoAttribute;
PROCEDURE [ccall] pango_attr_underline_color_new (red, green, blue: SHORTINT): PangoAttribute;
PROCEDURE [ccall] pango_attr_size_new (size: INTEGER): PangoAttribute;
PROCEDURE [ccall] pango_attr_size_new_absolute (size: INTEGER): PangoAttribute;
PROCEDURE [ccall] pango_attr_style_new (style: PangoStyle): PangoAttribute;
PROCEDURE [ccall] pango_attr_weight_new (weight: PangoWeight): PangoAttribute;
PROCEDURE [ccall] pango_attr_variant_new (variant: PangoVariant): PangoAttribute;
PROCEDURE [ccall] pango_attr_stretch_new (stretch: PangoStretch): PangoAttribute;
PROCEDURE [ccall] pango_attr_font_desc_new (desc:PangoFontDescription): PangoAttribute;
PROCEDURE [ccall] pango_attr_underline_new*(underline: PangoUnderline): PangoAttribute;
PROCEDURE [ccall] pango_attr_strikethrough_new (strikethrough: gboolean): PangoAttribute;
PROCEDURE [ccall] pango_attr_strikethrough_color_new (red, green, blue: SHORTINT): PangoAttribute;
PROCEDURE [ccall] pango_attr_rise_new (rise: INTEGER): PangoAttribute;
PROCEDURE [ccall] pango_attr_scale_new (scale_factor: REAL): PangoAttribute;
PROCEDURE [ccall] pango_attr_fallback_new (enable_fallback: gboolean): PangoAttribute;
PROCEDURE [ccall] pango_attr_letter_spacing_new (letter_spacing: INTEGER): PangoAttribute;
PROCEDURE [ccall] pango_attr_shape_new (IN ink_rect, logical_rect: PangoRectangle): PangoAttribute;
PROCEDURE [ccall] pango_attr_shape_new_with_data (IN ink_rect, logical_rect: PangoRectangle; data: gpointer; copy_func: PangoAttrDataCopyFunc; destroy_func: GLib.GDestroyNotify): PangoAttribute;

PROCEDURE [ccall] pango_attr_list_new* ():PangoAttrList ;
PROCEDURE [ccall] pango_attr_list_ref (list: PangoAttrList): PangoAttrList;
PROCEDURE [ccall] pango_attr_list_unref* (list: PangoAttrList);
PROCEDURE [ccall] pango_attr_list_copy (list: PangoAttrList): PangoAttrList;
PROCEDURE [ccall] pango_attr_list_insert* (list: PangoAttrList; attr: PangoAttribute);
PROCEDURE [ccall] pango_attr_list_insert_before (list: PangoAttrList; attr: PangoAttribute);
PROCEDURE [ccall] pango_attr_list_change* (list: PangoAttrList; attr: PangoAttribute);
PROCEDURE [ccall] pango_attr_list_splice (list: PangoAttrList; other: PangoAttrList; pos: INTEGER; len: INTEGER);
PROCEDURE [ccall] pango_attr_list_filter (list: PangoAttrList; func: PangoAttrFilterFunc; data: gpointer): PangoAttrList;
(*    *)
	PROCEDURE [ccall] pango_attr_list_get_iterator (list: PangoAttrList): PangoAttrIterator;
	PROCEDURE [ccall] pango_attr_iterator_range (iterator: PangoAttrIterator; OUT start, end: INTEGER);
	PROCEDURE [ccall] pango_attr_iterator_next (iterator: PangoAttrIterator): BOOLEAN;
	PROCEDURE [ccall] pango_attr_iterator_copy (iterator: PangoAttrIterator): PangoAttrIterator;
	PROCEDURE [ccall] pango_attr_iterator_destroy (iterator: PangoAttrIterator);
	PROCEDURE [ccall] pango_attr_iterator_get (iterator: PangoAttrIterator; type: PangoAttrType): PangoAttribute;
	PROCEDURE [ccall] pango_attr_iterator_get_font (iterator: PangoAttrIterator; desc:PangoFontDescription ; language: PangoLanguage; OUT extra_attrs: GSList);
	PROCEDURE [ccall] pango_attr_iterator_get_attrs (iterator: PangoAttrIterator): GSList;
	(*
	PROCEDURE [ccall] pango_parse_markup (markup_text: PString; length: INTEGER; accel_marker: gunichar; attr_list: PangoAttrList; OUT text: PString; OUT accel_char: gunichar; error: GError): BOOLEAN;
*)


	PROCEDURE [ccall] pango_break (text: PString; length: INTEGER; IN analysis: PangoAnalysis; OUT attrs: PangoLogAttrs; attrs_len: INTEGER);
	PROCEDURE [ccall] pango_find_paragraph_boundary (text: PString; length: INTEGER; OUT paragraph_delimiter_index,next_paragraph_start: INTEGER);
	PROCEDURE [ccall] pango_get_log_attrs (text: PString; length: INTEGER; level: INTEGER; language:PangoLanguage ; OUT log_attrs: PangoLogAttrs; attrs_len: INTEGER);
	PROCEDURE [ccall] pango_itemize (context: PangoContext; text: PString; start_index: INTEGER; length: INTEGER; attrs:PangoAttrList ; cached_iter:PangoAttrIterator ): GList;
	PROCEDURE [ccall] pango_itemize_with_base_dir (context: PangoContext; base_dir: PangoDirection; text: PString; start_index: INTEGER; length: INTEGER; attrs: PangoAttrList; cached_iter:PangoAttrIterator):GList; (* of PangoItem *)

(*  Glyph storage *)
	PROCEDURE [ccall] pango_glyph_string_new* (): PangoGlyphString;
	PROCEDURE [ccall] pango_glyph_string_free* (string: PangoGlyphString);
	PROCEDURE [ccall] pango_glyph_string_set_size (string: PangoGlyphString; new_len: INTEGER);
	PROCEDURE [ccall] pango_glyph_string_copy (string: PangoGlyphString): PangoGlyphString;
	PROCEDURE [ccall] pango_glyph_string_extents (glyphs: PangoGlyphString; font:PangoFont ; IN ink_rect, logical_rect: PangoRectangle);
	PROCEDURE [ccall] pango_glyph_string_extents_range (glyphs: PangoGlyphString; start, end: INTEGER; font:PangoFont; IN ink_rect, logical_rect: PangoRectangle);
	PROCEDURE [ccall] pango_glyph_string_get_logical_widths (glyphs: PangoGlyphString; text: PString; length: INTEGER; embedding_level: INTEGER; OUT logical_widths: INTEGER);
	PROCEDURE [ccall] pango_glyph_string_index_to_x (glyphs: PangoGlyphString; text: PString; length: INTEGER; analysis: PangoAnalysis; index: INTEGER; trailing: gboolean; OUT x_pos: INTEGER);
	PROCEDURE [ccall] pango_glyph_string_x_to_index (glyphs: PangoGlyphString; text: PString; length: INTEGER; analysis: PangoAnalysis; x_pos: INTEGER; OUT index, trailing: INTEGER);
	(*  Turn a string of characters into a string of glyphs  *) 
	PROCEDURE [ccall] pango_shape (IN text: String; length: INTEGER; IN analysis: PangoAnalysis; glyphs: PangoGlyphString);
	
	PROCEDURE [ccall] pango_reorder_items (logical_items:GList (* of PangoItem*)): GList;

	PROCEDURE [ccall] pango_item_new (): PangoItem;
	PROCEDURE [ccall] pango_item_copy (item: PangoItem): PangoItem;
	PROCEDURE [ccall] pango_item_free (item: PangoItem);
	PROCEDURE [ccall] pango_item_split (orig: PangoItem; split_index, split_offset: INTEGER): PangoItem;

PROCEDURE [ccall] pango_glyph_item_split (orig: PangoGlyphItem; text: PString; split_index: INTEGER): PangoGlyphItem;
PROCEDURE [ccall] pango_glyph_item_free (glyph_item: PangoGlyphItem);
PROCEDURE [ccall] pango_glyph_item_apply_attrs (glyph_item: PangoGlyphItem; text: PString; list: PangoAttrList): GSList;
PROCEDURE [ccall] pango_glyph_item_letter_space (glyph_item: PangoGlyphItem; text: PString; IN log_attrs: PangoLogAttrs; letter_spacing: INTEGER);




PROCEDURE [ccall] pango_tab_array_new (initial_size: INTEGER; positions_in_pixels: gboolean):PangoTabArray ;
(* PROCEDURE [ccall] pango_tab_array_new_with_positions (size: INTEGER; positions_in_pixels: gboolean; first_alignment: PangoTabAlign; first_position: INTEGER; ...): ;
*)
PROCEDURE [ccall] pango_tab_array_copy (src: PangoTabArray): PangoTabArray;
PROCEDURE [ccall] pango_tab_array_free (tab_array: PangoTabArray);
PROCEDURE [ccall] pango_tab_array_get_size (tab_array: PangoTabArray): INTEGER;
PROCEDURE [ccall] pango_tab_array_resize (tab_array: PangoTabArray; new_size: INTEGER);
PROCEDURE [ccall] pango_tab_array_set_tab (tab_array: PangoTabArray; tab_index: INTEGER; alignment: PangoTabAlign; location: INTEGER);
PROCEDURE [ccall] pango_tab_array_get_tab (tab_array: PangoTabArray; tab_index: INTEGER; alignment: PangoTabAlign; OUT location: INTEGER);
PROCEDURE [ccall] pango_tab_array_get_tabs (tab_array: PangoTabArray; OUT alignments: ARRAY [untagged] OF PangoTabAlign; OUT locations: POINTER TO ARRAY [untagged] OF INTEGER);
PROCEDURE [ccall] pango_tab_array_get_positions_in_pixels (tab_array: PangoTabArray): BOOLEAN;

(* PangoLayout *)
PROCEDURE [ccall] pango_layout_new* (context:PangoContext ): PangoLayout;
PROCEDURE [ccall] pango_layout_copy (src:PangoLayout ):PangoLayout ;
PROCEDURE [ccall] pango_layout_context_changed (layout: PangoLayout);

PROCEDURE [ccall] pango_layout_get_context* (layout: PangoLayout):PangoContext ;
PROCEDURE [ccall] pango_layout_set_attributes* (layout: PangoLayout; attrs: PangoAttrList);
PROCEDURE [ccall] pango_layout_get_attributes (layout: PangoLayout): PangoAttrList;
PROCEDURE [ccall] pango_layout_set_text* (layout: PangoLayout; text: PString; length: INTEGER);
PROCEDURE [ccall] pango_layout_get_text* (layout: PangoLayout): PString;
PROCEDURE [ccall] pango_layout_set_markup (layout: PangoLayout; markup: PString; length: INTEGER);
PROCEDURE [ccall] pango_layout_set_markup_with_accel (layout: PangoLayout; markup: PString; length: INTEGER; accel_marker: gunichar; OUT accel_char: gunichar);
PROCEDURE [ccall] pango_layout_set_font_description* (layout: PangoLayout; desc: PangoFontDescription);
PROCEDURE [ccall] pango_layout_get_font_description (layout: PangoLayout):PangoFontDescription ;
PROCEDURE [ccall] pango_layout_set_width (layout: PangoLayout; width: INTEGER);
PROCEDURE [ccall] pango_layout_get_width (layout: PangoLayout): INTEGER;
PROCEDURE [ccall] pango_layout_set_wrap (layout: PangoLayout; wrap: PangoWrapMode);
PROCEDURE [ccall] pango_layout_get_wrap (layout: PangoLayout): PangoWrapMode;
PROCEDURE [ccall] pango_layout_set_indent* (layout: PangoLayout; indent: INTEGER);
PROCEDURE [ccall] pango_layout_get_indent (layout: PangoLayout): INTEGER;
PROCEDURE [ccall] pango_layout_set_spacing (layout: PangoLayout; spacing: INTEGER);
PROCEDURE [ccall] pango_layout_get_spacing (layout: PangoLayout): INTEGER;
PROCEDURE [ccall] pango_layout_set_justify (layout: PangoLayout; justify: gboolean);
PROCEDURE [ccall] pango_layout_get_justify (layout: PangoLayout): gboolean;
PROCEDURE [ccall] pango_layout_set_auto_dir (layout: PangoLayout; auto_dir: gboolean);
PROCEDURE [ccall] pango_layout_get_auto_dir (layout: PangoLayout): gboolean;
PROCEDURE [ccall] pango_layout_set_alignment (layout: PangoLayout; alignment: PangoAlignment);
PROCEDURE [ccall] pango_layout_get_alignment (layout: PangoLayout): PangoAlignment;
PROCEDURE [ccall] pango_layout_set_single_paragraph_mode* (layout: PangoLayout; setting: gboolean);
PROCEDURE [ccall] pango_layout_get_single_paragraph_mode (layout: PangoLayout): gboolean;
PROCEDURE [ccall] pango_layout_set_ellipsize (layout: PangoLayout; ellipsize: PangoEllipsizeMode);
PROCEDURE [ccall] pango_layout_get_ellipsize (layout: PangoLayout): PangoEllipsizeMode;
PROCEDURE [ccall] pango_layout_set_tabs (layout: PangoLayout; tabs:PangoTabArray );
PROCEDURE [ccall] pango_layout_get_tabs (layout: PangoLayout):PangoTabArray ;



PROCEDURE [ccall] pango_layout_get_log_attrs (layout: PangoLayout; OUT attrs: PangoLogAttrs; OUT n_attrs: INTEGER);
PROCEDURE [ccall] pango_layout_index_to_pos (layout: PangoLayout; index: INTEGER; OUT pos: PangoRectangle);
PROCEDURE [ccall] pango_layout_get_cursor_pos (layout: PangoLayout; index: INTEGER; OUT [nil] strong_pos, weak_pos: PangoRectangle);
PROCEDURE [ccall] pango_layout_move_cursor_visually (layout: PangoLayout; strong: gboolean; old_index: INTEGER; old_trailing: INTEGER; direction: INTEGER; OUT new_index, new_trailing: INTEGER);
PROCEDURE [ccall] pango_layout_xy_to_index (layout: PangoLayout; x, y: INTEGER; OUT index,trailing: INTEGER): BOOLEAN;
PROCEDURE [ccall] pango_layout_get_extents (layout: PangoLayout; OUT ink_rect, logical_rect: PangoRectangle);
PROCEDURE [ccall] pango_layout_get_pixel_extents (layout: PangoLayout; OUT [nil] ink_rect, logical_rect: PangoRectangle);
PROCEDURE [ccall] pango_layout_get_size (layout: PangoLayout; OUT width , height: INTEGER);
PROCEDURE [ccall] pango_layout_get_pixel_size* (layout: PangoLayout; OUT width , height: INTEGER);
PROCEDURE [ccall] pango_layout_get_line_count (layout: PangoLayout): INTEGER;

PROCEDURE [ccall] pango_layout_get_line* (layout: PangoLayout; line: INTEGER): PangoLayoutLine;
PROCEDURE [ccall] pango_layout_get_lines (layout: PangoLayout): GSList;

PROCEDURE [ccall] pango_layout_line_ref (line: PangoLayoutLine): PangoLayoutLine;
PROCEDURE [ccall] pango_layout_line_unref (line: PangoLayoutLine);
PROCEDURE [ccall] pango_layout_line_x_to_index* (line: PangoLayoutLine; x_pos: INTEGER; OUT index,trailing: INTEGER): BOOLEAN;
PROCEDURE [ccall] pango_layout_line_index_to_x* (line: PangoLayoutLine; index: INTEGER; trailing: gboolean; OUT x_pos: INTEGER);
PROCEDURE [ccall] pango_layout_line_get_x_ranges (line: PangoLayoutLine; start_index: INTEGER; end_index: INTEGER; OUT ranges: POINTER TO ARRAY [untagged] OF INTEGER; OUT n_ranges: INTEGER);
PROCEDURE [ccall] pango_layout_line_get_extents (line: PangoLayoutLine; OUT [nil] ink_rect, logical_rect: PangoRectangle);
PROCEDURE [ccall] pango_layout_line_get_pixel_extents* (layout_line: PangoLayoutLine; OUT [nil] ink_rect, logical_rect: PangoRectangle);

PROCEDURE [ccall] pango_layout_get_iter (layout: PangoLayout): PangoLayoutIter;
PROCEDURE [ccall] pango_layout_iter_free (iter: PangoLayoutIter);
PROCEDURE [ccall] pango_layout_iter_get_index (iter: PangoLayoutIter): INTEGER;
PROCEDURE [ccall] pango_layout_iter_get_run (iter: PangoLayoutIter): PangoGlyphItem;
PROCEDURE [ccall] pango_layout_iter_get_line (iter: PangoLayoutIter): PangoLayoutLine;
PROCEDURE [ccall] pango_layout_iter_at_last_line (iter: PangoLayoutIter): BOOLEAN;
PROCEDURE [ccall] pango_layout_iter_next_char (iter: PangoLayoutIter): BOOLEAN;
PROCEDURE [ccall] pango_layout_iter_next_cluster (iter: PangoLayoutIter): BOOLEAN;
PROCEDURE [ccall] pango_layout_iter_next_run (iter: PangoLayoutIter): BOOLEAN;
PROCEDURE [ccall] pango_layout_iter_next_line (iter: PangoLayoutIter): BOOLEAN;
PROCEDURE [ccall] pango_layout_iter_get_char_extents (iter: PangoLayoutIter; OUT logical_rect: PangoRectangle);
PROCEDURE [ccall] pango_layout_iter_get_cluster_extents (iter: PangoLayoutIter; OUT [nil] ink_rect, logical_rect: PangoRectangle);
PROCEDURE [ccall] pango_layout_iter_get_run_extents (iter: PangoLayoutIter; OUT [nil] ink_rect, logical_rect: PangoRectangle);
PROCEDURE [ccall] pango_layout_iter_get_line_extents (iter: PangoLayoutIter; OUT [nil] ink_rect, logical_rect: PangoRectangle);
PROCEDURE [ccall] pango_layout_iter_get_line_yrange (iter: PangoLayoutIter; OUT y0, y1: INTEGER);
PROCEDURE [ccall] pango_layout_iter_get_layout_extents (iter: PangoLayoutIter; OUT [nil] ink_rect, logical_rect: PangoRectangle);
PROCEDURE [ccall] pango_layout_iter_get_baseline (iter: PangoLayoutIter): INTEGER;


(*  PangoScript *)
	PROCEDURE [ccall] pango_script_iter_new (text: PString; length: INTEGER): PangoScriptIter;
	PROCEDURE [ccall] pango_script_for_unichar (ch: gunichar): PangoScript;
	PROCEDURE [ccall] pango_script_iter_free (iter:PangoScriptIter );

	PROCEDURE [ccall] pango_script_iter_get_range (iter: PangoScriptIter; OUT start, end: PString; script: PangoScript);
	PROCEDURE [ccall] pango_script_iter_next (iter:PangoScriptIter ): BOOLEAN;
	PROCEDURE [ccall] pango_script_get_sample_language (script: PangoScript): PangoLanguage ;

	PROCEDURE [ccall] pango_language_includes_script (language: PangoLanguage; script: PangoScript): BOOLEAN;
	PROCEDURE [ccall] pango_font_find_shaper (font: PangoFont; language: PangoLanguage; ch: gunichar): PangoEngineShape;


END LibsPango.
