MODULE Bits;

(* History:
	Date:	Author:	Change:
	13-May-2009	Rainer Neubauer	Start of development.
	30-Oct-2009	Rainer Neubauer	Release
 *)

	PROCEDURE^ ByteOfSet* (set: SET): BYTE;
	PROCEDURE^ IntOfSet* (set: SET): INTEGER;
	PROCEDURE^ ShortOfSet* (set: SET): SHORTINT;



	PROCEDURE BitOfByte* (value: BYTE; numberOfBit: INTEGER): BOOLEAN;
	BEGIN
		RETURN numberOfBit IN (BITS (value) - {8 .. MAX (SET)})
	END BitOfByte;

	PROCEDURE BitOfInt* (value, numberOfBit: INTEGER): BOOLEAN;
	BEGIN
		RETURN numberOfBit IN BITS (value)
	END BitOfInt;

	PROCEDURE BitOfShort* (value: SHORTINT; numberOfBit: INTEGER): BOOLEAN;
	BEGIN
		RETURN numberOfBit IN (BITS (value) - {16 .. MAX (SET)})
	END BitOfShort;

	PROCEDURE Byte* (value: SHORTINT): BYTE;
		VAR set: SET;
	BEGIN
		set := BITS (value);
		RETURN ByteOfSet (set - {8 .. MAX (SET)})
	END Byte;

	PROCEDURE Byte1AndNotByte2* (byte1, byte2: BYTE): BYTE;
		VAR set1, set2: SET;
	BEGIN
		set1 := BITS (byte1) - {8 .. MAX (SET)};
		set2 := BITS (byte2) - {8 .. MAX (SET)};
		RETURN ByteOfSet (set1 - set2)
	END Byte1AndNotByte2;

	PROCEDURE ByteNOT* (byte: BYTE): BYTE;
		VAR set: SET;
	BEGIN
		set := - BITS (byte);
		RETURN ByteOfSet (set - {8 .. MAX (SET)})
	END ByteNOT;

	PROCEDURE ByteOfInt* (value, numberOfByte: INTEGER): BYTE;
		VAR set: SET;
	BEGIN
		CASE numberOfByte OF
		| 0: set := BITS (value)
		| 1: set := BITS (ASH (value, - 8))
		| 2: set := BITS (ASH (value, - 16))
		| 3: set := BITS (ASH (value, - 24))
		ELSE
			set := {}
		END;
		RETURN ByteOfSet (set - {8 .. MAX (SET)})
	END ByteOfInt;

	PROCEDURE ByteOfSet* (set: SET): BYTE;
	BEGIN
		IF ORD (set) > MAX (BYTE) THEN
			RETURN SHORT (SHORT (ORD (set) - 256))
		ELSE
			RETURN SHORT (SHORT (ORD (set)))
		END
	END ByteOfSet;

	PROCEDURE ByteOfShort* (value: SHORTINT; numberOfByte: INTEGER): BYTE;
		VAR set: SET;
	BEGIN
		CASE numberOfByte OF
		| 0: set := BITS (value)
		| 1: set := BITS (ASH (value, - 8))
		ELSE
			set := {}
		END;
		RETURN ByteOfSet (set - {8 .. MAX (SET)})
	END ByteOfShort;

	PROCEDURE BytesAND* (byte1, byte2: BYTE): BYTE;
		VAR set1, set2: SET;
	BEGIN
		set1 := BITS (byte1) - {8 .. MAX (SET)};
		set2 := BITS (byte2) - {8 .. MAX (SET)};
		RETURN ByteOfSet (set1 * set2)
	END BytesAND;

	PROCEDURE BytesOR* (byte1, byte2: BYTE): BYTE;
		VAR set1, set2: SET;
	BEGIN
		set1 := BITS (byte1) - {8 .. MAX (SET)};
		set2 := BITS (byte2) - {8 .. MAX (SET)};
		RETURN ByteOfSet (set1 + set2)
	END BytesOR;

	PROCEDURE BytesXOR* (byte1, byte2: BYTE): BYTE;
		VAR set1, set2: SET;
	BEGIN
		set1 := BITS (byte1) - {8 .. MAX (SET)};
		set2 := BITS (byte2) - {8 .. MAX (SET)};
		RETURN ByteOfSet (set1 / set2)
	END BytesXOR;

	PROCEDURE Int1AndNotInt2* (int1, int2: INTEGER): INTEGER;
		VAR set1, set2: SET;
	BEGIN
		set1 := BITS (int1);
		set2 := BITS (int2);
		RETURN ORD (set1 - set2)
	END Int1AndNotInt2;

	PROCEDURE IntNOT* (int: INTEGER): INTEGER;
		VAR set: SET;
	BEGIN
		set := - BITS (int);
		RETURN ORD (set)
	END IntNOT;

	PROCEDURE IntOfByte* (value: BYTE): INTEGER;
	BEGIN
		RETURN ORD (BITS (value) - {8 .. MAX (SET)})
	END IntOfByte;

	PROCEDURE IntOfSet* (set: SET): INTEGER;
	BEGIN
		RETURN ORD (set)
	END IntOfSet;

	PROCEDURE IntOfShort* (value: SHORTINT): INTEGER;
	BEGIN
		RETURN ORD (BITS (value) - {16 .. MAX (SET)})
	END IntOfShort;

	PROCEDURE IntsAND* (int1, int2: INTEGER): INTEGER;
		VAR set1, set2: SET;
	BEGIN
		set1 := BITS (int1);
		set2 := BITS (int2);
		RETURN ORD (set1 * set2)
	END IntsAND;

	PROCEDURE IntsOR* (int1, int2: INTEGER): INTEGER;
		VAR set1, set2: SET;
	BEGIN
		set1 := BITS (int1);
		set2 := BITS (int2);
		RETURN ORD (set1 + set2)
	END IntsOR;

	PROCEDURE IntsXOR* (int1, int2: INTEGER): INTEGER;
		VAR set1, set2: SET;
	BEGIN
		set1 := BITS (int1);
		set2 := BITS (int2);
		RETURN ORD (set1 / set2)
	END IntsXOR;

	PROCEDURE JoinBytes* (highByte, lowByte: BYTE): SHORTINT;
		VAR highSet, lowSet: SET;
	BEGIN
		highSet := BITS (ASH (highByte, 8)) - {16..MAX (SET)};
		lowSet := BITS (lowByte) - {8..MAX (SET)};
		RETURN ShortOfSet (highSet + lowSet)
	END JoinBytes;

	PROCEDURE JoinNibbles* (highNibble, lowNibble: BYTE): BYTE;
		VAR highSet, lowSet, set: SET;
	BEGIN
		highSet := BITS (ASH (highNibble, 4)) - {8 .. MAX (SET)};
		lowSet := BITS (lowNibble) - {4 .. MAX (SET)};
		RETURN ByteOfSet (highSet + lowSet)
	END JoinNibbles;

	PROCEDURE JoinShorts* (highWord, lowWord: SHORTINT): INTEGER;
		VAR highSet, lowSet: SET;
	BEGIN
		highSet := BITS (ASH (highWord, 16));
		lowSet := BITS (lowWord) - {16 .. MAX (SET)};
		RETURN ORD (highSet + lowSet)
	END JoinShorts;

	PROCEDURE SetOfByte* (byte: BYTE): SET;
	BEGIN
		RETURN BITS (byte) - {8 .. MAX (SET)}
	END SetOfByte;

	PROCEDURE SetOfInt* (int: INTEGER): SET;
	BEGIN
		RETURN BITS (int)
	END SetOfInt;

	PROCEDURE SetOfShort* (short: SHORTINT): SET;
	BEGIN
		RETURN BITS (short) - {16 .. MAX (SET)}
	END SetOfShort;

	PROCEDURE Short* (value: INTEGER): SHORTINT;
		VAR set: SET;
	BEGIN
		set := BITS (value);
		RETURN ShortOfSet (set - {16 .. MAX (SET)})
	END Short;

	PROCEDURE Short1AndNotShort2* (short1, short2: SHORTINT): SHORTINT;
		VAR set1, set2: SET;
	BEGIN
		set1 := BITS (short1) - {16 .. MAX (SET)};
		set2 := BITS (short2) - {16 .. MAX (SET)};
		RETURN ShortOfSet (set1 - set2)
	END Short1AndNotShort2;

	PROCEDURE ShortNOT* (short: SHORTINT): SHORTINT;
		VAR set: SET;
	BEGIN
		set := - BITS (short);
		RETURN ShortOfSet (set - {16 .. MAX (SET)})
	END ShortNOT;

	PROCEDURE ShortOfByte* (value: BYTE): SHORTINT;
	BEGIN
		RETURN ShortOfSet (BITS (value) - {8 .. MAX (SET)})
	END ShortOfByte;

	PROCEDURE ShortOfInt* (value, numberOfWord: INTEGER): SHORTINT;
		VAR set: SET;
	BEGIN
		CASE numberOfWord OF
		| 0: set := BITS (value)
		| 1: set := BITS (ASH (value, - 16))
		ELSE
			set := {}
		END;
		RETURN ShortOfSet (set - {16 .. MAX (SET)})
	END ShortOfInt;

	PROCEDURE ShortOfSet* (set: SET): SHORTINT;
	BEGIN
		IF ORD (set) > MAX (SHORTINT) THEN
			RETURN SHORT (ORD (set) - 65536)
		ELSE
			RETURN SHORT (ORD (set))
		END
	END ShortOfSet;

	PROCEDURE ShortsAND* (short1, short2: SHORTINT): SHORTINT;
		VAR set1, set2: SET;
	BEGIN
		set1 := BITS (short1) - {16 .. MAX (SET)};
		set2 := BITS (short2) - {16 .. MAX (SET)};
		RETURN ShortOfSet (set1 * set2)
	END ShortsAND;

	PROCEDURE ShortsOR* (short1, short2: SHORTINT): SHORTINT;
		VAR set1, set2: SET;
	BEGIN
		set1 := BITS (short1) - {16 .. MAX (SET)};
		set2 := BITS (short2) - {16 .. MAX (SET)};
		RETURN ShortOfSet (set1 + set2)
	END ShortsOR;

	PROCEDURE ShortsXOR* (short1, short2: SHORTINT): SHORTINT;
		VAR set1, set2: SET;
	BEGIN
		set1 := BITS (short1) - {16 .. MAX (SET)};
		set2 := BITS (short2) - {16 .. MAX (SET)};
		RETURN ShortOfSet (set1 / set2)
	END ShortsXOR;


	PROCEDURE SplitByte* (value: BYTE; OUT highNibble, lowNibble: BYTE);
	BEGIN
		highNibble := ByteOfSet (BITS (ASH (value, - 4)) - {4..MAX (SET)});
		lowNibble := ByteOfSet (BITS (value) - {4..MAX (SET)})
	END SplitByte;

	PROCEDURE SplitInt* (value: INTEGER; OUT highWord, lowWord: SHORTINT);
	BEGIN
		highWord := ShortOfSet (BITS (ASH (value, - 16)) - {16..MAX (SET)});
		lowWord := ShortOfSet (BITS (value) - {16..MAX (SET)})
	END SplitInt;

	PROCEDURE SplitShort* (value: SHORTINT; OUT highByte, lowByte: BYTE);
	BEGIN
		highByte := ByteOfSet (BITS (ASH (value, - 8)) - {8..MAX (SET)});
		lowByte := ByteOfSet (BITS (value) - {8..MAX (SET)})
	END SplitShort;

	PROCEDURE PutBitIntoByte* (VAR value: BYTE; numberOfBit: INTEGER; bit: BOOLEAN);
	BEGIN
		IF bit THEN
			value := ByteOfSet (BITS (value) + {numberOfBit} - {8..MAX (SET)})
		ELSE
			value := ByteOfSet (BITS (value) - {numberOfBit} - {8..MAX (SET)})
		END
	END PutBitIntoByte;

	PROCEDURE PutBitIntoInt* (VAR value: INTEGER; numberOfBit: INTEGER; bit: BOOLEAN);
	BEGIN
		IF bit THEN
			value := ORD (BITS (value) + {numberOfBit})
		ELSE
			value := ORD (BITS (value) - {numberOfBit})
		END
	END PutBitIntoInt;

	PROCEDURE PutBitIntoShort* (VAR value: SHORTINT; numberOfBit: INTEGER; bit: BOOLEAN);
	BEGIN
		IF bit THEN
			value := ShortOfSet (BITS (value) + {numberOfBit} - {16..MAX (SET)})
		ELSE
			value := ShortOfSet (BITS (value) - {numberOfBit} - {16..MAX (SET)})
		END
	END PutBitIntoShort;

	PROCEDURE PutByteIntoInt* (VAR value: INTEGER; numberOfByte: INTEGER; byte: BYTE);
		VAR bitsToAdd, bitsToSubtract: SET;
	BEGIN
		CASE numberOfByte OF
		|0: bitsToAdd := BITS (byte) - {8..MAX (SET)};
			bitsToSubtract := {0..7}
		|1: bitsToAdd := BITS (ASH (byte, 8)) - {16..MAX (SET)};
			bitsToSubtract := {8..15}
		|2: bitsToAdd := BITS (ASH (byte, 16)) - {24..MAX (SET)};
			bitsToSubtract := {16..23}
		|3: bitsToAdd := BITS (ASH (byte, 24));
			bitsToSubtract := {24..31}
		ELSE
			bitsToAdd := {};
			bitsToSubtract := {}
		END;
		value := ORD (BITS (value) - bitsToSubtract + bitsToAdd)
	END PutByteIntoInt;

	PROCEDURE PutByteIntoShort* (VAR value: SHORTINT; numberOfByte: INTEGER; byte: BYTE);
		VAR bitsToAdd, bitsToSubtract: SET;
	BEGIN
		CASE numberOfByte OF
		|0: bitsToAdd := BITS (byte) - {8..MAX (SET)};
			bitsToSubtract := {0..7}
		|1: bitsToAdd := BITS (ASH (byte, 8)) - {16..MAX (SET)};
			bitsToSubtract := {8..15}
		ELSE
			bitsToAdd := {};
			bitsToSubtract := {}
		END;
		value := ShortOfSet (BITS (value) - bitsToSubtract + bitsToAdd)
	END PutByteIntoShort;

	PROCEDURE PutShortIntoInt* (VAR value: INTEGER; numberOfWord: INTEGER; word: SHORTINT);
		VAR bitsToAdd, bitsToSubtract: SET;
	BEGIN
		CASE numberOfWord OF
		|0: bitsToAdd := BITS (word) - {16..MAX (SET)};
			bitsToSubtract := {0..15}
		|1: bitsToAdd := BITS (ASH (word, 16));
			bitsToSubtract := {16..31}
		ELSE
			bitsToAdd := {};
			bitsToSubtract := {}
		END;
		value := ORD (BITS (value) - bitsToSubtract + bitsToAdd)
	END PutShortIntoInt;

END Bits.