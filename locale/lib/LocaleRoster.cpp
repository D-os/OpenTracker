/* 
** Distributed under the terms of the OpenBeOS License.
** Copyright 2003-2004. All rights reserved.
**
** Authors:	Axel Dörfler, axeld@pinc-software.de
**			Oliver Tappe, zooey@hirschkaefer.de
*/

#include <Catalog.h>
#include <Collator.h>
#include <Country.h>
#include <Directory.h>
#include <Entry.h>
#include <FindDirectory.h>
#include <Language.h>
#include <Locale.h>
#include <LocaleRoster.h>
#include <Node.h>
#include <Path.h>
#include <String.h>

const char *BLocaleRoster::kPriorityAttr = "LOCALE:priority";

typedef BCatalogAddOn* (*InstantiateCatalogFunc)(const char *name, 
																 const char *language,
																 BCatalogAddOnInfo *info);

struct BCatalogAddOnInfo {
	BString fName;
	BString fPath;
	image_id fAddOnImage;
	InstantiateCatalogFunc fInstantiateFunc;
	uint8 fPriority;
	uint8 fUsedCount;

	BCatalogAddOnInfo( const BString& name, const BString& path, uint8 priority)
		:	fName( name)
		,	fPath( path)
		,	fAddOnImage( B_NO_INIT)
		,	fInstantiateFunc( NULL)
		,	fPriority( priority)
		,	fUsedCount( 0)
	{
	}

	~BCatalogAddOnInfo() {
		if (fAddOnImage >= B_OK)
			unload_add_on( fAddOnImage);
	}
	
	bool operator< (const BCatalogAddOnInfo& right) const {
		return (fPriority < right.fPriority);
	}
};


BLocaleRoster::BLocaleRoster()
{
	// ToDo: change this to fetch preferred languages from prefs
	fPreferredLanguages.AddItem("Deutsch");
	fPreferredLanguages.AddItem("English");
}


BLocaleRoster::~BLocaleRoster()
{
	CleanupCatalogAddOns();
}


status_t 
BLocaleRoster::GetDefaultCollator(BCollator **collator)
{
	// It should just use the archived collator from the locale settings;
	// if that is not available, just return the standard collator
	if (!collator)
		return B_BAD_VALUE;
	*collator = new BCollator();
	return B_OK;
}


status_t 
BLocaleRoster::GetDefaultLanguage(BLanguage **language)
{
	if (!language)
		return B_BAD_VALUE;
	*language = new BLanguage(NULL);
	return B_OK;
}


status_t 
BLocaleRoster::GetDefaultCountry(BCountry **country)
{
	if (!country)
		return B_BAD_VALUE;
	*country = new BCountry();
	return B_OK;
}

status_t 
BLocaleRoster::GetPreferredLanguages(BList *languages)
{
	if (!languages)
		return B_BAD_VALUE;
	languages->MakeEmpty();
	languages->AddList( &fPreferredLanguages);
	return B_OK;
}

static int CompareInfos( const void* left, const void* right)
{
	return ((BCatalogAddOnInfo*)right)->fPriority 
				- ((BCatalogAddOnInfo*)left)->fPriority;
}

void 
BLocaleRoster::InitializeCatalogAddOns() 
{
	BDirectory addOnFolder;
	entry_ref eref;
	BNode node;
	dirent* dent;
	status_t err;
	char buf[4096];
	int32 count;
	int32 priority;

	directory_which folders[] = {
		B_COMMON_ADDONS_DIRECTORY,
		B_BEOS_ADDONS_DIRECTORY,
		static_cast< directory_which>( -1)
	};
	BPath addOnPath;
	for( int f=0; folders[f]>=0; ++f) {
		find_directory( folders[f], &addOnPath);
		BString addOnFolderName( addOnPath.Path());
		addOnFolderName << "/locale/catalogs";
		err = addOnFolder.SetTo( addOnFolderName.String());
		if (err != B_OK)
			continue;
		// scan through all the folder's entries for catalog add-ons:
		while ((count = addOnFolder.GetNextDirents((dirent* )buf, 4096)) > 0) {
			dent = (dirent* )buf;
			while (count-- > 0) {
				if (strcmp(dent->d_name, ".") && strcmp(dent->d_name, "..")) {
					// we have found (what should be) a catalog-add-on:
					eref.device = dent->d_pdev;
					eref.directory = dent->d_pino;
					eref.set_name( dent->d_name);
					node.SetTo( &eref);
					priority = 100;
						// default priority is very low
					node.ReadAttr( kPriorityAttr, B_UINT8_TYPE, 0, 
										&priority, sizeof( int32));
					fCatalogAddOnInfos.AddItem(
						(void*)new BCatalogAddOnInfo( dent->d_name, 
																addOnFolderName, 
																priority)
					);
				}
				// Bump the dirent-pointer by length of the dirent just handled:
				dent = (dirent* )((char* )dent + dent->d_reclen);
			}
		}
	}
	fCatalogAddOnInfos.SortItems( CompareInfos);
}

