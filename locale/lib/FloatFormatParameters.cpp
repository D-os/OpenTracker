#include <FloatFormatParameters.h>

// flags
enum {
	MINIMAL_FRACTION_DIGITS_SET			= 0x01,
	MAXIMAL_FRACTION_DIGITS_SET			= 0x02,
	USE_CAPITALS_SET					= 0x04,
	FLOAT_FORMAT_TYPE_SET				= 0x08,
	ALWAYS_USE_FRACTION_SEPARATOR_SET	= 0x10,
	KEEP_TRAILING_FRACTION_ZEROS_SET	= 0x20,
};

// constructor
BFloatFormatParameters::BFloatFormatParameters(
	const BFloatFormatParameters *parent)
	: fParent(parent),
	  fMinimalFractionDigits(0),
	  fMaximalFractionDigits(100),
	  fUseCapitals(false),
	  fFloatFormatType(B_AUTO_FLOAT_FORMAT),
	  fAlwaysUseFractionSeparator(false),
	  fKeepTrailingFractionZeros(false),
	  fFlags(0)
{
}

// copy constructor
BFloatFormatParameters::BFloatFormatParameters(
	const BFloatFormatParameters &other)
	: fParent(other.fParent),
	  fMinimalFractionDigits(other.fMinimalFractionDigits),
	  fMaximalFractionDigits(other.fMaximalFractionDigits),
	  fUseCapitals(other.fUseCapitals),
	  fFloatFormatType(other.fFloatFormatType),
	  fAlwaysUseFractionSeparator(other.fAlwaysUseFractionSeparator),
	  fKeepTrailingFractionZeros(other.fKeepTrailingFractionZeros),
	  fFlags(other.fFlags)
{
}

// destructor
BFloatFormatParameters::~BFloatFormatParameters()
{
}

// SetMinimalFractionDigits
void
BFloatFormatParameters::SetMinimalFractionDigits(size_t minFractionDigits)
{
	fMinimalFractionDigits = minFractionDigits;
	fFlags |= MINIMAL_FRACTION_DIGITS_SET;
}

// MinimalFractionDigits
size_t
BFloatFormatParameters::MinimalFractionDigits() const
{
	if (fFlags & MINIMAL_FRACTION_DIGITS_SET)
		return fMinimalFractionDigits;
	if (fParent)
		return fParent->MinimalFractionDigits();
	return fMinimalFractionDigits;
}

// SetMaximalFractionDigits
void
BFloatFormatParameters::SetMaximalFractionDigits(size_t maxFractionDigits)
{
	fMaximalFractionDigits = maxFractionDigits;
	fFlags |= MAXIMAL_FRACTION_DIGITS_SET;
}

// MaximalFractionDigits
size_t
BFloatFormatParameters::MaximalFractionDigits() const
{
	if (fFlags & MAXIMAL_FRACTION_DIGITS_SET)
		return fMaximalFractionDigits;
	if (fParent)
		return fParent->MaximalFractionDigits();
	return fMaximalFractionDigits;
}

// SetUseCapitals
void
BFloatFormatParameters::SetUseCapitals(bool useCapitals)
{
	fUseCapitals = useCapitals;
	fFlags |= USE_CAPITALS_SET;
}

// UseCapitals
bool
BFloatFormatParameters::UseCapitals() const
{
	if (fFlags & USE_CAPITALS_SET)
		return fUseCapitals;
	if (fParent)
		return fParent->UseCapitals();
	return fUseCapitals;
}

// SetFloatFormatType
void
BFloatFormatParameters::SetFloatFormatType(float_format_type type)
{
	fFloatFormatType = type;
	fFlags |= FLOAT_FORMAT_TYPE_SET;
}

// FloatFormatType
float_format_type
BFloatFormatParameters::FloatFormatType() const
{
	if (fFlags & FLOAT_FORMAT_TYPE_SET)
		return fFloatFormatType;
	if (fParent)
		return fParent->FloatFormatType();
	return fFloatFormatType;
}

// SetAlwaysUseFractionSeparator
void
BFloatFormatParameters::SetAlwaysUseFractionSeparator(
	bool alwaysUseFractionSeparator)
{
	fAlwaysUseFractionSeparator = alwaysUseFractionSeparator;
	fFlags |= ALWAYS_USE_FRACTION_SEPARATOR_SET;
}

// AlwaysUseFractionSeparator
bool
BFloatFormatParameters::AlwaysUseFractionSeparator() const
{
	if (fFlags & ALWAYS_USE_FRACTION_SEPARATOR_SET)
		return fAlwaysUseFractionSeparator;
	if (fParent)
		return fParent->AlwaysUseFractionSeparator();
	return fAlwaysUseFractionSeparator;
}

// SetKeepTrailingFractionZeros
void
BFloatFormatParameters::SetKeepTrailingFractionZeros(
	bool keepTrailingFractionZeros)
{
	fKeepTrailingFractionZeros = keepTrailingFractionZeros;
	fFlags |= KEEP_TRAILING_FRACTION_ZEROS_SET;
}

// KeepTrailingFractionZeros
bool
BFloatFormatParameters::KeepTrailingFractionZeros() const
{
	if (fFlags & KEEP_TRAILING_FRACTION_ZEROS_SET)
		return fKeepTrailingFractionZeros;
	if (fParent)
		return fParent->KeepTrailingFractionZeros();
	return fKeepTrailingFractionZeros;
}

// SetParentFloatParameters
void
BFloatFormatParameters::SetParentFloatParameters(
	const BFloatFormatParameters *parent)
{
	fParent = parent;
}

// ParentFloatParameters
const BFloatFormatParameters *
BFloatFormatParameters::ParentFloatParameters() const
{
	return fParent;
}

// =
BFloatFormatParameters &
BFloatFormatParameters::operator=(const BFloatFormatParameters &other)
{
	fParent = other.fParent;
	fMinimalFractionDigits = other.fMinimalFractionDigits;
	fMaximalFractionDigits = other.fMaximalFractionDigits;
	fUseCapitals = other.fUseCapitals;
	fFloatFormatType = other.fFloatFormatType;
	fAlwaysUseFractionSeparator = other.fAlwaysUseFractionSeparator;
	fKeepTrailingFractionZeros = other.fKeepTrailingFractionZeros;
	fFlags = other.fFlags;
}

