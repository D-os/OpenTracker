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
	BCatalog catalog("Test");
	if (catalog.InitCheck() == B_OK) {
		printf("translating %s yields %s\n", "test", catalog.GetString( "test"));
	}
	delete testApp;
}

