#include <NumberFormatImpl.h>

static const bool kDefaultUseGrouping = false;

// constructor
BNumberFormatImpl::BNumberFormatImpl()
	: fGroupingUsed(kDefaultUseGrouping)
{
}

// destructor
BNumberFormatImpl::~BNumberFormatImpl()
{
}

// SetGroupingUsed
status_t
BNumberFormatImpl::SetGroupingUsed(bool useGrouping)
{
	fGroupingUsed = useGrouping;
}

// IsGroupingUsed
bool
BNumberFormatImpl::IsGroupingUsed() const
{
	return fGroupingUsed;
}

// =
BNumberFormatImpl &
BNumberFormatImpl::operator=(const BNumberFormatImpl &other)
{
	BFormatImpl::operator=(other);
	fGroupingUsed = other.fGroupingUsed;
}

