/* 
** Distributed under the terms of the OpenBeOS License.
** Copyright 2003-2004. All rights reserved.
**
** Authors:	Axel Dörfler, axeld@pinc-software.de
**			Oliver Tappe, zooey@hirschkaefer.de
*/

#include <stdio.h>		// for debug only

#include <Autolock.h>
#include <Catalog.h>
#include <Collator.h>
#include <Country.h>
#include <DefaultCatalog.h>
#include <Directory.h>
#include <Entry.h>
#include <FindDirectory.h>
#include <fs_index.h> 
#include <Language.h>
#include <Locale.h>
#include <LocaleRoster.h>
#include <Node.h>
#include <Path.h>
#include <String.h>
#include <Volume.h>
#include <VolumeRoster.h>

static const char *kPriorityAttr = "ADDON:priority";

typedef BCatalogAddOn *(*InstantiateCatalogFunc)(const char *name, 
	const char *language, int32 fingerprint);

typedef BCatalogAddOn *(*CreateCatalogFunc)(const char *name, 
	const char *language);

static BLocaleRoster gLocaleRoster;
BLocaleRoster *be_locale_roster = &gLocaleRoster;


/*
 * info about a single catalog-add-on (representing a catalog type):
 */
struct BCatalogAddOnInfo {
	BString fName;
	BString fPath;
	image_id fAddOnImage;
	InstantiateCatalogFunc fInstantiateFunc;
	CreateCatalogFunc fCreateFunc;
	uint8 fPriority;
	BList fLoadedCatalogs;
	bool fIsEmbedded;
		// an embedded add-on actually isn't an add-on, it is included
		// as part of the library. The DefaultCatalog is such a beast!

	BCatalogAddOnInfo(const BString& name, const BString& path, uint8 priority);
	~BCatalogAddOnInfo();
	bool MakeSureItsLoaded();
	void UnloadIfPossible();
};


BCatalogAddOnInfo::BCatalogAddOnInfo(const BString& name, const BString& path, 
	uint8 priority)
	:	
	fName(name),
	fPath(path),
	fAddOnImage(B_NO_INIT),
	fInstantiateFunc(NULL),
	fCreateFunc(NULL),
	fPriority(priority),
	fIsEmbedded(path.Length()==0)
{
}


BCatalogAddOnInfo::~BCatalogAddOnInfo()
{
	int32 count = fLoadedCatalogs.CountItems();
	for (int32 i=0; i<count; ++i) {
		BCatalogAddOn* cat 
			= reinterpret_cast<BCatalogAddOn*>(fLoadedCatalogs.ItemAt(i));
		delete cat;
	}
	fLoadedCatalogs.MakeEmpty();
	UnloadIfPossible();
}
	

bool
BCatalogAddOnInfo::MakeSureItsLoaded() {
	if (!fIsEmbedded && fAddOnImage < B_OK) {
		// add-on has not been loaded yet, so we try to load it:
		BString fullAddOnPath(fPath);
		fullAddOnPath << "/" << fName;
		fAddOnImage = load_add_on(fullAddOnPath.String());
		if (fAddOnImage >= B_OK) {
			get_image_symbol(fAddOnImage, "instantiate_catalog",
				B_SYMBOL_TYPE_TEXT, (void **)&fInstantiateFunc);
			get_image_symbol(fAddOnImage, "create_catalog",
				B_SYMBOL_TYPE_TEXT, (void **)&fCreateFunc);
			return true;
		} else
			return false;
	}
	return true;
}


void
BCatalogAddOnInfo::UnloadIfPossible() {
	if (!fIsEmbedded && fLoadedCatalogs.IsEmpty()) {
		unload_add_on(fAddOnImage);
		fAddOnImage = B_NO_INIT;
		fInstantiateFunc = NULL;
		fCreateFunc = NULL;
	}
}


