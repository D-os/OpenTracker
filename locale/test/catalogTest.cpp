/* 
** Copyright 2003, Oliver Tappe, zooey@hirschkaefer.de. All rights reserved.
** Distributed under the terms of the OpenBeOS License.
*/

#include <stdio.h>

#include <Application.h>

#include <Catalog.h>
#include <DefaultCatalog.h>
#include <Locale.h>

int
main(int argc, char **argv)
{
	BApplication* testApp 
		= new BApplication("application/x-vnd.otlocale.test-catalog");
	BCatalog cat1("Test");
	if (cat1.InitCheck() == B_OK) {
		printf("translating %s in cat1 yields %s\n", "test", cat1.GetString( "test"));
	}
	BCatalog cat2("Test2");
	if (cat2.InitCheck() == B_OK) {
		printf("translating %s in cat2 yields %s\n", "test", cat2.GetString( "test"));
	}
	status_t res;
	BString s;
	size_t hashVal = __stl_hash_string("string\x01\x01");
	DefaultCatalog catalog("TestCat", "german", "./TestCat.catalog", true);
	if (catalog.InitCheck() == B_OK) {
		res = catalog.SetString("string", "Schnur");
		assert( res == B_OK);
		res = catalog.SetString(hashVal, "Schnur_id");
			// add a second entry for the same hash-value, but with different translation
		assert( res == B_OK);
		res = catalog.SetString("string", "String", "programming");
		assert( res == B_OK);
		res = catalog.SetString("string", "Textpuffer", "programming", "Deutsches Fachbuch");
		assert( res == B_OK);
		res = catalog.SetString("string", "Leine", "", "Deutsches Fachbuch");
		assert( res == B_OK);
		res = catalog.WriteToDisk();
		assert( res == B_OK);
		s = catalog.GetString("string");
		assert( s == "Schnur");
		s = catalog.GetString(hashVal);
		assert( s == "Schnur_id");
		s = catalog.GetString("string", "programming");
		assert( s == "String");
		s = catalog.GetString("string", "programming", "Deutsches Fachbuch");
		assert( s == "Textpuffer");
		s = catalog.GetString("string", "", "Deutsches Fachbuch");
		assert( s == "Leine");
	}
	DefaultCatalog catalog2("TestCat", "german", "./TestCat.catalog", false);
	if (catalog2.InitCheck() == B_OK) {
		s = catalog2.GetString("string");
		assert( s == "Schnur");
		s = catalog2.GetString(hashVal);
		assert( s == "Schnur_id");
		s = catalog2.GetString("string", "programming");
		assert( s == "String");
		s = catalog2.GetString("string", "programming", "Deutsches Fachbuch");
		assert( s == "Textpuffer");
		s = catalog2.GetString("string", "", "Deutsches Fachbuch");
		assert( s == "Leine");
	}
	delete testApp;
}
