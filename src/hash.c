// hash.c
// Hash table implementation that uses open addressing

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "mem.h"
#include "hash.h"

#define UNUSED_SLOT 0
#define USED_SLOT 1

struct Item {
	Hkey key;
	Hvalue value;
	int used_slot;
};

static int is_empty(const Htable* table);
static unsigned int hash(const Hkey key, unsigned int tablesize);
static int linear_probe(const Htable* table, const Hkey key, unsigned int* collision_count);
static int key_compare(const Hkey a, const Hkey b);
static Htable resize_table(Htable* table, unsigned int new_size);

int is_empty(const Htable* table) {
	assert(table != NULL);
	return table->items == NULL;
}

unsigned int hash(const Hkey key, unsigned int tablesize) {
	unsigned int hash_number = 5381;
  unsigned int size = strlen(key);
  int c;

  for (unsigned int i = 0; i < size; i++) {
		c = key[i];
		hash_number = ((hash_number << 5) + hash_number) + c;
  }
  return hash_number % tablesize;
}

int linear_probe(const Htable* table, const Hkey key, unsigned int* collision_count) {
	int index = hash(key, ht_get_size(table));
	int counter = 0;
	for (; index < ht_get_size(table); index++, counter++) {
		if (key_compare(table->items[index].key, key) || table->items[index].used_slot == UNUSED_SLOT)
			return index;

		if (counter >= ht_get_size(table))
			return -1;

		if (index+1 >= ht_get_size(table)) {
			++(*collision_count);
			index = 0;
		}
		++(*collision_count);
	}
	return index;
}

int key_compare(const Hkey a, const Hkey b) {
	return strcmp(a, b) == 0;
}

Htable resize_table(Htable* table, unsigned int new_size) {
	assert(table != NULL);

	if (ht_num_elements(table) > new_size)
		return *table;

	Htable new_table = ht_create(new_size);
	if (ht_get_size(&new_table) != new_size)
		return *table;	// Allocation failed

	struct Item item;
	for (int i = 0; i < ht_get_size(table); i++) {
		item = table->items[i];
		if (item.used_slot != UNUSED_SLOT) {
			ht_insert_element(&new_table, item.key, item.value);
		}
	}
	ht_free(table);
	return new_table;
}

Htable ht_create(unsigned int size) {
	if (!size) size = 1;

	Htable table = {
		.items = mcalloc(sizeof(struct Item), size),
		.count = 0,
		.size = size
	};
	if (!table.items) // Allocation failed!
		table.size = 0;
	return table;
}

Htable ht_create_empty() {
	Htable table = {
		.items = NULL,
		.count = 0,
		.size = 0
	};
	return table;
}

unsigned int ht_insert_element(Htable* table, const Hkey key, const Hvalue value) {
	assert(table != NULL);
	if (is_empty(table)) {
		Htable new_table = ht_create(HASH_TABLE_INIT_SIZE);
		if (ht_get_size(&new_table) == HASH_TABLE_INIT_SIZE) {
			*table = new_table;
		}
		else return 0;
	}
	if (ht_num_elements(table) > (ht_get_size(table) / 2)) {
		*table = resize_table(table, table->size * 2);
	}

	unsigned int collisions = 0;
	int index = linear_probe(table, key, &collisions);
	if (index >= 0 && index < ht_get_size(table)) {
		struct Item item = { .value = value, .used_slot = USED_SLOT };
		strncpy(item.key, key, HTABLE_KEY_SIZE);
		table->items[index] = item;
		table->count++;
	}
	assert(ht_lookup(table, key) != NULL);
	return collisions;
}

const Hvalue* ht_lookup(const Htable* table, const Hkey key) {
	assert(table != NULL);
	if (ht_get_size(table) == 0) return NULL;
	unsigned int collisions = 0;
	int index = linear_probe(table, key, &collisions);
	if (index >= 0 && index < ht_get_size(table)) {
		struct Item* item = &table->items[index];
		if (item->used_slot == UNUSED_SLOT)
			return NULL;
		if (key_compare(item->key, key))
			return &item->value;
	}
	return NULL;
}

int ht_element_exists(const Htable* table, const Hkey key) {
	assert(table != NULL);
	return ht_lookup(table, key) != NULL;
}

void ht_remove_element(Htable* table, const Hkey key) {
	assert(table != NULL);
	if (ht_num_elements(table) < (ht_get_size(table) / 4)) {
		*table = resize_table(table, table->size / 2);
	}

	unsigned int collisions = 0;
	int index = linear_probe(table, key, &collisions);
	if (index >= 0 && index < ht_get_size(table)) {
		table->items[index].used_slot = UNUSED_SLOT;
		table->count--;

		struct Item item;
		for (; index < ht_get_size(table);) {
			if (index + 1 >= ht_get_size(table)) index = 0;

			item = table->items[++index];
			if (item.used_slot != UNUSED_SLOT) {
				table->items[index].used_slot = UNUSED_SLOT;
				table->count--;	// We are just moving this slot, not adding a new one, so decrement the count
				ht_insert_element(table, item.key, item.value);	// This function will increment count by 1
			}
			if (item.used_slot == UNUSED_SLOT)
				break;
		}
		assert(!ht_element_exists(table, key));
	}
}

unsigned int ht_get_size(const Htable* table) {
	assert(table != NULL);
	return table->size;
}

unsigned int ht_num_elements(const Htable* table) {
	assert(table != NULL);
	return table->count;
}

void ht_free(Htable* table) {
	assert(table != NULL);
	if (table->items) {
		mfree(table->items, table->size * sizeof(struct Item));
		table->items = NULL;
		table->size = 0;
		table->count = 0;
	}
}