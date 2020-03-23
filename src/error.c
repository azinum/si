// error.c

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"

struct Error_state {
	enum Error_codes status;
	int use_colors;
};

static struct Error_state err_state = {
	.status = NO_ERR,
	.use_colors = 1
};

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
	va_list args;
  va_start(args, format);
	vprintf(format, args);
  va_end(args);
}

int get_error() {
	return err_state.status;
}