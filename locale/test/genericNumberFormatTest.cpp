/* 
** Copyright 2004, Ingo Weinhold, bonefish@cs.tu-berlin.de. All rights reserved.
** Distributed under the terms of the OpenBeOS License.
*/

#include <stdio.h>
#include <string.h>

#include <GenericNumberFormat.h>
#include <IntegerFormatParameters.h>

void
format_number(const char *test, const BGenericNumberFormat &format,
			  const BIntegerFormatParameters *parameters, int64 number)
{
	const int bufferSize = 1024;
	char buffer[bufferSize];
	status_t error = format.FormatInteger(parameters, number, buffer,
		bufferSize);
	if (error == B_OK)
		printf("%-20s `%s'\n", test, buffer);
	else
		printf("%-20s Failed to format number: %s\n", test, strerror(error));
}

void
format_number(BGenericNumberFormat &format, int64 number)
{
	printf("number: %lld\n", number);
	BIntegerFormatParameters parameters(
		format.DefaultIntegerFormatParameters());
	format_number("  default:", format, &parameters, number);
	parameters.SetFormatWidth(25);
	format_number("  right aligned:", format, &parameters, number);
	parameters.SetAlignment(B_ALIGN_FORMAT_LEFT);
	format_number("  left aligned:", format, &parameters, number);
	parameters.SetFormatWidth(1);
	parameters.SetAlignment(B_ALIGN_FORMAT_RIGHT);
	parameters.SetSignPolicy(B_USE_POSITIVE_SIGN);
	format_number("  use plus:", format, &parameters, number);
	parameters.SetSignPolicy(B_USE_SPACE_FOR_POSITIVE_SIGN);
	format_number("  space for plus:", format, &parameters, number);
	parameters.SetSignPolicy(B_USE_NEGATIVE_SIGN_ONLY);
	parameters.SetMinimalIntegerDigits(0);
	format_number("  min digits 0:", format, &parameters, number);
	parameters.SetMinimalIntegerDigits(5);
	format_number("  min digits 5:", format, &parameters, number);
	parameters.SetMinimalIntegerDigits(20);
	format_number("  min digits 20:", format, &parameters, number);
	parameters.SetMinimalIntegerDigits(30);
	format_number("  min digits 30:", format, &parameters, number);
}


int
main()
{
	BGenericNumberFormat format;
	format_number(format, 0);
	format_number(format, 1234567890LL);
	format_number(format, -1234567890LL);
	format.DefaultIntegerFormatParameters()->SetUseGrouping(true);
	format_number(format, 0);
	format_number(format, 1234567890LL);
	format_number(format, -1234567890LL);
	return 0;
}