// helper function that make sure an attribute-index exists:
// ToDo: maybe this should be put into some util-file?
static void EnsureIndexExists( const char *attrName) {
	BVolume bootVol;
	BVolumeRoster volRoster;
	if (volRoster.GetBootVolume(&bootVol) != B_OK)
		return;
	struct index_info idxInfo;
	if (fs_stat_index(bootVol.Device(), attrName, &idxInfo) != 0) {
		status_t res = fs_create_index(bootVol.Device(), attrName, 
			B_STRING_TYPE, 0);
		if (res == 0)
			// ToDo: write info about index creation to syslog
			;
		else
			// ToDo: write info about index creation failure to syslog
			;
	}
}


/*
 * the global data that is shared between all roster-objects:
 */
struct RosterData {
	BLocker fLock;
	BList fCatalogAddOnInfos;
	BMessage fPreferredLanguages;
	//
	RosterData();
	~RosterData();
	void InitializeCatalogAddOns();
	void CleanupCatalogAddOns();
	static int CompareInfos(const void *left, const void *right);
};
static RosterData gRosterData;


RosterData::RosterData()
	:
	fLock( "LocaleRosterData")
{
	BAutolock lock( fLock);
	assert( lock.IsLocked());

	// make sure the indices required for catalog-traversal are there:
	EnsureIndexExists(BLocaleRoster::kCatLangAttr);
	EnsureIndexExists(BLocaleRoster::kCatSigAttr);

	InitializeCatalogAddOns();
	// ToDo: change this to fetch preferred languages from prefs
	fPreferredLanguages.AddString("language", "german");
	fPreferredLanguages.AddString("language", "english");
}


RosterData::~RosterData()
{
	BAutolock lock( fLock);
	assert( lock.IsLocked());
	CleanupCatalogAddOns();
}


int
RosterData::CompareInfos(const void *left, const void *right)
{
	return ((BCatalogAddOnInfo*)right)->fPriority 
		- ((BCatalogAddOnInfo*)left)->fPriority;
}


void 
RosterData::InitializeCatalogAddOns() 
{
	BAutolock lock( fLock);
	assert( lock.IsLocked());

	// add info about embedded default catalog:
	BCatalogAddOnInfo *defaultCatalogAddOnInfo
		= new BCatalogAddOnInfo("Default", "", 
			 DefaultCatalog::gDefaultCatalogAddOnPriority);
	defaultCatalogAddOnInfo->fInstantiateFunc = DefaultCatalog::Instantiate;
	defaultCatalogAddOnInfo->fCreateFunc = DefaultCatalog::Create;
	fCatalogAddOnInfos.AddItem((void*)defaultCatalogAddOnInfo);

	directory_which folders[] = {
		B_COMMON_ADDONS_DIRECTORY,
		B_BEOS_ADDONS_DIRECTORY,
		static_cast<directory_which>(-1)
	};
	BPath addOnPath;
	BDirectory addOnFolder;
	char buf[4096];
	status_t err;
	for (int f=0; folders[f]>=0; ++f) {
		find_directory(folders[f], &addOnPath);
		BString addOnFolderName(addOnPath.Path());
		addOnFolderName << "/locale/catalogs";
		err = addOnFolder.SetTo(addOnFolderName.String());
		if (err != B_OK)
			continue;

		// scan through all the folder's entries for catalog add-ons:
		int32 count;
		int8 priority;
		entry_ref eref;
		BNode node;
		BEntry entry;
		dirent *dent;
		while ((count = addOnFolder.GetNextDirents((dirent *)buf, 4096)) > 0) {
			dent = (dirent *)buf;
			while (count-- > 0) {
				if (strcmp(dent->d_name, ".") && strcmp(dent->d_name, "..")) {
					// we have found (what should be) a catalog-add-on:
					eref.device = dent->d_pdev;
					eref.directory = dent->d_pino;
					eref.set_name(dent->d_name);
					entry.SetTo(&eref, true);
						// traverse through any links to get to the real thang!
					node.SetTo(&entry);
					priority = -1;
					if (node.ReadAttr(kPriorityAttr, B_INT8_TYPE, 0, 
						&priority, sizeof(int8)) <= 0) {
						// add-on has no priority-attribute yet, so we load it to
						// fetch the priority from the corresponding symbol...
						BString fullAddOnPath(addOnFolderName);
						fullAddOnPath << "/" << dent->d_name;
						image_id image = load_add_on(fullAddOnPath.String());
						if (image >= B_OK) {
							uint8 *prioPtr;
							if (get_image_symbol(image, "gCatalogAddOnPriority",
								B_SYMBOL_TYPE_TEXT, 
								(void **)&prioPtr) == B_OK) {
								priority = *prioPtr;
								node.WriteAttr(kPriorityAttr, B_INT8_TYPE, 0, 
									&priority, sizeof(int8));
							}
							unload_add_on(image);
						}
#ifdef DEBUG
						else
							printf("Could not load add-on %s, error: %s\n", 
								fullAddOnPath.String(), strerror(image));
#endif
					}
					if (priority >= 0) {
						// add-ons with priority<0 will be ignored
						fCatalogAddOnInfos.AddItem(
							(void*)new BCatalogAddOnInfo(dent->d_name, 
								addOnFolderName, priority)
						);
					}
				}
				// Bump the dirent-pointer by length of the dirent just handled:
				dent = (dirent *)((char *)dent + dent->d_reclen);
			}
		}
	}
	fCatalogAddOnInfos.SortItems(CompareInfos);
}


