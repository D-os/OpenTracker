/* 
** Copyright 2003, Oliver Tappe, zooey@hirschkaefer.de. All rights reserved.
** Distributed under the terms of the OpenBeOS License.
*/

#include <stdio.h>
#include <unistd.h>

#include <Application.h>
#include <Catalog.h>
#include <DefaultCatalog.h>
#include <Entry.h>
#include <Locale.h>
#include <Path.h>
#include <Roster.h>

class CatalogTest {
	public:
		void Run();
		void Check();
};


#define catSig "x-vnd.Be.locale.catalogTest"
#define catName catSig".catalog"

void 
CatalogTest::Run() {
	printf("app...", be_locale);
	status_t res;
	BString s("string");
	s << "\x01" << typeid(*this).name() << "\x01";
	size_t hashVal = __stl_hash_string(s.String());
	assert(be_locale != NULL);
	system("mkdir -p ./locale/catalogs/"catSig);

	// create an empty catalog of default type...
	BCatalog cat1("Default", catSig, "German");
	assert(cat1.InitCheck() == B_OK);
	// ...and populate the catalog with some data:
	DefaultCatalog *catalog = dynamic_cast<DefaultCatalog*>(cat1.fCatalog);
	assert(catalog != NULL);
	res = catalog->SetString("string", "Schnur", typeid(*this).name());
	assert(res == B_OK);
	res = catalog->SetString(hashVal, "Schnur_id");
		// add a second entry for the same hash-value, but with different translation
	assert(res == B_OK);
	res = catalog->SetString("string", "String", "programming");
	assert(res == B_OK);
	res = catalog->SetString("string", "Textpuffer", "programming", "Deutsches Fachbuch");
	assert(res == B_OK);
	res = catalog->SetString("string", "Leine", typeid(*this).name(), "Deutsches Fachbuch");
	assert(res == B_OK);
	res = catalog->WriteToDisk("./locale/catalogs/"catSig"/german.catalog");
	assert(res == B_OK);
	// check if we are getting back the correct strings:
	s = catalog->GetString("string", typeid(*this).name());
	assert(s == "Schnur");
	s = catalog->GetString(hashVal);
	assert(s == "Schnur_id");
	s = catalog->GetString("string", "programming");
	assert(s == "String");
	s = catalog->GetString("string", "programming", "Deutsches Fachbuch");
	assert(s == "Textpuffer");
	s = catalog->GetString("string", typeid(*this).name(), "Deutsches Fachbuch");
	assert(s == "Leine");

	printf("ok.\n");
	Check();
}


void 
CatalogTest::Check() {
	status_t res;
	printf("app-check...");
	BString s("string");
	s << "\x01" << typeid(*this).name() << "\x01";
	size_t hashVal = __stl_hash_string(s.String());
	// ok, we now try to re-load the catalog that has just been written:
	//
	// actually, the following code can be seen as an example of what an 
	// app needs in order to translate strings:
	BCatalog cat;
	res = be_locale->GetAppCatalog(&cat);
	assert(res == B_OK);
	// fetch basic data:
	int32 fingerprint;
	res = cat.GetFingerprint(&fingerprint);
	assert(res == B_OK);
	BString lang;
	res = cat.GetLanguage(&lang);
	assert(res == B_OK);
	BString sig;
	res = cat.GetSignature(&sig);
	assert(res == B_OK);
	// now check strings:
	s = TR("string");
	assert(s == "Schnur");
	s = TR_ID(hashVal);
	assert(s == "Schnur_id");
	s = TR_ALL("string", "programming", "");
	assert(s == "String");
	s = TR_ALL("string", "programming", "Deutsches Fachbuch");
	assert(s == "Textpuffer");
	s = TR_CMT("string", "Deutsches Fachbuch");
	assert(s == "Leine");

	// now check if trying to access same catalog by specifying its data works:
	BCatalog cat2(sig.String(), lang.String(), fingerprint);
	assert(cat2.InitCheck() == B_OK);
	// now check if trying to access same catalog with wrong fingerprint fails:
	BCatalog cat3(sig.String(), lang.String(), fingerprint*-1);
	assert(cat3.InitCheck() == B_NO_INIT);
	printf("ok.\n");
}


int
main(int argc, char **argv)
{
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

	CatalogTest catTest;
	catTest.Run();

	char cwd[B_FILE_NAME_LENGTH];
	getcwd(cwd, B_FILE_NAME_LENGTH);
	BString addonName(cwd);
	addonName << "/" "catalogTestAddOn";
	image_id image = load_add_on(addonName.String());
	assert(image >= B_OK);
	void (*runAddonFunc)() = 0;
	get_image_symbol(image, "run_test_add_on",
		B_SYMBOL_TYPE_TEXT, (void **)&runAddonFunc);
	assert(runAddonFunc);
	runAddonFunc();

	catTest.Check();

	delete testApp;
}
