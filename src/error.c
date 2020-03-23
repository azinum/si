// error.c

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"

struct Error_state {
	enum Error_codes status;
	int show_warnings;
};

static struct Error_state err_state = {
	.status = NO_ERR,
	.show_warnings = 1,
};

void error_init(int show_warnings) {
	err_state.status = NO_ERR;
	err_state.show_warnings = show_warnings;
}

int is_error() {
	return err_state.status != NO_ERR;
}

void error(const char* format, ...) {
	va_list args;
	va_start(args, format);

	vprintf(format, args);
	va_end(args);

	err_state.status = ERR;
}

void warn(const char* format, ...) {
	if (!err_state.show_warnings)
		return;
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

int get_error() {
	return err_state.status;
}