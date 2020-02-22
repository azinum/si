// list.c

#include <stdlib.h>
#include <assert.h>

#include "error.h"
#include "list.h"

int list_init(void* list, const unsigned int size, unsigned int* count) {
	void* tmp = calloc(size, *count);
	if (!tmp) {
		error("Failed to allocate memory for list\n");
		*count = 0;
		return ALLOC_ERR;
	}
	list = tmp;
	return NO_ERR;
}

int list_free(void* list, unsigned int* size) {
	if (list != NULL) {
		free(list);
		*size = 0;
		return NO_ERR;
	}
	return ERR;
}