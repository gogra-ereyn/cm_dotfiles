#include <stdarg.h>
#include <stdio.h>


void t_error(int status, int errnum, const char *format, ...)
{
	va_list args;
	va_start(args, format);

	vfprintf(stderr, format, args);
	if (errnum)
		fprintf(stderr, ": %s", strerror(errnum));

	fprintf(stderr, "\n");
	va_end(args);
	exit(status);
}
