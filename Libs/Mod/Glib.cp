MODULE LibsGlib ["libglib-2.0-0.dll"];
IMPORT SYSTEM;
CONST
(*  Unix  *)
	G_DIR_SEPARATOR* = '/';  G_SEARCHPATH_SEPARATOR* = ':'; 

	G_PRIORITY_HIGH* = -100;
	G_PRIORITY_DEFAULT* = 0;
	G_PRIORITY_HIGH_IDLE* = 100;
	G_PRIORITY_DEFAULT_IDLE* = 200;
	G_PRIORITY_LOW* = 300;

TYPE 
  gint8* = BYTE;
  guint8* = SHORTCHAR;
  gint16 = SHORTINT;
  guint16* = CHAR;

  gssize = INTEGER;
  gsize = INTEGER;

  gint* = INTEGER;
  gint32* = INTEGER;
  guint32* = INTEGER;
  guint* = INTEGER;
	glong* = INTEGER; 
	gulong* = INTEGER; 
  gushort = CHAR;
  gfloat* = SHORTREAL;
  gdouble* = REAL;
  gint64* = LONGINT;
	guint64* = LONGINT;

  gpointer* = INTEGER;
	gconstpointer*= INTEGER;
  gchar* = SHORTCHAR;
  guchar* = SHORTCHAR;
  gunichar2 = CHAR;
  gunichar* = INTEGER; 
  gboolean* = INTEGER;


TYPE 

	String  = ARRAY [untagged] OF SHORTCHAR;
	WString = ARRAY [untagged] OF CHAR;
	UString = ARRAY [untagged] OF gunichar;
	PString* = POINTER TO String;
	PWString = POINTER TO WString;
	PUString = POINTER TO UString;
	PAString = POINTER TO ARRAY [untagged] OF String;

TYPE    
	GPid = INTEGER;

  GTimeVal = RECORD [untagged]
    tv_sec : INTEGER;
    tv_usec: INTEGER;
  END;

  GQuark* = INTEGER;

  GFunc = PROCEDURE   (data, user_data: gpointer);
  GCompareFunc* = PROCEDURE   (a,b: gpointer ): INTEGER;
  GEqualFunc = PROCEDURE   (a,b: gpointer ): INTEGER;
  GCompareDataFunc* = PROCEDURE   (a,b: gpointer; user_data: gpointer): INTEGER;
  GDestroyNotify* = PROCEDURE   (data: gpointer );
  GHashFunc = PROCEDURE   (key: gpointer ): INTEGER;
  GHFunc = PROCEDURE (key,value,user_data: gpointer );
  GTranslateFunc = PROCEDURE   (str :PString; data: gpointer ): PString;


(* gmem.h *)
TYPE GAllocator = POINTER TO RECORD [untagged] END;
TYPE GMemChunk = POINTER TO RECORD [untagged] END;

TYPE
  GMemVTable = RECORD [untagged]
    malloc     : PROCEDURE   (n_bytes: INTEGER ): gpointer;
    realloc    : PROCEDURE   (mem : gpointer; n_bytes :INTEGER ):gpointer;
    free       : PROCEDURE   (data: gpointer );
    (*  optional; set to NULL if not used !  *)
    calloc     : PROCEDURE   (n_blocks: INTEGER;  n_block_bytes: INTEGER ): gpointer;
    try_malloc : PROCEDURE   (n_bytes: INTEGER ): gpointer;
    try_realloc: PROCEDURE   (mem : gpointer; n_bytes :INTEGER ):gpointer;
  END;

CONST
  G_ALLOC_ONLY = 1;
  G_ALLOC_AND_FREE = 2;




(* ghook.h *)
TYPE
  GHook = POINTER TO RECORD [untagged]
    data     : gpointer;
    next     : GHook;
    prev     : GHook;
    ref_count: INTEGER;
    hook_id  : INTEGER;
    flags    : INTEGER;
    func     : gpointer;
    destroy  : GDestroyNotify;
  END;

  GHookList = POINTER TO RECORD [untagged]
    seq_id       : INTEGER;
    hook_size    : SET;               (*  bit field. hook_size:16, is_setup:1. *)
    hooks        : GHook;
    hook_memchunk: GMemChunk;
    finalize_hook: GHookFinalizeFunc;
    dummy1        : gpointer;
    dummy2        : gpointer;
  END;


  GHookFinalizeFunc = PROCEDURE   (hook_list:GHookList; hook :GHook );
  GHookFunc = PROCEDURE   (data: gpointer );
  GHookCheckFunc = PROCEDURE   (data: gpointer ): gboolean;
  GHookCompareFunc = PROCEDURE   (new_hook:GHook;  sibling :GHook ): INTEGER;
  GHookFindFunc = PROCEDURE   (hook: GHook; data : gpointer ): gboolean;
  GHookMarshaller = PROCEDURE   (hook:GHook ;marshal_data:gpointer );
  GHookCheckMarshaller = GHookFindFunc;


TYPE GHookFlagMask = INTEGER; (* enum: GHookFlagMask *)
CONST
  G_HOOK_FLAG_ACTIVE = 1;
  G_HOOK_FLAG_IN_CALL = 2;
  G_HOOK_FLAG_MASK = 0FH;
(*  End of enumeration: GHookFlagMask *)
CONST
  G_HOOK_FLAG_USER_SHIFT = 4;

(*  gerror.h  *)
TYPE 
	GError* = POINTER TO RECORD [untagged] 
		domain: GQuark;
  	code:INTEGER;
		message:PString;
  END;


(*  gstrfuncs.h. *)
TYPE  GAsciiType = INTEGER; (* enum: GAsciiType *)
CONST
  G_ASCII_ALNUM = 1;
  G_ASCII_ALPHA = 2;
  G_ASCII_CNTRL = 4;
  G_ASCII_DIGIT = 8;
  G_ASCII_GRAPH = 16;
  G_ASCII_LOWER = 32;
  G_ASCII_PRINT = 64;
  G_ASCII_PUNCT = 128;
  G_ASCII_SPACE = 256;
  G_ASCII_UPPER = 512;
  G_ASCII_XDIGIT = 1024;
(*  End of enumeration: GAsciiType *)


(*  gunicode.h *)

TYPE  GUnicodeType = INTEGER; (* Enumeration:  *)
CONST
  G_UNICODE_CONTROL = 0;
  G_UNICODE_FORMAT = 1;
  G_UNICODE_UNASSIGNED = 2;
  G_UNICODE_PRIVATE_USE = 3;
  G_UNICODE_SURROGATE = 4;
  G_UNICODE_LOWERCASE_LETTER = 5;
  G_UNICODE_MODIFIER_LETTER = 6;
  G_UNICODE_OTHER_LETTER = 7;
  G_UNICODE_TITLECASE_LETTER = 8;
  G_UNICODE_UPPERCASE_LETTER = 9;
  G_UNICODE_COMBINING_MARK = 10;
  G_UNICODE_ENCLOSING_MARK = 11;
  G_UNICODE_NON_SPACING_MARK = 12;
  G_UNICODE_DECIMAL_NUMBER = 13;
  G_UNICODE_LETTER_NUMBER = 14;
  G_UNICODE_OTHER_NUMBER = 15;
  G_UNICODE_CONNECT_PUNCTUATION = 16;
  G_UNICODE_DASH_PUNCTUATION = 17;
  G_UNICODE_CLOSE_PUNCTUATION = 18;
  G_UNICODE_FINAL_PUNCTUATION = 19;
  G_UNICODE_INITIAL_PUNCTUATION = 20;
  G_UNICODE_OTHER_PUNCTUATION = 21;
  G_UNICODE_OPEN_PUNCTUATION = 22;
  G_UNICODE_CURRENCY_SYMBOL = 23;
  G_UNICODE_MODIFIER_SYMBOL = 24;
  G_UNICODE_MATH_SYMBOL = 25;
  G_UNICODE_OTHER_SYMBOL = 26;
  G_UNICODE_LINE_SEPARATOR = 27;
  G_UNICODE_PARAGRAPH_SEPARATOR = 28;
  G_UNICODE_SPACE_SEPARATOR = 29;
(*  End of enumeration: GUnicodeType *)

TYPE GUnicodeBreakType = INTEGER; (* enum: GUnicodeBreakType *)
CONST
  G_UNICODE_BREAK_MANDATORY = 0;
  G_UNICODE_BREAK_CARRIAGE_RETURN = 1;
  G_UNICODE_BREAK_LINE_FEED = 2;
  G_UNICODE_BREAK_COMBINING_MARK = 3;
  G_UNICODE_BREAK_SURROGATE = 4;
  G_UNICODE_BREAK_ZERO_WIDTH_SPACE = 5;
  G_UNICODE_BREAK_INSEPARABLE = 6;
  G_UNICODE_BREAK_NON_BREAKING_GLUE = 7;
  G_UNICODE_BREAK_CONTINGENT = 8;
  G_UNICODE_BREAK_SPACE = 9;
  G_UNICODE_BREAK_AFTER = 10;
  G_UNICODE_BREAK_BEFORE = 11;
  G_UNICODE_BREAK_BEFORE_AND_AFTER = 12;
  G_UNICODE_BREAK_HYPHEN = 13;
  G_UNICODE_BREAK_NON_STARTER = 14;
  G_UNICODE_BREAK_OPEN_PUNCTUATION = 15;
  G_UNICODE_BREAK_CLOSE_PUNCTUATION = 16;
  G_UNICODE_BREAK_QUOTATION = 17;
  G_UNICODE_BREAK_EXCLAMATION = 18;
  G_UNICODE_BREAK_IDEOGRAPHIC = 19;
  G_UNICODE_BREAK_NUMERIC = 20;
  G_UNICODE_BREAK_INFIX_SEPARATOR = 21;
  G_UNICODE_BREAK_SYMBOL = 22;
  G_UNICODE_BREAK_ALPHABETIC = 23;
  G_UNICODE_BREAK_PREFIX = 24;
  G_UNICODE_BREAK_POSTFIX = 25;
  G_UNICODE_BREAK_COMPLEX_CONTEXT = 26;
  G_UNICODE_BREAK_AMBIGUOUS = 27;
  G_UNICODE_BREAK_UNKNOWN = 28;
  G_UNICODE_BREAK_NEXT_LINE = 29;
  G_UNICODE_BREAK_WORD_JOINER = 30;

(*  End of enumeration: GUnicodeBreakType *)



TYPE   GNormalizeMode = INTEGER; (* enum: GNormalizeMode *)
CONST
  G_NORMALIZE_DEFAULT = 0;     
  G_NORMALIZE_DEFAULT_COMPOSE = 1; 
  G_NORMALIZE_ALL = 2;    
  G_NORMALIZE_ALL_COMPOSE = 3; 
(*  End of enumeration: GNormalizeMode *)


(*  gstdio.h *)
TYPE
  stat = RECORD
    (* !!! *)
  END;


(*  gfileutils.h *)
TYPE   GFileError = INTEGER; (* enum: GFileError *)
CONST
  G_FILE_ERROR_EXIST = 0;
  G_FILE_ERROR_ISDIR = 1;
  G_FILE_ERROR_ACCES = 2;
  G_FILE_ERROR_NAMETOOLONG = 3;
  G_FILE_ERROR_NOENT = 4;
  G_FILE_ERROR_NOTDIR = 5;
  G_FILE_ERROR_NXIO = 6;
  G_FILE_ERROR_NODEV = 7;
  G_FILE_ERROR_ROFS = 8;
  G_FILE_ERROR_TXTBSY = 9;
  G_FILE_ERROR_FAULT = 10;
  G_FILE_ERROR_LOOP = 11;
  G_FILE_ERROR_NOSPC = 12;
  G_FILE_ERROR_NOMEM = 13;
  G_FILE_ERROR_MFILE = 14;
  G_FILE_ERROR_NFILE = 15;
  G_FILE_ERROR_BADF = 16;
  G_FILE_ERROR_INVAL = 17;
  G_FILE_ERROR_PIPE = 18;
  G_FILE_ERROR_AGAIN = 19;
  G_FILE_ERROR_INTR = 20;
  G_FILE_ERROR_IO = 21;
  G_FILE_ERROR_PERM = 22;
  G_FILE_ERROR_NOSYS = 23;
  G_FILE_ERROR_FAILED = 24;
(*  End of enumeration: GFileError *)

TYPE   GFileTest = INTEGER; (* enum: GFileTest *)
CONST
  G_FILE_TEST_IS_REGULAR = 1;
  G_FILE_TEST_IS_SYMLINK = 2;
  G_FILE_TEST_IS_DIR = 4;
  G_FILE_TEST_IS_EXECUTABLE = 8;
  G_FILE_TEST_EXISTS = 16;
(*  End of enumeration: GFileTest *)



(* gdataset.h *)
(*  Keyed Data List  *)
TYPE	GData* = POINTER TO RECORD [untagged]  END;
			GDataForeachFunc = PROCEDURE  (key_id: GQuark; data: gpointer; user_data: gpointer );

(*  gdir.h *)
TYPE  GDir = POINTER TO RECORD [untagged]  END;
(*  gmappedfile.h *)
TYPE  GMappedFile = POINTER TO RECORD [untagged]  END;

(*  gconvert.h *)
TYPE  GConvertError = INTEGER; (* enum: GConvertError *)
CONST
  G_CONVERT_ERROR_NO_CONVERSION = 0;
  G_CONVERT_ERROR_ILLEGAL_SEQUENCE = 1;
  G_CONVERT_ERROR_FAILED = 2;
  G_CONVERT_ERROR_PARTIAL_INPUT = 3;
  G_CONVERT_ERROR_BAD_URI = 4;
  G_CONVERT_ERROR_NOT_ABSOLUTE_PATH = 5;
(*  End of enumeration: GConvertError *)

TYPE GIConv = POINTER TO RECORD [untagged]  END;

(*  gkeyfile.h *)
TYPE  GKeyFile = POINTER TO RECORD  END;
TYPE   GKeyFileError = INTEGER; (* enum: GKeyFileError *)
CONST
  G_KEY_FILE_ERROR_UNKNOWN_ENCODING = 0;
  G_KEY_FILE_ERROR_PARSE = 1;
  G_KEY_FILE_ERROR_NOT_FOUND = 2;
  G_KEY_FILE_ERROR_KEY_NOT_FOUND = 3;
  G_KEY_FILE_ERROR_GROUP_NOT_FOUND = 4;
  G_KEY_FILE_ERROR_INVALID_VALUE = 5;
(*  End of enumeration: GKeyFileError *)

TYPE   GKeyFileFlags = INTEGER; (* enum: GKeyFileFlags *)
CONST 
  G_KEY_FILE_NONE = 0;
  G_KEY_FILE_KEEP_COMMENTS = 1;
  G_KEY_FILE_KEEP_TRANSLATIONS = 2;
(*  End of enumeration: GKeyFileFlags *)


(* gcache.h *)
TYPE
  GCache = POINTER TO RECORD  END;

  GCacheNewFunc = PROCEDURE   (key: gpointer ): gpointer;
  GCacheDupFunc = GCacheNewFunc;
  GCacheDestroyFunc = PROCEDURE   (value: gpointer );

