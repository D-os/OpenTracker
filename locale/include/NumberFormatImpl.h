#ifndef _B_NUMBER_FORMAT_IMPL_H_
#define _B_NUMBER_FORMAT_IMPL_H_

#include <FormatImpl.h>

struct format_field_position;

class BNumberFormatImpl : public BFormatImpl {
	public:
		BNumberFormatImpl();
		virtual ~BNumberFormatImpl();

		// formatting

		virtual status_t Format(double number, BString *buffer) = 0;
		virtual status_t Format(double number, BString *buffer,
								format_field_position *positions,
								int32 positionCount = 1,
								int32 *fieldCount = NULL,
								bool allFieldPositions = false) = 0;


		virtual status_t SetGroupingUsed(bool useGrouping);
		virtual bool IsGroupingUsed() const;

		// TODO: ...

	protected:
		BNumberFormatImpl &operator=(const BNumberFormatImpl &other);

	private:
		bool	fGroupingUsed;
};


#endif	// _B_NUMBER_FORMAT_IMPL_H_
