/* 
** Copyright 2003, Oliver Tappe, zooey@hirschkaefer.de. All rights reserved.
** Distributed under the terms of the OpenBeOS License.
*/

#include <stdio.h>
#include <unistd.h>

#include <Application.h>
#include <StopWatch.h>

#include <Catalog.h>
#include <DefaultCatalog.h>
#include <Entry.h>
#include <Locale.h>
#include <Path.h>
#include <Roster.h>

const uint32 kNumStrings = 10000;

BString strs[kNumStrings];
BString ctxs[kNumStrings];

BString trls[kNumStrings];

const char *translated;

class CatalogSpeed {
	public:
		void TestCreation();
		void TestLookup();
		void TestIdCreation();
		void TestIdLookup();
};

#define catSig "x-vnd.Be.locale.catalogSpeed"
#define catName catSig".catalog"


void
CatalogSpeed::TestCreation()
{
	for (int i = 0; i < kNumStrings; i++) {
		strs[i] << "native-string#" << 1000000+i;
		ctxs[i] << typeid(*this).name();
		trls[i] << "translation#" << 4000000+i;
	}

	BStopWatch watch("catalogSpeed", true);

	status_t res;
	BString s("string");
	s << "\x01" << typeid(*this).name() << "\x01";
	size_t hashVal = __stl_hash_string(s.String());
	assert(be_locale != NULL);
	system("mkdir -p ./locale/catalogs/"catSig);

	// create an empty catalog of default type...
	BCatalog cat1("Default", catSig, "klingon");
	assert(cat1.InitCheck() == B_OK);
	// ...and populate the catalog with some data:
	DefaultCatalog *catalog = dynamic_cast<DefaultCatalog*>(cat1.fCatalog);
	assert(catalog != NULL);

	for (int i = 0; i < kNumStrings; i++) {
		catalog->SetString(strs[i].String(), trls[i].String(), ctxs[i].String());
	}
	watch.Suspend();
	printf("\tadded %d strings in           %9Ld usecs\n", 
		catalog->CountItems(), watch.ElapsedTime());

	watch.Reset();
	watch.Resume();
	res = catalog->WriteToDisk("./locale/catalogs/"catSig"/klingon.catalog");
	assert( res == B_OK);
	watch.Suspend();
	printf("\t%d strings written to disk in %9Ld usecs\n", 
		catalog->CountItems(), watch.ElapsedTime());
}


void
CatalogSpeed::TestLookup()
{
	BStopWatch watch("catalogSpeed", true);

	BCatalog *cat = be_catalog = new BCatalog(catSig, "klingon");
	
	assert(cat != NULL);
	assert(cat->InitCheck() == B_OK);
	DefaultCatalog *catalog = dynamic_cast<DefaultCatalog*>(cat->fCatalog);
	assert(catalog != NULL);
	watch.Suspend();
	printf("\t%d strings read from disk in  %9Ld usecs\n", 
		catalog->CountItems(), watch.ElapsedTime());

	watch.Reset();
	watch.Resume();
	for (int i = 0; i < kNumStrings; i++) {
		translated = TR(strs[i].String());
	}
	watch.Suspend();
	printf("\tlooked up %d strings in       %9Ld usecs\n", 
		kNumStrings, watch.ElapsedTime());

	delete cat;
}


void
CatalogSpeed::TestIdCreation()
{
	BStopWatch watch("catalogSpeed", true);
	watch.Suspend();

	status_t res;
	BString s("string");
	s << "\x01" << typeid(*this).name() << "\x01";
	size_t hashVal = __stl_hash_string(s.String());
	assert(be_locale != NULL);
	system("mkdir -p ./locale/catalogs/"catSig);

	// create an empty catalog of default type...
	BCatalog cat1("Default", catSig, "klingon");
	assert(cat1.InitCheck() == B_OK);
	// ...and populate the catalog with some data:
	DefaultCatalog *catalog = dynamic_cast<DefaultCatalog*>(cat1.fCatalog);
	assert(catalog != NULL);

	for (int i = 0; i < kNumStrings; i++) {
		trls[i] = BString("id_translation#") << 6000000+i;
	}
	watch.Reset();
	watch.Resume();
	for (int i = 0; i < kNumStrings; i++) {
		catalog->SetString(i, trls[i].String());
	}
	watch.Suspend();
	printf("\tadded %d strings by id in     %9Ld usecs\n", 
		catalog->CountItems(), watch.ElapsedTime());

	watch.Reset();
	watch.Resume();
	res = catalog->WriteToDisk("./locale/catalogs/"catSig"/klingon.catalog");
	assert( res == B_OK);
	watch.Suspend();
	printf("\t%d strings written to disk in %9Ld usecs\n", 
		catalog->CountItems(), watch.ElapsedTime());
}


void
CatalogSpeed::TestIdLookup()
{
	BStopWatch watch("catalogSpeed", true);

	BCatalog *cat = be_catalog = new BCatalog(catSig, "klingon");

	assert(cat != NULL);
	assert(cat->InitCheck() == B_OK);
	DefaultCatalog *catalog = dynamic_cast<DefaultCatalog*>(cat->fCatalog);
	assert(catalog != NULL);
	watch.Suspend();
	printf("\t%d strings read from disk in  %9Ld usecs\n", 
		catalog->CountItems(), watch.ElapsedTime());

	watch.Reset();
	watch.Resume();
	for (int i = 0; i < kNumStrings; i++) {
		translated = TR_ID(i);
	}
	watch.Suspend();
	printf("\tlooked up %d strings in       %9Ld usecs\n", 
		kNumStrings, watch.ElapsedTime());

	delete cat;
}


int
main(int argc, char **argv)
{
	status_t res;

	BApplication* testApp 
		= new BApplication("application/"catSig);

	// change to app-folder:
	app_info appInfo;
	be_app->GetAppInfo(&appInfo);
	BEntry appEntry(&appInfo.ref);
	BEntry appFolder;
	appEntry.GetParent( &appFolder);
	BPath appPath;
	appFolder.GetPath( &appPath);
	chdir( appPath.Path());

	CatalogSpeed catSpeed;
	printf("\t------------------------------------------------\n");
	printf("\tstring-based catalog usage:\n");
	printf("\t------------------------------------------------\n");
	catSpeed.TestCreation();
	catSpeed.TestLookup();
	printf("\t------------------------------------------------\n");
	printf("\tid-based catalog usage:\n");
	printf("\t------------------------------------------------\n");
	catSpeed.TestIdCreation();
	catSpeed.TestIdLookup();

	delete testApp;
	
	return 0;
}