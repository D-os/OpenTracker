#ifndef _DEFAULT_CATALOG_H_
#define _DEFAULT_CATALOG_H_

#include <hash_map>

#include <Catalog.h>
#include <String.h>

namespace BPrivate {


/*
 * The key-type for the hash_map which maps native strings or IDs to
 * the corresponding translated string.
 * The key-type should be efficient to use if it is just created by an ID
 * but it should also support being created from up to three strings,
 * which as a whole specify the key to the translated string.
 */
struct CatKey {
	BString fKey;
		// the key-string consists of three values separated by a special 
		// token:
		// - the native string
		// - the context of the string's usage
		// - a comment that can be used to separate strings that
		//   are identical otherwise (useful for the translator)
	size_t fHashVal;
		// the hash-value of fKey
	uint32 fFlags;
		// with respect to the catalog-editor, each translation can be
		// in different states (empty, unchecked, checked, etc.).
		// This state (and potential other flags) lives in the fFlags member.
	CatKey(const char *str, const char *ctx, const char *cmt);
	CatKey(uint32 id);
	CatKey();
	bool operator== (const CatKey& right) const;
};


/*
 * the hash-access functor which is being used to access the hash-value
 * stored inside of each key.
 */
__STL_TEMPLATE_NULL struct hash<CatKey>
{
  size_t operator()(const BPrivate::CatKey &key) const;
};


/*
 * The implementation of the Locale Kit's standard catalog-type.
 * Currently it only maps CatKey to a BString (the translated string),
 * but the value-type might change to add support for shortcuts and/or
 * graphical data (like button-images and the like).
 */
class DefaultCatalog : public BCatalogAddOn {

	public:
		DefaultCatalog(const char *signature, const char *language,
			int32 fingerprint);
				// constructor for normal use
		DefaultCatalog(entry_ref *appOrAddOnRef);
				// constructor for embedded catalog
		DefaultCatalog(const char *path, const char *signature, 
			const char *language);
				// constructor for editor-app
					   
		~DefaultCatalog();

		// overrides of BCatalogAddOn:
		const char *GetString(const char *string, const char *context=NULL,
						const char *comment=NULL);
		const char *GetString(uint32 id);
		//
		status_t SetString(const char *string, const char *translated, 
					const char *context=NULL, const char *comment=NULL);
		status_t SetString(uint32 id, const char *translated);
		void UpdateFingerprint();

		// interface for catalog-editor-app (and testing apps):
		status_t ReadFromFile(const char *path = NULL);
		status_t ReadFromAttribute(entry_ref *appOrAddOnRef);
		status_t ReadFromResource(entry_ref *appOrAddOnRef);
		status_t WriteToFile(const char *path = NULL);
		status_t WriteToAttribute(entry_ref *appOrAddOnRef);
		status_t WriteToResource(entry_ref *appOrAddOnRef);
		//
		void MakeEmpty();
		int32 CountItems() const;
		void Resize(int32 size);

		static BCatalogAddOn *Instantiate(const char *signature,
								const char *language,
								int32 fingerprint);
		static BCatalogAddOn *InstantiateEmbedded(entry_ref *appOrAddOnRef);
		static BCatalogAddOn *Create(const char *signature,
								const char *language);
		static const uint8 DefaultCatalog::gDefaultCatalogAddOnPriority;

	private:
		status_t Flatten(BDataIO *dataIO);
		status_t Unflatten(BDataIO *dataIO);
		int32 ComputeFingerprint() const;
		void UpdateAttributes(BFile& catalogFile);

		typedef hash_map<CatKey, BString> CatMap;
		CatMap 				fCatMap;
		mutable BString 	fPath;
};


}	// namespace BPrivate

using namespace BPrivate;

#endif	/* _DEFAULT_CATALOG_H_ */
