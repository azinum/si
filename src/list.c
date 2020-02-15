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

int list_free(void* list, unsigned int* size) {
	if (list != NULL) {
		free(list);
		*size = 0;
		return NO_ERR;
	}
	return ERR;
}