#ifndef _B_FLOAT_FORMAT_IMPL_H_
#define _B_FLOAT_FORMAT_IMPL_H_

#include <NumberFormatImpl.h>
#include <FloatFormatParameters.h>

struct format_field_position;
class BFloatFormat;

class BFloatFormatImpl : public BNumberFormatImpl {
	public:
		BFloatFormatImpl();
		virtual ~BFloatFormatImpl();

		// formatting

		virtual status_t Format(const BFloatFormat *format, double number,
								BString *buffer) const = 0;
		virtual status_t Format(const BFloatFormat *format, double number,
								BString *buffer,
								format_field_position *positions,
								int32 positionCount = 1,
								int32 *fieldCount = NULL,
								bool allFieldPositions = false) const = 0;

		// TODO: ...

		BFloatFormatParameters *DefaultFloatFormatParameters();
		const BFloatFormatParameters *DefaultFloatFormatParameters() const;

	private:
		BFloatFormatParameters	fParameters;
};


#endif	// _B_FLOAT_FORMAT_IMPL_H_
