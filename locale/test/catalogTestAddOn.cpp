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

#define TR_CONTEXT "CatalogTestAddOn"

#define catSig "add-ons/catalogTest/catalogTestAddOn"
#define catName catSig".catalog"


void 
CatalogTestAddOn::Run() {
	printf("addon...");
	status_t res;
	BString s;
	s << "string" << "\x01" << TR_CONTEXT << "\x01";
	size_t hashVal = __stl_hash_string(s.String());
	assert(be_locale != NULL);
	system("mkdir -p ./locale/catalogs/"catSig);

	// create an empty catalog of default type...
	BPrivate::EditableCatalog cat1("Default", catSig, "German");
	assert(cat1.InitCheck() == B_OK);

	// ...and populate the catalog with some data:
	res = cat1.SetString("string", "Schnur_A", TR_CONTEXT);
	assert(res == B_OK);
	res = cat1.SetString(hashVal, "Schnur_id_A");
		// add a second entry for the same hash-value, but with different translation
	assert(res == B_OK);
	res = cat1.SetString("string", "String_A", "programming");
	assert(res == B_OK);
	res = cat1.SetString("string", "Textpuffer_A", "programming", "Deutsches Fachbuch");
	assert(res == B_OK);
	res = cat1.SetString("string", "Leine_A", TR_CONTEXT, "Deutsches Fachbuch");
	assert(res == B_OK);
	res = cat1.WriteToFile("./locale/catalogs/"catSig"/german.catalog");
	assert(res == B_OK);

	// check if we are getting back the correct strings:
	s = cat1.GetString("string", TR_CONTEXT);
	assert(s == "Schnur_A");
	s = cat1.GetString(hashVal);
	assert(s == "Schnur_id_A");
	s = cat1.GetString("string", "programming");
	assert(s == "String_A");
	s = cat1.GetString("string", "programming", "Deutsches Fachbuch");
	assert(s == "Textpuffer_A");
	s = cat1.GetString("string", TR_CONTEXT, "Deutsches Fachbuch");
	assert(s == "Leine_A");

	// now we create a new (base) catalog and embed this one into the app-file:
	BPrivate::EditableCatalog cat2("Default", catSig, "English");
	assert(cat2.InitCheck() == B_OK);
	// the following string is unique to the embedded catalog:
	res = cat2.SetString("string", "string_A", "base");
	assert(res == B_OK);
	// the following id is unique to the embedded catalog:
	res = cat2.SetString(32, "hashed string_A");
	assert(res == B_OK);
	// the following string will be hidden by the definition inside the german catalog:
	res = cat2.SetString("string", "hidden_A", TR_CONTEXT);
	assert(res == B_OK);
	entry_ref addOnRef;
	res = get_add_on_ref(&addOnRef);
	assert(res == B_OK);
	res = cat2.WriteToResource(&addOnRef);
	assert(res == B_OK);

	printf("ok.\n");
	Check();
}


void 
CatalogTestAddOn::Check() {
	status_t res;
	printf("addon-check...");
	BString s;
	s << "string" << "\x01" << TR_CONTEXT << "\x01";
	size_t hashVal = __stl_hash_string(s.String());
	// ok, we now try to re-load the catalog that has just been written:
	//
	// actually, the following code can be seen as an example of what an 
	// add_on needs in order to translate strings:
	BCatalog cat;
	res = get_add_on_catalog(&cat, catSig);
	assert(res == B_OK);
	// fetch basic data:
	int32 fingerprint;
	res = cat.GetFingerprint(&fingerprint);
	assert(res == B_OK);
	BString lang;
	res = cat.GetLanguage(&lang);
	assert(res == B_OK);
	assert(lang == "german");
	BString sig;
	res = cat.GetSignature(&sig);
	assert(res == B_OK);
	assert(sig == catSig);

	// now check strings:
	s = TR_ID(hashVal);
	assert(s == "Schnur_id_A");
	s = TR_ALL("string", "programming", "");
	assert(s == "String_A");
	s = TR_ALL("string", "programming", "Deutsches Fachbuch");
	assert(s == "Textpuffer_A");
	s = TR_CMT("string", "Deutsches Fachbuch");
	assert(s == "Leine_A");
	// the following string should be found in the embedded catalog only:
	s = TR_ALL("string", "base", "");
	assert(s == "string_A");
	// the following id should be found in the embedded catalog only:
	s = TR_ID(32);
	assert(s == "hashed string_A");
	// the following id doesn't exist anywhere (hopefully):
	s = TR_ID(-1);
	assert(s == "");
	// the following string exists twice, in the embedded as well as in the 
	// external catalog. So we should get the external translation (as it should
	// override the embedded one):
	s = TR("string");
	assert(s == "Schnur_A");

	// check access to app-catalog from insided add-on:
//	s = be_app_catalog->GetString("string", TR_CONTEXT);
//	assert(s == "Schnur");

	printf("ok.\n");
}


extern "C" void run_test_add_on()
{
	CatalogTestAddOn catTest;
	catTest.Run();
}
