#ifndef _UNICODE_CHAR_H_
#define _UNICODE_CHAR_H_

#include <SupportDefs.h>


enum unicode_char_category
{
	// Non-category for unassigned and non-character code points.
	B_UNICODE_UNASSIGNED				= 0,

	B_UNICODE_UPPERCASE_LETTER			= 1,	// Lu
	B_UNICODE_LOWERCASE_LETTER			= 2,	// Ll
	B_UNICODE_TITLECASE_LETTER			= 3,	// Lt
	B_UNICODE_MODIFIER_LETTER			= 4,	// Lm
	B_UNICODE_OTHER_LETTER				= 5,	// Lo
	B_UNICODE_NON_SPACING_MARK			= 6,	// Mn
	B_UNICODE_ENCLOSING_MARK			= 7,	// Me
	B_UNICODE_COMBINING_SPACING_MARK	= 8,	// Mc
	B_UNICODE_DECIMAL_DIGIT_NUMBER		= 9,	// Nd
	B_UNICODE_LETTER_NUMBER				= 10,	// Nl
	B_UNICODE_OTHER_NUMBER				= 11,	// No
	B_UNICODE_SPACE_SEPARATOR			= 12,	// Zs
	B_UNICODE_LINE_SEPARATOR			= 13,	// Zl
	B_UNICODE_PARAGRAPH_SEPARATOR		= 14,	// Zp
	B_UNICODE_CONTROL_CHAR				= 15,	// Cc
	B_UNICODE_FORMAT_CHAR				= 16,	// Cf
	B_UNICODE_PRIVATE_USE_CHAR			= 17,	// Co
	B_UNICODE_SURROGATE					= 18,	// Cs
	B_UNICODE_DASH_PUNCTUATION			= 19,	// Pd
	B_UNICODE_START_PUNCTUATION			= 20,	// Ps
	B_UNICODE_END_PUNCTUATION			= 21,	// Pe
	B_UNICODE_CONNECTOR_PUNCTUATION		= 22,	// Pc
	B_UNICODE_OTHER_PUNCTUATION			= 23,	// Po
	B_UNICODE_MATH_SYMBOL				= 24,	// Sm
	B_UNICODE_CURRENCY_SYMBOL			= 25,	// Sc
	B_UNICODE_MODIFIER_SYMBOL			= 26,	// Sk
	B_UNICODE_OTHER_SYMBOL				= 27,	// So
	B_UNICODE_INITIAL_PUNCTUATION		= 28,	// Pi
	B_UNICODE_FINAL_PUNCTUATION			= 29,	// Pf
	B_UNICODE_GENERAL_OTHER_TYPES		= 30,	// Cn

	B_UNICODE_CATEGORY_COUNT
};


/**
 * This specifies the language directional property of a character set.
 */

enum unicode_char_direction { 
	B_UNICODE_LEFT_TO_RIGHT               = 0, 
	B_UNICODE_RIGHT_TO_LEFT               = 1, 
	B_UNICODE_EUROPEAN_NUMBER             = 2,
	B_UNICODE_EUROPEAN_NUMBER_SEPARATOR   = 3,
	B_UNICODE_EUROPEAN_NUMBER_TERMINATOR  = 4,
	B_UNICODE_ARABIC_NUMBER               = 5,
	B_UNICODE_COMMON_NUMBER_SEPARATOR     = 6,
	B_UNICODE_BLOCK_SEPARATOR             = 7,
	B_UNICODE_SEGMENT_SEPARATOR           = 8,
	B_UNICODE_WHITE_SPACE_NEUTRAL         = 9, 
	B_UNICODE_OTHER_NEUTRAL               = 10, 
	B_UNICODE_LEFT_TO_RIGHT_EMBEDDING     = 11,
	B_UNICODE_LEFT_TO_RIGHT_OVERRIDE      = 12,
	B_UNICODE_RIGHT_TO_LEFT_ARABIC        = 13,
	B_UNICODE_RIGHT_TO_LEFT_EMBEDDING     = 14,
	B_UNICODE_RIGHT_TO_LEFT_OVERRIDE      = 15,
	B_UNICODE_POP_DIRECTIONAL_FORMAT      = 16,
	B_UNICODE_DIR_NON_SPACING_MARK        = 17,
	B_UNICODE_BOUNDARY_NEUTRAL            = 18,
	
	B_UNICODE_DIRECTION_COUNT
};


/**
 * Script range as defined in the Unicode standard.
 */

