#ifndef _CATALOG_H_
#define _CATALOG_H_

#include <SupportDefs.h>
#include <String.h>

class BCatalogAddOn;
class BLocale;
class BMessage;


class BCatalog {

	public:
		BCatalog();
		BCatalog(const char *signature, const char *language = NULL,
			int32 fingerprint = 0);
		~BCatalog();

		const char *GetString(const char *string, const char *context=NULL,
						const char *comment=NULL);
		const char *GetString(uint32 id);

		status_t GetData(const char *name, BMessage *msg);
		status_t GetData(uint32 id, BMessage *msg);

		status_t GetSignature(BString *sig);
		status_t GetLanguage(BString *lang);
		status_t GetFingerprint(int32 *fp);

		status_t InitCheck() const;

	private:
		BCatalog(const char *type, const char *signature, const char *language);
		static status_t GetAppCatalog(BCatalog*);

		BCatalogAddOn *fCatalog;

		BCatalog(const BCatalog&);
		const BCatalog& operator= (const BCatalog&);
			// hide assignment and copy-constructor

		friend class BLocale;
		friend class CatalogSpeed;
		friend class CatalogTest;
		friend class CatalogTestAddOn;
		friend status_t get_add_on_catalog(BCatalog*, const char*, int32);
};


extern BCatalog* be_catalog;


#ifndef B_AVOID_TRANSLATION_MACROS
#include <typeinfo>
// macros for easy catalog-access, define B_AVOID_TRANSLATION_MACROS if
// you don't want these
//
// N.B.: these are only valid in class-scope (i.e. you can only use them
//       in methods, not in functions, as they make use of RTTI!
#undef TR
#define TR(str) \
	(be_catalog->GetString((str), typeid(*this).name()))

#undef TR_CMT
#define TR_CMT(str,cmt) \
	(be_catalog->GetString((str), typeid(*this).name(), (cmt)))

#undef TR_ALL
#define TR_ALL(str,ctx,cmt) \
	(be_catalog->GetString((str), (ctx), (cmt)))

#undef TR_ID
#define TR_ID(id) \
	(be_catalog->GetString((id)))

#endif	/* B_AVOID_TRANSLATION_MACROS */


/************************************************************************/
// For BCatalog add-on implementations:

class BCatalogAddOn {
		friend class BLocaleRoster;
	public:
		BCatalogAddOn(const char *signature, const char *language,
					  int32 fingerprint);
		virtual ~BCatalogAddOn();

		virtual const char *GetString(const char *string, 
								const char *context=NULL,
								const char *comment=NULL) = 0;
		virtual const char *GetString(uint32 id) = 0;

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
		virtual void UpdateFingerprint();

		BString 			fSignature;
		BString 			fLanguageName;
		int32				fFingerprint;
		BCatalogAddOn 		*fNext;
		status_t 			fInitCheck;
		
		friend class BCatalog;
};

// every catalog-add-on should export these symbols...
// ...the function that instantiates a catalog for this add-on-type...
extern "C" BCatalogAddOn *instantiate_catalog(const char *signature,
							const char *language, int32 fingerprint);
// ...the function that creates an empty catalog for this add-on-type...
extern "C" BCatalogAddOn *create_catalog(const char *signature,
							const char *language);
// ...and the priority which will be used to order the catalog-add-ons:
extern uint8 gCatalogAddOnPriority;


/*
 * BCatalog - inlines for trivial accessors:
 */
inline const char *
BCatalog::GetString(const char *string, const char *context, const char *comment)
{
	if (fCatalog)
		return fCatalog->GetString(string, context, comment);
	else
		return string;
}


inline const char *
BCatalog::GetString(uint32 id)
{
	if (fCatalog)
		return fCatalog->GetString(id);
	else
		return "";
}


inline status_t 
BCatalog::GetData(const char *name, BMessage *msg)
{
	if (fCatalog)
		return fCatalog->GetData(name, msg);
	else
		return B_NO_INIT;
}


inline status_t 
BCatalog::GetData(uint32 id, BMessage *msg)
{
	if (fCatalog)
		return fCatalog->GetData(id, msg);
	else
		return B_NO_INIT;
}


inline status_t
BCatalog::GetSignature(BString *sig) {
	if (!sig)
		return B_BAD_VALUE;
	if (!fCatalog)
		return B_NO_INIT;
	*sig = fCatalog->fSignature;
	return B_OK;
}


inline status_t 
BCatalog::GetLanguage(BString *lang) {
	if (!lang)
		return B_BAD_VALUE;
	if (!fCatalog)
		return B_NO_INIT;
	*lang = fCatalog->fLanguageName;
	return B_OK;
}	


inline status_t 
BCatalog::GetFingerprint(int32 *fp) {
	if (!fp)
		return B_BAD_VALUE;
	if (!fCatalog)
		return B_NO_INIT;
	*fp = fCatalog->fFingerprint;
	return B_OK;
}


inline status_t
BCatalog::InitCheck() const
{
	return fCatalog 
				? fCatalog->InitCheck() 
				: B_NO_INIT;
}


#endif	/* _CATALOG_H_ */
