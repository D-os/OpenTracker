#include <algorithm>
#include <new>
#include <stdlib.h>

#include <GenericNumberFormat.h>
#include <String.h>
#include <UnicodeChar.h>

// constants (more below the helper classes)

static const int kMaxIntDigitCount = 20;	// int64: 19 + sign, uint64: 20

// Symbol

// constructor
BGenericNumberFormat::Symbol::Symbol(const char *symbol)
	: symbol(NULL),
	  length(0),
	  char_count(0)
{
	SetTo(symbol);
}

// destructor
BGenericNumberFormat::Symbol::~Symbol()
{
	Unset();
}

// SetTo
status_t
BGenericNumberFormat::Symbol::SetTo(const char *symbol)
{
	// unset old
	if (this->symbol) {
		free(this->symbol);
		length = 0;
		char_count = 0;
	}
	// set new
	if (symbol) {
		this->symbol = strdup(symbol);
		if (!this->symbol)
			return B_NO_MEMORY;
		length = strlen(this->symbol);
		char_count = BUnicodeChar::UTF8StringLength(this->symbol);
	}
	return B_OK;
}


// GroupingInfo
class BGenericNumberFormat::GroupingInfo {
	public:
		GroupingInfo()
			: fSeparators(NULL),
			  fSeparatorCount(0),
			  fSizes(NULL),
			  fSizeCount(0),
			  fSumSizes(NULL),
			  fSumSeparators(NULL)
		{
		}

		GroupingInfo(const char **separators, int32 separatorCount,
					 const size_t *sizes, int32 sizeCount)
			: fSeparators(NULL),
			  fSeparatorCount(0),
			  fSizes(NULL),
			  fSizeCount(0),
			  fSumSizes(NULL),
			  fSumSeparators(NULL)
		{
			SetTo(separators, separatorCount, sizes, sizeCount);
		}

		~GroupingInfo()
		{
			Unset();
		}

		status_t SetTo(const char **separators, int32 separatorCount,
					   const size_t *sizes, int32 sizeCount)
		{
			// unset old
			Unset();
			// set new
			if (!separators && separatorCount <= 0 || !sizes && sizeCount <= 0)
				return B_OK;
			// allocate arrays
			fSeparators = new(nothrow) Symbol[separatorCount];
			fSizes = new(nothrow) int32[sizeCount];
			fSumSizes = new(nothrow) int32[sizeCount];
			fSumSeparators = new(nothrow) Symbol*[separatorCount];
			if (!fSeparators || !fSizes || !fSumSizes || !fSumSeparators) {
				Unset();
				return B_NO_MEMORY;
			}
			fSeparatorCount = separatorCount;
			fSizeCount = sizeCount;
			// separators
			for (int i = 0; i < separatorCount; i++) {
				status_t error = fSeparators[i].SetTo(separators[i]);
				if (error != B_OK) {
					Unset();
					return error;
				}
			}
			// sizes and sum arrays
			int32 sumSize = -1;
			for (int32 i = 0; i < sizeCount; i++) {
				fSizes[i] = (int32)sizes[i];
				sumSize += fSizes[i];
				fSumSizes[i] = sumSize;
				fSumSeparators[i] = &fSeparators[min(i, fSeparatorCount)];
			}
		}

		void Unset()
		{
			if (fSeparators) {
				delete[] fSeparators;
				fSeparators = NULL;
			}
			fSeparatorCount = 0;
			if (fSizes) {
				delete[] fSizes;
				fSizes = NULL;
			}
			fSizeCount = 0;
			if (fSumSizes) {
				delete[] fSumSizes;
				fSumSizes = NULL;
			}
			if (fSumSeparators) {
				delete[] fSumSeparators;
				fSumSeparators = NULL;
			}
		}

		const Symbol *SeparatorForDigit(int32 position) const
		{
			for (int i = fSizeCount - 1; i >= 0; i--) {
				if (fSumSizes[i] <= position) {
					if (fSumSizes[i] == position
						|| i == fSizeCount - 1
						   && (position - fSumSizes[i]) % fSizes[i] == 0) {
						return fSumSeparators[i];
					}
					return NULL;
				}
			}
			return NULL;
		}

