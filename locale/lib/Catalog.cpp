/* 
** Copyright 2003, Axel Dörfler, axeld@pinc-software.de. All rights reserved.
** Distributed under the terms of the OpenBeOS License.
*/


#include <Catalog.h>

#include <stdio.h>	// <- for debugging only
#include <ctype.h>


BCatalog::BCatalog(BLocale *locale, const char *signature)
{
	// ToDo: the collator construction will have to change...
	//		build catalog add-on chain here!

	fCatalog = new BCatalogAddOn(locale, signature, NULL);
}


BCatalog::~BCatalog()
{
	delete fCatalog;
}


const char *
BCatalog::GetString(const char *string)
{
	return fCatalog->GetString(string);
}


const char *
BCatalog::GetString(uint32 id)
{
	return fCatalog->GetString(id);
}


status_t 
BCatalog::GetData(const char *name, BMessage &msg)
{
	return fCatalog->GetData(name, msg);
}


status_t 
BCatalog::GetData(uint32 id, BMessage &msg)
{
	return fCatalog->GetData(id, msg);
}


//	#pragma mark -


BCatalogAddOn::BCatalogAddOn(const BLocale *locale, const char *signature, const BCatalogAddOn *next)
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
BCatalogAddOn::SetString(const char *string, const char *translated)
{
	return EOPNOTSUPP;
}


status_t 
BCatalogAddOn::SetString(int32 id, const char *translated)
{
	return EOPNOTSUPP;
}


status_t 
BCatalogAddOn::SetData(const char *name, BMessage &msg)
{
	return EOPNOTSUPP;
}


status_t 
BCatalogAddOn::SetData(uint32 id, BMessage &msg)
{
	return EOPNOTSUPP;
}