void
RosterData::CleanupCatalogAddOns() 
{
	BAutolock lock( fLock);
	assert( lock.IsLocked());
	int32 count = fCatalogAddOnInfos.CountItems();
	for (int32 i=0; i<count; ++i) {
		BCatalogAddOnInfo *info = (BCatalogAddOnInfo*)fCatalogAddOnInfos.ItemAt(i);
		delete info;
	}
	fCatalogAddOnInfos.MakeEmpty();
}


/*
 * BLocaleRoster, the exported interface to the locale roster data:
 */
const char *BLocaleRoster::kCatLangAttr = "BEOS:LOCALE_LANGUAGE";
const char *BLocaleRoster::kCatSigAttr = "BEOS:LOCALE_SIGNATURE";
const char *BLocaleRoster::kCatFingerprintAttr = "BEOS:LOCALE_FINGERPRINT";


BLocaleRoster::BLocaleRoster()
{
}


BLocaleRoster::~BLocaleRoster()
{
}


status_t 
BLocaleRoster::GetDefaultCollator(BCollator **collator) const
{
	// It should just use the archived collator from the locale settings;
	// if that is not available, just return the standard collator
	if (!collator)
		return B_BAD_VALUE;
	*collator = new BCollator();
	return B_OK;
}


status_t 
BLocaleRoster::GetDefaultLanguage(BLanguage **language) const
{
	if (!language)
		return B_BAD_VALUE;
	*language = new BLanguage(NULL);
	return B_OK;
}


status_t 
BLocaleRoster::GetDefaultCountry(BCountry **country) const
{
	if (!country)
		return B_BAD_VALUE;
	*country = new BCountry();
	return B_OK;
}


status_t 
BLocaleRoster::GetPreferredLanguages(BMessage *languages) const
{
	if (!languages)
		return B_BAD_VALUE;

	BAutolock lock( gRosterData.fLock);
	assert( lock.IsLocked());

	*languages = gRosterData.fPreferredLanguages;
	return B_OK;
}


status_t 
BLocaleRoster::SetPreferredLanguages(BMessage *languages)
{
	BAutolock lock( gRosterData.fLock);
	assert( lock.IsLocked());

	if (languages)
		gRosterData.fPreferredLanguages = *languages;
	else
		gRosterData.fPreferredLanguages.MakeEmpty();
	return B_OK;
}


