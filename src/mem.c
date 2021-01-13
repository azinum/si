// mem.c
// Keep track of allocated memory

#include "mem.h"
#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#if defined(TRACK_MEMORY)

#define mem_info_update(add_total, add_count) \
info.alloc_total += add_total; \
info.alloc_count += add_count

struct Memory_info {
  int alloc_total;
  int alloc_count;
};

static struct Memory_info info = {
  .alloc_total = 0,
  .alloc_count = 0,
};

static void memory_info() {
	printf(
		"Memory info:\n"
		"  Memory allocated: %.2g KB (%i bytes)\n"
		"  Allocation count: %i\n",
		info.alloc_total / 1000.0f,
		info.alloc_total,
		info.alloc_count
	);
}

static int alloc_count() {
  return info.alloc_count;
}

static int alloc_total() {
  return info.alloc_total;
}

#else

#define mem_info_update(add_total, add_count)

#define memory_info()

#define alloc_count() 0

#define alloc_total() 0

#endif

void memory_print_info() {
  memory_info();
}

int memory_alloc_count() {
  return alloc_count();
}

int memory_alloc_total() {
  return alloc_total();
}

void* mmalloc(const unsigned int size) {
  void* pointer = malloc(size);
  if (!pointer)
    return NULL;
  mem_info_update(size, 1);
  return pointer;
}

void* mcalloc(const unsigned int size, const unsigned int count) {
  void* pointer = calloc(size, count);
  if (!pointer)
    return NULL;
  mem_info_update(size * count, 1);
  return pointer;
}

void* mrealloc(void* pointer, const unsigned int old_size, const unsigned int new_size) {
  int diff = new_size - old_size;
  (void)diff; // Silence warning (unused variable) when not tracking memory
  void* temp = realloc(pointer, new_size);
  if (!temp)
    return NULL;
  mem_info_update(diff, 0);
  return temp;
}

void mfree(void* pointer, const unsigned int size) {
  assert(pointer);
  if (!pointer)
    return;
  free(pointer);
  pointer = NULL;
  mem_info_update(-size, -1);
}
