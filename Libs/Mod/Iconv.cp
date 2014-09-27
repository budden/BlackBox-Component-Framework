MODULE LibsIconv ["libiconv2.dll"];

	IMPORT WinApi;

	TYPE
		PtrVoid = WinApi.PtrVoid;
		PtrSTR* = WinApi.PtrSTR;
		PtrLSTR* = POINTER TO ARRAY [untagged] OF CHAR;
		size_t* = INTEGER;

		iconv_t* = PtrVoid;

	PROCEDURE [ccall] iconv_open* ["libiconv_open"] (tocode, fromcode: PtrSTR): iconv_t;
	PROCEDURE [ccall] iconv_close* ["libiconv_close"] (cd: iconv_t): INTEGER;

	PROCEDURE [ccall] iconv* ["libiconv"] (cd: iconv_t; VAR [nil] inbuf: PtrSTR; VAR inbytesleft: size_t; VAR [nil] outbuf: PtrSTR; VAR outbytesleft: size_t): size_t;

	PROCEDURE [ccall] iconv_encode* ["libiconv"] (cd: iconv_t; VAR [nil] inbuf: PtrLSTR; VAR inbytesleft: size_t; VAR [nil] outbuf: PtrSTR; VAR outbytesleft: size_t): size_t;

	PROCEDURE [ccall] iconv_decode* ["libiconv"] (cd: iconv_t; VAR [nil] inbuf: PtrSTR; VAR inbytesleft: size_t; VAR [nil] outbuf: PtrLSTR; VAR outbytesleft: size_t): size_t;

END LibsIconv.