	private:
		Symbol	*fSeparators;
		int32	fSeparatorCount;
		int32	*fSizes;
		int32	fSizeCount;
		int32	*fSumSizes;
		Symbol	**fSumSeparators;
};


// SignSymbols
class BGenericNumberFormat::SignSymbols {
	public:
		SignSymbols()
			: fPlusPrefix(),
			  fMinusPrefix(),
			  fPadPlusPrefix(),
			  fNoForcePlusPrefix(),
			  fPlusSuffix(),
			  fMinusSuffix(),
			  fPadPlusSuffix(),
			  fNoForcePlusSuffix()
		{
		}

		SignSymbols(const char *plusPrefix, const char *minusPrefix,
			const char *padPlusPrefix, const char *noForcePlusPrefix,
			const char *plusSuffix, const char *minusSuffix,
			const char *padPlusSuffix, const char *noForcePlusSuffix)
			: fPlusPrefix(plusPrefix),
			  fMinusPrefix(minusPrefix),
			  fPadPlusPrefix(padPlusPrefix),
			  fNoForcePlusPrefix(noForcePlusPrefix),
			  fPlusSuffix(plusSuffix),
			  fMinusSuffix(minusSuffix),
			  fPadPlusSuffix(padPlusSuffix),
			  fNoForcePlusSuffix(noForcePlusSuffix)
		{
		}

		~SignSymbols()
		{
		}

		status_t SetTo(const char *plusPrefix, const char *minusPrefix,
			const char *padPlusPrefix, const char *noForcePlusPrefix,
			const char *plusSuffix, const char *minusSuffix,
			const char *padPlusSuffix, const char *noForcePlusSuffix)
		{
			status_t error = B_OK;
			if (error == B_OK)
				error = fPlusPrefix.SetTo(plusPrefix);
			if (error == B_OK)
				error = fMinusPrefix.SetTo(minusPrefix);
			if (error == B_OK)
				error = fPadPlusPrefix.SetTo(noForcePlusPrefix);
			if (error == B_OK)
				error = fNoForcePlusPrefix.SetTo(noForcePlusPrefix);
			if (error == B_OK)
				error = fPlusSuffix.SetTo(plusSuffix);
			if (error == B_OK)
				error = fMinusSuffix.SetTo(minusSuffix);
			if (error == B_OK)
				error = fPadPlusSuffix.SetTo(noForcePlusSuffix);
			if (error == B_OK)
				error = fNoForcePlusSuffix.SetTo(noForcePlusSuffix);
			if (error != B_OK)
				Unset();
			return error;
		}

		void Unset()
		{
			fPlusPrefix.Unset();
			fMinusPrefix.Unset();
			fNoForcePlusPrefix.Unset();
			fPadPlusPrefix.Unset();
			fPlusSuffix.Unset();
			fMinusSuffix.Unset();
			fNoForcePlusSuffix.Unset();
			fPadPlusSuffix.Unset();
		}

		const Symbol *PlusPrefix() const
		{
			return &fPlusPrefix;
		}

		const Symbol *MinusPrefix() const
		{
			return &fMinusPrefix;
		}

		const Symbol *PadPlusPrefix() const
		{
			return &fPadPlusPrefix;
		}

		const Symbol *NoForcePlusPrefix() const
		{
			return &fNoForcePlusPrefix;
		}

		const Symbol *PlusSuffix() const
		{
			return &fPlusSuffix;
		}

		const Symbol *MinusSuffix() const
		{
			return &fMinusSuffix;
		}

		const Symbol *PadPlusSuffix() const
		{
			return &fPadPlusSuffix;
		}

		const Symbol *NoForcePlusSuffix() const
		{
			return &fNoForcePlusSuffix;
		}

	private:
		Symbol	fPlusPrefix;
		Symbol	fMinusPrefix;
		Symbol	fPadPlusPrefix;
		Symbol	fNoForcePlusPrefix;
		Symbol	fPlusSuffix;
		Symbol	fMinusSuffix;
		Symbol	fPadPlusSuffix;
		Symbol	fNoForcePlusSuffix;
};


// BufferWriter
class BGenericNumberFormat::BufferWriter {
	public:
		BufferWriter(char *buffer = NULL, int32 bufferSize = 0)
		{
			SetTo(buffer, bufferSize);
		}

