#ifndef _CATALOG_H_
#define _CATALOG_H_

#include <SupportDefs.h>
#include <String.h>

class BCatalogAddOn;
class BLocale;
class BMessage;


class BCatalog {

	public:
		BCatalog(const char *signature, const char *language = NULL);
		~BCatalog();

		const char *GetString(const char *string, const char *context=NULL,
						const char *comment=NULL);
		const char *GetString(uint32 id);

		status_t GetData(const char *name, BMessage *msg);
		status_t GetData(uint32 id, BMessage *msg);

		status_t InitCheck() const;

	private:
		BCatalogAddOn *fCatalog;

};


/************************************************************************/

// For BCatalog add-on implementations:

class BCatalogAddOn {
		friend class BLocaleRoster;
	public:
		BCatalogAddOn(const char *signature, const char *language);
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

		status_t InitCheck() const;

		// the following could be used to localize non-textual data (e.g. icons),
		// but these will only be implemented if there's demand for such a 
		// feature:
		virtual status_t GetData(const char *name, BMessage *msg);
		virtual status_t GetData(uint32 id, BMessage *msg);
		virtual bool CanHaveData() const;
		virtual status_t SetData(const char *name, BMessage *msg);
		virtual status_t SetData(uint32 id, BMessage *msg);

	protected:
		BString 			fSignature;
		BString 			fLanguageName;
		BCatalogAddOn 		*fNext;
		status_t 			fInitCheck;
};

inline status_t 
BCatalogAddOn::InitCheck() const
{ 
	return fInitCheck;
}

// every catalog-add-on should export two symbols...
// ...the function that instantiates a catalog for this add-on-type...
extern "C" BCatalogAddOn *instantiate_catalog(const char* signature,
							const char* language);
// ...and the priority which will be used to order the catalog-add-ons:
extern uint8 gCatalogAddOnPriority;

#endif	/* _CATALOG_H_ */
