/* 
** Copyright 2003, Oliver Tappe, zooey@hirschkaefer.de. All rights reserved.
** Distributed under the terms of the OpenBeOS License.
*/

#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <regex.h>

#include <Catalog.h>
using namespace BPrivate;
#include <Entry.h>
#include <File.h>
#include <String.h>


bool showKeys = false;
bool showSummary = false;
bool showWarnings = false;
const char *inputFile = NULL;
BString outputFile;
const char *catalogSig = NULL;
BString rxString("\\bbe_catalog\\s*->\\s*GetString\\s*");


BString str, ctx, cmt;
bool haveID;
int32 id;


EditableCatalog *catalog = NULL;


void
usage() 
{
	fprintf(stderr,
		"usage: collectcatkeys [-psw] [-r <regex>] [-o <outfile>] <prepCppFile> <catalogSig>\n"
		"  -o <outfile>\texplicitly specifies the name of the output-file\n"
		"  -p\t\tprint keys as they are found\n"
		"  -r <regex>\tchanges the regex used by the key-scanner to the one given,\n"
		"      \t\tthe default is:   \\bbe_catalog\\s*->\\s*GetString\\s*\n"
		"  -s\t\tshow summary\n"
		"  -w\t\tshow warnings about catalog-accesses that couldn't be resolved completely\n");
	exit(-1);
}


bool
fetchStr(const char *&in, BString &str, bool lookForID)
{
	int parLevel = 0;
	while(isspace(*in) || *in == '(') {
		if (*in == '(')
			parLevel++;
		in++;
	}
	if (*in == '"') {
		in++;
		bool quoted = false;
		while (*in != '"' || quoted) {
			if (quoted) {
				if (*in == 'n')
					str.Append("\n",1);
				else if (*in == 't')
					str.Append("\t",1);
				else if (*in == '"')
					str.Append("\"",1);
				else
					// dump quote from unknown quoting-sequence:
					str.Append(in,1);
				quoted = false;
			} else {
				quoted = (*in == '\\');
				if (!quoted)
					str.Append(in,1);
			}
			in++;
		}
		in++;
	} else if (!memcmp(in, "__null", 6)) {
		// NULL is preprocessed into __null, which we parse as ""
		in += 6;
	} else if (lookForID && (isdigit(*in) || *in == '-' || *in == '+')) {
		// try to parse an ID (a long):
		errno = 0;
		char *next;
		id = strtol(in, &next, 10);
		if (id != 0 || errno == 0) {
			haveID = true;
			in = next;
		}
	} else
		return false;
	while(isspace(*in) || *in == ')') {
		if (*in == ')') {
			if (!parLevel)
				return true;
			parLevel--;
		}
		in++;
	}
	return true;
}


bool
fetchKey(const char *&in)
{
	str = ctx = cmt = "";
	haveID = false;
	// fetch native string or id:
	if (!fetchStr(in, str, true))
		return false;
	if (*in == ',') {
		in++;
		// fetch context:
		if (!fetchStr(in, ctx, false))
			return false;
		if (*in == ',') {
			in++;
			// fetch comment:
			if (!fetchStr(in, cmt, false))
				return false;
		}
	}
	return true;
}


void
collectAllCatalogKeys(BString& inputStr)
{
	const int errbufsz = 256;
	char errbuf[errbufsz];
	regex_t rxprg;
	int rxflags = REG_EXTENDED;
	int rxres = regcomp(&rxprg, rxString.String(), rxflags);
	if (rxres != 0) {
		regerror(rxres, &rxprg, errbuf, errbufsz);
		fprintf(stderr, "regex-compilation error %s\n", errbuf);
		return;
	}
	status_t res;
	regmatch_t rxmatch;
	const char *in = inputStr.String();
	while(regexec(&rxprg, in, 1, &rxmatch, 0) == 0) {
		const char *start = in+rxmatch.rm_so;
		in += rxmatch.rm_eo;
		if (fetchKey(in)) {
			if (haveID) {
				if (showKeys)
					printf("CatKey(%ld)\n", id);
				res = catalog->SetString(id, "");
				if (res != B_OK) {
					fprintf(stderr, "couldn't add key %ld - error: %s\n", 
						id, strerror(res));
					exit(-1);
				}
			} else {
				if (showKeys)
					printf("CatKey(\"%s\", \"%s\", \"%s\")\n", str.String(), 
						ctx.String(), cmt.String());
				res = catalog->SetString(str.String(), ctx.String(), cmt.String());
				if (res != B_OK) {
					fprintf(stderr, "couldn't add key %s,%s,%s - error: %s\n", 
						str.String(), ctx.String(), cmt.String(), strerror(res));
					exit(-1);
				}
			}
		} else if (showWarnings) {
			const char *end = strchr(in, ';');
			BString match;
			if (end)
				match.SetTo(start, end-start+1);
			else
				match.SetTo(start, 40);
			fprintf(stderr, "Warning: couldn't resolve catalog-access:\n\t%s\n",
				match.String());
		}
	}
}


int
main(int argc, char **argv)
{
	while ((++argv)[0]) {
		if (argv[0][0] == '-' && argv[0][1] != '-') {
			char *arg = argv[0] + 1;
			char c;
			while ((c = *arg++) != '\0') {
				if (c == 'p')
					showKeys = true;
				else if (c == 's')
					showSummary = true;
				else if (c == 'w')
					showWarnings = true;
				else if (c == 'o') {
					outputFile = (++argv)[0];
					break;
				}
				else if (c == 'r') {
					rxString = (++argv)[0];
					break;
				}
			}
		} else if (!strcmp(argv[0], "--help")) {
			usage();
		} else {
			if (!inputFile)
				inputFile = argv[0];
			else if (!catalogSig)
				catalogSig = argv[0];
			else
				usage();
		}
	}
	if (!outputFile.Length() && inputFile) {
		// generate default output-file from input-file by replacing 
		// the extension with .catkeys:
		outputFile = inputFile;
		int32 dot = outputFile.FindLast('.');
		if (dot >= B_OK)
			outputFile.Truncate(dot);
		outputFile << ".catkeys";
	}
	if (!inputFile || !catalogSig || !outputFile.Length())
		usage();
	
	BFile inFile;
	status_t res = inFile.SetTo(inputFile, B_READ_ONLY);
	if (res != B_OK) {
		fprintf(stderr, "unable to open inputfile %s - error: %s\n", inputFile, 
			strerror(res));
		exit(-1);
	}
	off_t sz;
	inFile.GetSize(&sz);
	if (sz > 0) {
		BString inputStr;
		char *buf = inputStr.LockBuffer(sz);
		off_t rsz = inFile.Read(buf, sz);
		if (rsz < sz) {
			fprintf(stderr, "couldn't read %Ld bytes from %s (got only %Ld)\n", 
				sz, inputFile, rsz);
			exit(-1);
		}
		inputStr.UnlockBuffer(rsz);
		catalog = new EditableCatalog("Default", catalogSig, "native");
		collectAllCatalogKeys(inputStr);
		res = catalog->WriteToFile(outputFile.String());
		if (res != B_OK) {
			fprintf(stderr, "couldn't write catalog to %s - error: %s\n", 
				outputFile.String(), strerror(res));
			exit(-1);
		}
		if (showSummary) {
			int32 count = catalog->CountItems();
			if (count)
				fprintf(stderr, "%ld key%s found and written to %s\n",
					count, (count==1 ? "": "s"), outputFile.String());
			else
				fprintf(stderr, "no keys found\n");
		}
		delete catalog;
	}

	BEntry inEntry(inputFile);
	inEntry.Remove();
	
	return res;
}
