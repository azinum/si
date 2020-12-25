// error.h

#ifndef _ERROR_H
#define _ERROR_H

enum Error_codes {
	NO_ERR = 0,
	ERR,
	PARSE_ERR,
	LEX_ERR,
	ALLOC_ERR,
	REALLOC_ERR,
	COMPILE_ERR,
	STACK_ERR,
	RUNTIME_ERR,

  WARN,
};

void error_init(int show_warnings);

int is_error();

void error(const char* format, ...);

void warn(const char* format, ...);

int get_error();

#endif