		void SetTo(char *buffer = NULL, int32 bufferSize = 0)
		{
			fBuffer = buffer;
			fBufferSize = bufferSize;
			fPosition = 0;
			fCharCount = 0;
			fDryRun = (!fBuffer || (fBufferSize == 0));
			if (!fDryRun)
				fBuffer[0] = '\0';
		}

		int32 StringLength() const
		{
			return fPosition;
		}

		int32 CharCount() const
		{
			return fCharCount;
		}

		bool IsOverflow() const
		{
			return (fPosition >= fBufferSize);
		}

		void Append(const char *bytes, size_t length, size_t charCount)
		{
			int32 newPosition = fPosition + length;
			fDryRun |= (newPosition >= fBufferSize);
			if (!fDryRun && length > 0) {
				memcpy(fBuffer + fPosition, bytes, length);
				fBuffer[newPosition] = '\0';
			}
			fPosition = newPosition;
			fCharCount += charCount;
		}

		void Append(const Symbol &symbol)
		{
			Append(symbol.symbol, symbol.length, symbol.char_count);
		}

		void Append(const Symbol *symbol)
		{
			if (symbol)
				Append(*symbol);
		}

		void Append(char c, int32 count)	// ASCII 128 chars only!
		{
			if (count < 0)
				return;
			int32 newPosition = fPosition + count;
			fDryRun |= (newPosition >= fBufferSize);
			if (!fDryRun && count > 0) {
				memset(fBuffer + fPosition, c, count);
				fBuffer[newPosition] = '\0';
			}
			fPosition = newPosition;
			fCharCount += count;
		}

	private:
		char	*fBuffer;
		int32	fBufferSize;
		int32	fPosition;
		int32	fCharCount;
		bool	fDryRun;
};


// Integer
class BGenericNumberFormat::Integer {
	public:
		Integer(int64 number)
			: fDigitCount(0),
			  fNegative(number < 0)
		{
			if (fNegative)
				Init(0ULL - (uint64)number);
			else
				Init(number);
		}

		Integer(uint64 number)
			: fDigitCount(0),
			  fNegative(false)
		{
			Init(number);
		}

		int DigitCount() const
		{
			return fDigitCount;
		}

		bool IsNegative() const
		{
			return fNegative;
		}

		char *ToString(char *str) const
		{
			if (fDigitCount == 0) {
				str[0] = '0';
				str[1] = '\0';
			} else if (fNegative) {
				str[0] = '-';
				for (int i = 0; i < fDigitCount; i++)
					str[i + 1] = '0' + fDigits[fDigitCount - i - 1];
				str[fDigitCount + 1] = '\0';
			} else {
				for (int i = 0; i < fDigitCount; i++)
					str[i] = '0' + fDigits[fDigitCount - i - 1];
				str[fDigitCount] = '\0';
			}
			return str;
		}

		void Format(BufferWriter &writer, const Symbol *digitSymbols,
			const SignSymbols *signSymbols,
			number_format_sign_policy signPolicy,
			const GroupingInfo *groupingInfo, int32 minDigits)
		{
			const Symbol *suffix;
			// write sign prefix
			if (fNegative) {
				writer.Append(signSymbols->MinusPrefix());
				suffix = signSymbols->MinusSuffix();
			} else {
				switch (signPolicy) {
					case B_USE_NEGATIVE_SIGN_ONLY:
						writer.Append(signSymbols->NoForcePlusPrefix());
						suffix = signSymbols->NoForcePlusSuffix();
						break;
					case B_USE_SPACE_FOR_POSITIVE_SIGN:
						writer.Append(signSymbols->PadPlusPrefix());
						suffix = signSymbols->PadPlusSuffix();
						break;
					case B_USE_POSITIVE_SIGN:
						writer.Append(signSymbols->PlusPrefix());
						suffix = signSymbols->PlusSuffix();
						break;
				}
			}
			// the digits
			if (fDigitCount == 0 && minDigits < 1) {
				// special case for zero and less the one minimal digit
				writer.Append(digitSymbols[0]);
			} else {
				// not zero
				if (groupingInfo) {
					// use grouping
					// pad with zeros up to minDigits
					int32 digitCount = max(fDigitCount, minDigits);
					for (int i = minDigits - 1; i >= fDigitCount; i--) {
						if (i != digitCount - 1)
							writer.Append(groupingInfo->SeparatorForDigit(i));
						writer.Append(digitSymbols[0]);
					}
					// write digits
					for (int i = fDigitCount - 1; i >= 0; i--) {
						if (i != digitCount - 1)
							writer.Append(groupingInfo->SeparatorForDigit(i));
						writer.Append(digitSymbols[fDigits[i]]);
					}
				} else {
					// no grouping
					// pad with zeros up to minDigits
					for (int i = fDigitCount; i < minDigits; i++)
						writer.Append(digitSymbols[0]);
					// write digits
					for (int i = fDigitCount - 1; i >= 0; i--)
						writer.Append(digitSymbols[fDigits[i]]);
				}
			}
			// append suffix
			writer.Append(suffix);
		}

