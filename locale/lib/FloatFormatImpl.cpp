#include <FloatFormatImpl.h>

// constructor
BFloatFormatImpl::BFloatFormatImpl()
	: BNumberFormatImpl(),
	  fParameters()
{
}

// destructor
BFloatFormatImpl::~BFloatFormatImpl()
{
}

// DefaultFloatFormatParameters
BFloatFormatParameters *
BFloatFormatImpl::DefaultFloatFormatParameters()
{
	return &fParameters;
}

// DefaultFloatFormatParameters
const BFloatFormatParameters *
BFloatFormatImpl::DefaultFloatFormatParameters() const
{
	return &fParameters;
}

