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

/**	This implements the default catalog-type for the opentracker locale kit
 *    Alternatively, this could be used as a full add-on, but currently this
 *    is provided as part of liblocale.so.
 */

DefaultCatalog::DefaultCatalog(const char *signature, const char *language,
	BCatalogAddOnInfo* info)
	:	
	BCatalogAddOn(signature, language, info)
{
	app_info appInfo;
	be_app->GetAppInfo(&appInfo); 
	node_ref nref;
	nref.device = appInfo.ref.device;
	nref.node = appInfo.ref.directory;
	BDirectory appDir(&nref);
	BString catalogName("catalogs/");
	catalogName << signature << "/" << language << ".catalog";
	BPath catalogPath(&appDir, catalogName.String());
	BFile catalogFile;
	catalogFile.SetTo(catalogPath.Path(), B_READ_ONLY);
	if (catalogFile.InitCheck() != B_OK) {
		BPath commonEtcPath;
		find_directory(B_COMMON_ETC_DIRECTORY, &commonEtcPath);
		if (commonEtcPath.InitCheck() == B_OK) {
			catalogName = BString(commonEtcPath.Path()) 
								<< "/locale/catalogs/" 
								<< signature << "/" << language << ".catalog";
			catalogFile.SetTo(catalogName.String(), B_READ_ONLY);
		}
	}
	if (catalogFile.InitCheck() != B_OK) {
		BPath systemEtcPath;
		find_directory(B_BEOS_ETC_DIRECTORY, &systemEtcPath);
		if (systemEtcPath.InitCheck() == B_OK) {
			catalogName = BString(systemEtcPath.Path()) << "/locale/catalogs/" 
				<< signature << "/" << language;
			catalogFile.SetTo(catalogName.String(), B_READ_ONLY);
		}
	}
	if (catalogFile.InitCheck() != B_OK) {
		fInitCheck = B_NAME_NOT_FOUND;
		return;
	}

	BMessage catalogMsg;
	fInitCheck = catalogMsg.Unflatten(&catalogFile);
	if (fInitCheck == B_OK) {
		// ToDo: create hash-map from catalogMsg
	}
}

DefaultCatalog::~DefaultCatalog()
{
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



BCatalogAddOn *
DefaultCatalog::Instantiate(const char *signature, const char *language,
	BCatalogAddOnInfo *info)
{
	DefaultCatalog *catalog = new DefaultCatalog(signature, language, info);
	if (catalog && catalog->InitCheck() != B_OK) {
		delete catalog;
		return NULL;
	}
	return catalog;
}

const uint8 DefaultCatalog::gDefaultCatalogAddOnPriority = 1;
	// give highest priority to our embedded catalog-add-on
