/* 
** Distributed under the terms of the OpenBeOS License.
** Copyright 2003-2004. All rights reserved.
**
** Authors:	Axel Dörfler, axeld@pinc-software.de
**			Oliver Tappe, zooey@hirschkaefer.de
*/


#include <Application.h>
#include <Catalog.h>
#include <Locale.h>
#include <LocaleRoster.h>
#include <Roster.h>


BCatalog* be_catalog = NULL;

BCatalog::BCatalog()
	:
	fCatalog(NULL)
{
}


BCatalog::BCatalog(const char *signature, const char *language, 
	int32 fingerprint)
{
	fCatalog = be_locale_roster->LoadCatalog(signature, language, fingerprint);
}


BCatalog::BCatalog(const char *type, const char *signature, 
	const char *language)
{
	fCatalog = be_locale_roster->CreateCatalog(type, signature, language);
}


BCatalog::~BCatalog()
{
	if (be_catalog == this)
		be_catalog = NULL;
	be_locale_roster->UnloadCatalog(fCatalog);
}


status_t 
BCatalog::GetAppCatalog(BCatalog* catalog) {
	app_info appInfo;
	if (!be_app || be_app->GetAppInfo(&appInfo) != B_OK)
		return B_ENTRY_NOT_FOUND;
	BString sig(appInfo.signature);

	// drop supertype from mimetype (should be "application/"):
	int32 pos = sig.FindFirst('/');
	if (pos >= 0)
		sig.Remove(0, pos+1);

	int32 fingerprint = 0;
		// ToDo: try to fetch fingerprint from app-file (attribute)!
	catalog->fCatalog 
		= be_locale_roster->LoadCatalog(sig.String(), NULL,	fingerprint);

	be_catalog = catalog;

	return catalog->InitCheck();
}


//	#pragma mark -


BCatalogAddOn::BCatalogAddOn(const char *signature, const char *language,
	int32 fingerprint)
	:
	fSignature(signature),
	fLanguageName(language),
	fFingerprint(fingerprint),
	fNext(NULL),
	fInitCheck(B_NO_INIT)
{
	fLanguageName.ToLower();
		// canonicalize language-name to lowercase
}


BCatalogAddOn::~BCatalogAddOn()
{
}


bool 
BCatalogAddOn::CanHaveData() const
{
	return false;
}


bool 
BCatalogAddOn::CanWriteData() const
{
	return false;
}


status_t 
BCatalogAddOn::SetString(const char *string, const char *translated,
	const char *context, const char *comment)
{
	return EOPNOTSUPP;
}


status_t 
BCatalogAddOn::SetString(int32 id, const char *translated)
{
	return EOPNOTSUPP;
}


void
BCatalogAddOn::UpdateFingerprint()
{
	fFingerprint = 0;
		// base implementation always yields the same fingerprint,
		// which means that no version-mismatch detection is possible.
}


status_t 
BCatalogAddOn::InitCheck() const
{ 
	return fInitCheck;
}


status_t 
BCatalogAddOn::GetData(const char *name, BMessage *msg)
{
	return EOPNOTSUPP;
}


status_t 
BCatalogAddOn::GetData(uint32 id, BMessage *msg)
{
	return EOPNOTSUPP;
}

status_t 
BCatalogAddOn::SetData(const char *name, BMessage *msg)
{
	return EOPNOTSUPP;
}


status_t 
BCatalogAddOn::SetData(uint32 id, BMessage *msg)
{
	return EOPNOTSUPP;
}

