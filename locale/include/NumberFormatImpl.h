#ifndef _B_NUMBER_FORMAT_IMPL_H_
#define _B_NUMBER_FORMAT_IMPL_H_

#include <FormatImpl.h>
#include <NumberFormatParameters.h>

struct format_field_position;
class BNumberFormat;

class BNumberFormatImpl : public BFormatImpl {
	public:
		BNumberFormatImpl();
		virtual ~BNumberFormatImpl();

		BNumberFormatParameters *DefaultNumberFormatParameters();
		const BNumberFormatParameters *DefaultNumberFormatParameters() const;

	private:
		BNumberFormatParameters	fParameters;
};


#endif	// _B_NUMBER_FORMAT_IMPL_H_
