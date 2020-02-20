// list.h

#ifndef _LIST_H
#define _LIST_H

#include <assert.h>
#include <stdlib.h>

#define list_push(list, size, value) { \
	assert(list != NULL); \
	typeof(*list)* new_list = realloc(list, (1 + size) * (sizeof(*list))); \
	if (new_list) { \
		list = new_list; \
		list[size++] = value; \
	} \
} \

#define list_assign(list, size, index, value) { \
	assert(list != NULL); \
	if (index < size) { \
		list[index] = value; \
	} \
} \

// size of type, count of elements to allocate
int list_init(void* list, const unsigned int size, const unsigned int count);


int list_free(void* list, unsigned int* size);

#endif