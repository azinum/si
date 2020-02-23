// list.c

#include <stdlib.h>
#include <assert.h>

#include "error.h"
#include "list.h"

void* list_init(const unsigned int size, unsigned int count) {
	void* list = calloc(size, count);
	if (!list) {
		error("Failed to allocate memory for list\n");
		return NULL;
	}
	return list;
}

int list_free(void* list, unsigned int* size) {
	if (list != NULL) {
		free(list);
		*size = 0;
		return NO_ERR;
	}
	return ERR;
}