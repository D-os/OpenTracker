#ifndef _B_INTEGER_FORMAT_IMPL_H_
#define _B_INTEGER_FORMAT_IMPL_H_

#include <NumberFormatImpl.h>
#include <IntegerFormatParameters.h>

struct format_field_position;
class BIntegerFormat;

class BIntegerFormatImpl : public BNumberFormatImpl {
	public:
		BIntegerFormatImpl();
		virtual ~BIntegerFormatImpl();

		// formatting

		virtual status_t Format(const BIntegerFormat *format, int64 number,
								BString *buffer) const = 0;
		virtual status_t Format(const BIntegerFormat *format, int64 number,
								BString *buffer,
								format_field_position *positions,
								int32 positionCount = 1,
								int32 *fieldCount = NULL,
								bool allFieldPositions = false) const = 0;

		// TODO: ...

		BIntegerFormatParameters *DefaultIntegerFormatParameters();
		const BIntegerFormatParameters *DefaultIntegerFormatParameters() const;

	private:
		BIntegerFormatParameters	fParameters;
};


#endif	// _B_INTEGER_FORMAT_IMPL_H_
