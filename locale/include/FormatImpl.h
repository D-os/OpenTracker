#ifndef _B_FORMAT_IMPL_H_
#define _B_FORMAT_IMPL_H_

#include <SupportDefs.h>

class BFormatImpl {
	public:
		BFormatImpl();
		virtual ~BFormatImpl();

		void RegisterReference();
		void UnregisterReference();
		BFormatImpl *ExclusiveReference();

		virtual BFormatImpl *Clone() const = 0;

	protected:
		BFormatImpl &operator=(const BFormatImpl &other);

	private:
		vint32	fReferences;
};

#endif	// _B_FORMAT_IMPL_H_
