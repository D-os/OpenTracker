#include <NumberFormat.h>
#include <NumberFormatImpl.h>

// copy constructor
BNumberFormat::BNumberFormat(const BNumberFormat &other)
	: BFormat(other),
	  BNumberFormatParameters(other)
{
}

// destructor
BNumberFormat::~BNumberFormat()
{
}

// =
BNumberFormat &
BNumberFormat::operator=(const BNumberFormat &other)
{
	BFormat::operator=(other);
	BNumberFormatParameters::operator=(other);
}

// constructor
BNumberFormat::BNumberFormat(BNumberFormatImpl *impl)
	: BFormat(impl),
	  BNumberFormatParameters(impl ? impl->DefaultNumberFormatParameters()
	  							   : NULL)
{
}

// NumberFormatImpl
inline
BNumberFormatImpl *
BNumberFormat::NumberFormatImpl() const
{
	return static_cast<BNumberFormatImpl*>(fImpl);
}

