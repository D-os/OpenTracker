#include <IntegerFormatImpl.h>

// constructor
BIntegerFormatImpl::BIntegerFormatImpl()
	: BNumberFormatImpl(),
	  fParameters()
{
}

// destructor
BIntegerFormatImpl::~BIntegerFormatImpl()
{
}

// DefaultIntegerFormatParameters
BIntegerFormatParameters *
BIntegerFormatImpl::DefaultIntegerFormatParameters()
{
	return &fParameters;
}

// DefaultIntegerFormatParameters
const BIntegerFormatParameters *
BIntegerFormatImpl::DefaultIntegerFormatParameters() const
{
	return &fParameters;
}

