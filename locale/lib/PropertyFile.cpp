/* 
** Copyright 2003, Axel Dörfler, axeld@pinc-software.de. All rights reserved.
** Distributed under the terms of the OpenBeOS License.
*/


#include "PropertyFile.h"
#include "UnicodeProperties.h"

#include <Path.h>
#include <FindDirectory.h>


status_t
PropertyFile::SetTo(const char *directory, const char *name)
{
#ifdef GENERATE_PROPERTIES
	BPath path(directory,name);
	status_t status = BFile::SetTo(path.Path(), B_WRITE_ONLY | B_CREATE_FILE);
#else
	BPath path;
	status_t status = find_directory(B_BEOS_ETC_DIRECTORY, &path);
	if (status < B_OK)
		return status;

	path.Append(directory);
	path.Append(name);
	status = BFile::SetTo(path.Path(), B_READ_ONLY);
#endif
	if (status < B_OK)
		return status;

#ifdef GENERATE_PROPERTIES
	static UnicodePropertiesHeader header = {
		sizeof(UnicodePropertiesHeader),
		B_HOST_IS_BENDIAN,
		PROPERTIES_FORMAT,
		3, 0, 0		// version (taken from the ICU data version)
	};
	return Write(&header, sizeof(header));
#else	// GENERATE_PROPERTIES
	UnicodePropertiesHeader header;
	ssize_t bytes = Read(&header, sizeof(header));
	if (bytes < sizeof(header)
		|| header.size != sizeof(header)
		|| header.isBigEndian != B_HOST_IS_BENDIAN
		|| header.format != PROPERTIES_FORMAT)
		return B_BAD_DATA;
	return B_OK;
#endif
};


off_t 
PropertyFile::Size()
{
	off_t size;
	if (GetSize(&size) < B_OK)
		return 0;

	return size - sizeof(UnicodePropertiesHeader);
}


ssize_t
PropertyFile::WritePadding(size_t length)
{
	static uint8 padding[16] = {
		0xaa, 0xaa, 0xaa, 0xaa,
		0xaa, 0xaa, 0xaa, 0xaa,
		0xaa, 0xaa, 0xaa, 0xaa,
		0xaa, 0xaa, 0xaa, 0xaa
	};

	while (length >= 16) {
		Write(padding, 16);
		length -= 16;
	}
	if (length > 0)
		Write(padding, length);
}