BCatalogAddOn*
BLocaleRoster::CreateCatalog(const char *type, const char *signature, 
	const char *language)
{
	if (!type || !signature || !language)
		return NULL;

	BAutolock lock( gRosterData.fLock);
	assert( lock.IsLocked());

	int32 count = gRosterData.fCatalogAddOnInfos.CountItems();
	for (int32 i=0; i<count; ++i) {
		BCatalogAddOnInfo *info 
			= (BCatalogAddOnInfo*)gRosterData.fCatalogAddOnInfos.ItemAt(i);
		if (info->fName.ICompare(type)!=0 || !info->MakeSureItsLoaded() 
			|| !info->fCreateFunc)
			continue;

		BCatalogAddOn *catalog = info->fCreateFunc(signature, language);
		if (catalog) {
			info->fLoadedCatalogs.AddItem(catalog);
			info->UnloadIfPossible();
			return catalog;
		}
	}

	return NULL;
}


BCatalogAddOn*
BLocaleRoster::LoadCatalog(const char *signature, const char *language,
	int32 fingerprint)
{
	if (!signature)
		return NULL;

	BAutolock lock( gRosterData.fLock);
	assert( lock.IsLocked());

	int32 count = gRosterData.fCatalogAddOnInfos.CountItems();
	for (int32 i=0; i<count; ++i) {
		BCatalogAddOnInfo *info 
			= (BCatalogAddOnInfo*)gRosterData.fCatalogAddOnInfos.ItemAt(i);

		if (!info->MakeSureItsLoaded() || !info->fInstantiateFunc)
			continue;
		BMessage languages;
		if (language)
			// try to load language with given name:
			languages.AddString("language", language);
		else
			// try to load one of the preferred languages:
			GetPreferredLanguages( &languages);

		BCatalogAddOn *catalog;
		const char *lang;
		for (int32 l=0; languages.FindString("language", l, &lang)==B_OK; ++l) {
			catalog = info->fInstantiateFunc(signature, lang, fingerprint);
			if (catalog) {
				info->fLoadedCatalogs.AddItem(catalog);
/*
The following could be used to chain-load catalogs for languages that 
depend on other languages. We just need to define how we'd like to express
the language hierarchy (such that this code can traverse from child to parent
language). The implementation below uses the filename for that (i.e. it
traverses from "english-british-oxford" to "english-british" to "english"):
				char *pos;
				BCatalogAddOn *currCatalog=catalog, *nextCatalog;
				while ((pos = strrchr(lang, '-')) != NULL) {
					// language is based on parent, so we load that, too:
					*pos = '\0';
					nextCatalog = info->fInstantiateFunc(signature, lang, fingerprint);
					if (nextCatalog) {
						info->fLoadedCatalogs.AddItem(nextCatalog);
						currCatalog->fNext = nextCatalog;
						currCatalog = nextCatalog;
					}
				}
*/
				return catalog;
			}
		}
		info->UnloadIfPossible();
	}

	return NULL;
}


status_t
BLocaleRoster::UnloadCatalog(BCatalogAddOn *catalog)
{
	if (!catalog)
		return B_BAD_VALUE;

	BAutolock lock( gRosterData.fLock);
	assert( lock.IsLocked());

	int32 count = gRosterData.fCatalogAddOnInfos.CountItems();
	for (int32 i=0; i<count; ++i) {
		BCatalogAddOnInfo *info 
			= reinterpret_cast<BCatalogAddOnInfo*>(
				gRosterData.fCatalogAddOnInfos.ItemAt(i)
			);
		if (info->fLoadedCatalogs.HasItem(catalog)) {
			info->fLoadedCatalogs.RemoveItem(catalog);
			delete catalog;
			info->UnloadIfPossible();
			return B_OK;
		}
	}
	return B_ERROR;
}
