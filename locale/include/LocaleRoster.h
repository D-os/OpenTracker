#ifndef _LOCALE_ROSTER_H_
#define _LOCALE_ROSTER_H_


#include <SupportDefs.h>

class BLanguage;
class BLocale;
class BCollator;
class BCountry;
class BCatalog;

enum {
	B_LOCALE_CHANGED	= '_LCC',
};


class BLocaleRoster {
	public:
		BLocaleRoster();
		~BLocaleRoster();

//		status_t GetCatalog(BLocale *,const char *mimeType, BCatalog *catalog);
//		status_t GetCatalog(const char *mimeType, BCatalog *catalog);
//		status_t SetCatalog(BLocale *,const char *mimeType, BCatalog *catalog);

//		status_t GetLocaleFor(const char *langCode,const char *countryCode);

		status_t GetDefaultCollator(BCollator **);
		status_t GetDefaultLanguage(BLanguage **);
		status_t GetDefaultCountry(BCountry **);

//		status_t GetPreferredLanguages(BList *);
//		status_t SetPreferredLanguages(BList *);

//		status_t GetDefaultLanguage(BLanguage *);
//		status_t SetDefaultLanguage(BLanguage *);

	private:
};

#endif	/* _LOCALE_ROSTER_H_ */