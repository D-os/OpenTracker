/* 
** Copyright 2003, Oliver Tappe, zooey@hirschkaefer.de. All rights reserved.
** Distributed under the terms of the OpenBeOS License.
*/

#include <stdio.h>

#include <Application.h>

#include <Catalog.h>
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
	delete testApp;
}