	private:
		void Init(uint64 number)
		{
			fDigitCount = 0;
			while (number) {
				fDigits[fDigitCount] = number % 10;
				number /= 10;
				fDigitCount++;
			}
		}

	private:
		uchar	fDigits[kMaxIntDigitCount];
		int32	fDigitCount;
		bool	fNegative;
};

// constants

// digit symbols
static const BGenericNumberFormat::Symbol kDefaultDigitSymbols[] = {
	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"
};

// decimal separator symbol
static const BGenericNumberFormat::Symbol kDefaultDecimalSeparator = ".";

// grouping separator symbols
static const char *kDefaultGroupingSeparators[] = { "," };
static const int32 kDefaultGroupingSeparatorCount
	= sizeof(kDefaultGroupingSeparators) / sizeof(const char*);

// grouping sizes
static const size_t kDefaultGroupingSizes[] = { 3 };
static const int32 kDefaultGroupingSizeCount
	= sizeof(kDefaultGroupingSizes) / sizeof(size_t);

// grouping info
static const BGenericNumberFormat::GroupingInfo kDefaultGroupingInfo(
	kDefaultGroupingSeparators, kDefaultGroupingSeparatorCount,
	kDefaultGroupingSizes, kDefaultGroupingSizeCount
);

// exponent symbol
static const BGenericNumberFormat::Symbol kDefaultExponentSymbol = "e";
static const BGenericNumberFormat::Symbol kDefaultUpperCaseExponentSymbol
	= "E";

// NaN symbol
static const BGenericNumberFormat::Symbol kDefaultNaNSymbol = "NaN";
static const BGenericNumberFormat::Symbol kDefaultUpperCaseNaNSymbol = "NaN";

// infinity symbol
static const BGenericNumberFormat::Symbol kDefaultInfinitySymbol
	= "infinity";
static const BGenericNumberFormat::Symbol kDefaultUpperCaseInfinitySymbol
	= "INFINITY";

// negative infinity symbol
static const BGenericNumberFormat::Symbol kDefaultNegativeInfinitySymbol
	= "-infinity";
static const BGenericNumberFormat::Symbol
	kDefaultUpperCaseNegativeInfinitySymbol = "-INFINITY";

// sign symbols
static const BGenericNumberFormat::SignSymbols kDefaultSignSymbols(
	"+", "-", " ", "",	// prefixes
	"", "", "", ""		// suffixes
);

// exponent sign symbols
static const BGenericNumberFormat::SignSymbols kDefaultExponentSignSymbols(
	"+", "-", " ", "",	// prefixes
	"", "", "", ""		// suffixes
);


// constructor
BGenericNumberFormat::BGenericNumberFormat()
	: fIntegerParameters(),
	  fFloatParameters(),
	  fDigitSymbols(NULL),
	  fDecimalSeparator(NULL),
	  fGroupingInfo(NULL),
	  fExponentSymbol(NULL),
	  fUpperCaseExponentSymbol(NULL),
	  fNaNSymbol(NULL),
	  fUpperCaseNaNSymbol(NULL),
	  fInfinitySymbol(NULL),
	  fUpperCaseInfinitySymbol(NULL),
	  fNegativeInfinitySymbol(NULL),
	  fUpperCaseNegativeInfinitySymbol(NULL),
	  fSignSymbols(NULL),
	  fExponentSignSymbols(NULL)
{
}

