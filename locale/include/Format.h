#ifndef _B_FORMAT_H_
#define _B_FORMAT_H_

// types of fields contained in formatted strings
enum {
	// number format fields
	B_CURRENCY_FIELD,
	B_DECIMAL_SEPARATOR_FIELD,
	B_EXPONENT_FIELD,
	B_EXPONENT_SIGN_FIELD,
	B_EXPONENT_SYMBOL_FIELD,
	B_FRACTION_FIELD,
	B_GROUPING_SEPARATOR_FIELD,
	B_INTEGER_FIELD,
	B_PERCENT_FIELD,
	B_PERMILLE_FIELD,
	B_SIGN_FIELD,

	// date format fields
	// TODO: ...
};

// structure filled in while formatting
struct format_field_position {
	uint32	field_type;
	int32	start;
	int32	length;
};

class BFormat {
	public:
		BFormat();
		virtual ~BFormat();

		// Will be convenient, if the subclasses are not thread-safe.
		// And they won't be how I plan their interfaces.
		virtual BFormat *Clone() const = 0;
};

#endif	// _B_FORMAT_H_
