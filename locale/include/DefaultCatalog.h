#ifndef _DEFAULT_CATALOG_H_
#define _DEFAULT_CATALOG_H_

#include <hash_map>

#include <Catalog.h>
#include <String.h>

namespace BPrivate {

struct CatKey {
	BString fKey;
		// the key consists of three values separated by a special token:
		// - the native string
		// - the context of the string's usage
		// - a comment that can be used to separate strings that
		//   are identical otherwise (useful for the translator)
	size_t fHashVal;
		// the hash-value of fKey
	CatKey(const char *str, const char *ctx, const char *cmt);
	CatKey(uint32 id);
	CatKey();
	bool operator== (const CatKey& right) const;
};

__STL_TEMPLATE_NULL struct hash<CatKey>
{
  size_t operator()(const BPrivate::CatKey &key) const;
};

class DefaultCatalog : public BCatalogAddOn {

	public:
		DefaultCatalog(const char *signature, const char *language);
		DefaultCatalog(const char *signature, const char *language,
			const char *path, bool create = false);
		~DefaultCatalog();

		const char *GetString(const char *string, const char *context=NULL,
						const char *comment=NULL);
		const char *GetString(uint32 id);

		status_t SetString(const char *string, const char *translated, 
					const char *context=NULL, const char *comment=NULL);
		status_t SetString(uint32 id, const char *translated);

		status_t ReadFromDisk(const char *path);
		status_t WriteToDisk(const char *path = NULL) const;

		void MakeEmpty();

		static BCatalogAddOn *Instantiate(const char *signature,
								const char *language);
		static const uint8 DefaultCatalog::gDefaultCatalogAddOnPriority;

void resize( size_t sz) { fCatMap.resize(sz); }
size_t size() const { return fCatMap.size(); }

	private:
		typedef hash_map<CatKey, BString> CatMap;
		CatMap fCatMap;
		mutable BString fPath;
};

}	// namespace BPrivate

using namespace BPrivate;

#endif	/* _DEFAULT_CATALOG_H_ */
