/* 
** Copyright 2003, Axel Dörfler, axeld@pinc-software.de. All rights reserved.
** Distributed under the terms of the OpenBeOS License.
*/


#include <Collator.h>
#include <UnicodeChar.h>
#include <String.h>
#include <Message.h>

#include <ctype.h>


struct input_context {
	input_context(bool ignorePunctuation)
		:
		next_char(0),
		ignore_punctuation(ignorePunctuation)
	{
	}

	uint32	next_char;
	bool	ignore_punctuation;
};


static const char *kSignature = "application/x-vnd.locale-collator.deutsch-din2";

// conversion array for character ranges 192 - 223 & 224 - 255
static const uint8 kNoDiacrits[] = {
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
		return kNoDiacrits[c - 192];
	if (c == 223)
		return 's';
	if (c >= 224 && c < 256)
		return kNoDiacrits[c - 224];

	return BUnicodeChar::ToLower(c);
}


static inline uint32
getNextChar(const char **string, input_context &context)
{
	uint32 c = context.next_char;
	if (c != 0) {
		context.next_char = 0;
		return c;
	}

	do {
		c = BUnicodeChar::FromUTF8(string);
	} while (context.ignore_punctuation
		&& (BUnicodeChar::IsPunctuation(c) || BUnicodeChar::IsSpace(c)));

	switch (c) {
		case 223:	// ß
			context.next_char = 's';
			return 's';
		case 196:	// Ae
			context.next_char = 'e';
			return 'A';
		case 214:	// Oe
			context.next_char = 'e';
			return 'O';
		case 220:	// Ue
			context.next_char = 'e';
			return 'U';
		case 228:	// ae
			context.next_char = 'e';
			return 'a';
		case 246:	// oe
			context.next_char = 'e';
			return 'o';
		case 252:	// ue
			context.next_char = 'e';
			return 'u';
	}

	return c;
}


static char *
putPrimarySortKey(const char *string, char *buffer, int32 length, bool ignorePunctuation)
{
	input_context context(ignorePunctuation);

	uint32 c;
	for (int32 i = 0; (c = getNextChar(&string, context)) != 0 && i < length; i++) {
		if (c < 0x80)
			*buffer++ = tolower(c);
		else
			BUnicodeChar::ToUTF8(getPrimaryChar(c), &buffer);
	}

	return buffer;
}


//	#pragma mark -

// ToDo: should probably override B_COLLATE_SECONDARY & B_COLLATE_TERTIARY,
//		and not B_COLLATE_PRIMARY!
// ToDo: specify the length in BString::UnlockBuffer() to speed things up!

/**	The German collator just utilizes the BCollatorAddOn class for most
 *	collator strengths, it only implements B_COLLATE_PRIMARY.
 *	When you specify this strength, "ä" will be replaced by "ae", "ö"
 *	by "oe" and so on for all the German umlauts. For all other characters,
 *	it will do the exact same thing as it's parent class.
 *	This method is called DIN-2 and its intended usage is for sorting names.
 *	It is used in German telephone books, for example.
 */

class CollatorDeutsch : public BCollatorAddOn {
	public:
		CollatorDeutsch();
		CollatorDeutsch(BMessage *archive);
		~CollatorDeutsch();
		
		virtual void GetSortKey(const char *string, BString *key, int8 strength,
							bool ignorePunctuation);
		virtual int Compare(const char *a, const char *b, int32 length, int8 strength,
							bool ignorePunctuation);

		// (un-)archiving API
		virtual status_t Archive(BMessage *archive, bool deep);
		static BArchivable *Instantiate(BMessage *archive);
};


CollatorDeutsch::CollatorDeutsch()
{
}


CollatorDeutsch::CollatorDeutsch(BMessage *archive)
	: BCollatorAddOn(archive)
{
}


CollatorDeutsch::~CollatorDeutsch()
{
}


void 
CollatorDeutsch::GetSortKey(const char *string, BString *key, int8 strength,
	bool ignorePunctuation)
{
	if (strength == B_COLLATE_QUATERNARY) {
		// the difference between tertiary and quaternary collation strength
		// are usually a different handling of punctuation characters
		ignorePunctuation = false;
	}

	int32 length = strlen(string);

	switch (strength) {
		case B_COLLATE_PRIMARY:
		{
			char *begin = key->LockBuffer(length * 2);
				// the primary key needs to make space for doubled characters (like 'ß')

			char *end = putPrimarySortKey(string, begin, length, ignorePunctuation);
			*end = '\0';

			key->UnlockBuffer(end - begin);
			break;
		}

		case B_COLLATE_SECONDARY:
		{
			char *begin = key->LockBuffer(length * 3 + 1);
				// the primary key + the secondary key + separator char

			char *buffer = putPrimarySortKey(string, begin, length, ignorePunctuation);
			*buffer++ = '\01';
				// separator

			input_context context(ignorePunctuation);
			uint32 c;
			for (int32 i = 0; (c = getNextChar(&string, context)) && i < length; i++) {
				if (c < 0x80)
					*buffer++ = tolower(c);
				else
					BUnicodeChar::ToUTF8(BUnicodeChar::ToLower(c), &buffer);
			}
			*buffer = '\0';

			key->UnlockBuffer(buffer - begin);
			break;
		}

		case B_COLLATE_TERTIARY:
		case B_COLLATE_QUATERNARY:
		{
			char *begin = key->LockBuffer(length * 3 + 1);
				// the primary key + the tertiary key + separator char

			char *buffer = putPrimarySortKey(string, begin, length, ignorePunctuation);
			*buffer++ = '\01';
				// separator

			input_context context(ignorePunctuation);
			uint32 c;
			for (int32 i = 0; (c = getNextChar(&string, context)) && i < length; i++) {
				BUnicodeChar::ToUTF8(c, &buffer);
			}
			*buffer = '\0';

			key->UnlockBuffer(buffer + length - begin);
			break;
		}

		case B_COLLATE_IDENTICAL:
		default:
			key->SetTo(string, length);
			break;
	}
}


int 
CollatorDeutsch::Compare(const char *a, const char *b, int32 length, int8 strength,
	bool ignorePunctuation)
{
	if (strength != B_COLLATE_PRIMARY)
		return BCollatorAddOn::Compare(a, b, length, strength, ignorePunctuation);

	input_context contextA(ignorePunctuation);
	input_context contextB(ignorePunctuation);

	for (int32 i = 0; i < length; i++) {
		uint32 charA = getNextChar(&a, contextA);
		uint32 charB = getNextChar(&b, contextB);
		if (charA == 0)
			return charB == 0 ? 0 : -(int32)charB;
		else if (charB == 0)
			return (int32)charA;

		charA = getPrimaryChar(charA);
		charB = getPrimaryChar(charB);

		if (charA != charB)
			return (int32)charA - (int32)charB;
	}

	return 0;
}


status_t 
CollatorDeutsch::Archive(BMessage *archive, bool deep)
{
	status_t status = BArchivable::Archive(archive, deep);

	// add the add-on signature, so that the roster can load
	// us on demand!
	if (status == B_OK)
		status = archive->AddString("add_on", kSignature);

	return status;
}


BArchivable *
CollatorDeutsch::Instantiate(BMessage *archive)
{
	if (validate_instantiation(archive, "CollatorDeutsch"))
		return new CollatorDeutsch(archive);

	return NULL;
}


//	#pragma mark -


extern "C" BCollatorAddOn *
instantiate_collator(void)
{
	return new CollatorDeutsch();
}
