#include <Format.h>
#include <FormatImpl.h>

// copy constructor
BFormat::BFormat(const BFormat &other)
	: fImpl(NULL)
{
	*this = other;
}

// destructor
BFormat::~BFormat()
{
	if (fImpl)
		fImpl->UnregisterReference();
}

// =
BFormat &
BFormat::operator=(const BFormat &other)
{
	if (fImpl != other.fImpl) {
		if (fImpl)
			fImpl->UnregisterReference();
		fImpl = other.fImpl;
		if (fImpl)
			fImpl->RegisterReference();
	}
	return *this;
}

// PrepareWriteAccess
status_t
BFormat::PrepareWriteAccess()
{
	if (!fImpl)
		return B_NO_INIT;
	BFormatImpl *impl = fImpl->ExclusiveReference();
	if (!impl)
		return B_NO_MEMORY;
	fImpl = impl;
	return B_OK;
}

// constructor
BFormat::BFormat(BFormatImpl *impl)
	: fImpl(impl)
{
	if (fImpl)
		fImpl->RegisterReference();
}

