// list.h

#ifndef _LIST_H
#define _LIST_H

// size of type, count of elements to allocate
void* list_init(const unsigned int size, const unsigned int count);

int list_push(void* list, const unsigned int size, unsigned int* count);

#endif