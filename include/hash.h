// hash.h

#ifndef _HASH_H
#define _HASH_H

typedef int Hvalue;
#define HTABLE_KEY_SIZE 32 - sizeof(Hvalue)
typedef char Hkey[HTABLE_KEY_SIZE];

struct Item;

typedef struct {
	struct Item* items;
	unsigned int count;	// Count of used slots
	unsigned int size;	// Total size of the hash table
} Htable;

// Important!
// It's recommended that the size is a prime number
// This will result in less key collisions
Htable ht_create(unsigned int size);

Htable ht_create_empty();

int ht_is_empty(const Htable* table);

unsigned int ht_insert_element(Htable* table, const Hkey key, const Hvalue value);

const Hvalue* ht_lookup(const Htable* table, const Hkey key);

const Hvalue* ht_lookup_byindex(const Htable* table, const unsigned int index);

const Hkey* ht_lookup_key(const Htable* table, const unsigned int index);

int ht_element_exists(const Htable* table, const Hkey key);

void ht_remove_element(Htable* table, const Hkey key);

unsigned int ht_get_size(const Htable* table);

unsigned int ht_num_elements(const Htable* table);

void ht_free(Htable* table);

#endif