(* garray.h *)
(*    Resizable arrays *)
TYPE
  GArray* = POINTER TO RECORD [untagged]
    data: PString;
    len : INTEGER;
  END;


(*  Byte arrays  Implemented as a GArray, but type-safe.  *)  
TYPE
	GByteArray = POINTER TO RECORD [untagged]
    data: POINTER TO ARRAY [untagged] OF BYTE;
    len : INTEGER;
  END;


(*    Resizable pointer array.  This interface is much less complicated   *)
(*  * than the above.  Add appends a pointer.  Remove fills any cleared   *)
(*  * spot and shortens the array. remove_fast will again distort order.  *)
TYPE
  GPtrArray* = POINTER TO RECORD [untagged]
    pdata: POINTER TO ARRAY [untagged] OF gpointer;
    len  : INTEGER;
  END;


(* gstring.h *)
TYPE 
  GString = POINTER TO RECORD [untagged]
    str          : PString;
    len          : INTEGER;
    allocated_len: INTEGER;
  END;

  GStringChunk = POINTER TO RECORD [untagged] END;

(* glist.h *) (*  Doubly linked lists  *)
TYPE
	GList* = POINTER TO RECORD [untagged] 
		data*: gpointer;
		next*, prev*: GList
	END;

(* gslist.h *) (*  Singly linked lists  *)
TYPE
  GSList* = POINTER TO RECORD [untagged]
    data*: gpointer;
    next*: GSList;
  END;


(* gqueue.h *) (*  Queues  *)
TYPE
  GQueue* = POINTER TO RECORD [untagged]
    head*  : GList;
    tail*  : GList;
    length*: INTEGER;
  END;


(* gnode.h *) (*  N-way tree implementation  *)
TYPE
  GNode* = POINTER TO RECORD [untagged]
    data*    : gpointer;
    next*    : GNode;
    prev*    : GNode;
    parent*  : GNode;
    children*: GNode;
  END;

 GCopyFunc = PROCEDURE (src, data:gpointer): gpointer;
 GNodeTraverseFunc = PROCEDURE (node:GNode; data : gpointer): gboolean;
 GNodeForeachFunc = PROCEDURE (node:GNode; data:gpointer);


TYPE   GTraverseFlags = INTEGER; (* enum *)
CONST  G_TRAVERSE_LEAVES = 1;  G_TRAVERSE_NON_LEAVES = 2;  G_TRAVERSE_ALL = 3;

TYPE  GTraverseType = INTEGER;(* enum  *)
CONST G_IN_ORDER = 0; G_PRE_ORDER = 1; G_POST_ORDER = 2; G_LEVEL_ORDER = 3;



(* gtree.h *) (*  Balanced binary trees  *)
TYPE
  GTree * = POINTER TO RECORD [untagged]   END;
  GTraverseFunc = PROCEDURE   (key, value, data :gpointer ): gboolean;

(* ghash.h *) (*  Hash tables  *)
TYPE
  GHashTable = POINTER TO RECORD [untagged]  END;
  GHRFunc = PROCEDURE   (key :gpointer; value: gpointer; user_data: gpointer ): gboolean;

(* grel.h *) (*  * Indexed Relations *)
TYPE
  GRelation = POINTER TO RECORD [untagged]  END;
  GTuples = POINTER TO RECORD [untagged]
    len: INTEGER;
  END;


(*  giochannel.h *)
TYPE   GSource = POINTER TO RECORD [untagged] END;

TYPE  GIOStatus = INTEGER; (* enum *)
CONST G_IO_STATUS_ERROR = 0;  G_IO_STATUS_NORMAL = 1;  G_IO_STATUS_EOF = 2;  G_IO_STATUS_AGAIN = 3;


TYPE
  GIOFunc = PROCEDURE (source: GIOChannel; condition: GIOCondition ; data : gpointer ): gboolean;


  GIOChannel = POINTER TO RECORD [untagged]
    ref_count        : INTEGER;
    funcs            : GIOFuncs;
    encoding         : PString;
    read_cd          : GIConv;
    write_cd         : GIConv;
    line_term        : PString;  
    line_term_len    : INTEGER;    
    buf_size         : INTEGER;
    read_buf         : GString;  
    encoded_read_buf : GString;  
    write_buf        : GString;  
    partial_write_buf: ARRAY 6 OF gchar; 
    flags        : SET;    (*  use_buffer:1, do_encode:1, close_on_unref:1, is_readable:1, is_writeable:1, is_seekable:1. *)
    reserved1        : gpointer;
    reserved2        : gpointer;
  END;


 
  GIOFuncs = POINTER TO RECORD [untagged] 
    io_read        : PROCEDURE  (channel: GIOChannel; buf: PString;count: INTEGER; OUT bytes_read: INTEGER; OUT error: GError ): GIOStatus;;
    io_write       : PROCEDURE   (channel: GIOChannel; buf: PString;count: INTEGER;
OUT bytes_written: INTEGER; OUT error: GError ): GIOStatus;;
    io_seek        : PROCEDURE    (channel: GIOChannel; offset:LONGINT; type: GSeekType;OUT error: GError ): GIOStatus;;
    io_close       : PROCEDURE (channel: GIOChannel; OUT error: GError ): GIOStatus;
    io_create_watch: PROCEDURE (channel: GIOChannel; condition: GIOCondition ): GIConv;
    io_free        : PROCEDURE (channel: GIOChannel );
    io_set_flags   : PROCEDURE (channel: GIOChannel; flags: GIOFlags; OUT error: GError ): GIOStatus;
    io_get_flags   : PROCEDURE (channel: GIOChannel ): GIOFlags;
  END;





TYPE  GSeekType = INTEGER;
(* enum: GSeekType *)

CONST
  G_SEEK_CUR = 0;
  G_SEEK_SET = 1;
  G_SEEK_END = 2;
(*  End of enumeration: GSeekType *)

TYPE
  GIOCondition = INTEGER;
(* enum: GIOCondition *)

CONST
  G_IO_IN = 1;
  G_IO_OUT = 4;
  G_IO_PRI = 2;
  G_IO_ERR = 8;
  G_IO_HUP = 16;
  G_IO_NVAL = 32;

(*  End of enumeration: GIOCondition *)
TYPE
  GIOFlags = INTEGER;
(* enum: GIOFlags *)

CONST
  G_IO_FLAG_APPEND = 1;
  G_IO_FLAG_NONBLOCK = 2;
  G_IO_FLAG_IS_READABLE = 4;   (*  Read only flag  *)
  G_IO_FLAG_IS_WRITEABLE = 8;   (*  Read only flag  *)
  G_IO_FLAG_IS_SEEKABLE = 16;   (*  Read only flag  *)
  G_IO_FLAG_MASK = 31;
  G_IO_FLAG_GET_MASK = G_IO_FLAG_MASK;
  G_IO_FLAG_SET_MASK = 3;

(*  End of enumeration: GIOFlags *)

TYPE
  GIOError = INTEGER;
(* enum: GIOError *)

CONST
  G_IO_ERROR_NONE = 0;
  G_IO_ERROR_AGAIN = 1;
  G_IO_ERROR_INVAL = 2;
  G_IO_ERROR_UNKNOWN = 3;

(*  End of enumeration: GIOError *)

TYPE GIOChannelError = INTEGER; (* enum: GIOChannelError *)
CONST
  G_IO_CHANNEL_ERROR_FBIG = 0;
  G_IO_CHANNEL_ERROR_INVAL = 1;
  G_IO_CHANNEL_ERROR_IO = 2;
  G_IO_CHANNEL_ERROR_ISDIR = 3;
  G_IO_CHANNEL_ERROR_NOSPC = 4;
  G_IO_CHANNEL_ERROR_NXIO = 5;
  G_IO_CHANNEL_ERROR_OVERFLOW = 6;
  G_IO_CHANNEL_ERROR_PIPE = 7;
  G_IO_CHANNEL_ERROR_FAILED = 8;
(*  End of enumeration: GIOChannelError *)


(*  gshell.h *)
TYPE
  GShellError = INTEGER;
CONST
  G_SHELL_ERROR_BAD_QUOTING = 0;
  G_SHELL_ERROR_EMPTY_STRING = 1;
  G_SHELL_ERROR_FAILED = 2;
(*  grand.h *)
(*    GRand - a good and fast random number generator  *)
TYPE
  GRand = POINTER TO RECORD [untagged]
  END;
(*  gdate.h *)

TYPE
  GTime = INTEGER;
  GDateYear = SHORTINT;
  GDateDay = BYTE;   (*  day of the month  *)
  
	GDateDesc = RECORD
    julian_days: INTEGER;  
		fields: SET	(* julian:1, dmy:1, day:6, month:4, year:16. *)
  END;
  GDate = POINTER TO GDateDesc;

