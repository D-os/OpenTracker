/* 
** Copyright 2003, Axel Dörfler, axeld@pinc-software.de. All rights reserved.
** Distributed under the terms of the OpenBeOS License.
*/


#include <Collator.h>
#include <Locale.h>
#include <Message.h>

#include <stdio.h>
#include <stdlib.h>


const char *kStrings[] = {
	"a-b-c",
	"a b c",
	"A.b.c",
	"ä,b,c",
	"abc",
	"gehen",
	"géhen",
	"aus",
	"äUß",
	"auss",
	"WO",
	"wÖ",
	"SO",
	"so",
	"açñ",
	"acn",
	"pêche",
	"pêché",
	"peché",
	"peche",
	"pecher",
	"eñe",
	"ene",
	"nz",
	"ña",
	"llamar",
	"luz",
};
const uint32 kNumStrings = sizeof(kStrings) / sizeof(kStrings[0]);

BCollator *gCollator;


int
compareStrings(const void *_a, const void *_b)
{
	const char *a = *(const char **)_a;
	const char *b = *(const char **)_b;

	return gCollator->Compare(a, b);
}


void
printArray(const char *label, const char **strings, size_t size)
{
	puts(label);

	uint32 bucket = 1;
	for (uint32 i = 0; i < size; i++) {
		if (i > 0) {
			int compare = gCollator->Compare(strings[i], strings[i - 1]);
			if (compare > 0)
				printf("\n% 2u)", bucket++);
			else if (compare < 0) {
				printf("\t*** broken sort order!\n");
				exit(-1);
			}

			// Test sort key generation

			BString a, b;
			gCollator->GetSortKey(strings[i - 1], &a);
			gCollator->GetSortKey(strings[i], &b);

			int keyCompare = strcmp(a.String(), b.String());
			if (keyCompare > 0 || (keyCompare == 0 && compare != 0))
				printf(" (*** \"%s\" wrong keys \"%s\" ***) ", a.String(), b.String());
		} else
			printf("% 2u)", bucket++);
		printf("\t%s", strings[i]);
	}
	putchar('\n');
}


void
usage()
{
	fprintf(stderr,
		"usage: collatorTest [-i]\n" // [addon-name]\n"
		"  -i\tignore punctuation (defaults to: punctuation matters)\n");
	exit(-1);
}


int
main(int argc, char **argv)
{
	// ToDo: get collator asked for in the command line arguments

	bool ignorePunctuation = false;

	while ((++argv)[0]) {
		if (!strcmp(argv[0], "-i"))
			ignorePunctuation = true;
		else if (!strcmp(argv[0], "--help"))
			usage();
	}

	// test archiving/unarchiving collator

	gCollator = be_locale->Collator();
	BMessage archive;
	if (gCollator->Archive(&archive, true) != B_OK)
		fprintf(stderr, "Archiving failed!\n");
	else {
		BArchivable *unarchived = instantiate_object(&archive);
		gCollator = dynamic_cast<BCollator *>(unarchived);
		if (gCollator == NULL) {
			fprintf(stderr, "Unarchiving failed!\n");

			delete unarchived;
			gCollator = be_locale->Collator();
		}
	}

	// test the BCollator::Compare() and GetSortKey() methods

	const char *strengthLabels[] = {"primary:  ", "secondary:", "tertiary: "};
	uint32 strengths[] = {B_COLLATE_PRIMARY, B_COLLATE_SECONDARY, B_COLLATE_TERTIARY};

	gCollator->SetIgnorePunctuation(ignorePunctuation);

	for (uint32 i = 0; i < sizeof(strengths) / sizeof(strengths[0]); i++) {
		gCollator->SetDefaultStrength(strengths[i]);
		qsort(kStrings, kNumStrings, sizeof(char *), compareStrings);

		printArray(strengthLabels[i], kStrings, kNumStrings);
	}
}

