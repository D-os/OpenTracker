/* 
** Copyright 2003, Axel Dörfler, axeld@pinc-software.de. All rights reserved.
** Distributed under the terms of the OpenBeOS License.
*/


#include <LocaleRoster.h>
#include <Locale.h>
#include <Language.h>
#include <Country.h>
#include <Collator.h>


BLocaleRoster::BLocaleRoster()
{
}


BLocaleRoster::~BLocaleRoster()
{
}


status_t 
BLocaleRoster::GetDefaultCollator(BCollator **collator)
{
	// It should just use the archived collator from the locale settings;
	// if that is not available, just return the standard collator
	*collator = new BCollator();
	return B_OK;
}


status_t 
BLocaleRoster::GetDefaultLanguage(BLanguage **language)
{
	*language = new BLanguage(NULL);
	return B_OK;
}


status_t 
BLocaleRoster::GetDefaultCountry(BCountry **country)
{
	*country = new BCountry();
	return B_OK;
}

