#ifndef _CATALOG_IN_ADD_ON_H_
#define _CATALOG_IN_ADD_ON_H_

#include <Catalog.h>
#include <Locale.h>
#include <LocaleRoster.h>

BCatalog *be_catalog = NULL;

status_t get_add_on_catalog(BCatalog* cat, const char *sig, int32 fingerprint) {
	if (!cat)
		return B_BAD_VALUE;
	cat->fCatalog = be_locale_roster->LoadCatalog(sig, NULL, fingerprint);
	be_catalog = cat;
	return cat->InitCheck();
}


#endif	/* _CATALOG_IN_ADD_ON_H_ */
