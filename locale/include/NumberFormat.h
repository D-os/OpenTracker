#ifndef _B_NUMBER_FORMAT_H_
#define _B_NUMBER_FORMAT_H_

#include <Format.h>
#include <NumberFormatParameters.h>

class BNumberFormatImpl;

class BNumberFormat : public BFormat, public BNumberFormatParameters {
	public:
		BNumberFormat(const BNumberFormat &other);
		~BNumberFormat();

		BNumberFormat &operator=(const BNumberFormat &other);

		BNumberFormat(BNumberFormatImpl *impl);		// conceptually private

	private:
		inline BNumberFormatImpl *NumberFormatImpl() const;
};


#endif	// _B_NUMBER_FORMAT_H_