// destructor
BGenericNumberFormat::~BGenericNumberFormat()
{
}

// FormatInteger
status_t
BGenericNumberFormat::FormatInteger(
	const BIntegerFormatParameters *parameters, int64 number, BString *buffer,
	format_field_position *positions, int32 positionCount, int32 *fieldCount,
	bool allFieldPositions) const
{
	if (!buffer)
		return B_BAD_VALUE;
	char localBuffer[1024];
	status_t error = FormatInteger(parameters, number, localBuffer,
		sizeof(localBuffer), positions, positionCount, fieldCount,
		allFieldPositions);
	if (error == B_OK) {
		buffer->Append(localBuffer);
		// TODO: Check, if the allocation succeeded.
	}
	return error;
}

// FormatInteger
status_t
BGenericNumberFormat::FormatInteger(
	const BIntegerFormatParameters *parameters, int64 number, char *buffer,
	size_t bufferSize, format_field_position *positions, int32 positionCount,
	int32 *fieldCount, bool allFieldPositions) const
{
	// TODO: Check parameters.
	if (!parameters)
		parameters = DefaultIntegerFormatParameters();
	if (bufferSize <= parameters->FormatWidth())
		return EOVERFLOW;
	// decompose number into digits
	Integer integer(number);
	// prepare some parameters
	const GroupingInfo *groupingInfo = NULL;
	if (parameters->UseGrouping())
		groupingInfo = GetGroupingInfo();
	// compute the length of the formatted string
	BufferWriter writer;
	integer.Format(writer, DigitSymbols(),
		GetSignSymbols(), parameters->SignPolicy(), groupingInfo,
		parameters->MinimalIntegerDigits());
	int32 stringLength = writer.StringLength();
	int32 charCount = writer.CharCount();
	// consider alignment and check the available space in the buffer
	int32 padding = max(0L, (int32)parameters->FormatWidth() - charCount);
	if (bufferSize <= stringLength + padding)
		return EOVERFLOW;
	// prepare for writing
	writer.SetTo(buffer, bufferSize);
	// write padding for right field alignment
	if (parameters->Alignment() == B_ALIGN_FORMAT_RIGHT && padding > 0)
		writer.Append(' ', padding);
	// write the number
	integer.Format(writer, DigitSymbols(),
		GetSignSymbols(), parameters->SignPolicy(), groupingInfo,
		parameters->MinimalIntegerDigits());
	// write padding for left field alignment
	if (parameters->Alignment() == B_ALIGN_FORMAT_LEFT && padding > 0)
		writer.Append(' ', padding);
	return B_OK;
}

// SetDefaultIntegerFormatParameters
status_t
BGenericNumberFormat::SetDefaultIntegerFormatParameters(
	const BIntegerFormatParameters *parameters)
{
	if (!parameters)
		return B_BAD_VALUE;
	fIntegerParameters = *parameters;
	return B_OK;
}

// DefaultIntegerFormatParameters
BIntegerFormatParameters *
BGenericNumberFormat::DefaultIntegerFormatParameters()
{
	return &fIntegerParameters;
}

// DefaultIntegerFormatParameters
const BIntegerFormatParameters *
BGenericNumberFormat::DefaultIntegerFormatParameters() const
{
	return &fIntegerParameters;
}

// SetDefaultFloatFormatParameters
status_t
BGenericNumberFormat::SetDefaultFloatFormatParameters(
	const BFloatFormatParameters *parameters)
{
	if (!parameters)
		return B_BAD_VALUE;
	fFloatParameters = *parameters;
	return B_OK;
}

// DefaultFloatFormatParameters
BFloatFormatParameters *
BGenericNumberFormat::DefaultFloatFormatParameters()
{
	return &fFloatParameters;
}

// DefaultFloatFormatParameters
const BFloatFormatParameters *
BGenericNumberFormat::DefaultFloatFormatParameters() const
{
	return &fFloatParameters;
}

