#include <FormatImpl.h>

// constructor
BFormatImpl::BFormatImpl()
	: fReferences(0)
{
}

// destructor
BFormatImpl::~BFormatImpl()
{
}

// RegisterReference
void
BFormatImpl::RegisterReference()
{
	atomic_add(&fReferences, 1);
}

// UnregisterReference
void
BFormatImpl::UnregisterReference()
{
	int32 references = atomic_add(&fReferences, -1);
	if (references == 1)
		delete this;
}

// ExclusiveReference
BFormatImpl *
BFormatImpl::ExclusiveReference()
{
	if (fReferences > 1) {
		BFormatImpl *clone = Clone();
		if (clone)
			UnregisterReference();
		return clone;
	}
	return this;
}

// =
BFormatImpl &
BFormatImpl::operator=(const BFormatImpl &other)
{
	return *this;
}