void
BLocaleRoster::CleanupCatalogAddOns() 
{
	int32 count = fCatalogAddOnInfos.CountItems();
	for( int32 i=0; i<count; ++i) {
		BCatalogAddOnInfo* info = (BCatalogAddOnInfo*)fCatalogAddOnInfos.ItemAt( i);
		delete info;
	}
	fCatalogAddOnInfos.MakeEmpty();
}

BCatalogAddOn*
BLocaleRoster::LoadCatalog(const char* signature, const char* language)
{
	if (!signature)
		return NULL;

	int32 count = fCatalogAddOnInfos.CountItems();
	for( int32 i=0; i<count; ++i) {
		BCatalogAddOnInfo* info = (BCatalogAddOnInfo*)fCatalogAddOnInfos.ItemAt( i);

		if (info->fAddOnImage < B_OK) {
			// add-on has not been loaded yet, so we try to load it:
			BString fullAddOnPath( info->fPath);
			fullAddOnPath << "/" << info->fName;
			info->fAddOnImage = load_add_on( fullAddOnPath.String());
			if (info->fAddOnImage < B_OK)
				continue;
					// add-on couldn't be loaded, try next one
		}

		BCatalogAddOn *catalog = NULL;
		if (get_image_symbol(info->fAddOnImage, "instantiate_catalog",
									B_SYMBOL_TYPE_TEXT, 
									(void **)&info->fInstantiateFunc) == B_OK) {
			BList languages;
			if (language)
				// try to load language with given name:
				languages.AddItem( (void*)language);
			else
				// try to load one of the preferred languages:
				languages.AddList( &fPreferredLanguages);

			int32 langCount = languages.CountItems();
			for( int32 l=0; l<langCount; ++l) {
				BString lang = (const char*)languages.ItemAt( l);
				catalog = info->fInstantiateFunc(signature, lang.String(), info);
				if (catalog != NULL) {
					catalog->fAddOnInfo = info;
					info->fUsedCount++;
					int32 pos;
					BCatalogAddOn *currCatalog=catalog, *nextCatalog;
					while( (pos = lang.FindLast( '-')) > B_OK) {
						// language is based on parent, so we load that, too:
						lang.Truncate( pos);
						nextCatalog = info->fInstantiateFunc(signature, lang.String(), info);
						if (nextCatalog) {
							currCatalog->fNext = nextCatalog;
							currCatalog = nextCatalog;
						}
					}
					return catalog;
				}
			}
		} 
		if (!catalog) {
			unload_add_on(info->fAddOnImage);
			info->fAddOnImage = B_NO_INIT;
			info->fInstantiateFunc = NULL;
		}
	}

	return NULL;
}

status_t
BLocaleRoster::UnloadCatalog(BCatalogAddOn* catalog)
{
	if (!catalog)
		return B_BAD_VALUE;
	int32 count = fCatalogAddOnInfos.CountItems();
	for( int32 i=0; i<count; ++i) {
		BCatalogAddOnInfo* info = (BCatalogAddOnInfo*)fCatalogAddOnInfos.ItemAt( i);
		if (info == catalog->fAddOnInfo) {
			delete catalog;
			info->fUsedCount--;
			if (!info->fUsedCount) {
				unload_add_on( info->fAddOnImage);
				info->fAddOnImage = B_NO_INIT;
				info->fInstantiateFunc = NULL;
			}
			return B_OK;
		}
	}
	return B_ERROR;
}

