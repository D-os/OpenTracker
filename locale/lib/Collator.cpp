/* 
** Copyright 2003, Axel Dörfler, axeld@pinc-software.de. All rights reserved.
** Distributed under the terms of the OpenBeOS License.
*/


#include <Collator.h>
#include <UnicodeChar.h>
#include <String.h>

#include <stdio.h>	// <- for debugging only
#include <ctype.h>


// ToDo: specify the length in BString::UnlockBuffer() to speed things up!
// ToDo: secondary collation does not work correctly
// ToDo: tertiary collation does not work correctly, as well

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


//	#pragma mark -


BCollator::BCollator()
	:
	fStrength(B_COLLATE_PRIMARY)
{
	// ToDo: the collator construction will have to change...
	
	fCollator = new BCollatorAddOn();
}


BCollator::~BCollator()
{
	delete fCollator;
}


void 
BCollator::GetSortKey(const char *string, BString *key, int8 strength)
{
	if (strength == B_COLLATE_DEFAULT)
		strength = fStrength;

	fCollator->GetSortKey(string, key, strength);
}


int 
BCollator::Compare(const char *a, const char *b, int32 length, int8 strength)
{
	if (length == -1)	// match the whole string
		length = 0x7fffffff;

	return fCollator->Compare(a, b, length, strength == B_COLLATE_DEFAULT ? fStrength : strength);
}


//	#pragma mark -


BCollatorAddOn::BCollatorAddOn()
{
}


BCollatorAddOn::~BCollatorAddOn()
{
}


void 
BCollatorAddOn::GetSortKey(const char *string, BString *key, int8 strength)
{
	int32 length = strlen(string);

	switch (strength) {
		case B_COLLATE_PRIMARY:
		{
			char *buffer = key->LockBuffer(length);
				// the key can't be longer than the original
			uint32 c;
			for (int32 i = 0; (c = BUnicodeChar::FromUTF8(&string)) && i < length; i++) {
				if (c < 0x80)
					*buffer++ = tolower(c);
				else
					BUnicodeChar::ToUTF8(getPrimaryChar(c), &buffer);
			}
			*buffer = '\0';
			key->UnlockBuffer();
			break;
		}

		case B_COLLATE_SECONDARY:
		{
			char *buffer = key->LockBuffer(length);
				// the key can't be longer than the original
			uint32 c;
			for (int32 i = 0; (c = BUnicodeChar::FromUTF8(&string)) && i < length; i++) {
				if (c < 0x80)
					*buffer++ = tolower(c);
				else
					BUnicodeChar::ToUTF8(BUnicodeChar::ToLower(c), &buffer);
			}
			*buffer = '\0';
			key->UnlockBuffer();
			break;
		}

		case B_COLLATE_TERTIARY:
		case B_COLLATE_QUATERNARY:
		case B_COLLATE_IDENTICAL:
		default:
			key->SetTo(string, length);
			break;
	}
}


int 
BCollatorAddOn::Compare(const char *a, const char *b, int32 length, int8 strength)
{
	switch (strength) {
		case B_COLLATE_PRIMARY:
		{
			for (int32 i = 0; i < length; i++) {
				uint32 charA = BUnicodeChar::FromUTF8(&a);
				uint32 charB = BUnicodeChar::FromUTF8(&b);
				if (charA == 0)
					return charB == 0 ? 0 : charB;
				else if (charB == 0)
					return -(int32)charA;

				charA = getPrimaryChar(charA);
				charB = getPrimaryChar(charB);

				if (charA != charB)
					return (int32)charA - (int32)charB;
			}
			return 0;
		}

		case B_COLLATE_SECONDARY:
		{
			for (int32 i = 0; i < length; i++) {
				uint32 charA = BUnicodeChar::FromUTF8(&a);
				uint32 charB = BUnicodeChar::FromUTF8(&b);
				if (charA == 0)
					return charB == 0 ? 0 : charB;
				else if (charB == 0)
					return -(int32)charA;

				charA = BUnicodeChar::ToLower(charA);
				charB = BUnicodeChar::ToLower(charB);

				if (charA != charB)
					return (int32)charA - (int32)charB;
			}
			return 0;
		}

		case B_COLLATE_TERTIARY:
		case B_COLLATE_QUATERNARY:
		case B_COLLATE_IDENTICAL:
		default:
			return strncmp(a, b, length);
	}
}

