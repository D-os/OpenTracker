#ifndef _B_FORMAT_IMPL_H_
#define _B_FORMAT_IMPL_H_

#include <FormatParameters.h>
#include <SupportDefs.h>

class BFormatImpl {
	public:
		BFormatImpl();
		virtual ~BFormatImpl();

		BFormatParameters *DefaultFormatParameters();
		const BFormatParameters *DefaultFormatParameters() const;

	private:
		BFormatParameters	fParameters;
};

#endif	// _B_FORMAT_IMPL_H_
