/* 
** Copyright 2003, Oliver Tappe, zooey@hirschkaefer.de. All rights reserved.
** Distributed under the terms of the OpenBeOS License.
*/

#include <stdio.h>

#include <Application.h>
#include <StopWatch.h>

#include <Catalog.h>
#include <DefaultCatalog.h>
#include <Locale.h>

const uint32 kNumStrings = 10000;

BString strs[kNumStrings];
BString ctxs[kNumStrings];
BString cmts[kNumStrings];

BString trls[kNumStrings];

int
main(int argc, char **argv)
{
	status_t res;

	BApplication* testApp 
		= new BApplication("application/x-vnd.otlocale.test-catalog");

	DefaultCatalog catalog("TestCat", "german", "./TestCat.catalog", true);
	if (catalog.InitCheck() == B_OK) {

		for (int i = 0; i < kNumStrings; i++) {
			strs[i] << "native-string#" << 1000000+i;
			ctxs[i] << "context " << 2000000+i;
			cmts[i] << "comment_" << 3000000+i;
			trls[i] << "translation#" << 4000000+i;
		}

		catalog.MakeEmpty();
		BStopWatch watch("catalogSpeed", true);
		for (int i = 0; i < kNumStrings; i++) {
			catalog.SetString(strs[i].String(), trls[i].String(), ctxs[i].String(),
				cmts[i].String());
		}
		watch.Suspend();
		printf("\tadded %d strings in %9Ld usecs\n", catalog.size(), 
			watch.ElapsedTime());

		for (int i = 0; i < kNumStrings; i++) {
			trls[i] = BString("another_translation#") << 5000000+i;
		}
		watch.Reset();
		watch.Resume();
		for (int i = 0; i < kNumStrings; i++) {
			catalog.SetString(strs[i].String(), trls[i].String(), ctxs[i].String(),
				cmts[i].String());
		}
		watch.Suspend();
		printf("\tchanged %d strings in %9Ld usecs\n", catalog.size(), 
			watch.ElapsedTime());

		watch.Reset();
		watch.Resume();
		res = catalog.WriteToDisk();
		assert( res == B_OK);
		watch.Suspend();
		printf("\t%d strings written to disk in %9Ld usecs\n", catalog.size(),
			watch.ElapsedTime());

		catalog.MakeEmpty();
		watch.Reset();
		watch.Resume();
		res = catalog.ReadFromDisk("./TestCat.catalog");
		assert( res == B_OK);
		watch.Suspend();
		printf("\t%d strings read from disk in %9Ld usecs\n", catalog.size(),
			watch.ElapsedTime());

		catalog.MakeEmpty();
		watch.Reset();
		watch.Resume();
		for (int i = 0; i < kNumStrings; i++) {
			catalog.SetString(strs[i].String(), trls[i].String());
		}
		watch.Suspend();
		printf("\tadded %d strings (without context and comment) in %9Ld usecs\n", 
			catalog.size(), watch.ElapsedTime());

		watch.Reset();
		watch.Resume();
		for (int i = 0; i < kNumStrings; i++) {
			catalog.SetString(strs[i].String(), trls[i].String());
		}
		watch.Suspend();
		printf("\tchanged %d strings (without context and comment) in %9Ld usecs\n", 
			catalog.size(), watch.ElapsedTime());

		catalog.MakeEmpty();
		for (int i = 0; i < kNumStrings; i++) {
			trls[i] = BString("id_translation#") << 6000000+i;
		}
		watch.Reset();
		watch.Resume();
		for (int i = 0; i < kNumStrings; i++) {
			catalog.SetString(i, trls[i].String());
		}
		watch.Suspend();
		printf("\tadded %d strings by id in %9Ld usecs\n", catalog.size(), 
			watch.ElapsedTime());

		for (int i = 0; i < kNumStrings; i++) {
			trls[i] = BString("another_id_translation#") << 7000000+i;
		}
		watch.Reset();
		watch.Resume();
		for (int i = 0; i < kNumStrings; i++) {
			catalog.SetString(i, trls[i].String());
		}
		watch.Suspend();
		printf("\tchanged %d strings by id in %9Ld usecs\n", catalog.size(), 
			watch.ElapsedTime());

	}

	return 0;
}
