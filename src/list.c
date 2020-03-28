// list.c

#include <stdlib.h>
#include <assert.h>

#include "error.h"
#include "list.h"

void* list_init(const unsigned int size, unsigned int count) {
	void* list = mcalloc(size, count);
	if (!list) {
		error("Failed to allocate memory for list\n");
		return NULL;
	}
	return list;
}