#ifndef _DEFAULT_CATALOG_H_
#define _DEFAULT_CATALOG_H_

#include <Catalog.h>
#include <String.h>

class BCatalogAddOn;
class BCatalogAddOnInfo;
class BLocale;
class BMessage;


class DefaultCatalog : public BCatalogAddOn {

	public:
		DefaultCatalog(const char *signature, const char *language,
			BCatalogAddOnInfo* info);
		~DefaultCatalog();

		const char *GetString(const char *string, const char *context=NULL,
							const char *comment=NULL);
		const char *GetString(uint32 id);

		static BCatalogAddOn* Instantiate(const char *signature,
										const char *language, BCatalogAddOnInfo *info);
		static const uint8 DefaultCatalog::gDefaultCatalogAddOnPriority;

	private:

};

#endif	/* _DEFAULT_CATALOG_H_ */
