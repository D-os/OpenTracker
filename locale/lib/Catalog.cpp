/* 
** Distributed under the terms of the OpenBeOS License.
** Copyright 2003-2004. All rights reserved.
**
** Authors:	Axel Dörfler, axeld@pinc-software.de
**			Oliver Tappe, zooey@hirschkaefer.de
*/


#include <Catalog.h>
#include <Locale.h>
#include <LocaleRoster.h>

#include <stdio.h>	// <- for debugging only


BCatalog::BCatalog(const char *signature, const char *language)
{
	fCatalog = be_locale_roster->LoadCatalog(signature, language);
}


BCatalog::~BCatalog()
{
	be_locale_roster->UnloadCatalog(fCatalog);
}


const char *
BCatalog::GetString(const char *string, const char *context, const char *comment)
{
	return fCatalog->GetString(string, context, comment);
}


const char *
BCatalog::GetString(uint32 id)
{
	return fCatalog->GetString(id);
}


status_t 
BCatalog::GetData(const char *name, BMessage *msg)
{
	return fCatalog->GetData(name, msg);
}


status_t 
BCatalog::GetData(uint32 id, BMessage *msg)
{
	return fCatalog->GetData(id, msg);
}


status_t
BCatalog::InitCheck() const
{
	return fCatalog 
				? fCatalog->InitCheck() 
				: B_NO_INIT;
}

//	#pragma mark -


BCatalogAddOn::BCatalogAddOn(const char *signature, const char *language)
	:
	fSignature(signature),
	fLanguageName(language),
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

