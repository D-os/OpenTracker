#ifndef _LOCALE_ROSTER_H_
#define _LOCALE_ROSTER_H_

#include <String.h>

class BLanguage;
class BLocale;
class BCollator;
class BCountry;
class BCatalog;
class BCatalogAddOn;

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

		status_t GetDefaultCollator(BCollator **) const;
		status_t GetDefaultLanguage(BLanguage **) const;
		status_t GetDefaultCountry(BCountry **) const;
		
		status_t GetPreferredLanguages(BMessage *) const;
		status_t SetPreferredLanguages(BMessage *);
			// the message contains one or more 'language'-string-fields 
			// which contain the language-name(s)

		status_t GetInstalledLanguages(BMessage *) const;
			// the message contains one or more 'language'-string-fields 
			// which contain the language-name(s)

		status_t GetInstalledCatalogs(BMessage *,
					const char* sigPattern = NULL,
					const char* langPattern = NULL,
					int32 fingerprint = 0) const;
			// the message contains...

//		status_t GetDefaultLanguage(BLanguage *);
//		status_t SetDefaultLanguage(BLanguage *);

		static const char *kCatLangAttr;
		static const char *kCatSigAttr;
		static const char *kCatFingerprintAttr;

	private:

		BCatalogAddOn* LoadCatalog(const char *signature, 
							const char *language = NULL,
							int32 fingerprint = 0);
		status_t UnloadCatalog(BCatalogAddOn *addOn);
		//
		BCatalogAddOn* CreateCatalog(const char *type,
							const char *signature, 
							const char *language);

		friend class BCatalog;
		friend status_t get_add_on_catalog(BCatalog*, const char*, int32);
};

#endif	/* _LOCALE_ROSTER_H_ */
