#include <FormatImpl.h>

// constructor
BFormatImpl::BFormatImpl()
	: fParameters()
{
}

// destructor
BFormatImpl::~BFormatImpl()
{
}

// DefaultFormatParameters
BFormatParameters *
BFormatImpl::DefaultFormatParameters()
{
	return &fParameters;
}

// DefaultFormatParameters
const BFormatParameters *
BFormatImpl::DefaultFormatParameters() const
{
	return &fParameters;
}