// SetDigitSymbols
status_t
BGenericNumberFormat::SetDigitSymbols(const char **digits)
{
	// check parameters
	if (digits) {
		for (int i = 0; i < 10; i++) {
			if (!digits[i])
				return B_BAD_VALUE;
		}
	}
	// unset old
	if (fDigitSymbols) {
		delete[] fDigitSymbols;
		fDigitSymbols = NULL;
	}
	// set new
	if (digits) {
		fDigitSymbols = new(nothrow) Symbol[10];
		if (!fDigitSymbols)
			return B_NO_MEMORY;
		for (int i = 0; i < 10; i++) {
			status_t error = fDigitSymbols[i].SetTo(digits[i]);
			if (error != B_OK) {
				SetDigitSymbols(NULL);
				return error;
			}
		}
	}
	return B_OK;
}

// SetDecimalSeparator
status_t
BGenericNumberFormat::SetDecimalSeparator(const char *decimalSeparator)
{
	return _SetSymbol(&fDecimalSeparator, decimalSeparator);
}

// SetGroupingInfo
status_t
BGenericNumberFormat::SetGroupingInfo(const char **groupingSeparators,
	size_t separatorCount, size_t *groupingSizes, size_t sizeCount)
{
	// check parameters
	if (groupingSeparators && separatorCount > 0 && groupingSizes
		&& sizeCount) {
		for (int i = 0; i < separatorCount; i++) {
			if (!groupingSeparators[i])
				return B_BAD_VALUE;
		}
	}
	// unset old
	if (fGroupingInfo) {
		delete fGroupingInfo;
		fGroupingInfo = NULL;
	}
	// set new
	if (groupingSeparators && separatorCount > 0 && groupingSizes
		&& sizeCount) {
		fGroupingInfo = new GroupingInfo;
		if (!fGroupingInfo)
			return B_NO_MEMORY;
		status_t error = fGroupingInfo->SetTo(groupingSeparators,
			separatorCount, groupingSizes, sizeCount);
		if (error != B_OK) {
			delete fGroupingInfo;
			fGroupingInfo = NULL;
			return error;
		}
	}
	return B_OK;
}

// SetExponentSymbol
status_t
BGenericNumberFormat::SetExponentSymbol(const char *exponentSymbol,
	const char *upperCaseExponentSymbol)
{
	status_t error = _SetSymbol(&fExponentSymbol, exponentSymbol);
	if (error == B_OK)
		error = _SetSymbol(&fUpperCaseExponentSymbol, upperCaseExponentSymbol);
	if (error != B_OK)
		SetExponentSymbol(NULL, NULL);
	return error;
}

// SetSpecialNumberSymbols
status_t
BGenericNumberFormat::SetSpecialNumberSymbols(const char *nan,
	const char *infinity, const char *negativeInfinity,
	const char *upperCaseNaN, const char *upperCaseInfinity,
	const char *upperCaseNegativeInfinity)
{
	status_t error = _SetSymbol(&fNaNSymbol, nan);
	if (error == B_OK)
		error = _SetSymbol(&fInfinitySymbol, infinity);
	if (error == B_OK)
		error = _SetSymbol(&fNegativeInfinitySymbol, negativeInfinity);
	if (error == B_OK)
		error = _SetSymbol(&fUpperCaseNaNSymbol, upperCaseNaN);
	if (error == B_OK)
		error = _SetSymbol(&fUpperCaseInfinitySymbol, upperCaseInfinity);
	if (error == B_OK) {
		error = _SetSymbol(&fUpperCaseNegativeInfinitySymbol,
			upperCaseNegativeInfinity);
	}
	if (error != B_OK)
		SetSpecialNumberSymbols(NULL, NULL, NULL, NULL, NULL, NULL);
	return error;
}

// SetSignSymbols
status_t
BGenericNumberFormat::SetSignSymbols(const char *plusPrefix,
			const char *minusPrefix, const char *padPlusPrefix,
			const char *noForcePlusPrefix, const char *plusSuffix,
			const char *minusSuffix, const char *padPlusSuffix,
			const char *noForcePlusSuffix)
{
	if (!fSignSymbols) {
		fSignSymbols = new(nothrow) SignSymbols;
		if (!fSignSymbols)
			return B_NO_MEMORY;
	}
	return fSignSymbols->SetTo(plusPrefix, minusPrefix, padPlusPrefix,
		noForcePlusPrefix, plusSuffix, minusSuffix, padPlusSuffix,
		noForcePlusSuffix);
}

