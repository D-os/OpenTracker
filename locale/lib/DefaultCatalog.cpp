/* 
** Copyright 2003, Oliver Tappe, zooey@hirschkaefer.de. All rights reserved.
** Distributed under the terms of the OpenBeOS License.
*/


#include <memory>

#include <Application.h>
#include <DataIO.h>
#include <Directory.h>
#include <File.h>
#include <FindDirectory.h>
#include <Message.h>
#include <Path.h>
#include <Roster.h>

#include <DefaultCatalog.h>
#include <LocaleRoster.h>

extern "C" uint32 adler32(uint32 adler, const uint8 *buf, uint32 len);
	// definition lives in adler32.c

#if B_BEOS_VERSION <= B_BEOS_VERSION_5
// B_BAD_DATA was introduced with DANO, so we define it for R5:
#	define B_BAD_DATA -2147483632L
#endif

/*
 *	This implements the default catalog-type for the opentracker locale kit.
 *  Alternatively, this could be used as a full add-on, but currently this
 *  is provided as part of liblocale.so.
 */

static const char *kCatFolder = "catalogs";
static const char *kCatExtension = ".catalog";

static const char *kCatMimeType = "x-vnd.Be.locale-catalog.default";

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
	*keyBuf = '\0';
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


DefaultCatalog::DefaultCatalog(const char *signature, const char *language,
	int32 fingerprint)
	:
	BCatalogAddOn(signature, language, fingerprint)
{
	app_info appInfo;
	be_app->GetAppInfo(&appInfo);
	node_ref nref;
	nref.device = appInfo.ref.device;
	nref.node = appInfo.ref.directory;
	BDirectory appDir(&nref);
	BString catalogName("locale/");
	catalogName << kCatFolder 
		<< "/" << fSignature 
		<< "/" << fLanguageName 
		<< kCatExtension;
	BPath catalogPath(&appDir, catalogName.String());
	status_t status = ReadFromDisk(catalogPath.Path());

	if (status != B_OK) {
		BPath commonEtcPath;
		find_directory(B_COMMON_ETC_DIRECTORY, &commonEtcPath);
		if (commonEtcPath.InitCheck() == B_OK) {
			catalogName = BString(commonEtcPath.Path()) 
							<< "/locale/" << kCatFolder 
							<< "/" << fSignature 
							<< "/" << fLanguageName 
							<< kCatExtension;
			status = ReadFromDisk(catalogName.String());
		}
	}

	if (status != B_OK) {
		BPath systemEtcPath;
		find_directory(B_BEOS_ETC_DIRECTORY, &systemEtcPath);
		if (systemEtcPath.InitCheck() == B_OK) {
			catalogName = BString(systemEtcPath.Path()) 
							<< "/locale/" << kCatFolder 
							<< "/" << fSignature 
							<< "/" << fLanguageName
							<< kCatExtension;
			status = ReadFromDisk(catalogName.String());
		}
	}

	// ToDo: send info about failures to syslog, as they can't be passed out
	//       properly (object is being destroyed upon error).
	fInitCheck = status;
}


DefaultCatalog::DefaultCatalog(const char *path, const char *signature, 
	const char *language)
	:
	BCatalogAddOn(signature, language, 0),
	fPath(path)
{
	fInitCheck = B_OK;
}


DefaultCatalog::~DefaultCatalog()
{
}


status_t
DefaultCatalog::ReadFromDisk(const char *path)
{
	if (!path)
		path = fPath.String();

	BFile catalogFile;
	status_t res = catalogFile.SetTo(path, B_READ_ONLY);
	if (res != B_OK)
		return res;

	fPath = path;

	off_t sz = 0;
	res = catalogFile.GetSize(&sz);
	if (res != B_OK)
		return res;

	auto_ptr<char> buf(new char [sz]);
	res = catalogFile.Read(buf.get(), sz);
	if (res < B_OK )
		return res;
		
	BMemoryIO memIO(buf.get(), sz);

	// create hash-map from mem-IO:
	BMessage archiveMsg;
	res = archiveMsg.Unflatten(&memIO);
	if (res != B_OK)
		return res;

	fCatMap.clear();
	int32 count = 0;
	int16 version;
	res = archiveMsg.FindInt16("c:ver", &version)
		|| archiveMsg.FindInt32("c:sz", &count);
	if (res == B_OK) {
		fLanguageName = archiveMsg.FindString("c:lang");
		fSignature = archiveMsg.FindString("c:sig");
		int32 foundFingerprint = archiveMsg.FindInt32("c:fpr");

		// if a specific fingerprint has been requested and the catalog does in fact
		// have a fingerprint, both are compared. If they mismatch, we do not accept
		// this catalog:
		if (foundFingerprint != 0 && fFingerprint != 0 
			&& foundFingerprint != fFingerprint)
			res = B_MISMATCHED_VALUES;
		fFingerprint = foundFingerprint;

		// some information needs to be copied to attributes. Although these
		// attributes should have been written when creating the catalog, 
		// we make sure that they are really there:
		UpdateAttributes(catalogFile);
	}

	if (res == B_OK && count > 0) {
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
		if (fFingerprint != ComputeFingerprint())
			return B_BAD_DATA;
	}
	return res;
}


