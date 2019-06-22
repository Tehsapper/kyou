#pragma once

#include <stddef.h>
#include <stdint.h>

typedef size_t (*hash_fptr) (const void*);
typedef int (*hash_comparator) (const void*, const void*);

struct hash_entry
{
	struct hash_entry* next;
	const void* key;
	void* value;
};

struct hash_table
{
	struct hash_entry** buckets;
	hash_fptr hash_func;
	hash_comparator comp_func;
	size_t size, entries;
};

struct hash_table* hash_create(hash_fptr f, hash_comparator c, size_t size);
void hash_delete(struct hash_table* table);

void hash_add(struct hash_table* table, const void* key, void* value);
void* hash_get(struct hash_table* table, const void* key);
void hash_remove(struct hash_table* table, const void* key);
void hash_resize(struct hash_table* table, size_t new_size);

float hash_load_factor(struct hash_table* table);

size_t djb2(const void*);
size_t uint64_hash(const void*);

int string_equals(const void* a, const void* b);
