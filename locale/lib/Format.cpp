#include <Format.h>
#include <FormatImpl.h>

// copy constructor
BFormat::BFormat(const BFormat &other)
	: BFormatParameters(other),
	  fImpl(other.fImpl)
{
}

// destructor
BFormat::~BFormat()
{
}

// =
BFormat &
BFormat::operator=(const BFormat &other)
{
	fImpl = other.fImpl;
	BFormatParameters::operator=(other);
	return *this;
}

// constructor
BFormat::BFormat(BFormatImpl *impl)
	: BFormatParameters(impl ? impl->DefaultFormatParameters() : NULL),
	  fImpl(impl)
{
}