status_t
DefaultCatalog::WriteToDisk(const char *path)
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

	UpdateFingerprint();
		// make sure we have the correct fingerprint before we write it

	int32 count = fCatMap.size();
	res = archive.AddInt32("c:sz", count)
		|| archive.AddInt16("c:ver", kCatArchiveVersion)
		|| archive.AddString("c:lang", fLanguageName.String())
		|| archive.AddString("c:sig", fSignature.String())
		|| archive.AddInt32("c:fpr", fFingerprint);
	if (res == B_OK)
		res = archive.Flatten(&catalogFile);

	BMallocIO mallocIO;
	mallocIO.SetBlockSize(count*20);
		// set a largish block-size in order to avoid reallocs

	CatMap::const_iterator iter;
	for (iter = fCatMap.begin(); res==B_OK && iter!=fCatMap.end(); ++iter) {
		archive.MakeEmpty();
		res = archive.AddString("c:key", iter->first.fKey.String())
			|| archive.AddInt32("c:hash", iter->first.fHashVal)
			|| archive.AddString("c:tstr", iter->second.String());
		if (res == B_OK)
			res = archive.Flatten(&mallocIO);
	}
	catalogFile.Write(mallocIO.Buffer(), mallocIO.BufferLength());
	// now set mimetype-, language- and signature-attributes:
	UpdateAttributes(catalogFile);

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


int32
DefaultCatalog::ComputeFingerprint() const
{
	uint32 adler = adler32(0, NULL, 0);

	int32 hash;
	CatMap::const_iterator iter;
	for (iter = fCatMap.begin(); iter!=fCatMap.end(); ++iter) {
		hash = B_HOST_TO_LENDIAN_INT32(iter->first.fHashVal);
		adler = adler32(adler, reinterpret_cast<uint8*>(&hash), sizeof(int32));
	}
	return adler;
}


void
DefaultCatalog::UpdateFingerprint()
{
	fFingerprint = ComputeFingerprint();
}


void
DefaultCatalog::UpdateAttributes(BFile& catalogFile)
{
	static const int bufSize = 256;
	char buf[bufSize];
	if (catalogFile.ReadAttr("BEOS:MIME", B_STRING_TYPE, 0, &buf, bufSize) <= 0
		|| strcmp(kCatMimeType, buf) != 0) {
		catalogFile.WriteAttr("BEOS:MIME", B_STRING_TYPE, 0, 
			kCatMimeType, strlen(kCatMimeType)+1);
	}
	if (catalogFile.ReadAttr(BLocaleRoster::kCatLangAttr, B_STRING_TYPE, 0, 
		&buf, bufSize) <= 0
		|| fLanguageName != buf) {
		catalogFile.WriteAttr(BLocaleRoster::kCatLangAttr, B_STRING_TYPE, 0, 
			fLanguageName.String(), fLanguageName.Length()+1);
	}
	if (catalogFile.ReadAttr(BLocaleRoster::kCatSigAttr, B_STRING_TYPE, 0, 
		&buf, bufSize) <= 0
		|| fSignature != buf) {
		catalogFile.WriteAttr(BLocaleRoster::kCatSigAttr, B_STRING_TYPE, 0, 
			fSignature.String(), fSignature.Length()+1);
	}
	int32 fingerprint;
	if (catalogFile.ReadAttr(BLocaleRoster::kCatFingerprintAttr, B_INT32_TYPE, 0, 
		&fingerprint, sizeof(int32)) <= 0
		|| fFingerprint != fingerprint) {
		catalogFile.WriteAttr(BLocaleRoster::kCatFingerprintAttr, B_INT32_TYPE, 0, 
			&fFingerprint, sizeof(int32));
	}
}


BCatalogAddOn *
DefaultCatalog::Instantiate(const char *signature, const char *language, 
	int32 fingerprint)
{
	DefaultCatalog *catalog = new DefaultCatalog(signature, language, fingerprint);
	if (catalog && catalog->InitCheck() != B_OK) {
		delete catalog;
		return NULL;
	}
	return catalog;
}


BCatalogAddOn *
DefaultCatalog::Create(const char *signature, const char *language)
{
	DefaultCatalog *catalog = new DefaultCatalog("", signature, language);
	if (catalog && catalog->InitCheck() != B_OK) {
		delete catalog;
		return NULL;
	}
	return catalog;
}


const uint8 DefaultCatalog::gDefaultCatalogAddOnPriority = 1;
	// give highest priority to our embedded catalog-add-on
