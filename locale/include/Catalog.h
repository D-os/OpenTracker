#ifndef _CATALOG_H_
#define _CATALOG_H_

#include <SupportDefs.h>


class BCatalogAddOn;


class BCatalog {
	public:
		BCatalog(const BLocale *locale, const char *signature);
		~BCatalog();

		const char *GetString(const char *string);
		const char *GetString(uint32 id);

		status_t GetData(const char *name, BMessage &msg);
		status_t GetData(uint32 id, BMessage &msg);

	private:
		BCatalogAddOn *fCatalog;
};


/************************************************************************/

// For BCatalog add-on implementations:

class BCatalogAddOn {
	public:
		BCatalogAddOn(const BLocale *locale, const char *signature, const BCatalogAddOn *next);
		virtual ~BCatalogAddOn() = 0;

		virtual const char *GetString(const char *string) = 0;
		virtual const char *GetString(uint32 id) = 0;
		virtual status_t GetData(const char *name, BMessage &msg) = 0;
		virtual status_t GetData(uint32 id, BMessage &msg) = 0;

		virtual bool CanHaveData() const;
		virtual bool CanWriteData() const;

		virtual status_t SetString(const char *string, const char *translated);
		virtual status_t SetString(int32 id, const char *translated);
		virtual status_t SetData(const char *name, BMessage &msg);
		virtual status_t SetData(uint32 id, BMessage &msg);
};

#endif	/* _CATALOG_H_ */