(* Note: directly manipulating structs is generally a bad idea, but     *)
(* in this case it's an *incredibly* bad idea, because all or part      *)
(* of this struct can be invalid at any given time.  *)


TYPE
  tm = RECORD
(*  make struct tm known without having to include time.h  *)
  END;
  Ptr_tm = POINTER TO tm;

(*  enum used to specify order of appearance in parsed date strings  *)
TYPE  GDateDMY = INTEGER; (* enum: GDateDMY *)
CONST
  G_DATE_DAY = 0;
  G_DATE_MONTH = 1;
  G_DATE_YEAR = 2;
(*  End of enumeration: GDateDMY *)

(*  actual week and month values  *)
TYPE   GDateWeekday = INTEGER; (* enum: GDateWeekday *)
CONST
  G_DATE_BAD_WEEKDAY = 0;
  G_DATE_MONDAY = 1;
  G_DATE_TUESDAY = 2;
  G_DATE_WEDNESDAY = 3;
  G_DATE_THURSDAY = 4;
  G_DATE_FRIDAY = 5;
  G_DATE_SATURDAY = 6;
  G_DATE_SUNDAY = 7;

(*  End of enumeration: GDateWeekday *)

TYPE   GDateMonth = INTEGER;
(* enum: GDateMonth *)
CONST
  G_DATE_BAD_MONTH = 0;
  G_DATE_JANUARY = 1;
  G_DATE_FEBRUARY = 2;
  G_DATE_MARCH = 3;
  G_DATE_APRIL = 4;
  G_DATE_MAY = 5;
  G_DATE_JUNE = 6;
  G_DATE_JULY = 7;
  G_DATE_AUGUST = 8;
  G_DATE_SEPTEMBER = 9;
  G_DATE_OCTOBER = 10;
  G_DATE_NOVEMBER = 11;
  G_DATE_DECEMBER = 12;
(*  End of enumeration: GDateMonth *)

CONST
  G_DATE_BAD_JULIAN = 0;
  G_DATE_BAD_DAY = 0;
  G_DATE_BAD_YEAR = 0;

(*  gtimer.h *)
(*  Timer  *)

TYPE  GTimer = POINTER TO RECORD [untagged]  END;

CONST
  G_USEC_PER_SEC = 1000000; (*  microseconds per second  *)


(*  gmarkup.h - Simple XML-like string parser/writer  *)

TYPE  GMarkupParseContext = POINTER TO RECORD [untagged]  END;
TYPE  GMarkupParser = RECORD [untagged]
		  (*  Called for open tags <foo bar="baz">  *)
    start_element: PROCEDURE (context: GMarkupParseContext; element_name :PString; 
																attribute_names, attribute_values: PAString; user_data :gpointer;OUT error: GError );
    	(*  Called for close tags </foo>  *)
    end_element  : PROCEDURE    (context: GMarkupParseContext; element_name :PString; user_data :gpointer;OUT error: GError );
	    (*  Called for character data  *)(*  text is not nul-terminated  *)
    text    : PROCEDURE (context: GMarkupParseContext;  text:PString;text_len :INTEGER; user_data :gpointer;OUT error: GError );
	    (*    Called for strings that should be re-saved verbatim in this same position, but are not otherwise interpretable. *)
    passthrough  : PROCEDURE (context: GMarkupParseContext;  text:PString;text_len :INTEGER; user_data :gpointer;OUT error: GError );
	    (*    Called on error *)   (*   The GError should not be freed.  *)
    error        : PROCEDURE   (context: GMarkupParseContext; error: GError;   user_data :gpointer );
  END;

TYPE  GMarkupError = INTEGER; (* enum: GMarkupError *)
CONST
  G_MARKUP_ERROR_BAD_UTF8 = 0;
  G_MARKUP_ERROR_EMPTY = 1;
  G_MARKUP_ERROR_PARSE = 2;
  G_MARKUP_ERROR_UNKNOWN_ELEMENT = 3;
  G_MARKUP_ERROR_UNKNOWN_ATTRIBUTE = 4;
  G_MARKUP_ERROR_INVALID_CONTENT = 5;

(*  End of enumeration: GMarkupError *)


TYPE
  GMarkupParseFlags = INTEGER;
CONST
  G_MARKUP_DO_NOT_USE_THIS_UNSUPPORTED_FLAG = 1;
(*  End of enumeration: GMarkupParseFlags *)







(* gmessages.h *)
CONST
  G_LOG_LEVEL_USER_SHIFT = 8;

(*  Glib log levels and flags.  *)
(*                              *)
(* enum: GLogLevelFlags *)

(*  log flags  *)
  G_LOG_FLAG_RECURSION = 1;
  G_LOG_FLAG_FATAL = 2;

(*  GLib log levels  *)
  G_LOG_LEVEL_ERROR = 4;   (*  always fatal  *)
  G_LOG_LEVEL_CRITICAL = 8;
  G_LOG_LEVEL_WARNING = 16;
  G_LOG_LEVEL_MESSAGE = 32;
  G_LOG_LEVEL_INFO = 64;
  G_LOG_LEVEL_DEBUG = 128;
  G_LOG_LEVEL_MASK = -4;

TYPE
  GLogLevelFlags = INTEGER;
(*  End of enumeration: GLogLevelFlags *)

(*  GLib log levels that are considered fatal by default  *)

CONST
  G_LOG_FATAL_MASK = 5;

TYPE
  GLogFunc = PROCEDURE   (log_domain: PString;log_level:
                       GLogLevelFlags ; message :PString; user_data:
                       gpointer );
(*  Logging mechanism   *)
TYPE
  GPrintFunc = PROCEDURE   (string :PString );

(* goption.h *)
TYPE
  GOptionContext = POINTER TO RECORD [untagged]  END;
  GOptionGroup = POINTER TO RECORD [untagged]  END;
  GOptionEntry = POINTER TO RECORD [untagged]
    long_name      : PString;
    short_name     : gchar;
    flags          : INTEGER;
    arg            : GOptionArg;
    arg_data       : gpointer;
    description    : PString;
    arg_description: PString;
  END;


TYPE  GOptionArg = INTEGER; (* enum: GOptionArg *)
CONST
  G_OPTION_ARG_NONE = 0;
  G_OPTION_ARG_STRING = 1;
  G_OPTION_ARG_INT = 2;
  G_OPTION_ARG_CALLBACK = 3;
  G_OPTION_ARG_FILENAME = 4;
  G_OPTION_ARG_STRING_ARRAY = 5;
  G_OPTION_ARG_FILENAME_ARRAY = 6;
(*  End of enumeration: GOptionArg *)


TYPE  GOptionFlags = INTEGER; (* enum: GOptionFlags *)
CONST
  G_OPTION_FLAG_HIDDEN = 1;
  G_OPTION_FLAG_IN_MAIN = 2;
  G_OPTION_FLAG_REVERSE = 4;
  G_OPTION_FLAG_NO_ARG = 8;
  G_OPTION_FLAG_FILENAME = 16;
  G_OPTION_FLAG_OPTIONAL_ARG = 32;
  G_OPTION_FLAG_NOALIAS = 64;
(*  End of enumeration: GOptionFlags *)
TYPE   
  GOptionArgFunc = PROCEDURE   (option_name: PString; value: PString; data: gpointer; OUT error: GError ): gboolean;
  GOptionParseFunc = PROCEDURE   (context :GOptionContext;  group: GOptionContext; data: gpointer; OUT error: GError ): gboolean;
  GOptionErrorFunc = PROCEDURE   (context: GOptionContext; group: GOptionContext; data: gpointer; OUT error: GError);

TYPE   GOptionError = INTEGER; (* enum: GOptionError *)
CONST
  G_OPTION_ERROR_UNKNOWN_OPTION = 0;
  G_OPTION_ERROR_BAD_VALUE = 1;
  G_OPTION_ERROR_FAILED = 2;
(*  End of enumeration: GOptionError *)

CONST
  G_OPTION_REMAINING = '';

(* gpattern.h *)
TYPE  GPatternSpec = POINTER TO RECORD [untagged] END;

(* gspawn.h *)
TYPE
	GSpawnChildSetupFunc* = PROCEDURE(user_data: gpointer);
  GSpawnError = INTEGER; (* enum: GSpawnError *)
CONST
  G_SPAWN_ERROR_FORK = 0;   
  G_SPAWN_ERROR_READ = 1;   
  G_SPAWN_ERROR_CHDIR = 2;  
  G_SPAWN_ERROR_ACCES = 3;  
  G_SPAWN_ERROR_PERM = 4;   
  G_SPAWN_ERROR_2BIG = 5;   
  G_SPAWN_ERROR_NOEXEC = 6; 
  G_SPAWN_ERROR_NAMETOOLONG = 7; 
  G_SPAWN_ERROR_NOENT = 8;  
  G_SPAWN_ERROR_NOMEM = 9;  
  G_SPAWN_ERROR_NOTDIR = 10;
  G_SPAWN_ERROR_LOOP = 11;  
  G_SPAWN_ERROR_TXTBUSY = 12; 
  G_SPAWN_ERROR_IO = 13; 
  G_SPAWN_ERROR_NFILE = 14;
  G_SPAWN_ERROR_MFILE = 15;
  G_SPAWN_ERROR_INVAL = 16;
  G_SPAWN_ERROR_ISDIR = 17;
  G_SPAWN_ERROR_LIBBAD = 18;
  G_SPAWN_ERROR_FAILED = 19;
(*  End of enumeration: GSpawnError *)


TYPE  GSpawnFlags* = INTEGER; (* enum: GSpawnFlags *)
CONST
  G_SPAWN_LEAVE_DESCRIPTORS_OPEN = 1;
  G_SPAWN_DO_NOT_REAP_CHILD = 2;
  G_SPAWN_SEARCH_PATH = 4;
  G_SPAWN_STDOUT_TO_DEV_NULL = 8;
  G_SPAWN_STDERR_TO_DEV_NULL = 16;
  G_SPAWN_CHILD_INHERITS_STDIN = 32;
  G_SPAWN_FILE_AND_ARGV_ZERO = 64;
(*  End of enumeration: GSpawnFlags *)


(* gmain.h *)
TYPE   
  GMainContext = POINTER TO  RECORD  [untagged] 
  END;


  GPollFD = RECORD [untagged] 
    fd     : INTEGER;
    events : SHORTINT;
    revents: SHORTINT;
  END;

	GPollFunc = PROCEDURE (IN ufds:ARRAY [untagged] OF GPollFD; nfsd, timeout:INTEGER ): INTEGER;
	GChildWatchFunc = PROCEDURE ( pid:GPid;status:INTEGER;data: gpointer);
	GSourceFunc* = PROCEDURE [ccall] ( data:gpointer ): gboolean;

	GSourceFuncs = RECORD [untagged] 
    prepare         : PROCEDURE;
    check           : PROCEDURE;
    dispatch        : PROCEDURE;
    finalize        : PROCEDURE;
    closure_callback: GSourceFunc;
    closure_marshal : PROCEDURE;
	END;

  GSourceCallbackFuncs = RECORD [untagged] 
    ref  : GDestroyNotify;
    unref: GDestroyNotify;
    get  :  PROCEDURE (cb_data: gpointer; source:GSource; func:GSourceFunc; VAR data:gpointer );
  END;

	(*  Memory profiler and checker, has to be enabled via g_mem_set_vtable()  *)
	VAR glib_mem_profiler_table: POINTER TO GMemVTable;


(* -- -- -- *)

(* gmem.h *)
	(*  Memory allocation functions  *)
	PROCEDURE [ccall] g_mem_is_system_malloc (): gboolean;

	PROCEDURE [ccall] g_malloc (n_bytes: INTEGER ): gpointer;
	PROCEDURE [ccall] g_malloc0 (n_bytes: INTEGER ): gpointer;
	PROCEDURE [ccall] g_try_malloc (n_bytes: INTEGER ): gpointer;
	PROCEDURE [ccall] g_try_malloc0 (n_bytes: INTEGER ): gpointer;
	PROCEDURE [ccall] g_try_realloc (mem: gpointer; n_bytes: INTEGER ): gpointer;
	PROCEDURE [ccall] g_realloc (mem: gpointer; n_bytes: INTEGER ): gpointer;
	PROCEDURE [ccall] g_free* (mem: gpointer );

	PROCEDURE [ccall] g_memdup (mem: gpointer; byte_size: INTEGER ): gpointer;

	PROCEDURE [ccall] g_mem_set_vtable (VAR vtable: GMemVTable ); (*  has to be first GLib function called if being used *)

	PROCEDURE [ccall] g_mem_profile;


	PROCEDURE [ccall] g_mem_chunk_new (name: PString; atom_size: INTEGER;area_size: INTEGER; type: INTEGER ): GMemChunk;
	PROCEDURE [ccall] g_mem_chunk_destroy (mem_chunk: GMemChunk );
	PROCEDURE [ccall] g_mem_chunk_alloc (mem_chunk: GMemChunk ): gpointer;
	PROCEDURE [ccall] g_mem_chunk_alloc0 (mem_chunk: GMemChunk ): gpointer;
	PROCEDURE [ccall] g_mem_chunk_free (mem_chunk: GMemChunk; mem: gpointer );
	PROCEDURE [ccall] g_mem_chunk_clean (mem_chunk: GMemChunk );
	PROCEDURE [ccall] g_mem_chunk_reset (mem_chunk: GMemChunk );
	PROCEDURE [ccall] g_mem_chunk_print (mem_chunk: GMemChunk );
	PROCEDURE [ccall] g_mem_chunk_info ;
	PROCEDURE [ccall] g_blow_chunks ;

	(*  Generic allocators  *)
	PROCEDURE [ccall] g_allocator_new (name: PString; n_preallocs: INTEGER ): GAllocator;
	PROCEDURE [ccall] g_allocator_free (allocator: GAllocator );

	PROCEDURE [ccall] g_list_push_allocator (allocator: GAllocator );
	PROCEDURE [ccall] g_list_pop_allocator;

	PROCEDURE [ccall] g_slist_push_allocator (allocator: GAllocator );
	PROCEDURE [ccall] g_slist_pop_allocator ;

	PROCEDURE [ccall] g_node_push_allocator (allocator: GAllocator );
	PROCEDURE [ccall] g_node_pop_allocator;


(*  gstrfuncs.h. *)
	PROCEDURE [ccall] g_ascii_tolower (c: gchar ): gchar;
	PROCEDURE [ccall] g_ascii_toupper (c: gchar ): gchar;
	PROCEDURE [ccall] g_ascii_digit_value (c: gchar ): INTEGER;
	PROCEDURE [ccall] g_ascii_xdigit_value (c: gchar ): INTEGER;

	PROCEDURE [ccall] g_strerror (errnum: INTEGER ): PString;
	PROCEDURE [ccall] g_strsignal (signum: INTEGER ): PString;

	PROCEDURE [ccall] g_strdelimit (string: PString; delimiters: PString;new_delimiter: gchar ): PString;
	PROCEDURE [ccall] g_strcanon (string: PString; valid_chars: PString;substitutor: gchar ): PString;
	PROCEDURE [ccall] g_strreverse (string: PString ): PString;

	PROCEDURE [ccall] g_strlcpy (dest: PString; src: PString;dest_size: INTEGER ): INTEGER;
	PROCEDURE [ccall] g_strlcat (dest: PString; src: PString; dest_size: INTEGER ): INTEGER;
	PROCEDURE [ccall] g_strstr_len (haystack: PString; haystack_len: INTEGER; needle: PString ): PString;
	PROCEDURE [ccall] g_strrstr (haystack: PString; needle: PString ): PString;
	PROCEDURE [ccall] g_strrstr_len (haystack: PString; haystack_len: INTEGER; needle: PString ): PString;
	PROCEDURE [ccall] g_str_has_suffix (str: PString; suffix: PString ): gboolean;
	PROCEDURE [ccall] g_str_has_prefix (str: PString; prefix: PString ): gboolean;


	PROCEDURE [ccall] g_strchug (string: PString ): PString; (*  removes leading spaces  *)
	PROCEDURE [ccall] g_strchomp (string: PString ): PString; (*  removes trailing spaces  *)


	PROCEDURE [ccall] g_ascii_strcasecmp (s1: PString; s2: PString ): INTEGER;
	PROCEDURE [ccall] g_ascii_strncasecmp (s1: PString; s2: PString; n: INTEGER ): INTEGER;
	PROCEDURE [ccall] g_ascii_strdown (str: PString; len: INTEGER ): PString;
	PROCEDURE [ccall] g_ascii_strup (str: PString; len: INTEGER ): PString;

	PROCEDURE [ccall] g_strdup (str: PString ): PString;
	PROCEDURE [ccall] g_strndup (str: PString; n: INTEGER ): PString;
	PROCEDURE [ccall] g_strnfill (length: INTEGER; fill_char: gchar ): PString;

	PROCEDURE [ccall] g_strcompress (source: PString ): PString;
	PROCEDURE [ccall] g_strescape (source: PString; exceptions: PString ): PString;


	(*  String to/from double conversion functions  *)
	PROCEDURE [ccall] g_strtod (nptr: PString; OUT endptr: PString ): REAL;
	PROCEDURE [ccall] g_ascii_strtod (nptr: PString; OUT endptr: PString ): REAL;
	PROCEDURE [ccall] g_ascii_strtoull (nptr: PString; OUT endptr: PString;base: INTEGER ): LONGINT;

	PROCEDURE [ccall] g_ascii_dtostr (buffer: PString; buf_len: INTEGER; d: REAL ): PString;
	PROCEDURE [ccall] g_ascii_formatd (buffer: PString; buf_len: INTEGER; format: PString; d: REAL ): PString;


	(*    NULL terminated string arrays.   *)
	PROCEDURE [ccall] g_strsplit (string: PString; delimiter: PString;max_tokens: INTEGER ): PAString;
	PROCEDURE [ccall] g_strsplit_set (string: PString; delimiters: PString;max_tokens: INTEGER ): PAString;
	PROCEDURE [ccall] g_strjoinv (separator: PString; str_array: PAString ): PString;
	PROCEDURE [ccall] g_strfreev (str_array: PAString );
	PROCEDURE [ccall] g_strdupv (str_array: PAString ): PAString;
	PROCEDURE [ccall] g_strv_length (str_array: PAString ): INTEGER;
	PROCEDURE [ccall] g_stpcpy (dest: PString; src: PString ): PString;
	PROCEDURE [ccall] g_strip_context (msgid: PString; msgval: PString ): PString;

	(*  gunicode.h *)
	PROCEDURE [ccall] g_get_charset (OUT charset: PString ): gboolean;
	PROCEDURE [ccall] g_unicode_canonical_ordering (string: PUString; len: INTEGER );
	PROCEDURE [ccall] g_unicode_canonical_decomposition (ch: gunichar; result_len: INTEGER ): PUString;
	PROCEDURE [ccall] g_unichar_break_type (c: gunichar ): GUnicodeBreakType;
	PROCEDURE [ccall] g_unichar_validate (ch: gunichar ): gboolean;
	PROCEDURE [ccall] g_unichar_isalnum (c: gunichar ): gboolean;
	PROCEDURE [ccall] g_unichar_isalpha (c: gunichar ): gboolean;
	PROCEDURE [ccall] g_unichar_iscntrl (c: gunichar ): gboolean;
	PROCEDURE [ccall] g_unichar_isdigit (c: gunichar ): gboolean;
	PROCEDURE [ccall] g_unichar_isgraph (c: gunichar ): gboolean;
	PROCEDURE [ccall] g_unichar_islower (c: gunichar ): gboolean;
	PROCEDURE [ccall] g_unichar_isprint (c: gunichar ): gboolean;
	PROCEDURE [ccall] g_unichar_ispunct (c: gunichar ): gboolean;
	PROCEDURE [ccall] g_unichar_isspace (c: gunichar ): gboolean;
	PROCEDURE [ccall] g_unichar_isupper (c: gunichar ): gboolean;
	PROCEDURE [ccall] g_unichar_isxdigit (c: gunichar ): gboolean;
	PROCEDURE [ccall] g_unichar_istitle (c: gunichar ): gboolean;
	PROCEDURE [ccall] g_unichar_isdefined (c: gunichar ): gboolean;
	PROCEDURE [ccall] g_unichar_iswide (c: gunichar ): gboolean;
	PROCEDURE [ccall] g_unichar_type (c: gunichar ): GUnicodeType;

	PROCEDURE [ccall] g_unichar_toupper (c: gunichar ): gunichar;
	PROCEDURE [ccall] g_unichar_tolower (c: gunichar ): gunichar;
	PROCEDURE [ccall] g_unichar_totitle (c: gunichar ): gunichar;

	PROCEDURE [ccall] g_unichar_digit_value (c: gunichar ): INTEGER;
	PROCEDURE [ccall] g_unichar_xdigit_value (c: gunichar ): INTEGER;
	PROCEDURE [ccall] g_unichar_to_utf8 (c: gunichar; OUT outbuf: String ): INTEGER; (* outbuf must have at least 6 bytes of space.*)
	PROCEDURE [ccall] g_unichar_get_mirror_char (ch: gunichar; OUT mirrored_ch: gunichar): gboolean;

	PROCEDURE [ccall] g_utf8_validate (str: PString; max_len: INTEGER; OUT end: PString): gboolean;
	PROCEDURE [ccall] g_utf8_strup (str: PString; len: INTEGER ): PString;
	PROCEDURE [ccall] g_utf8_strdown (str: PString; len: INTEGER ): PString;
	PROCEDURE [ccall] g_utf8_casefold (str: PString; len: INTEGER ): PString;
	PROCEDURE [ccall] g_utf8_normalize (str: PString; len: INTEGER; mode: GNormalizeMode ): PString;
	PROCEDURE [ccall] g_utf8_collate (str1: PString; str2: PString ): INTEGER;
	PROCEDURE [ccall] g_utf8_collate_key (str: PString; len: INTEGER ): PString;
	PROCEDURE [ccall] g_utf8_collate_key_for_filename (str: PString; len: INTEGER ): PString;

	PROCEDURE [ccall] g_utf8_get_char (p: PString ): gunichar;
	PROCEDURE [ccall] g_utf8_get_char_validated (p: PString; max_len: INTEGER ): gunichar;
	PROCEDURE [ccall] g_utf8_offset_to_pointer (str: PString; offset: INTEGER ): PString;
	PROCEDURE [ccall] g_utf8_pointer_to_offset (str: PString; pos: PString ): INTEGER;
	PROCEDURE [ccall] g_utf8_prev_char (p: PString ): PString;
	PROCEDURE [ccall] g_utf8_find_next_char (p: PString; end: PString ): PString;
	PROCEDURE [ccall] g_utf8_find_prev_char (str: PString; p: PString ): PString;
	PROCEDURE [ccall] g_utf8_strlen (p: PString; max: INTEGER ): INTEGER;
	PROCEDURE [ccall] g_utf8_strncpy (dest, src: PString; n: INTEGER ): PString;
	PROCEDURE [ccall] g_utf8_strchr (p: PString; len: INTEGER; c: gunichar ): PString;
	PROCEDURE [ccall] g_utf8_strrchr (p: PString; len: INTEGER; c: gunichar ): PString;
	PROCEDURE [ccall] g_utf8_strreverse (str: PString; len: INTEGER ): PString;
	
	PROCEDURE [ccall] g_utf8_to_utf16 (str: PString; len: INTEGER; OUT items_read, items_written: INTEGER; OUT error: GError): PWString;
	PROCEDURE [ccall] g_utf8_to_ucs4 (str: PString; len: INTEGER; OUT items_read, items_written: INTEGER; OUT error: GError ): PUString;
	PROCEDURE [ccall] g_utf8_to_ucs4_fast (str: PString; len: INTEGER;OUT items_written: INTEGER): PUString;
	PROCEDURE [ccall] g_ucs4_to_utf8 (str: PUString; len: INTEGER; OUT items_read, items_written: INTEGER;OUT error: GError ): PString;
	PROCEDURE [ccall] g_utf16_to_utf8* (IN str: WString; len: INTEGER; OUT [nil] items_read,items_written: INTEGER; OUT [nil] error: GError ): PString;
	PROCEDURE [ccall] g_utf16_to_ucs4 (str: PWString; len: INTEGER; OUT items_read, items_written: INTEGER;OUT error: GError ): PUString;
	PROCEDURE [ccall] g_ucs4_to_utf16 (str: PUString; len: INTEGER;OUT items_read, items_written: INTEGER;OUT error: GError ): PWString;

	(*  gconvert.h *)
	PROCEDURE [ccall] g_convert_error_quark (): GQuark;
	PROCEDURE [ccall] g_iconv_open (to_codeset, from_codeset: PString ): GIConv;
	PROCEDURE [ccall] g_iconv_close (converter: GIConv ): INTEGER;
	PROCEDURE [ccall] g_iconv (converter: GIConv; VAR inbuf: PString; VAR inbytes_left: INTEGER; VAR outbuf: PString; VAR outbytes_left: INTEGER ): INTEGER;
	PROCEDURE [ccall] g_convert (str: PString; len: INTEGER; to,from: PString; OUT bytes_read, bytes_written: INTEGER; OUT error: GError ): PString;
	PROCEDURE [ccall] g_convert_with_iconv (str: PString; len: INTEGER;converter: GIConv; OUT bytes_read, bytes_written: INTEGER; OUT error: GError ): PString;

	PROCEDURE [ccall] g_convert_with_fallback (str: PString; len: INTEGER; to, from, fallback: PString; OUT bytes_read, bytes_written: INTEGER; OUT error: GError ): PString;

	PROCEDURE [ccall] g_locale_to_utf8* (opsysstring: PString; len: INTEGER; OUT [nil] bytes_read, bytes_written: INTEGER; OUT [nil] error: GError ): PString;
	PROCEDURE [ccall] g_locale_from_utf8 (utf8string: PString; len: INTEGER; OUT bytes_read, bytes_written: INTEGER;OUT error: GError ): PString;

	PROCEDURE [ccall] g_filename_to_utf8 (opsysstring: PString; len: INTEGER; OUT bytes_read, bytes_written: INTEGER; OUT error: GError ): PString;
	PROCEDURE [ccall] g_filename_from_utf8 (utf8string: PString; len: INTEGER; OUT bytes_read, bytes_written: INTEGER; OUT error: GError ): PString;
	PROCEDURE [ccall] g_filename_from_uri (uri: PString; OUT [nil] hostname: PString; OUT error: GError ): PString;
	PROCEDURE [ccall] g_filename_to_uri (filename: PString; hostname: PString; OUT error: GError ): PString;
	PROCEDURE [ccall] g_filename_display_name (filename: PString ): PString;
	PROCEDURE [ccall] g_filename_display_basename (filename: PString ): PString;
	PROCEDURE [ccall] g_get_filename_charsets (OUT charsets: PAString ): gboolean;
	PROCEDURE [ccall] g_uri_list_extract_uris (uri_list: PString ): PAString; (* g_strfreev *)


(* ghook.h *)
	PROCEDURE [ccall] g_hook_list_init (hook_list: GHookList; hook_size: INTEGER );
	PROCEDURE [ccall] g_hook_list_clear (hook_list: GHookList );
	PROCEDURE [ccall] g_hook_alloc (hook_list: GHookList ): GHook;
	PROCEDURE [ccall] g_hook_free (hook_list: GHookList; hook: GHook );
	PROCEDURE [ccall] g_hook_ref (hook_list: GHookList; hook: GHook ): GHook;
	PROCEDURE [ccall] g_hook_unref (hook_list: GHookList; hook: GHook );
	PROCEDURE [ccall] g_hook_destroy (hook_list: GHookList;hook_id: INTEGER ): gboolean;
	PROCEDURE [ccall] g_hook_destroy_link (hook_list: GHookList; hook: GHook );
	PROCEDURE [ccall] g_hook_prepend (hook_list: GHookList; hook: GHook );
	PROCEDURE [ccall] g_hook_insert_before (hook_list: GHookList; sibling: GHook;hook: GHook );
	PROCEDURE [ccall] g_hook_insert_sorted (hook_list: GHookList; hook: GHook;func: GHookCompareFunc );
	PROCEDURE [ccall] g_hook_get (hook_list: GHookList;hook_id: INTEGER ): GHook;
	PROCEDURE [ccall] g_hook_find (hook_list: GHookList; need_valids: gboolean; func: GHookFindFunc; data: gpointer ): GHook;
	PROCEDURE [ccall] g_hook_find_data (hook_list: GHookList;need_valids: gboolean;data: gpointer ): GHook;
	PROCEDURE [ccall] g_hook_find_func (hook_list: GHookList;need_valids: gboolean;func: gpointer ): GHook;
	PROCEDURE [ccall] g_hook_find_func_data (hook_list: GHookList;need_valids: gboolean; func: gpointer;data: gpointer ): GHook;
	PROCEDURE [ccall] g_hook_first_valid (hook_list: GHookList;may_be_in_call: gboolean ): GHook;
	PROCEDURE [ccall] g_hook_next_valid (hook_list: GHookList; hook: GHook; may_be_in_call: gboolean ): GHook;
	PROCEDURE [ccall] g_hook_compare_ids (new_hook: GHook; sibling: GHook ): INTEGER;
	PROCEDURE [ccall] g_hook_list_invoke (hook_list: GHookList; may_recurse: gboolean );
	PROCEDURE [ccall] g_hook_list_invoke_check (hook_list: GHookList; may_recurse: gboolean );
	PROCEDURE [ccall] g_hook_list_marshal (hook_list: GHookList; may_recurse: gboolean; marshaller: GHookMarshaller;marshal_data: gpointer );
	PROCEDURE [ccall] g_hook_list_marshal_check (hook_list: GHookList;may_recurse: gboolean; marshaller: GHookCheckMarshaller;marshal_data: gpointer );

(*  gerror.h  *)
	PROCEDURE [ccall] g_error_new_literal (domain: GQuark; code: INTEGER; message: PString ): GError;
	PROCEDURE [ccall] g_error_free (error: GError );
	PROCEDURE [ccall] g_error_copy (error: GError ): GError;
	PROCEDURE [ccall] g_error_matches (error: GError; domain: GQuark; code: INTEGER ): gboolean;


	(*  gutils.h *)
	(*  The returned strings are newly allocated with g_malloc()  *)
	PROCEDURE [ccall] g_get_user_name (): PString;
	PROCEDURE [ccall] g_get_real_name (): PString;
	PROCEDURE [ccall] g_get_home_dir (): PString;
	PROCEDURE [ccall] g_get_tmp_dir (): PString;
	PROCEDURE [ccall] g_get_host_name (): PString;
	PROCEDURE [ccall] g_get_prgname (): PString;
	PROCEDURE [ccall] g_set_prgname (prgname: PString );
	PROCEDURE [ccall] g_get_application_name (): PString;
	PROCEDURE [ccall] g_set_application_name (application_name: PString );
	PROCEDURE [ccall] g_get_user_data_dir (): PString;
	PROCEDURE [ccall] g_get_user_config_dir (): PString;
	PROCEDURE [ccall] g_get_user_cache_dir (): PString;

	PROCEDURE [ccall] g_get_current_dir (): PString;
	PROCEDURE [ccall] g_path_get_basename (file_name: PString ): PString;
	PROCEDURE [ccall] g_path_get_dirname (file_name: PString ): PString;

	PROCEDURE [ccall] g_path_is_absolute (file_name: PString ): gboolean;
	PROCEDURE [ccall] g_path_skip_root (file_name: PString ): PString;

	PROCEDURE [ccall] g_get_system_data_dirs (): PAString;
	PROCEDURE [ccall] g_get_system_config_dirs (): PAString;
	PROCEDURE [ccall] g_get_language_names (): PAString;

	PROCEDURE [ccall] g_getenv (variable: PString ): PString;
	PROCEDURE [ccall] g_setenv (variable: PString; value: PString; overwrite: gboolean ): gboolean;
	PROCEDURE [ccall] g_unsetenv (variable: PString );
	PROCEDURE [ccall] g_listenv (): PAString;

	PROCEDURE [ccall] g_nullify_pointer (VAR nullify_location: gpointer );

	(*  gstdio.h *)
	PROCEDURE [ccall] g_access (filename: PString; mode: INTEGER ): INTEGER;
	PROCEDURE [ccall] g_chmod (filename: PString; mode: INTEGER ): INTEGER;
	PROCEDURE [ccall] g_open (filename: PString; flags: INTEGER; mode: INTEGER ): INTEGER;
	PROCEDURE [ccall] g_creat (filename: PString; mode: INTEGER ): INTEGER;
	PROCEDURE [ccall] g_rename (oldfilename: PString; newfilename: PString ): INTEGER;
	PROCEDURE [ccall] g_mkdir (filename: PString; mode: INTEGER ): INTEGER;
	PROCEDURE [ccall] g_chdir (path: PString ): INTEGER;
	(*  gstdio.h *)
	PROCEDURE [ccall] g_stat (filename: PString; OUT buf: stat ): INTEGER;
	PROCEDURE [ccall] g_lstat (filename: PString; OUT buf: stat ): INTEGER;
	PROCEDURE [ccall] g_unlink (filename: PString ): INTEGER;
	PROCEDURE [ccall] g_remove (filename: PString ): INTEGER;
	PROCEDURE [ccall] g_rmdir (filename: PString ): INTEGER;

	(*  PROCEDURE [ccall] g_fopen   (filename: PString; mode: PString):PFILE; *)
	(* PROCEDURE [ccall] g_freopen (filename: PString; mode: PString; stream:PFILE):PFILE; *)

	(*  gfileutils.h *)
	PROCEDURE [ccall] g_file_error_quark (): GQuark;
	PROCEDURE [ccall] g_file_error_from_errno (err_no: INTEGER ): GFileError;
	PROCEDURE [ccall] g_file_test (filename: PString; test: GFileTest ): gboolean;

	PROCEDURE [ccall] g_file_get_contents (filename: PString; OUT contents: PString;OUT length: INTEGER; OUT error: GError): gboolean;
	PROCEDURE [ccall] g_file_set_contents (filename: PString; contents: PString;length: INTEGER; OUT error: GError ): gboolean;
	PROCEDURE [ccall] g_file_read_link (filename: PString;OUT error: GError ): PString;
	PROCEDURE [ccall] g_mkstemp (tmpl: PString ): INTEGER;
	PROCEDURE [ccall] g_file_open_tmp (tmpl: PString; OUT name_used: PString; OUT error: GError ): INTEGER;
	PROCEDURE [ccall] g_build_pathv (separator: PString; args: PAString ): PString;
	PROCEDURE [ccall] g_build_filenamev (args: PAString ): PString;
	PROCEDURE [ccall] g_mkdir_with_parents (pathname: PString; mode: INTEGER ): INTEGER;

	(* gdataset.h *)
	PROCEDURE [ccall] g_datalist_init (VAR datalist: GData );
	PROCEDURE [ccall] g_datalist_clear (VAR datalist: GData );
	PROCEDURE [ccall] g_datalist_id_get_data (VAR datalist: GData;key_id: GQuark ): gpointer;
	PROCEDURE [ccall] g_datalist_id_set_data_full (VAR datalist: GData; key_id: GQuark; data: gpointer;
                                         destroy_func: GDestroyNotify );

	PROCEDURE [ccall] g_datalist_id_remove_no_notify (VAR datalist: GData; key_id: GQuark ): gpointer;
	PROCEDURE [ccall] g_datalist_foreach (VAR datalist: GData; func: GDataForeachFunc;user_data: gpointer );
	PROCEDURE [ccall] g_datalist_set_flags (VAR datalist: GData; flags: INTEGER );
	PROCEDURE [ccall] g_datalist_unset_flags (VAR datalist: GData; flags: INTEGER );
	PROCEDURE [ccall] g_datalist_get_flags (VAR datalist: GData ): INTEGER;

	(*  Location Associated Keyed Data  *)
	PROCEDURE [ccall] g_dataset_destroy (dataset_location: gpointer );
	PROCEDURE [ccall] g_dataset_id_get_data (dataset_location: gpointer;key_id: GQuark ): gpointer;
	PROCEDURE [ccall] g_dataset_id_set_data_full (dataset_location: gpointer;key_id: GQuark; data: gpointer;
                                        destroy_func: GDestroyNotify );
	PROCEDURE [ccall] g_dataset_id_remove_no_notify (dataset_location: gpointer; key_id: GQuark ): gpointer;
	PROCEDURE [ccall] g_dataset_foreach (dataset_location: gpointer; func: GDataForeachFunc; user_data: gpointer );


	(*  gdir.h *)
	PROCEDURE [ccall] g_dir_open (path: PString; flags: INTEGER; OUT error: GError ): GDir;
	PROCEDURE [ccall] g_dir_read_name (dir: GDir ): PString;
	PROCEDURE [ccall] g_dir_rewind (dir: GDir );
	PROCEDURE [ccall] g_dir_close (dir: GDir );

	(*  gmappedfile.h *)
	PROCEDURE [ccall] g_mapped_file_new (filename: PString; writable: gboolean; OUT error: GError ): GMappedFile;
	PROCEDURE [ccall] g_mapped_file_get_length (file: GMappedFile ): INTEGER;
	PROCEDURE [ccall] g_mapped_file_get_contents (file: GMappedFile ): PString;
	PROCEDURE [ccall] g_mapped_file_free (file: GMappedFile );

	(*  gkeyfile.h *)

	PROCEDURE [ccall] g_key_file_new (): GKeyFile;
	PROCEDURE [ccall] g_key_file_free (key_file: GKeyFile );
	PROCEDURE [ccall] g_key_file_set_list_separator (key_file: GKeyFile;separator: gchar );

	PROCEDURE [ccall] g_key_file_load_from_file (key_file: GKeyFile; file: PString;
                                       flags: GKeyFileFlags;
                                       OUT error: GError ): gboolean;

	PROCEDURE [ccall] g_key_file_load_from_data (key_file: GKeyFile; data: PString;
                                       length: INTEGER; flags: GKeyFileFlags;
                                       OUT error: GError ): gboolean;

	PROCEDURE [ccall] g_key_file_load_from_data_dirs (key_file: GKeyFile;
                                            file: PString;
                                            OUT full_path: PString;
                                            flags: GKeyFileFlags;
                                            OUT error: GError ): gboolean;

	PROCEDURE [ccall] g_key_file_to_data (key_file: GKeyFile; OUT length: INTEGER; OUT error: GError ): PString;

	PROCEDURE [ccall] g_key_file_get_start_group (key_file: GKeyFile ): PString;
	PROCEDURE [ccall] g_key_file_get_groups (key_file: GKeyFile; OUT length: INTEGER ): PAString;

	PROCEDURE [ccall] g_key_file_get_keys (key_file: GKeyFile; group_name: PString;
                                 OUT length: INTEGER;
                                 OUT error: GError ): PAString;

	PROCEDURE [ccall] g_key_file_has_group (key_file: GKeyFile;
                                  group_name: PString ): gboolean;

	PROCEDURE [ccall] g_key_file_has_key (key_file: GKeyFile; group_name: PString;
                                key: PString;
                                OUT error: GError ): gboolean;

	PROCEDURE [ccall] g_key_file_get_value (key_file: GKeyFile; group_name: PString;
                                  key: PString;
                                  OUT error: GError ): PString;

	PROCEDURE [ccall] g_key_file_set_value (key_file: GKeyFile; group_name: PString;
                                  key: PString; value: PString );

	PROCEDURE [ccall] g_key_file_get_string (key_file: GKeyFile; group_name: PString;
                                   key: PString;
                                   OUT error: GError ): PString;

	PROCEDURE [ccall] g_key_file_set_string (key_file: GKeyFile; group_name: PString;
                                   key: PString; string: PString );

	PROCEDURE [ccall] g_key_file_get_locale_string (key_file: GKeyFile;
                                          group_name: PString; key: PString;
                                          locale: PString;
                                          OUT error: GError ): PString;

	PROCEDURE [ccall] g_key_file_set_locale_string (key_file: GKeyFile;
                                          group_name: PString; key: PString;
                                          locale: PString; string: PString );

	PROCEDURE [ccall] g_key_file_get_boolean (key_file: GKeyFile;
                                    group_name: PString; key: PString;
                                    OUT error: GError ): gboolean;

	PROCEDURE [ccall] g_key_file_set_boolean (key_file: GKeyFile;
                                    group_name: PString; key: PString;
                                    value: gboolean );

	PROCEDURE [ccall] g_key_file_get_integer (key_file: GKeyFile;
                                    group_name: PString; key: PString;
                                    OUT error: GError ): INTEGER;

	PROCEDURE [ccall] g_key_file_set_integer (key_file: GKeyFile;
                                    group_name: PString; key: PString;
                                    value: INTEGER );

	PROCEDURE [ccall] g_key_file_get_string_list (key_file: GKeyFile;
                                        group_name: PString; key: PString;
                                        OUT length: INTEGER;
                                        OUT error: GError ): PAString;

	PROCEDURE [ccall] g_key_file_get_boolean_list (key_file: GKeyFile;
                                         group_name: PString; key: PString;
                                         OUT length: INTEGER;
                                         OUT error: GError ): POINTER TO ARRAY [untagged] OF gboolean;

	PROCEDURE [ccall] g_key_file_set_boolean_list (key_file: GKeyFile;
                                         group_name: PString; key: PString;
                                         list: ARRAY OF gboolean;
                                         length: INTEGER );

	PROCEDURE [ccall] g_key_file_get_integer_list (key_file: GKeyFile;
                                         group_name: PString; key: PString;
                                         OUT length: INTEGER;
                                         OUT error: GError ): POINTER TO ARRAY [untagged] OF gboolean;

	PROCEDURE [ccall] g_key_file_set_integer_list (key_file: GKeyFile;
                                         group_name: PString; key: PString;
                                         list: ARRAY OF INTEGER;
                                         length: INTEGER );

	PROCEDURE [ccall] g_key_file_set_comment (key_file: GKeyFile;
                                    group_name: PString; key: PString;
                                    comment: PString; OUT error: GError );

	PROCEDURE [ccall] g_key_file_get_comment (key_file: GKeyFile;
                                    group_name: PString; key: PString;
                                    OUT error: GError ): PString;

	PROCEDURE [ccall] g_key_file_remove_comment (key_file: GKeyFile;
                                       group_name: PString; key: PString;
                                       OUT error: GError );

	PROCEDURE [ccall] g_key_file_remove_key (key_file: GKeyFile; group_name: PString;
                                   key: PString; OUT error: GError );

	PROCEDURE [ccall] g_key_file_remove_group (key_file: GKeyFile;
                                     group_name: PString;
                                     OUT error: GError );

	PROCEDURE [ccall] g_key_file_set_string_list (key_file: GKeyFile;
                                        group_name: PString; key: PString;
                                        list: ARRAY OF PString;
                                        length: INTEGER );

	PROCEDURE [ccall] g_key_file_get_locale_string_list (key_file: GKeyFile;
                                               group_name: PString;
                                               key: PString; locale: PString;
                                               OUT length: INTEGER;
                                               OUT error: GError ): PAString;

	PROCEDURE [ccall] g_key_file_set_locale_string_list (key_file: GKeyFile;
                                               group_name: PString;
                                               key: PString; locale: PString;
                                               list: ARRAY OF PString;
                                               length: INTEGER );


	(* gquark.h *)
	PROCEDURE [ccall] g_quark_try_string (string: PString ): GQuark;
	PROCEDURE [ccall] g_quark_from_static_string (string: PString ): GQuark;
	PROCEDURE [ccall] g_quark_from_string (string: PString ): GQuark;
	PROCEDURE [ccall] g_quark_to_string (quark: GQuark ): PString;

	(* gcache.h *)
	PROCEDURE [ccall] g_cache_new (value_new_func: GCacheNewFunc;
                         value_destroy_func: GCacheDestroyFunc;
                         key_dup_func: GCacheDupFunc;
                         key_destroy_func: GCacheDestroyFunc;
                         hash_key_func: GHashFunc;
                         hash_value_func: GHashFunc;
                         key_equal_func: GEqualFunc ): GCache;

	PROCEDURE [ccall] g_cache_destroy (cache: GCache );
	PROCEDURE [ccall] g_cache_insert (cache: GCache; key: gpointer ): gpointer;
	PROCEDURE [ccall] g_cache_remove (cache: GCache; value: gpointer );
	PROCEDURE [ccall] g_cache_key_foreach (cache: GCache; func: GHFunc; user_data: gpointer );
	PROCEDURE [ccall] g_cache_value_foreach (cache: GCache; func: GHFunc; user_data: gpointer );



	(* garray.h *)
	PROCEDURE [ccall] g_array_new (zero_terminated: gboolean; clear: gboolean; element_size: INTEGER ): GArray;
	PROCEDURE [ccall] g_array_sized_new (zero_terminated: gboolean; clear: gboolean; element_size, reserved_size: INTEGER ): GArray;
	PROCEDURE [ccall] g_array_free (array: GArray; free_segment: gboolean ): PString;
	PROCEDURE [ccall] g_array_append_vals (array: GArray; data: gpointer; len: INTEGER ): GArray;
	PROCEDURE [ccall] g_array_prepend_vals (array: GArray; data: gpointer;len: INTEGER ): GArray;
	PROCEDURE [ccall] g_array_insert_vals (array: GArray; index: INTEGER;data: gpointer;len: INTEGER ): GArray;
	PROCEDURE [ccall] g_array_set_size (array: GArray;length: INTEGER ): GArray;
	PROCEDURE [ccall] g_array_remove_index (array: GArray;index: INTEGER ): GArray;
	PROCEDURE [ccall] g_array_remove_index_fast (array: GArray;index: INTEGER ): GArray;
	PROCEDURE [ccall] g_array_remove_range (array: GArray; index: INTEGER;length: INTEGER ): GArray;

	PROCEDURE [ccall] g_array_sort (array: GArray; compare_func: GCompareFunc );
	PROCEDURE [ccall] g_array_sort_with_data (array: GArray;compare_func: GCompareDataFunc; user_data: gpointer );




	PROCEDURE [ccall] g_ptr_array_new (): GPtrArray;
	PROCEDURE [ccall] g_ptr_array_sized_new (reserved_size: INTEGER ): GPtrArray;
	PROCEDURE [ccall] g_ptr_array_free (array: GPtrArray; free_seg: gboolean ): gpointer;
	PROCEDURE [ccall] g_ptr_array_set_size (array: GPtrArray; length: INTEGER );
	PROCEDURE [ccall] g_ptr_array_remove_index (array: GPtrArray; index: INTEGER ): gpointer;
	PROCEDURE [ccall] g_ptr_array_remove_index_fast (array: GPtrArray; index: INTEGER ): gpointer;
	PROCEDURE [ccall] g_ptr_array_remove (array: GPtrArray; data: gpointer ): gboolean;
	PROCEDURE [ccall] g_ptr_array_remove_fast (array: GPtrArray; data: gpointer ): gboolean;
	PROCEDURE [ccall] g_ptr_array_remove_range (array: GPtrArray; index: INTEGER; length: INTEGER );
	PROCEDURE [ccall] g_ptr_array_add (array: GPtrArray; data: gpointer );
	PROCEDURE [ccall] g_ptr_array_sort (array: GPtrArray; compare_func: GCompareFunc );
	PROCEDURE [ccall] g_ptr_array_sort_with_data (array: GPtrArray; compare_func: GCompareDataFunc; user_data: gpointer);
	PROCEDURE [ccall] g_ptr_array_foreach (array: GPtrArray; func: GFunc; user_data: gpointer );




	PROCEDURE [ccall] g_byte_array_new (): GByteArray;

	PROCEDURE [ccall] g_byte_array_sized_new (reserved_size: INTEGER ): GByteArray;

	PROCEDURE [ccall] g_byte_array_free (array: GByteArray; free_segment: gboolean ): gpointer;

	PROCEDURE [ccall] g_byte_array_append (array: GByteArray; IN data: ARRAY [untagged] OF BYTE; len: INTEGER ): GByteArray;
	PROCEDURE [ccall] g_byte_array_prepend (array: GByteArray; IN data: ARRAY [untagged] OF BYTE; len: INTEGER ): GByteArray;
	PROCEDURE [ccall] g_byte_array_set_size (array: GByteArray; length: INTEGER ): GByteArray;
	PROCEDURE [ccall] g_byte_array_remove_index (array: GByteArray; index: INTEGER ): GByteArray;
	PROCEDURE [ccall] g_byte_array_remove_index_fast (array: GByteArray; index: INTEGER ): GByteArray;
	PROCEDURE [ccall] g_byte_array_remove_range (array: GByteArray; index: INTEGER; length: INTEGER ): GByteArray;
	PROCEDURE [ccall] g_byte_array_sort (array: GByteArray; compare_func: GCompareFunc );
	PROCEDURE [ccall] g_byte_array_sort_with_data (array: GByteArray;compare_func: GCompareDataFunc; user_data: gpointer );

	(* gstring.h *)

	(*  String Chunks  *)

	PROCEDURE [ccall] g_string_chunk_new (size: INTEGER ): GStringChunk;
	PROCEDURE [ccall] g_string_chunk_free (chunk: GStringChunk );
	PROCEDURE [ccall] g_string_chunk_insert (chunk: GStringChunk; string: PString ): PString;
	PROCEDURE [ccall] g_string_chunk_insert_len (chunk: GStringChunk; string: PString;len: INTEGER ): PString;
	PROCEDURE [ccall] g_string_chunk_insert_const (chunk: GStringChunk; string: PString ): PString;

	(*  Strings  *)

	PROCEDURE [ccall] g_string_new (init: PString ): GString;
	PROCEDURE [ccall] g_string_new_len (init: PString; len: INTEGER ): GString;
	PROCEDURE [ccall] g_string_sized_new (dfl_size: INTEGER ): GString;
	PROCEDURE [ccall] g_string_free (string: GString;free_segment: gboolean ): PString;
	PROCEDURE [ccall] g_string_equal (v, v2: GString ): gboolean;
	PROCEDURE [ccall] g_string_hash (str: GString ): INTEGER;
	PROCEDURE [ccall] g_string_assign (string: GString;rval: PString ): GString;
	PROCEDURE [ccall] g_string_truncate (string: GString; len: INTEGER ): GString;
	PROCEDURE [ccall] g_string_set_size (string: GString;len: INTEGER ): GString;
	PROCEDURE [ccall] g_string_insert_len (string: GString; pos: INTEGER;val: PString; len: INTEGER ): GString;
	PROCEDURE [ccall] g_string_append (string: GString;val: PString ): GString;
	PROCEDURE [ccall] g_string_append_len (string: GString; val: PString;len: INTEGER ): GString;
	PROCEDURE [ccall] g_string_append_c (string: GString;c: gchar ): GString;
	PROCEDURE [ccall] g_string_append_unichar (string: GString;wc: gunichar ): GString;
	PROCEDURE [ccall] g_string_prepend (string: GString;val: PString ): GString;
	PROCEDURE [ccall] g_string_prepend_c (string: GString;c: gchar ): GString;
	PROCEDURE [ccall] g_string_prepend_unichar (string: GString;wc: gunichar ): GString;
	PROCEDURE [ccall] g_string_prepend_len (string: GString; val: PString;len: INTEGER ): GString;
	PROCEDURE [ccall] g_string_insert (string: GString; pos: INTEGER;val: PString ): GString;
	PROCEDURE [ccall] g_string_insert_c (string: GString; pos: INTEGER;c: gchar ): GString;
	PROCEDURE [ccall] g_string_insert_unichar (string: GString; pos: INTEGER;wc: gunichar ): GString;
	PROCEDURE [ccall] g_string_erase (string: GString; pos: INTEGER;len: INTEGER ): GString;
	PROCEDURE [ccall] g_string_ascii_down (string: GString ): GString;
	PROCEDURE [ccall] g_string_ascii_up (string: GString ): GString;
	(* *** *)

	(* glist.h *)
	PROCEDURE [ccall] g_list_alloc (): GList;
	PROCEDURE [ccall] g_list_length (list: GList ): INTEGER;
	PROCEDURE [ccall] g_list_first* (list: GList ): GList;
	PROCEDURE [ccall] g_list_last (list: GList ): GList;
	PROCEDURE [ccall] g_list_free* (list: GList );
	PROCEDURE [ccall] g_list_free_1 (list: GList );
	PROCEDURE [ccall] g_list_append* (list: GList; data: gpointer ): GList;
	PROCEDURE [ccall] g_list_prepend (list: GList; data: gpointer ): GList;
	PROCEDURE [ccall] g_list_insert (list: GList; data: gpointer; position: INTEGER ): GList;
	PROCEDURE [ccall] g_list_insert_before (list: GList; sibling: GList;data: gpointer ): GList;
	PROCEDURE [ccall] g_list_insert_sorted (list: GList; data: gpointer; func: GCompareFunc ): GList;
	PROCEDURE [ccall] g_list_concat (list1: GList; list2: GList ): GList;
	PROCEDURE [ccall] g_list_remove (list: GList; data: gpointer ): GList;
	PROCEDURE [ccall] g_list_remove_all (list: GList; data: gpointer ): GList;
	PROCEDURE [ccall] g_list_remove_link (list: GList; llink: GList ): GList;
	PROCEDURE [ccall] g_list_delete_link (list: GList;link: GList ): GList;
	PROCEDURE [ccall] g_list_reverse (list: GList ): GList;
	PROCEDURE [ccall] g_list_copy (list: GList ): GList;
	PROCEDURE [ccall] g_list_nth (list: GList; n: INTEGER ): GList;
	PROCEDURE [ccall] g_list_nth_prev (list: GList; n: INTEGER ): GList;
	PROCEDURE [ccall] g_list_find (list: GList;data: gpointer ): GList;
	PROCEDURE [ccall] g_list_find_custom (list: GList; data: gpointer; func: GCompareFunc ): GList;
	PROCEDURE [ccall] g_list_position (list: GList; llink: GList ): INTEGER;
	PROCEDURE [ccall] g_list_index (list: GList; data: gpointer ): INTEGER;
	PROCEDURE [ccall] g_list_foreach (list: GList; func: GFunc; user_data: gpointer );
	PROCEDURE [ccall] g_list_sort (list: GList; compare_func: GCompareFunc ): GList;
	PROCEDURE [ccall] g_list_sort_with_data (list: GList; compare_func: GCompareDataFunc; user_data: gpointer ): GList;
	PROCEDURE [ccall] g_list_nth_data (list: GList; n: INTEGER ): gpointer;

	(* gslist.h *)
	PROCEDURE [ccall] g_slist_alloc (): GSList;
	PROCEDURE [ccall] g_slist_free (list: GSList );
	PROCEDURE [ccall] g_slist_free_1 (list: GSList );
	PROCEDURE [ccall] g_slist_append (list: GSList; data: gpointer ): GSList;
	PROCEDURE [ccall] g_slist_prepend (list: GSList; data: gpointer ): GSList;
	PROCEDURE [ccall] g_slist_insert (list: GSList; data: gpointer; position: INTEGER ): GSList;
	PROCEDURE [ccall] g_slist_insert_sorted (list: GSList; data: gpointer; func: GCompareFunc ): GSList;
	PROCEDURE [ccall] g_slist_insert_before (slist: GSList; sibling: GSList; data: gpointer ): GSList;
	PROCEDURE [ccall] g_slist_concat (list1: GSList; list2: GSList ): GSList;
	PROCEDURE [ccall] g_slist_remove (list: GSList; data: gpointer ): GSList;
	PROCEDURE [ccall] g_slist_remove_all (list: GSList; data: gpointer ): GSList;
	PROCEDURE [ccall] g_slist_remove_link (list: GSList; link: GSList ): GSList;
	PROCEDURE [ccall] g_slist_delete_link (list: GSList; link: GSList ): GSList;
	PROCEDURE [ccall] g_slist_reverse (list: GSList ): GSList;
	PROCEDURE [ccall] g_slist_copy (list: GSList ): GSList;
	PROCEDURE [ccall] g_slist_nth (list: GSList; n: INTEGER ): GSList;
	PROCEDURE [ccall] g_slist_find (list: GSList; data: gpointer ): GSList;
	PROCEDURE [ccall] g_slist_find_custom (list: GSList; data: gpointer; func: GCompareFunc ): GSList;
	PROCEDURE [ccall] g_slist_position (list: GSList; llink: GSList ): INTEGER;
	PROCEDURE [ccall] g_slist_index (list: GSList; data: gpointer ): INTEGER;
	PROCEDURE [ccall] g_slist_last (list: GSList ): GSList;
	PROCEDURE [ccall] g_slist_length (list: GSList ): INTEGER;
	PROCEDURE [ccall] g_slist_foreach (list: GSList; func: GFunc; user_data: gpointer );
	PROCEDURE [ccall] g_slist_sort (list: GSList; compare_func: GCompareFunc ): GSList;
	PROCEDURE [ccall] g_slist_sort_with_data (list: GSList; compare_func: GCompareDataFunc; user_data: gpointer ): GSList;
	PROCEDURE [ccall] g_slist_nth_data (list: GSList; n: INTEGER ): gpointer;
	(* gqueue.h *)
	PROCEDURE [ccall] g_queue_new ( ): GQueue;
	PROCEDURE [ccall] g_queue_free (queue: GQueue );
	PROCEDURE [ccall] g_queue_is_empty (queue: GQueue ): gboolean;
	PROCEDURE [ccall] g_queue_get_length (queue: GQueue ): INTEGER;
	PROCEDURE [ccall] g_queue_reverse (queue: GQueue );
	PROCEDURE [ccall] g_queue_copy (queue: GQueue ): GQueue;
	PROCEDURE [ccall] g_queue_foreach (queue: GQueue; func: GFunc; user_data: gpointer );
	PROCEDURE [ccall] g_queue_find (queue: GQueue; data: gpointer ): GList;
	PROCEDURE [ccall] g_queue_find_custom (queue: GQueue; data: gpointer; func: GCompareFunc ): GList;
	PROCEDURE [ccall] g_queue_sort (queue: GQueue; compare_func: GCompareDataFunc; user_data: gpointer );
	PROCEDURE [ccall] g_queue_push_head (queue: GQueue; data: gpointer );
	PROCEDURE [ccall] g_queue_push_tail (queue: GQueue; data: gpointer );
	PROCEDURE [ccall] g_queue_push_nth (queue: GQueue; data: gpointer; n: INTEGER );
	PROCEDURE [ccall] g_queue_pop_head (queue: GQueue ): gpointer;
	PROCEDURE [ccall] g_queue_pop_tail (queue: GQueue ): gpointer;
	PROCEDURE [ccall] g_queue_pop_nth (queue: GQueue; n: INTEGER ): gpointer;
	PROCEDURE [ccall] g_queue_peek_head (queue: GQueue ): gpointer;
	PROCEDURE [ccall] g_queue_peek_tail (queue: GQueue ): gpointer;
	PROCEDURE [ccall] g_queue_peek_nth (queue: GQueue; n: INTEGER ): gpointer;
	PROCEDURE [ccall] g_queue_index (queue: GQueue; data: gpointer ): INTEGER;
	PROCEDURE [ccall] g_queue_remove (queue: GQueue; data: gpointer );
	PROCEDURE [ccall] g_queue_remove_all (queue: GQueue; data: gpointer );
	PROCEDURE [ccall] g_queue_insert_before (queue: GQueue; sibling: GList; data: gpointer );
	PROCEDURE [ccall] g_queue_insert_after (queue: GQueue; sibling: GList; data: gpointer );
	PROCEDURE [ccall] g_queue_insert_sorted (queue: GQueue; data: gpointer; func: GCompareDataFunc; user_data: gpointer );
	PROCEDURE [ccall] g_queue_push_head_link (queue: GQueue; link: GList );
	PROCEDURE [ccall] g_queue_push_tail_link (queue: GQueue; link: GList );
	PROCEDURE [ccall] g_queue_push_nth_link (queue: GQueue; n: INTEGER; link: GList );
	PROCEDURE [ccall] g_queue_pop_head_link (queue: GQueue ): GList;
	PROCEDURE [ccall] g_queue_pop_tail_link (queue: GQueue ): GList;
	PROCEDURE [ccall] g_queue_pop_nth_link (queue: GQueue; n: INTEGER ): GList;
	PROCEDURE [ccall] g_queue_peek_head_link (queue: GQueue ): GList;
	PROCEDURE [ccall] g_queue_peek_tail_link (queue: GQueue ): GList;
	PROCEDURE [ccall] g_queue_peek_nth_link (queue: GQueue; n: INTEGER ): GList;
	PROCEDURE [ccall] g_queue_link_index (queue: GQueue; link: GList ): INTEGER;
	PROCEDURE [ccall] g_queue_unlink (queue: GQueue; link: GList );
	PROCEDURE [ccall] g_queue_delete_link (queue: GQueue; link: GList );

	(* gnode.h *)
	PROCEDURE [ccall] g_node_new (data: gpointer ): GNode;
	PROCEDURE [ccall] g_node_destroy (root: GNode );
	PROCEDURE [ccall] g_node_unlink (node: GNode );
	PROCEDURE [ccall] g_node_copy (node: GNode ): GNode;
	PROCEDURE [ccall] g_node_copy_deep (node: GNode; copy_func: GCopyFunc;data: gpointer ): GNode;
	PROCEDURE [ccall] g_node_insert (parent: GNode; position: INTEGER;node: GNode ): GNode;
	PROCEDURE [ccall] g_node_insert_before (parent: GNode; sibling: GNode;node: GNode ): GNode;
	PROCEDURE [ccall] g_node_insert_after (parent: GNode; sibling: GNode;node: GNode ): GNode;
	PROCEDURE [ccall] g_node_prepend (parent: GNode; node: GNode ): GNode;
	PROCEDURE [ccall] g_node_n_nodes (root: GNode; flags: GTraverseFlags ): INTEGER;
	PROCEDURE [ccall] g_node_get_root (node: GNode ): GNode;
	PROCEDURE [ccall] g_node_is_ancestor (node: GNode;descendant: GNode ): gboolean;
	PROCEDURE [ccall] g_node_depth (node: GNode ): INTEGER;
	PROCEDURE [ccall] g_node_max_height (root: GNode ): INTEGER;
	PROCEDURE [ccall] g_node_find (root: GNode; order: GTraverseType; flags: GTraverseFlags; data: gpointer): GNode;
	PROCEDURE [ccall] g_node_reverse_children (node: GNode );
	PROCEDURE [ccall] g_node_n_children (node: GNode ): INTEGER;
	PROCEDURE [ccall] g_node_nth_child (node: GNode; n: INTEGER ): GNode;
	PROCEDURE [ccall] g_node_last_child (node: GNode ): GNode;
	PROCEDURE [ccall] g_node_find_child (node: GNode; flags: GTraverseFlags;data: gpointer ): GNode;
	PROCEDURE [ccall] g_node_child_position (node: GNode;child: GNode ): INTEGER;
	PROCEDURE [ccall] g_node_child_index (node: GNode;data: gpointer ): INTEGER;
	PROCEDURE [ccall] g_node_first_sibling (node: GNode ): GNode;
	PROCEDURE [ccall] g_node_last_sibling (node: GNode ): GNode;
	PROCEDURE [ccall] g_node_traverse (root: GNode; order: GTraverseType; flags: GTraverseFlags; max_depth: INTEGER;
                             func: GNodeTraverseFunc; data: gpointer );

	PROCEDURE [ccall] g_node_children_foreach (node: GNode; flags: GTraverseFlags; func: GNodeForeachFunc; data: gpointer );

	(* gtree.h *)
	PROCEDURE [ccall] g_tree_new (key_compare_func: GCompareFunc ): GTree;
	PROCEDURE [ccall] g_tree_new_with_data(key_compare_func: GCompareDataFunc; key_compare_data: gpointer ): GTree;

	PROCEDURE [ccall] g_tree_new_full (key_compare_func: GCompareDataFunc; key_compare_data: gpointer; key_destroy_func: GDestroyNotify;value_destroy_func: GDestroyNotify ): GTree;

	PROCEDURE [ccall] g_tree_destroy (tree: GTree );
	PROCEDURE [ccall] g_tree_insert (tree: GTree; key: gpointer; value: gpointer );
	PROCEDURE [ccall] g_tree_replace (tree: GTree; key: gpointer; value: gpointer );
	PROCEDURE [ccall] g_tree_remove (tree: GTree; key: gpointer ): gboolean;
	PROCEDURE [ccall] g_tree_steal (tree: GTree; key: gpointer ): gboolean;
	PROCEDURE [ccall] g_tree_lookup (tree: GTree; key: gpointer ): gpointer;

	PROCEDURE [ccall] g_tree_lookup_extended (tree: GTree; lookup_key: gpointer; OUT orig_key,value: gpointer): gboolean;

	PROCEDURE [ccall] g_tree_foreach (tree: GTree; func: GTraverseFunc;user_data: gpointer );
	PROCEDURE [ccall] g_tree_search (tree: GTree; search_func: GCompareFunc;user_data: gpointer ): gpointer;
	PROCEDURE [ccall] g_tree_height (tree: GTree ): INTEGER;
	PROCEDURE [ccall] g_tree_nnodes (tree: GTree ): INTEGER;

	(* ghash.h *)
	PROCEDURE [ccall] g_hash_table_new (hash_func: GHashFunc; key_equal_func: GEqualFunc ): GHashTable;

	PROCEDURE [ccall] g_hash_table_new_full (hash_func: GHashFunc;
                                   key_equal_func: GEqualFunc;
                                   key_destroy_func: GDestroyNotify;
                                   value_destroy_func: GDestroyNotify ): GHashTable;
	PROCEDURE [ccall] g_hash_table_destroy (hash_table: GHashTable );
	PROCEDURE [ccall] g_hash_table_insert (hash_table: GHashTable; key: gpointer; value: gpointer );
	PROCEDURE [ccall] g_hash_table_replace (hash_table: GHashTable; key: gpointer; value: gpointer );
	PROCEDURE [ccall] g_hash_table_remove (hash_table: GHashTable; key: gpointer ): gboolean;
	PROCEDURE [ccall] g_hash_table_steal (hash_table: GHashTable; key: gpointer ): gboolean;
	PROCEDURE [ccall] g_hash_table_lookup (hash_table: GHashTable; key: gpointer ): gpointer;
	PROCEDURE [ccall] g_hash_table_lookup_extended (hash_table: GHashTable;lookup_key: gpointer;OUT orig_key,value: gpointer ): gboolean;
	PROCEDURE [ccall] g_hash_table_size (hash_table: GHashTable ): INTEGER;
	PROCEDURE [ccall] g_hash_table_find (hash_table: GHashTable; predicate: GHRFunc; user_data: gpointer ): gpointer;
	PROCEDURE [ccall] g_hash_table_foreach (hash_table: GHashTable; func: GHFunc; user_data: gpointer );
	PROCEDURE [ccall] g_hash_table_foreach_remove (hash_table: GHashTable; func: GHRFunc; user_data: gpointer ): INTEGER;
	PROCEDURE [ccall] g_hash_table_foreach_steal(hash_table: GHashTable; func: GHRFunc; user_data: gpointer ): INTEGER;


	(*  Hash Functions  *)
	PROCEDURE [ccall] g_str_hash (v: gpointer ): INTEGER;
	PROCEDURE [ccall] g_int_hash (v: gpointer ): INTEGER;
	PROCEDURE [ccall] g_direct_hash (v: gpointer ): INTEGER;

	PROCEDURE [ccall] g_str_equal (v1,v2: gpointer ): gboolean;
	PROCEDURE [ccall] g_int_equal (v1,v2: gpointer ): gboolean;
	PROCEDURE [ccall] g_direct_equal (v1, v2: gpointer ): gboolean;


	(* grel.h *)
	PROCEDURE [ccall] g_relation_new (fields: INTEGER ): GRelation;
	PROCEDURE [ccall] g_relation_destroy (relation: GRelation );
	PROCEDURE [ccall] g_relation_index (relation: GRelation; field: INTEGER; hash_func: GHashFunc; key_equal_func: GEqualFunc );
	PROCEDURE [ccall] g_relation_delete (relation: GRelation; key: gpointer; field: INTEGER ): INTEGER;
	PROCEDURE [ccall] g_relation_count (relation: GRelation; key: gpointer; field: INTEGER ): INTEGER;
	PROCEDURE [ccall] g_relation_print (relation: GRelation );
	PROCEDURE [ccall] g_relation_select (relation: GRelation; key: gpointer; field: INTEGER ): GTuples;
	PROCEDURE [ccall] g_tuples_destroy (tuples: GTuples );
	PROCEDURE [ccall] g_tuples_index (tuples: GTuples; index: INTEGER;field: INTEGER ): gpointer;

	(*  giochannel.h *)
	PROCEDURE [ccall] g_io_add_watch_full (channel: GIOChannel; priority: INTEGER; condition: GIOCondition; func: GIOFunc; user_data: gpointer; notify: GDestroyNotify ): INTEGER;

	PROCEDURE [ccall] g_io_add_watch (channel: GIOChannel; condition: GIOCondition; func: GIOFunc; user_data: gpointer ): INTEGER;
	PROCEDURE [ccall] g_io_channel_init (channel: GIOChannel );
	PROCEDURE [ccall] g_io_channel_ref (channel: GIOChannel ): GIOChannel;
	PROCEDURE [ccall] g_io_channel_unref (channel: GIOChannel );
	PROCEDURE [ccall] g_io_channel_shutdown (channel: GIOChannel; flush: gboolean; OUT error: GError ): GIOStatus;
	PROCEDURE [ccall] g_io_create_watch (channel: GIOChannel; condition: GIOCondition ): GIConv;

	PROCEDURE [ccall] g_io_channel_new_file (filename: PString; mode: PString; OUT error: GError ): GIOChannel;
	PROCEDURE [ccall] g_io_channel_unix_new (fd: INTEGER ): GIOChannel;
	PROCEDURE [ccall] g_io_channel_unix_get_fd (channel: GIOChannel ): INTEGER;

	PROCEDURE [ccall] g_io_channel_set_buffer_size (channel: GIOChannel; size: INTEGER );
	PROCEDURE [ccall] g_io_channel_get_buffer_size (channel: GIOChannel ): INTEGER;
	PROCEDURE [ccall] g_io_channel_get_buffer_condition (channel: GIOChannel ): GIOCondition;
	PROCEDURE [ccall] g_io_channel_set_flags (channel: GIOChannel; flags: GIOFlags; OUT error: GError ): GIOStatus;
	PROCEDURE [ccall] g_io_channel_get_flags (channel: GIOChannel ): GIOFlags;
	PROCEDURE [ccall] g_io_channel_set_line_term (channel: GIOChannel; line_term: PString; length: INTEGER );
	PROCEDURE [ccall] g_io_channel_get_line_term (channel: GIOChannel; OUT length: INTEGER ): PString;
	PROCEDURE [ccall] g_io_channel_set_buffered (channel: GIOChannel; buffered: gboolean );
	PROCEDURE [ccall] g_io_channel_get_buffered (channel: GIOChannel ): gboolean;
	PROCEDURE [ccall] g_io_channel_set_encoding (channel: GIOChannel;encoding: PString; OUT error: GError ): GIOStatus;
	PROCEDURE [ccall] g_io_channel_get_encoding (channel: GIOChannel ): PString;
	PROCEDURE [ccall] g_io_channel_set_close_on_unref (channel: GIOChannel; do_close: gboolean );
	PROCEDURE [ccall] g_io_channel_get_close_on_unref (channel: GIOChannel ): gboolean;
	PROCEDURE [ccall] g_io_channel_flush (channel: GIOChannel; OUT error: GError ): GIOStatus;

	PROCEDURE [ccall] g_io_channel_read_line (channel: GIOChannel; OUT str_return: PString; OUT length, terminator_pos: INTEGER; OUT error: GError ): GIOStatus;
	PROCEDURE [ccall] g_io_channel_read_line_string (channel: GIOChannel; buffer: GString; OUT terminator_pos: INTEGER; OUT error: GError ): GIOStatus;
	PROCEDURE [ccall] g_io_channel_read_to_end (channel: GIOChannel; OUT str_return: PString; OUT length: INTEGER; OUT error: GError ): GIOStatus;
	PROCEDURE [ccall] g_io_channel_read_chars (channel: GIOChannel; buf: PString; count: INTEGER; OUT bytes_read: INTEGER; OUT error: GError ): GIOStatus;
	PROCEDURE [ccall] g_io_channel_write_chars (channel: GIOChannel; buf: PString;count: INTEGER; OUT bytes_written: INTEGER; OUT error: GError ): GIOStatus;

	PROCEDURE [ccall] g_io_channel_read_unichar (channel: GIOChannel; OUT thechar: INTEGER; OUT error: GError ): GIOStatus;
	PROCEDURE [ccall] g_io_channel_write_unichar (channel: GIOChannel; thechar: gunichar;OUT error: GError ): GIOStatus;

	PROCEDURE [ccall] g_io_channel_read (channel: GIOChannel; buf: PString;count: INTEGER; OUT bytes_read: INTEGER ): GIOError;
	PROCEDURE [ccall] g_io_channel_write (channel: GIOChannel; buf: PString; count: INTEGER;OUT bytes_written: INTEGER ): GIOError;

	PROCEDURE [ccall] g_io_channel_seek (channel: GIOChannel; offset: LONGINT; type: GSeekType ): GIOError;
	PROCEDURE [ccall] g_io_channel_seek_position (channel: GIOChannel; offset: LONGINT; type: GSeekType; OUT error: GError ): GIOStatus;

	PROCEDURE [ccall] g_io_channel_close (channel: GIOChannel );

	(*  Error handling  *)
	PROCEDURE [ccall] g_io_channel_error_quark (): GQuark;
	PROCEDURE [ccall] g_io_channel_error_from_errno (en: INTEGER ): GIOChannelError;

	(*  gprimes.h *)
	PROCEDURE [ccall] g_spaced_primes_closest (num: INTEGER ): INTEGER;

	(*  gshell.h *)
	PROCEDURE [ccall] g_shell_quote (unquoted_string: PString ): PString;
	PROCEDURE [ccall] g_shell_unquote (quoted_string: PString; OUT error: GError ): PString;
	PROCEDURE [ccall] g_shell_parse_argv (command_line: PString; OUT argc: INTEGER; OUT argv: ARRAY [untagged] OF PString; OUT error: GError): gboolean;

	(*  grand.h *)
	PROCEDURE [ccall] g_rand_new_with_seed (seed: INTEGER ): GRand;
	PROCEDURE [ccall] g_rand_new ( ): GRand;
	PROCEDURE [ccall] g_rand_free (rand: GRand );
	PROCEDURE [ccall] g_rand_copy (rand: GRand ): GRand;
	PROCEDURE [ccall] g_rand_set_seed (rand: GRand; seed: INTEGER );
	PROCEDURE [ccall] g_rand_int (rand: GRand ): INTEGER;
	PROCEDURE [ccall] g_rand_int_range (rand: GRand; begin, end: INTEGER ): INTEGER;
	PROCEDURE [ccall] g_rand_double (rand: GRand ): REAL;
	PROCEDURE [ccall] g_rand_double_range (rand: GRand; begin, end: REAL ): REAL;

	PROCEDURE [ccall] g_random_set_seed (seed: INTEGER );
	PROCEDURE [ccall] g_random_int (): INTEGER;
	PROCEDURE [ccall] g_random_int_range (begin,end: INTEGER ): INTEGER;
	PROCEDURE [ccall] g_random_double (): REAL;
	PROCEDURE [ccall] g_random_double_range (begin,end: REAL ): REAL;
	(*
	PROCEDURE [ccall] g_rand_new_with_seed_array (VAR seed: INTEGER; seed_length: INTEGER ): GRand;
	PROCEDURE [ccall] g_rand_set_seed_array (rand: GRand; seed: Ptr_INTEGER; seed_length: INTEGER );
	*)


	(*  gdate.h *)
	PROCEDURE [ccall] g_date_new ( ): GDate;
	PROCEDURE [ccall] g_date_new_dmy (day: GDateDay; month: GDateMonth; year: GDateYear ): GDate;
	PROCEDURE [ccall] g_date_new_julian (julian_day: INTEGER ): GDate;
	PROCEDURE [ccall] g_date_free (date: GDate );

	PROCEDURE [ccall] g_date_valid (date: GDate ): gboolean;
	PROCEDURE [ccall] g_date_valid_day (day: GDateDay ): gboolean;
	PROCEDURE [ccall] g_date_valid_month (month: GDateMonth ): gboolean;
	PROCEDURE [ccall] g_date_valid_year (year: GDateYear ): gboolean;
	PROCEDURE [ccall] g_date_valid_weekday (weekday: GDateWeekday ): gboolean;
	PROCEDURE [ccall] g_date_valid_julian (julian_date: INTEGER ): gboolean;
	PROCEDURE [ccall] g_date_valid_dmy (day: GDateDay; month: GDateMonth; year: GDateYear ): gboolean;

	PROCEDURE [ccall] g_date_get_weekday (date: GDate ): GDateWeekday;
	PROCEDURE [ccall] g_date_get_month (date: GDate ): GDateMonth;
	PROCEDURE [ccall] g_date_get_year (date: GDate ): GDateYear;
	PROCEDURE [ccall] g_date_get_day (date: GDate ): GDateDay;

	PROCEDURE [ccall] g_date_get_julian (date: GDate ): INTEGER;
	PROCEDURE [ccall] g_date_get_day_of_year (date: GDate ): INTEGER;

	PROCEDURE [ccall] g_date_get_monday_week_of_year (date: GDate ): INTEGER;
	PROCEDURE [ccall] g_date_get_sunday_week_of_year (date: GDate ): INTEGER;
	PROCEDURE [ccall] g_date_get_iso8601_week_of_year (date: GDate ): INTEGER;

	(*    If you create a static date struct you need to clear it to get it  *)
	(*  * in a sane state before use. You can clear a whole array at         *)
	(*  * once with the ndates argument.                                     *)
	PROCEDURE [ccall] g_date_clear (date: GDate (* array *); n_dates: INTEGER);
	PROCEDURE [ccall] g_date_set_parse (date: GDate; str: PString );
	PROCEDURE [ccall] g_date_set_month (date: GDate; month: GDateMonth );
	PROCEDURE [ccall] g_date_set_day (date: GDate; day: GDateDay );
	PROCEDURE [ccall] g_date_set_year (date: GDate; year: GDateYear );
	PROCEDURE [ccall] g_date_set_dmy (date: GDate; day: GDateDay; month: GDateMonth;y: GDateYear );
	PROCEDURE [ccall] g_date_set_time (date: GDate; time: GTime );

	PROCEDURE [ccall] g_date_set_julian (date: GDate; julian_date: INTEGER );

	PROCEDURE [ccall] g_date_is_first_of_month (date: GDate ): gboolean;
	PROCEDURE [ccall] g_date_is_last_of_month (date: GDate ): gboolean;

	PROCEDURE [ccall] g_date_add_days (date: GDate; n_days: INTEGER );
	PROCEDURE [ccall] g_date_subtract_days (date: GDate; n_days: INTEGER );
	PROCEDURE [ccall] g_date_add_months (date: GDate; n_months: INTEGER );
	PROCEDURE [ccall] g_date_subtract_months (date: GDate; n_months: INTEGER );
	PROCEDURE [ccall] g_date_add_years (date: GDate; n_years: INTEGER );
	PROCEDURE [ccall] g_date_subtract_years (date: GDate; n_years: INTEGER );

	PROCEDURE [ccall] g_date_is_leap_year (year: GDateYear ): gboolean;
	PROCEDURE [ccall] g_date_get_days_in_month (month: GDateMonth; year: GDateYear ): BYTE;
	PROCEDURE [ccall] g_date_get_monday_weeks_in_year(year: GDateYear ): BYTE;
	PROCEDURE [ccall] g_date_get_sunday_weeks_in_year(year: GDateYear ): BYTE;

	PROCEDURE [ccall] g_date_days_between (date1,date2: GDate ): INTEGER;

	PROCEDURE [ccall] g_date_to_struct_tm (date: GDate; tm: Ptr_tm );
	PROCEDURE [ccall] g_date_clamp (date: GDate; min_date, max_date: GDate );
	PROCEDURE [ccall] g_date_order (date1, date2: GDate );

	PROCEDURE [ccall] g_date_strftime (s: PString; slen: INTEGER; format: PString; date: GDate): INTEGER;

	(*  qsort-friendly (with a cast...)  *)
	PROCEDURE [ccall] g_date_compare (lhs, rhs: GDate ): INTEGER;

	(*  gtimer.h *)
	PROCEDURE [ccall] g_timer_new (): GTimer;
	PROCEDURE [ccall] g_timer_destroy (timer: GTimer );
	PROCEDURE [ccall] g_timer_start (timer: GTimer );
	PROCEDURE [ccall] g_timer_stop (timer: GTimer );
	PROCEDURE [ccall] g_timer_reset (timer: GTimer );
	PROCEDURE [ccall] g_timer_continue (timer: GTimer );
	PROCEDURE [ccall] g_timer_elapsed (timer: GTimer; OUT microseconds: INTEGER ): REAL;

	PROCEDURE [ccall] g_usleep (microseconds: INTEGER );
	PROCEDURE [ccall] g_time_val_add (VAR time: GTimeVal; microseconds: INTEGER);

	(* gatomic.h *)

	PROCEDURE [ccall] g_atomic_int_exchange_and_add (VAR atomic: INTEGER; val: INTEGER ): INTEGER;
	PROCEDURE [ccall] g_atomic_int_add (VAR atomic: INTEGER; val: INTEGER );
	PROCEDURE [ccall] g_atomic_int_compare_and_exchange (VAR atomic: INTEGER;oldval: INTEGER;newval: INTEGER ): gboolean;
	PROCEDURE [ccall] g_atomic_int_get (VAR atomic: INTEGER ): INTEGER;

	PROCEDURE [ccall] g_atomic_pointer_compare_and_exchange (VAR  atomic: gpointer;oldval,newval: gpointer ): gboolean;
	PROCEDURE [ccall] g_atomic_pointer_get (VAR atomic: gpointer ): gpointer;




	(* gmarkup.h *)
	PROCEDURE [ccall] g_markup_error_quark ( ): GQuark;

	PROCEDURE [ccall] g_markup_parse_context_new (VAR parser: GMarkupParser; flags: GMarkupParseFlags; user_data: gpointer; user_data_dnotify: GDestroyNotify ): GMarkupParseContext;

	PROCEDURE [ccall] g_markup_parse_context_free (context: GMarkupParseContext );
	PROCEDURE [ccall] g_markup_parse_context_parse (context: GMarkupParseContext;text: PString; text_len:INTEGER; OUT error:GError ): gboolean;

	PROCEDURE [ccall] g_markup_parse_context_end_parse (context: GMarkupParseContext;
                                              OUT error: GError ): gboolean;

	PROCEDURE [ccall] g_markup_parse_context_get_element (context: GMarkupParseContext ): PString;


	PROCEDURE [ccall] g_markup_parse_context_get_position (context: GMarkupParseContext;
                                                 OUT line_number,char_number: INTEGER );


	PROCEDURE [ccall] g_markup_escape_text (text: PString; length: INTEGER ): PString;


	(* gmessages.h *)
	PROCEDURE [ccall] g_log_set_handler (log_domain: PString; log_levels: GLogLevelFlags;
                               log_func: GLogFunc;
                               user_data: gpointer ): INTEGER;

	PROCEDURE [ccall] g_log_remove_handler (log_domain: PString; handler_id: INTEGER );

	PROCEDURE [ccall] g_log_default_handler (log_domain: PString; log_level: GLogLevelFlags;
                                   message: PString;
                                   unused_data: gpointer );

	PROCEDURE [ccall] g_log_set_default_handler (log_func: GLogFunc;
                                       user_data: gpointer ): GLogFunc;


	PROCEDURE [ccall] g_log_set_fatal_mask (log_domain: PString;
                                  fatal_mask: GLogLevelFlags ): GLogLevelFlags;

	PROCEDURE [ccall] g_log_set_always_fatal (fatal_mask: GLogLevelFlags ): GLogLevelFlags;



	PROCEDURE [ccall] g_set_print_handler (func: GPrintFunc ): GPrintFunc;

	PROCEDURE [ccall] g_set_printerr_handler (func: GPrintFunc ): GPrintFunc;


	(* goption.h *)

	PROCEDURE [ccall] g_option_error_quark ( ): GQuark;
	PROCEDURE [ccall] g_option_context_new (parameter_string: PString ): GOptionContext;
	PROCEDURE [ccall] g_option_context_free (context: GOptionContext );
	PROCEDURE [ccall] g_option_context_set_help_enabled (context: GOptionContext; help_enabled: gboolean );
	PROCEDURE [ccall] g_option_context_get_help_enabled (context: GOptionContext ): gboolean;
	PROCEDURE [ccall] g_option_context_set_ignore_unknown_options (context: GOptionContext; ignore_unknown: gboolean );
	PROCEDURE [ccall] g_option_context_get_ignore_unknown_options (context: GOptionContext ): gboolean;
	PROCEDURE [ccall] g_option_context_add_main_entries (context: GOptionContext;
                                               entries: GOptionEntry;
                                               translation_domain: PString );
	PROCEDURE [ccall] g_option_context_parse (context: GOptionContext; OUT  argc: INTEGER; OUT  argv: ARRAY [untagged] OF PString;OUT error: GError ): gboolean;

	PROCEDURE [ccall] g_option_context_add_group (context: GOptionContext; group: GOptionContext );
	PROCEDURE [ccall] g_option_context_set_main_group (context: GOptionContext;group: GOptionContext );
	PROCEDURE [ccall] g_option_context_get_main_group (context: GOptionContext ): GOptionContext;
	PROCEDURE [ccall] g_option_group_new (name: PString; description: PString;
                                help_description: PString;
                                user_data: gpointer;
                                destroy: GDestroyNotify ): GOptionContext;

	PROCEDURE [ccall] g_option_group_set_parse_hooks (group: GOptionContext; pre_parse_func,post_parse_func: GOptionParseFunc );
	PROCEDURE [ccall] g_option_group_set_error_hook (group: GOptionContext; error_func: GOptionErrorFunc );
	PROCEDURE [ccall] g_option_group_free (group: GOptionContext );
	PROCEDURE [ccall] g_option_group_add_entries (group: GOptionContext; entries: GOptionEntry );
	PROCEDURE [ccall] g_option_group_set_translate_func (group: GOptionContext; func: GTranslateFunc; data: gpointer; destroy_notify: GDestroyNotify );
	PROCEDURE [ccall] g_option_group_set_translation_domain (group: GOptionContext; domain: PString );

	(* gpattern.h *)
	PROCEDURE [ccall] g_pattern_spec_new (pattern: PString ): GPatternSpec;
	PROCEDURE [ccall] g_pattern_spec_free (pspec: GPatternSpec );
	PROCEDURE [ccall] g_pattern_spec_equal (pspec1, pspec2: GPatternSpec ): gboolean;
	PROCEDURE [ccall] g_pattern_match (pspec: GPatternSpec; string_length: INTEGER; string: PString; string_reversed: PString ): gboolean;
	PROCEDURE [ccall] g_pattern_match_string (pspec: GPatternSpec; string: PString ): gboolean;
	PROCEDURE [ccall] g_pattern_match_simple (pattern: PString; string: PString ): gboolean;


	(* gqsort.h *)
	PROCEDURE [ccall] g_qsort_with_data (pbase: gpointer; total_elems: INTEGER;
                               size: INTEGER;
                               compare_func: GCompareDataFunc;
                               user_data: gpointer );

	(* gspawn.h *)

	PROCEDURE [ccall] g_spawn_error_quark ( ): GQuark;
	PROCEDURE [ccall] g_spawn_command_line_async (command_line: PString; OUT error: GError ): gboolean;
	PROCEDURE [ccall] g_spawn_command_line_sync (command_line: PString;
                                       OUT standard_output: PString;
                                       OUT standard_error: PString;
                                       OUT exit_status: GPid;
                                       OUT error: GError ): gboolean;


	PROCEDURE [ccall] g_spawn_close_pid (pid: GPid );

	PROCEDURE [ccall] g_spawn_async (working_directory: PString; 
			IN argv, envp: ARRAY [untagged] OF PString; flags: GSpawnFlags;
     child_setup: GSpawnChildSetupFunc; user_data: gpointer; 
			OUT child_pid: GPid; OUT error: GError ): gboolean;

	PROCEDURE [ccall] g_spawn_async_with_pipes (working_directory: PString;
			IN argv, envp: ARRAY [untagged] OF PString; 
			flags: GSpawnFlags;
			child_setup: GSpawnChildSetupFunc; user_data: gpointer;
			OUT child_pid: GPid;	OUT standard_input, standard_output, standard_error: INTEGER;
			OUT error: GError ): gboolean;


	PROCEDURE [ccall] g_spawn_sync (working_directory: PString; 
		IN argv, envp: ARRAY [untagged] OF PString; 
		flags: GSpawnFlags; child_setup: GSpawnChildSetupFunc;
   user_data: gpointer; 
   OUT standard_output: PString;
   OUT standard_error: PString;
		OUT exit_status: GPid; OUT error: GError ): gboolean;


(* gmain.h *)
PROCEDURE [ccall] g_main_context_new (  ): GMainContext;
PROCEDURE [ccall] g_main_context_ref ( context: GMainContext ): GMainContext;
PROCEDURE [ccall] g_main_context_unref ( context: GMainContext );
PROCEDURE [ccall] g_main_context_default (  ): GMainContext;
PROCEDURE [ccall] g_main_context_iteration ( context: GMainContext; may_block: gboolean ): gboolean;
PROCEDURE [ccall] g_main_context_pending ( context: GMainContext ): gboolean;

(*  For implementation of legacy interfaces  *)
PROCEDURE [ccall] g_main_context_find_source_by_id ( context: GMainContext;source_id: guint ): GSource;
PROCEDURE [ccall] g_main_context_find_source_by_user_data ( context: GMainContext;user_data: gpointer ): GSource;
PROCEDURE [ccall] g_main_context_find_source_by_funcs_user_data ( context: GMainContext;funcs: GSourceFuncs;user_data: gpointer ): GSource;

(*  Low level functions for implementing custom main loops.  *)
PROCEDURE [ccall] g_main_context_wakeup ( context: GMainContext );
PROCEDURE [ccall] g_main_context_acquire ( context: GMainContext ): gboolean;
PROCEDURE [ccall] g_main_context_release ( context: GMainContext );
PROCEDURE [ccall] g_main_context_wait ( context: GMainContext;cond: GMainContext;mutex: GMainContext ): gboolean;
PROCEDURE [ccall] g_main_context_prepare ( context: GMainContext;priority: gint ): gboolean;
PROCEDURE [ccall] g_main_context_query ( context: GMainContext;max_priority: gint; timeout_: gint;fds: GPollFD;n_fds: gint ): gint;
PROCEDURE [ccall] g_main_context_check ( context: GMainContext;max_priority: gint; fds: GPollFD;n_fds: gint ): gint;
PROCEDURE [ccall] g_main_context_dispatch ( context: GMainContext );
PROCEDURE [ccall] g_main_context_set_poll_func ( context: GMainContext;func: GPollFunc );
PROCEDURE [ccall] g_main_context_get_poll_func ( context: GMainContext ): GPollFunc;
(*  Low level functions for use by source implementations  *)
PROCEDURE [ccall] g_main_context_add_poll ( context: GMainContext; fd: GPollFD;priority: gint );
PROCEDURE [ccall] g_main_context_remove_poll ( context: GMainContext;fd: GPollFD );
PROCEDURE [ccall] g_main_depth (  ): INTEGER;
(*  GMainLoop:  *)
PROCEDURE [ccall] g_main_loop_new ( context: GMainContext;is_running: gboolean ): GMainContext;
PROCEDURE [ccall] g_main_loop_run ( loop: GMainContext );
PROCEDURE [ccall] g_main_loop_quit ( loop: GMainContext );
PROCEDURE [ccall] g_main_loop_ref ( loop: GMainContext ): GMainContext;
PROCEDURE [ccall] g_main_loop_unref ( loop: GMainContext );
PROCEDURE [ccall] g_main_loop_is_running ( loop: GMainContext ): gboolean;
PROCEDURE [ccall] g_main_loop_get_context ( loop: GMainContext ): GMainContext;
(*  Miscellaneous functions  *)
PROCEDURE [ccall] g_get_current_time ( result: GTimeVal );

(*  GSource:  *)
PROCEDURE [ccall] g_source_new ( source_funcs: GSourceFuncs; struct_size: guint ): GSource;
PROCEDURE [ccall] g_source_ref ( source: GSource ): GSource;
PROCEDURE [ccall] g_source_unref ( source: GSource );
PROCEDURE [ccall] g_source_attach ( source: GSource; context: GMainContext ): guint;
PROCEDURE [ccall] g_source_destroy ( source: GSource );
PROCEDURE [ccall] g_source_set_priority ( source: GSource; priority: gint );
PROCEDURE [ccall] g_source_get_priority ( source: GSource ): gint;
PROCEDURE [ccall] g_source_set_can_recurse ( source: GSource;can_recurse: gboolean );
PROCEDURE [ccall] g_source_get_can_recurse ( source: GSource ): gboolean;
PROCEDURE [ccall] g_source_get_id ( source: GSource ): guint;
PROCEDURE [ccall] g_source_get_context ( source: GSource ): GMainContext;
PROCEDURE [ccall] g_source_set_callback ( source: GSource; func: GSourceFunc;data: gpointer;notify: GDestroyNotify );


PROCEDURE [ccall] g_source_set_callback_indirect ( source: GSource;callback_data: gpointer;callback_funcs: GSourceCallbackFuncs );
PROCEDURE [ccall] g_source_add_poll ( source: GSource; fd: GPollFD );
PROCEDURE [ccall] g_source_remove_poll ( source: GSource; fd: GPollFD );
PROCEDURE [ccall] g_source_get_current_time ( source: GSource; timeval: GTimeVal );
PROCEDURE [ccall] g_idle_source_new (  ): GSource;
PROCEDURE [ccall] g_child_watch_source_new ( pid: GPid ): GSource;
PROCEDURE [ccall] g_timeout_source_new ( interval: guint ): GSource;


(*  Source manipulation by ID  *)
PROCEDURE [ccall] g_source_remove ( tag: guint ): gboolean;
PROCEDURE [ccall] g_source_remove_by_user_data ( user_data: gpointer ): gboolean;
PROCEDURE [ccall] g_source_remove_by_funcs_user_data ( funcs: GSourceFuncs; user_data: gpointer ): gboolean;

(*  Idles, child watchers and timeouts  *)
PROCEDURE [ccall] g_timeout_add_full* ( priority: gint; interval: guint; function: GSourceFunc; data: gpointer; notify: GDestroyNotify ): guint;
PROCEDURE [ccall] g_timeout_add ( interval: guint; function: GSourceFunc; data: gpointer ): guint;
PROCEDURE [ccall] g_child_watch_add_full ( priority: gint; pid: GPid; function: GChildWatchFunc; data: gpointer; notify: GDestroyNotify ): guint;
PROCEDURE [ccall] g_child_watch_add ( pid: GPid; function: GChildWatchFunc; data: gpointer ): guint;
PROCEDURE [ccall] g_idle_add ( function: GSourceFunc; data: gpointer ): guint;
PROCEDURE [ccall] g_idle_add_full* ( priority: gint; function: GSourceFunc;data: gpointer; notify: GDestroyNotify ): guint;
PROCEDURE [ccall] g_idle_remove_by_data ( data: gpointer ): gboolean;

END LibsGlib.
