#include <Collator.h>
#include <UnicodeChar.h>
#include <String.h>

#include <ctype.h>


// conversion array for character ranges 192 - 223 & 224 - 255
static uint8 gNoDiacrits[] = {
	'a','a','a','a','a','a','a',
	'c',
	'e','e','e','e',
	'i','i','i','i',
	240,	// eth
	'n',
	'o','o','o','o','o',
	247,	//
	'o',
	'u','u','u','u',
	'y',
	254,	// thorn
	'y'
};


static inline uint32
getPrimaryChar(uint32 c)
{
	if (c < 0x80)
		return tolower(c);

	if (c >= 192 && c < 223)
		return gNoDiacrits[c - 192];
	if (c == 223)
		return 's';
	if (c >= 224 && c < 256)
		return gNoDiacrits[c - 224];

	return BUnicodeChar::ToLower(c);
}

// ToDo: should probably override B_COLLATE_SECONDARY only!
// ToDo: specify the length in BString::UnlockBuffer() to speed things up!

/**	The German collator just utilizes the BCollatorAddOn class for most
 *	collator strengths, it only implements B_COLLATE_PRIMARY.
 *	When you specify this strength, "Ã¤" will be replaced by "ae", "Ã¶"
 *	by "oe" and so on for all the German umlauts. For all other characters,
 *	it will do the exact same thing as it's parent class.
 *	This method is used in German telephone books, for example.
 */

class CollatorDeutsch : public BCollatorAddOn {
	public:
		CollatorDeutsch();
		~CollatorDeutsch();
		
		virtual void GetSortKey(const char *string, BString *key, int8 strength);
		virtual int Compare(const char *a, const char *b, int32 length, int8 strength);
};


CollatorDeutsch::CollatorDeutsch()
{
}


CollatorDeutsch::~CollatorDeutsch()
{
}


void 
CollatorDeutsch::GetSortKey(const char *string, BString *key, int8 strength)
{
	if (strength != B_COLLATE_PRIMARY) {
		BCollatorAddOn::GetSortKey(string, key, strength);
		return;
	}

	int32 length = strlen(string);
	char *buffer = key->LockBuffer(2 * length);

	uint32 c;
	for (int32 i = 0; (c = BUnicodeChar::FromUTF8(&string)) && i < length; i++) {
		if (c < 0x80)
			*buffer++ = tolower(c);
		else if (c == (uint8)'Ä' || c == (uint8)'ä') {
			*buffer++ = 'a';
			*buffer++ = 'e';
		} else if (c == (uint8)'Ö' || c == (uint8)'ö') {
			*buffer++ = 'o';
			*buffer++ = 'e';
		} else if (c == (uint8)'Ü' || c == (uint8)'ü') {
			*buffer++ = 'u';
			*buffer++ = 'e';
		} else if (c == (uint8)'ß') {
			*buffer++ = 's';
			*buffer++ = 's';
		} else
			BUnicodeChar::ToUTF8(getPrimaryChar(c), &buffer);
	}
	*buffer = 0;

	key->UnlockBuffer();	
}


int 
CollatorDeutsch::Compare(const char *a, const char *b, int32 length, int8 strength)
{
	if (strength != B_COLLATE_PRIMARY)
		return BCollatorAddOn::Compare(a, b, length, strength);

	// ToDo: this is actually a very slow implementation, and should be
	// realized without calling the GetSortKey() method at all...

	BString keyA, keyB;
	GetSortKey(a, &keyA, B_COLLATE_PRIMARY);
	GetSortKey(b, &keyB, B_COLLATE_PRIMARY);

	a = keyA.String();
	b = keyB.String();

	for (int32 i = 0; i < length; i++) {
		uint32 charA = BUnicodeChar::FromUTF8(&a);
		uint32 charB = BUnicodeChar::FromUTF8(&b);
		if (charA == 0)
			return charB == 0 ? 0 : charB;
		else if (charB == 0)
			return -(int32)charA;

		if (charA != charB)
			return (int32)charA - (int32)charB;
	}
	return 0;
}

