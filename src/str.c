// str.c

#include <assert.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

#include "mem.h"
#include "str.h"

char* string_new_copy(char* old, int length) {
	int new_length = length + 1;	/* +1 for null terminator */
	char* new_string = mmalloc(sizeof(char) * new_length);
	if (!new_string) return NULL;
	strncpy(new_string, old, new_length);
	new_string[new_length - 1] = '\0';
	return new_string;
}

void string_to_number(char* string, double* number) {
	assert(number != NULL);
	char* end;
  *number = strtod(string, &end);
  if (*end != '\0')
		(void)0;	// String -> number failed, ignore for now
}

void string_free(char* string) {
	assert(string != NULL);
	unsigned int length = strlen(string) + 1;
	mfree(string, length * sizeof(char));
}