enum unicode_char_script {
	// Script names
	B_UNICODE_BASIC_LATIN,
	B_UNICODE_LATIN_1_SUPPLEMENT,
	B_UNICODE_LATIN_EXTENDED_A,
	B_UNICODE_LATIN_EXTENDED_B,
	B_UNICODE_IPA_EXTENSIONS,
	B_UNICODE_SPACING_MODIFIER_LETTERS,
	B_UNICODE_COMBINING_DIACRITICAL_MARKS,
	B_UNICODE_GREEK,
	B_UNICODE_CYRILLIC,
	B_UNICODE_ARMENIAN,
	B_UNICODE_HEBREW,
	B_UNICODE_ARABIC,
	B_UNICODE_SYRIAC,
	B_UNICODE_THAANA,
	B_UNICODE_DEVANAGARI,
	B_UNICODE_BENGALI,
	B_UNICODE_GURMUKHI,
	B_UNICODE_GUJARATI,
	B_UNICODE_ORIYA,
	B_UNICODE_TAMIL,
	B_UNICODE_TELUGU,
	B_UNICODE_KANNADA,
	B_UNICODE_MALAYALAM,
	B_UNICODE_SINHALA,
	B_UNICODE_THAI,
	B_UNICODE_LAO,
	B_UNICODE_TIBETAN,
	B_UNICODE_MYANMAR,
	B_UNICODE_GEORGIAN,
	B_UNICODE_HANGUL_JAMO,
	B_UNICODE_ETHIOPIC,
	B_UNICODE_CHEROKEE,
	B_UNICODE_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS,
	B_UNICODE_OGHAM,
	B_UNICODE_RUNIC,
	B_UNICODE_KHMER,
	B_UNICODE_MONGOLIAN,
	B_UNICODE_LATIN_EXTENDED_ADDITIONAL,
	B_UNICODE_GREEK_EXTENDED,
	B_UNICODE_GENERAL_PUNCTUATION,
	B_UNICODE_SUPERSCRIPTS_AND_SUBSCRIPTS,
	B_UNICODE_CURRENCY_SYMBOLS,
	B_UNICODE_COMBINING_MARKS_FOR_SYMBOLS,
	B_UNICODE_LETTERLIKE_SYMBOLS,
	B_UNICODE_NUMBER_FORMS,
	B_UNICODE_ARROWS,
	B_UNICODE_MATHEMATICAL_OPERATORS,
	B_UNICODE_MISCELLANEOUS_TECHNICAL,
	B_UNICODE_CONTROL_PICTURES,
	B_UNICODE_OPTICAL_CHARACTER_RECOGNITION,
	B_UNICODE_ENCLOSED_ALPHANUMERICS,
	B_UNICODE_BOX_DRAWING,
	B_UNICODE_BLOCK_ELEMENTS,
	B_UNICODE_GEOMETRIC_SHAPES,
	B_UNICODE_MISCELLANEOUS_SYMBOLS,
	B_UNICODE_DINGBATS,
	B_UNICODE_BRAILLE_PATTERNS,
	B_UNICODE_CJK_RADICALS_SUPPLEMENT,
	B_UNICODE_KANGXI_RADICALS,
	B_UNICODE_IDEOGRAPHIC_DESCRIPTION_CHARACTERS,
	B_UNICODE_CJK_SYMBOLS_AND_PUNCTUATION,
	B_UNICODE_HIRAGANA,
	B_UNICODE_KATAKANA,
	B_UNICODE_BOPOMOFO,
	B_UNICODE_HANGUL_COMPATIBILITY_JAMO,
	B_UNICODE_KANBUN,
	B_UNICODE_BOPOMOFO_EXTENDED,
	B_UNICODE_ENCLOSED_CJK_LETTERS_AND_MONTHS,
	B_UNICODE_CJK_COMPATIBILITY,
	B_UNICODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A,
	B_UNICODE_CJK_UNIFIED_IDEOGRAPHS,
	B_UNICODE_YI_SYLLABLES,
	B_UNICODE_YI_RADICALS,
	B_UNICODE_HANGUL_SYLLABLES,
	B_UNICODE_HIGH_SURROGATES,
	B_UNICODE_HIGH_PRIVATE_USE_SURROGATES,
	B_UNICODE_LOW_SURROGATES,
	B_UNICODE_PRIVATE_USE_AREA,
	B_UNICODE_CJK_COMPATIBILITY_IDEOGRAPHS,
	B_UNICODE_ALPHABETIC_PRESENTATION_FORMS,
	B_UNICODE_ARABIC_PRESENTATION_FORMS_A,
	B_UNICODE_COMBINING_HALF_MARKS,
	B_UNICODE_CJK_COMPATIBILITY_FORMS,
	B_UNICODE_SMALL_FORM_VARIANTS,
	B_UNICODE_ARABIC_PRESENTATION_FORMS_B,
	B_UNICODE_SPECIALS,
	B_UNICODE_HALFWIDTH_AND_FULLWIDTH_FORMS,

	B_UNICODE_SCRIPT_COUNT,
	B_UNICODE_NO_SCRIPT = B_UNICODE_SCRIPT_COUNT
};


/**
 * Values returned by the u_getCellWidth() function.
 */

enum unicode_cell_width
{
    B_UNICODE_ZERO_WIDTH              = 0,
    B_UNICODE_HALF_WIDTH              = 1,
    B_UNICODE_FULL_WIDTH              = 2,
    B_UNICODE_NEUTRAL_WIDTH           = 3,

    B_UNICODE_CELL_WIDTH_COUNT
};


class BUnicodeChar {
	public:
		static bool IsAlpha(uint32 c);
		static bool IsAlNum(uint32 c);
		static bool IsDigit(uint32 c);
		static bool IsHexDigit(uint32 c);
		static bool IsUpper(uint32 c);
		static bool IsLower(uint32 c);
		static bool IsSpace(uint32 c);
		static bool IsWhitespace(uint32 c);
		static bool IsControl(uint32 c);
		static bool IsPunctuation(uint32 c);
		static bool IsPrintable(uint32 c);
		static bool IsTitle(uint32 c);
		static bool IsDefined(uint32 c);
		static bool IsBase(uint32 c);

		static int8 Type(uint32 c);

		static uint32 ToLower(uint32 c);
		static uint32 ToUpper(uint32 c);
		static uint32 ToTitle(uint32 c);
		static int32 DigitValue(uint32 c);

		static void ToUTF8(uint32 c, char **out);
		static uint32 FromUTF8(const char **in);
		static uint32 FromUTF8(const char *in);

		static size_t UTF8StringLength(const char *str);
		static size_t UTF8StringLength(const char *str, size_t maxLength);

	private:
		BUnicodeChar();
};


inline uint32 
BUnicodeChar::FromUTF8(const char *in)
{
	const char *string = in;
	return FromUTF8(&string);
}


#endif	/* _UNICODE_CHAR_H_ */
