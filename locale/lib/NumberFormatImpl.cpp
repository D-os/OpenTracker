#include <NumberFormatImpl.h>

// constructor
BNumberFormatImpl::BNumberFormatImpl()
	: BFormatImpl(),
	  fParameters()
{
}

// destructor
BNumberFormatImpl::~BNumberFormatImpl()
{
}

// DefaultNumberFormatParameters
BNumberFormatParameters *
BNumberFormatImpl::DefaultNumberFormatParameters()
{
	return &fParameters;
}

// DefaultNumberFormatParameters
const BNumberFormatParameters *
BNumberFormatImpl::DefaultNumberFormatParameters() const
{
	return &fParameters;
}

