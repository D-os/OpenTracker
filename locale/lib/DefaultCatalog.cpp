/* 
** Copyright 2003, Oliver Tappe, zooey@hirschkaefer.de. All rights reserved.
** Distributed under the terms of the OpenBeOS License.
*/


#include <Application.h>
#include <Directory.h>
#include <File.h>
#include <FindDirectory.h>
#include <Message.h>
#include <Path.h>
#include <Roster.h>

#include <DefaultCatalog.h>
#include <LocaleRoster.h>

/*
 *	This implements the default catalog-type for the opentracker locale kit.
 *  Alternatively, this could be used as a full add-on, but currently this
 *  is provided as part of liblocale.so.
 */
size_t hash<CatKey>::operator()(const CatKey &key) const 
{
	return key.fHashVal;
}

static const char kSeparator = '\01';

CatKey::CatKey(const char *str, const char *ctx, const char *cmt)
{
	uint32 strLen = str ? strlen(str) : 0;
	uint32 ctxLen = ctx ? strlen(ctx) : 0;
	uint32 cmtLen = cmt ? strlen(cmt) : 0;
	int32 keyLen = strLen + ctxLen + cmtLen + 2;
	char *keyBuf = fKey.LockBuffer(keyLen);
	if (!keyBuf)
		return;
	if (strLen) {
		memcpy(keyBuf, str, strLen);
		keyBuf += strLen;
	}
	*keyBuf++ = kSeparator;
	if (ctxLen) {
		memcpy(keyBuf, ctx, ctxLen);
		keyBuf += ctxLen;
	}
	*keyBuf++ = kSeparator;
	if (cmtLen) {
		memcpy(keyBuf, cmt, cmtLen);
		keyBuf += cmtLen;
	}
	fKey.UnlockBuffer(keyLen);
}

DefaultCatalog::DefaultCatalog(const char *signature, const char *language)
	:
	BCatalogAddOn(signature, language)
{
	static const char *catFolder = "catalogs";
	static const char *catExtension = ".catalog";
	app_info appInfo;
	be_app->GetAppInfo(&appInfo); 
	node_ref nref;
	nref.device = appInfo.ref.device;
	nref.node = appInfo.ref.directory;
	BDirectory appDir(&nref);
	BString catalogName(catFolder);
	catalogName << "/" << fLanguageName << "/" << fSignature << catExtension;
	BPath catalogPath(&appDir, catalogName.String());
	status_t status = LoadFromDisk( catalogPath.Path());

	if (status != B_OK) {
		BPath commonEtcPath;
		find_directory(B_COMMON_ETC_DIRECTORY, &commonEtcPath);
		if (commonEtcPath.InitCheck() == B_OK) {
			catalogName = BString(commonEtcPath.Path()) 
							<< "/locale/" << catFolder 
							<< "/" << fLanguageName 
							<< "/" << fSignature << catExtension;
			status = LoadFromDisk(catalogName.String());
		}
	}

	if (status != B_OK) {
		BPath systemEtcPath;
		find_directory(B_BEOS_ETC_DIRECTORY, &systemEtcPath);
		if (systemEtcPath.InitCheck() == B_OK) {
			catalogName = BString(systemEtcPath.Path()) 
							<< "/locale/" << catFolder 
							<< "/" << fLanguageName
							<< "/" << fSignature << catExtension;
			status = LoadFromDisk(catalogName.String());
		}
	}

	fInitCheck = status;
}

DefaultCatalog::DefaultCatalog(const char *signature, const char *language,
	const char *path, bool create)
	:
	BCatalogAddOn(signature, language),
	fPath(path)
{
	fInitCheck = LoadFromDisk(path);
	if (fInitCheck != B_OK && create && path)
		// catalog not found, but we shall create it, so we say we're ok:
		fInitCheck = B_OK;
}

DefaultCatalog::~DefaultCatalog()
{
}

status_t
DefaultCatalog::LoadFromDisk(const char *path)
{
	if (!path)
		return B_BAD_VALUE;

	BFile catalogFile;
	status_t res = catalogFile.SetTo(path, B_READ_ONLY);
	if (res != B_OK)
		return res;

	fPath = path;
	BMessage catalogMsg;
	res = catalogMsg.Unflatten(&catalogFile);
	if (res != B_OK)
		return res;

	// ToDo: create hash-map from catalogMsg
	fCatMap.clear();
	
}

status_t
DefaultCatalog::WriteToDisk(const char *path) const
{
	status_t res = B_OK;
	BMessage archive;
	CatMap::const_iterator iter;
	for (iter = fCatMap.begin(); res==B_OK && iter!=fCatMap.end(); ++iter) {
		res = archive.AddString( "c:key", iter->first.fKey.String())
			|| archive.AddInt32( "c:hash", iter->first.fHashVal)
			|| archive.AddString( "c:tstr", iter->second.String());
	}

	if (res == B_OK) {
		if (path)
			fPath = path;
		BFile catalogFile;
		res = catalogFile.SetTo(fPath.String(), 
			B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	}
	return res;
}

const char *
DefaultCatalog::GetString(const char *string, const char *context, 
	const char *comment)
{
	return "default-string-by-string";
}

const char *
DefaultCatalog::GetString(uint32 id)
{
	return "default-string-by-id";
}

status_t
DefaultCatalog::SetString(const char *string, const char *translated, 
	const char *context, const char *comment)
{
	return B_OK;
}

status_t
DefaultCatalog::SetString(uint32 id, const char *translated)
{
	return B_OK;
}



BCatalogAddOn *
DefaultCatalog::Instantiate(const char *signature, const char *language)
{
	DefaultCatalog *catalog = new DefaultCatalog(signature, language);
	if (catalog && catalog->InitCheck() != B_OK) {
		delete catalog;
		return NULL;
	}
	return catalog;
}

const uint8 DefaultCatalog::gDefaultCatalogAddOnPriority = 1;
	// give highest priority to our embedded catalog-add-on
