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
	BPath path(directory,name);
	status_t status = BFile::SetTo(path.Path(), B_WRITE_ONLY | B_CREATE_FILE);
	if (status < B_OK)
		return status;

	static UnicodePropertiesHeader header = {
		sizeof(UnicodePropertiesHeader),
		B_HOST_IS_BENDIAN,
		PROPERTIES_FORMAT,
		{ 3, 0, 0 }		// version (taken from the ICU data version)
	};

	return Write(&header, sizeof(header));
}


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
	
	ssize_t len = (ssize_t)length;

	while (length >= 16) {
		Write(padding, 16);
		length -= 16;
	}
	if (length > 0)
		Write(padding, length);

	return len;
		// ToDo: Axel, please check this:
		// [zooey]: I assume we return the number of written chars...
}

