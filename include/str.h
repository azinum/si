// str.h

#ifndef _STR_H
#define _STR_H

#include "config.h"

char* string_new_copy(char* old, int length);

void string_to_number(char* string, double* number);

void string_free(char* string);

#endif