// SetExponentSignSymbols
status_t
BGenericNumberFormat::SetExponentSignSymbols(const char *plusPrefix,
	const char *minusPrefix, const char *plusSuffix, const char *minusSuffix)
{
	if (!fExponentSignSymbols) {
		fExponentSignSymbols = new(nothrow) SignSymbols;
		if (!fExponentSignSymbols)
			return B_NO_MEMORY;
	}
	return fExponentSignSymbols->SetTo(plusPrefix, minusPrefix, plusPrefix,
		plusPrefix, plusSuffix, minusSuffix, plusSuffix, plusSuffix);
}

// DigitSymbols
const BGenericNumberFormat::Symbol *
BGenericNumberFormat::DigitSymbols() const
{
	return (fDigitSymbols ? fDigitSymbols : kDefaultDigitSymbols);
}

// DecimalSeparator
const BGenericNumberFormat::Symbol *
BGenericNumberFormat::DecimalSeparator() const
{
	return (fDecimalSeparator ? fDecimalSeparator : &kDefaultDecimalSeparator);
}

// GetGroupingInfo
const BGenericNumberFormat::GroupingInfo *
BGenericNumberFormat::GetGroupingInfo() const
{
	return (fGroupingInfo ? fGroupingInfo : &kDefaultGroupingInfo);
}

// ExponentSymbol
const BGenericNumberFormat::Symbol *
BGenericNumberFormat::ExponentSymbol(bool upperCase) const
{
	if (fExponentSymbol) {
		return (upperCase && fUpperCaseExponentSymbol ? fUpperCaseExponentSymbol
													  : fExponentSymbol);
	}
	return (upperCase ? &kDefaultUpperCaseExponentSymbol
					  : &kDefaultExponentSymbol);
}

// NaNSymbol
const BGenericNumberFormat::Symbol *
BGenericNumberFormat::NaNSymbol(bool upperCase) const
{
	if (fNaNSymbol) {
		return (upperCase && fUpperCaseNaNSymbol ? fUpperCaseNaNSymbol
												 : fNaNSymbol);
	}
	return (upperCase ? &kDefaultUpperCaseNaNSymbol
					  : &kDefaultNaNSymbol);
}

// InfinitySymbol
const BGenericNumberFormat::Symbol *
BGenericNumberFormat::InfinitySymbol(bool upperCase) const
{
	if (fInfinitySymbol) {
		return (upperCase && fUpperCaseInfinitySymbol ? fUpperCaseInfinitySymbol
													  : fInfinitySymbol);
	}
	return (upperCase ? &kDefaultUpperCaseInfinitySymbol
					  : &kDefaultInfinitySymbol);
}

// NegativeInfinitySymbol
const BGenericNumberFormat::Symbol *
BGenericNumberFormat::NegativeInfinitySymbol(bool upperCase) const
{
	if (fNegativeInfinitySymbol) {
		return (upperCase && fUpperCaseNegativeInfinitySymbol
			? fUpperCaseNegativeInfinitySymbol : fNegativeInfinitySymbol);
	}
	return (upperCase ? &kDefaultUpperCaseNegativeInfinitySymbol
					  : &kDefaultNegativeInfinitySymbol);
}

// GetSignSymbols
const BGenericNumberFormat::SignSymbols *
BGenericNumberFormat::GetSignSymbols() const
{
	return (fSignSymbols ? fSignSymbols : &kDefaultSignSymbols);
}

// ExponentSignSymbols
const BGenericNumberFormat::SignSymbols *
BGenericNumberFormat::ExponentSignSymbols() const
{
	return (fExponentSignSymbols ? fExponentSignSymbols
								 : &kDefaultExponentSignSymbols);
}

// _SetSymbol
status_t
BGenericNumberFormat::_SetSymbol(Symbol **symbol, const char *str)
{
	if (!str) {
		// no symbol -- unset old
		if (*symbol) {
			delete *symbol;
			symbol = NULL;
		}
	} else {
		// allocate if not existing
		if (!*symbol) {
			*symbol = new(nothrow) Symbol;
			if (!*symbol)
				return B_NO_MEMORY;
		}
		// set symbol
		status_t error = (*symbol)->SetTo(str);
		if (error != B_OK) {
			delete *symbol;
			*symbol = NULL;
			return B_NO_MEMORY;
		}
	}
	return B_OK;
}

