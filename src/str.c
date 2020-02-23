// str.c

#include <assert.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

#include "str.h"

char* string_new_copy(char* old, int length) {
	int new_length = length + 1;	/* +1 for null terminator */
	char* new_string = malloc(sizeof(char) * new_length);
	if (!new_string) return NULL;
	strncpy(new_string, old, new_length);
	new_string[new_length - 1] = '\0';
	return new_string;
}

void string_to_number(char* string, obj_number* number) {
	assert(number != NULL);
	char* end;
  *number = strtod(string, &end);
  if (*end != '\0') assert(0);
}

void string_free(char* string) {
	assert(string != NULL);
	free(string);
}