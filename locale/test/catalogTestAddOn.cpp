/* 
** Copyright 2003, Oliver Tappe, zooey@hirschkaefer.de. All rights reserved.
** Distributed under the terms of the OpenBeOS License.
*/

#include <stdio.h>

#include <CatalogInAddOn.h>
#include <DefaultCatalog.h>

class CatalogTestAddOn {
	public:
		void Run();
		void Check();
};


#define catSig "add-ons/catalogTest/catalogTestAddOn"
#define catName catSig".catalog"


void 
CatalogTestAddOn::Run() {
	printf("addon...");
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
	res = catalog->SetString("string", "Schnur_A", typeid(*this).name());
	assert(res == B_OK);
	res = catalog->SetString(hashVal, "Schnur_id_A");
		// add a second entry for the same hash-value, but with different translation
	assert(res == B_OK);
	res = catalog->SetString("string", "String_A", "programming");
	assert(res == B_OK);
	res = catalog->SetString("string", "Textpuffer_A", "programming", "Deutsches Fachbuch");
	assert(res == B_OK);
	res = catalog->SetString("string", "Leine_A", typeid(*this).name(), "Deutsches Fachbuch");
	assert(res == B_OK);
	res = catalog->WriteToDisk("./locale/catalogs/"catSig"/german.catalog");
	assert(res == B_OK);
	// check if we are getting back the correct strings:
	s = catalog->GetString("string", typeid(*this).name());
	assert(s == "Schnur_A");
	s = catalog->GetString(hashVal);
	assert(s == "Schnur_id_A");
	s = catalog->GetString("string", "programming");
	assert(s == "String_A");
	s = catalog->GetString("string", "programming", "Deutsches Fachbuch");
	assert(s == "Textpuffer_A");
	s = catalog->GetString("string", typeid(*this).name(), "Deutsches Fachbuch");
	assert(s == "Leine_A");

	printf("ok.\n");
	Check();
}


void 
CatalogTestAddOn::Check() {
	status_t res;
	printf("addon-check...");
	BString s("string");
	s << "\x01" << typeid(*this).name() << "\x01";
	size_t hashVal = __stl_hash_string(s.String());
	// ok, we now try to re-load the catalog that has just been written:
	//
	// actually, the following code can be seen as an example of what an 
	// add_on needs in order to translate strings:
	BCatalog cat;
	int32 fingerprint = 0;
		// this should be replaced by the real fingerprint of the catalog that
		// is to be loaded
	res = get_add_on_catalog(&cat, catSig, fingerprint);
	assert(res == B_OK);
	s = TR("string");
	assert(s == "Schnur_A");
	s = TR_ID(hashVal);
	assert(s == "Schnur_id_A");
	s = TR_ALL("string", "programming", "");
	assert(s == "String_A");
	s = TR_ALL("string", "programming", "Deutsches Fachbuch");
	assert(s == "Textpuffer_A");
	s = TR_CMT("string", "Deutsches Fachbuch");
	assert(s == "Leine_A");
	printf("ok.\n");
}


extern "C" void run_test_add_on()
{
	CatalogTestAddOn catTest;
	catTest.Run();
}
