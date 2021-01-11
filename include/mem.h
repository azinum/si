// mem.h

#ifndef _MEM_H
#define _MEM_H

void memory_print_info();

int memory_alloc_count();

int memory_alloc_total();

void* mmalloc(const unsigned int size);

void* mcalloc(const unsigned int size, const unsigned int count);

void* mrealloc(void* pointer, const unsigned int old_size, const unsigned int new_size);

void mfree(void* pointer, const unsigned int size);

#endif
