#include <NumberFormat.h>
#include <NumberFormatImpl.h>

// copy constructor
BNumberFormat::BNumberFormat(const BNumberFormat &other)
	: BFormat(other)
{
}

// destructor
BNumberFormat::~BNumberFormat()
{
}

// Format
status_t
BNumberFormat::Format(double number, BString *buffer) const
{
	if (!fImpl)
		return B_NO_INIT;
	return NumberFormatImpl()->Format(number, buffer);
}

// Format
status_t
BNumberFormat::Format(double number, BString *buffer,
					  format_field_position *positions, int32 positionCount,
					  int32 *fieldCount, bool allFieldPositions) const
{
	if (!fImpl)
		return B_NO_INIT;
	return NumberFormatImpl()->Format(number, buffer, positions, positionCount,
		fieldCount, allFieldPositions);
}

// SetGroupingUsed
status_t
BNumberFormat::SetGroupingUsed(bool useGrouping)
{
	status_t error = PrepareWriteAccess();
	if (error != B_OK)
		return error;
	return NumberFormatImpl()->SetGroupingUsed(useGrouping);
}

// IsGroupingUsed
bool
BNumberFormat::IsGroupingUsed() const
{
	return (fImpl ? NumberFormatImpl()->IsGroupingUsed() : false);
}

// =
BNumberFormat &
BNumberFormat::operator=(const BNumberFormat &other)
{
	BFormat::operator=(other);
}

// constructor
BNumberFormat::BNumberFormat(BNumberFormatImpl *impl)
	: BFormat(impl)
{
}

// NumberFormatImpl
inline
BNumberFormatImpl *
BNumberFormat::NumberFormatImpl() const
{
	return static_cast<BNumberFormatImpl*>(fImpl);
}

