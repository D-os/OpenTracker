#include <NumberFormatParameters.h>

// flags
enum {
	USE_GROUPING_SET			= 0x01,
	SIGN_POLICY_SET				= 0x02,
	BASE_SET					= 0x04,
	USE_BASE_PREFIX_SET			= 0x08,
	MINIMAL_INTEGER_DIGITS_SET	= 0x10,
};

// constructor
BNumberFormatParameters::BNumberFormatParameters(
	const BNumberFormatParameters *parent)
	: fParent(parent),
	  fUseGrouping(false),
	  fSignPolicy(B_USE_NEGATIVE_SIGN_ONLY),
	  fBase(B_DECIMAL_BASE),
	  fUseBasePrefix(false),
	  fMinimalIntegerDigits(1),
	  fFlags(0)
{
}

// copy constructor
BNumberFormatParameters::BNumberFormatParameters(
	const BNumberFormatParameters &other)
	: fParent(other.fParent),
	  fUseGrouping(other.fUseGrouping),
	  fSignPolicy(other.fSignPolicy),
	  fBase(other.fBase),
	  fUseBasePrefix(other.fUseBasePrefix),
	  fMinimalIntegerDigits(other.fMinimalIntegerDigits),
	  fFlags(other.fFlags)
{
}

// destructor
BNumberFormatParameters::~BNumberFormatParameters()
{
}

// SetUseGrouping
void
BNumberFormatParameters::SetUseGrouping(bool useGrouping)
{
	fUseGrouping = useGrouping;
	fFlags |= USE_GROUPING_SET;
}

// UseGrouping
bool
BNumberFormatParameters::UseGrouping() const
{
	if (fFlags && USE_GROUPING_SET)
		return fUseGrouping;
	if (fParent)
		return fParent->UseGrouping();
	return fUseGrouping;
}

// SetSignPolicy
void
BNumberFormatParameters::SetSignPolicy(number_format_sign_policy policy)
{
	fSignPolicy = policy;
	fFlags |= SIGN_POLICY_SET;
}

// SignPolicy
number_format_sign_policy
BNumberFormatParameters::SignPolicy() const
{
	if (fFlags && SIGN_POLICY_SET)
		return fSignPolicy;
	if (fParent)
		return fParent->SignPolicy();
	return fSignPolicy;
}

// SetBase
void
BNumberFormatParameters::SetBase(number_format_base base)
{
	fBase = base;
	fFlags |= BASE_SET;
}

// Base
number_format_base
BNumberFormatParameters::Base() const
{
	if (fFlags && BASE_SET)
		return fBase;
	if (fParent)
		return fParent->Base();
	return fBase;
}

// SetUseBasePrefix
void
BNumberFormatParameters::SetUseBasePrefix(bool useBasePrefix)
{
	fUseBasePrefix = useBasePrefix;
	fFlags |= USE_BASE_PREFIX_SET;
}

// UseBasePrefix
bool
BNumberFormatParameters::UseBasePrefix() const
{
	if (fFlags && USE_BASE_PREFIX_SET)
		return fUseBasePrefix;
	if (fParent)
		return fParent->UseBasePrefix();
	return fUseBasePrefix;
}

// SetMinimalIntegerDigits
void
BNumberFormatParameters::SetMinimalIntegerDigits(size_t minIntegerDigits)
{
	fMinimalIntegerDigits = minIntegerDigits;
	fFlags |= MINIMAL_INTEGER_DIGITS_SET;
}

// MinimalIntegerDigits
size_t
BNumberFormatParameters::MinimalIntegerDigits() const
{
	if (fFlags && MINIMAL_INTEGER_DIGITS_SET)
		return fMinimalIntegerDigits;
	if (fParent)
		return fParent->MinimalIntegerDigits();
	return fMinimalIntegerDigits;
}

// SetParentNumberParameters
void
BNumberFormatParameters::SetParentNumberParameters(
	const BNumberFormatParameters *parent)
{
	fParent = parent;
}

// ParentNumberParameters
const BNumberFormatParameters *
BNumberFormatParameters::ParentNumberParameters() const
{
	return fParent;
}

// =
BNumberFormatParameters &
BNumberFormatParameters::operator=(const BNumberFormatParameters &other)
{
	fParent = other.fParent;
	fUseGrouping = other.fUseGrouping;
	fSignPolicy = other.fSignPolicy;
	fBase = other.fBase;
	fUseBasePrefix = other.fUseBasePrefix;
	fMinimalIntegerDigits = other.fMinimalIntegerDigits;
	fFlags = other.fFlags;
}

