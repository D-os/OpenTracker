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

#include <Catalog.h>
#include <LocaleRoster.h>

/**	This implements a compatibility catalog-type which uses the catalogs
 *    supplied by the Zeta Locale Kit.
 */

class ZetaCatalog : public BCatalogAddOn {

	public:
		ZetaCatalog(const char *signature, const char *language);
		~ZetaCatalog();

		const char *GetString(const char *string, const char *context=NULL,
						const char *comment=NULL);
		const char *GetString(uint32 id);

	private:

};

ZetaCatalog::ZetaCatalog(const char *signature, const char *language)
	:
	BCatalogAddOn(signature, language)
{
	app_info appInfo;
	be_app->GetAppInfo(&appInfo); 
	node_ref nref;
	nref.device = appInfo.ref.device;
	nref.node = appInfo.ref.directory;
	BDirectory appDir( &nref);

	// ToDo: implement loading of zeta-catalog 
	fInitCheck = B_OK;
}

ZetaCatalog::~ZetaCatalog()
{
}

const char *
ZetaCatalog::GetString(const char *string, const char *context=NULL,
	const char *comment=NULL)
{
	return "zeta-string-by-string";
}

const char *
ZetaCatalog::GetString(uint32 id)
{
	return "zeta-string-by-id";
}



extern "C" BCatalogAddOn *
instantiate_catalog(const char *signature, const char *language)
{
	ZetaCatalog *catalog = new ZetaCatalog(signature, language);
	if (catalog && catalog->InitCheck() != B_OK) {
		delete catalog;
		return NULL;
	}
	return catalog;
}

uint8 gCatalogAddOnPriority = 5;
	// priority for Zeta catalog-add-on
