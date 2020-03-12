// list.h

#ifndef _LIST_H
#define _LIST_H

#include <assert.h>

#include "mem.h"

#define list_push(list, size, value) do { \
	if (list == NULL) { list = list_init(sizeof(value), 1); list[0] = value; size = 1; break; } \
	void* new_list = mrealloc(list, size * sizeof(*list), (1 + size) * (sizeof(value))); \
	if (new_list) { \
		list = new_list; \
		list[size++] = value; \
	} \
} while (0); \

#define list_realloc(list, size, new_size) { \
	assert(list != NULL); \
	void* new_list = mrealloc(list, size * sizeof(*list), (new_size) * (sizeof(*list))); \
	if (new_list != NULL) { \
		list = new_list; \
		size = new_size; \
	} \
} \

#define list_shrink(list, size, num) { \
	assert(size - num >= 0); \
	list_realloc(list, size, size - num); \
}
 
#define list_assign(list, size, index, value) { \
	assert(list != NULL); \
	if (index < size) { \
		list[index] = value; \
	} else { assert(0); } \
} \

// size of type, count of elements to allocate
void* list_init(const unsigned int size, unsigned int count);

int list_free(void* list, unsigned int* size);

#endif