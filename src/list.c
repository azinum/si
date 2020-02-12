// list.c

#include <stdlib.h>
#include <assert.h>

#include "error.h"
#include "list.h"

void* list_init(const unsigned int size, const unsigned int count) {
	void* tmp = calloc(size, count);
	if (!tmp) {
		error("Failed to allocate memory for list\n");
		return NULL;
	}
	return tmp;
}

int list_push(void* list, const unsigned int size, unsigned int* count) {
	assert(list != NULL);
	
	return NO_ERR;
}