/* 
** Copyright 2003, Oliver Tappe, zooey@hirschkaefer.de. All rights reserved.
** Distributed under the terms of the OpenBeOS License.
*/


#include <memory>

#include <Application.h>
#include <Directory.h>
#include <File.h>
#include <FindDirectory.h>
#include <DataIO.h>
#include <Message.h>
#include <Path.h>
#include <Roster.h>

#include <DefaultCatalog.h>
#include <LocaleRoster.h>

#include <stdio.h>

/*
 *	This implements the default catalog-type for the opentracker locale kit.
 *  Alternatively, this could be used as a full add-on, but currently this
 *  is provided as part of liblocale.so.
 */

static const char *kCatFolder = "catalogs";
static const char *kCatExtension = ".catalog";

static const char *kCatMimeType = "x-vnd.Be.locale-catalog.default";
static const char *kCatLangAttr = "BEOS:LOCALE_LANGUAGE";
static const char *kCatSigAttr = "BEOS:LOCALE_SIGNATURE";

static int16 kCatArchiveVersion = 1;
	// version of the archive structure, bump this if you change the structure!

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
	fHashVal = __stl_hash_string(fKey.String());
}


CatKey::CatKey(uint32 id)
	:
	fHashVal(id)
{
}


CatKey::CatKey()
	:
	fHashVal(0)
{
}


bool 
CatKey::operator== (const CatKey& right) const
{
	return fHashVal == right.fHashVal
		&& fKey == right.fKey;
}


DefaultCatalog::DefaultCatalog(const char *signature, const char *language)
	:
	BCatalogAddOn(signature, language)
{
	app_info appInfo;
	be_app->GetAppInfo(&appInfo); 
	node_ref nref;
	nref.device = appInfo.ref.device;
	nref.node = appInfo.ref.directory;
	BDirectory appDir(&nref);
	BString catalogName(kCatFolder);
	catalogName << "/" << fLanguageName << "/" << fSignature << kCatExtension;
	BPath catalogPath(&appDir, catalogName.String());
	status_t status = ReadFromDisk( catalogPath.Path());

	if (status != B_OK) {
		BPath commonEtcPath;
		find_directory(B_COMMON_ETC_DIRECTORY, &commonEtcPath);
		if (commonEtcPath.InitCheck() == B_OK) {
			catalogName = BString(commonEtcPath.Path()) 
							<< "/locale/" << kCatFolder 
							<< "/" << fLanguageName 
							<< "/" << fSignature << kCatExtension;
			status = ReadFromDisk(catalogName.String());
		}
	}

	if (status != B_OK) {
		BPath systemEtcPath;
		find_directory(B_BEOS_ETC_DIRECTORY, &systemEtcPath);
		if (systemEtcPath.InitCheck() == B_OK) {
			catalogName = BString(systemEtcPath.Path()) 
							<< "/locale/" << kCatFolder 
							<< "/" << fLanguageName
							<< "/" << fSignature << kCatExtension;
			status = ReadFromDisk(catalogName.String());
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
	fInitCheck = ReadFromDisk(path);
	if (fInitCheck != B_OK && create && path)
		// catalog not found, but we shall create it, so we say we're ok:
		fInitCheck = B_OK;
}


DefaultCatalog::~DefaultCatalog()
{
}


status_t
DefaultCatalog::ReadFromDisk(const char *path)
{
	if (!path)
		return B_BAD_VALUE;

	BFile catalogFile;
	status_t res = catalogFile.SetTo(path, B_READ_ONLY);
	if (res != B_OK)
		return res;

	fPath = path;

	off_t sz = 0;
	res = catalogFile.GetSize( &sz);
	if (res != B_OK)
		return res;

	auto_ptr<char> buf(new char [sz]);
	res = catalogFile.Read( buf.get(), sz);
	if (res < B_OK )
		return res;
		
	BMemoryIO memIO( buf.get(), sz);

	// create hash-map from mem-IO:
	BMessage archiveMsg;
	res = archiveMsg.Unflatten(&memIO);
	if (res != B_OK)
		return res;

	fCatMap.clear();
	int32 count = archiveMsg.FindInt32("c:sz");
	int16 version = archiveMsg.FindInt16("c:ver");
	if (count > 0) {
		CatKey key;
		const char *keyStr;
		const char *translated;
		fCatMap.resize(count);
		for (int i=0; res==B_OK && i<count; ++i) {
			res = archiveMsg.Unflatten(&memIO);
			if (res == B_OK) {
				res = archiveMsg.FindString("c:key", &keyStr)
					|| archiveMsg.FindInt32("c:hash", (int32*)&key.fHashVal)
					|| archiveMsg.FindString("c:tstr", &translated);
			}
			if (res == B_OK) {
				key.fKey = keyStr;
				fCatMap.insert(make_pair(key, translated));
			}
		}
	}
	return res;
}


status_t
DefaultCatalog::WriteToDisk(const char *path) const
{
	status_t res = B_OK;
	BMessage archive;

	BFile catalogFile;
	if (res == B_OK) {
		if (path)
			fPath = path;
		res = catalogFile.SetTo(fPath.String(),
			B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	}

	int32 count = fCatMap.size();
	res = archive.AddInt32("c:sz", count)
		|| archive.AddInt16("c:ver", kCatArchiveVersion);
	if (res == B_OK)
		res = archive.Flatten(&catalogFile);
	CatMap::const_iterator iter;

	BMallocIO mallocIO;
	mallocIO.SetBlockSize( count*20);
		// set a largish block-size in order to avoid reallocs

	for (iter = fCatMap.begin(); res==B_OK && iter!=fCatMap.end(); ++iter) {
		archive.MakeEmpty();
		res = archive.AddString("c:key", iter->first.fKey.String())
			|| archive.AddInt32("c:hash", iter->first.fHashVal)
			|| archive.AddString("c:tstr", iter->second.String());
		if (res == B_OK)
			res = archive.Flatten(&mallocIO);
	}
	catalogFile.Write( mallocIO.Buffer(), mallocIO.BufferLength());

	return res;
}


void
DefaultCatalog::MakeEmpty()
{
	fCatMap.clear();
}


void
DefaultCatalog::Resize(int32 size)
{
	fCatMap.resize(size);
}


int32
DefaultCatalog::CountItems() const
{
	return fCatMap.size();
}


const char *
DefaultCatalog::GetString(const char *string, const char *context, 
	const char *comment)
{
	CatKey key(string, context, comment);
	CatMap::const_iterator iter = fCatMap.find(key);
	if (iter != fCatMap.end())
		return iter->second.String();
	else
		return string;
}


const char *
DefaultCatalog::GetString(uint32 id)
{
	CatMap::const_iterator iter = fCatMap.find(id);
	if (iter != fCatMap.end())
		return iter->second.String();
	else
		return NULL;
}


status_t
DefaultCatalog::SetString(const char *string, const char *translated, 
	const char *context, const char *comment)
{
	CatKey key(string, context, comment);
	fCatMap[key] = translated;
		// overwrite existing element
	return B_OK;
}


status_t
DefaultCatalog::SetString(uint32 id, const char *translated)
{
	fCatMap[id] = translated;
		// overwrite existing element
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
