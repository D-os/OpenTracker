#ifndef _CATALOG_H_
#define _CATALOG_H_

#include <SupportDefs.h>
#include <String.h>

class BCatalogAddOn;
class BCatalogAddOnInfo;
class BLocale;
class BMessage;


class BCatalog {

	public:
		BCatalog(const char *signature, const char *language = NULL);
		~BCatalog();

		const char *GetString(const char *string,
									 const char *context=NULL,
									 const char *comment=NULL);
		const char *GetString(uint32 id);

		status_t GetData(const char *name, BMessage *msg);
		status_t GetData(uint32 id, BMessage *msg);

	private:
		BCatalogAddOn *fCatalog;

};


/************************************************************************/

// For BCatalog add-on implementations:

class BCatalogAddOn {
		friend class BLocaleRoster;
	public:
		BCatalogAddOn(const char *signature, 
						  const char *language,
						  BCatalogAddOnInfo* info);
		virtual ~BCatalogAddOn();

		virtual const char *GetString(const char *string,
												const char *context=NULL,
												const char *comment=NULL) = 0;
		virtual const char *GetString(uint32 id) = 0;

		// ToDo: setting the translated strings is not yet implemented:
		virtual bool CanWriteData() const;
		virtual status_t SetString(const char *string, 
											const char *translated,
											const char *context=NULL,
											const char *comment=NULL);
		virtual status_t SetString(int32 id, const char *translated);

		// the following could be used to localize non-textual data (e.g. icons),
		// but these will only be implemented if there's demand for such a 
		// feature:
		virtual status_t GetData(const char *name, BMessage *msg);
		virtual status_t GetData(uint32 id, BMessage *msg);
		virtual bool CanHaveData() const;
		virtual status_t SetData(const char *name, BMessage *msg);
		virtual status_t SetData(uint32 id, BMessage *msg);

	protected:
		BString fSignature;
		BCatalogAddOnInfo *fAddOnInfo;
		BString fLanguageName;
		BCatalogAddOn *fNext;
};

// every catalog-add-on should export the followin function:
extern "C" BCatalogAddOn *instantiate_catalog( const char* signature,
															  const char* language,
															  BCatalogAddOnInfo* info);

#endif	/* _CATALOG_H_ */
