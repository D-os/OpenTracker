/* 
** Copyright 2003, Axel Dörfler, axeld@pinc-software.de. All rights reserved.
** Distributed under the terms of the OpenBeOS License.
*/


#include <Locale.h>
#include <LocaleRoster.h>
#include <List.h>


static BLocaleRoster gLocaleRoster;
BLocaleRoster *be_locale_roster = &gLocaleRoster;

static BLocale gLocale;
BLocale *be_locale = &gLocale;


BLocale::BLocale()
{
	be_locale_roster->GetDefaultCollator(&fCollator);
	be_locale_roster->GetDefaultCountry(&fCountry);
	be_locale_roster->GetDefaultLanguage(&fLanguage);
}


BLocale::~BLocale()
{
}


const char *
BLocale::GetString(uint32 id)
{
	// Note: this code assumes a certain order of the string bases

	if (id >= B_OTHER_STRINGS_BASE) {
		if (id == B_CODESET)
			return "UTF-8";

		return "";
	}
	if (id >= B_LANGUAGE_STRINGS_BASE)
		return fLanguage->GetString(id);

	return fCountry->GetString(id);
}

