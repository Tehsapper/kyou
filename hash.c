#include "hash.h"

#include "list.h"
#include <stdlib.h>
#include <string.h>

// http://xorshift.di.unimi.it/splitmix64.c
size_t uint64_hash(const void* a)
{
	uint64_t x = (uint64_t)a;
	x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
	x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
	x = x ^ (x >> 31);
	return x;
}

size_t djb2(const void* str)
{
	size_t hash = 5381;
	const char* s = str;
	int c;

	while ((c = *(s++)))
		hash = ((hash << 5) + hash) + c;

	return hash;
}

int string_equals(const void* a, const void* b)
{
	return strcmp((const char*)a, (const char*)b) == 0;
}

struct hash_table* hash_create(hash_fptr f, hash_comparator c, size_t size)
{
	struct hash_table* table = malloc(sizeof(struct hash_table));

	table->hash_func = f;
	table->comp_func = c;
	table->size = size;
	table->entries = 0;
	table->buckets = malloc(sizeof(struct hash_entry*) * size);
	
	for (size_t i = 0; i < size; ++i)
		table->buckets[i] = NULL;

	return table;
}

void hash_delete(struct hash_table* table)
{
	/* assuming that hash_entry is simillar to list */
	for (size_t i = 0; i < table->size; ++i)
		if(table->buckets[i] != NULL)
			list_delete_nodes((struct list_node*)(table->buckets[i]), false);

	free(table->buckets);
	free(table);
}

void hash_add(struct hash_table* table, const void* key, void* value)
{
	size_t index = table->hash_func(key) % table->size;

	struct hash_entry* entry = malloc(sizeof(struct hash_entry));
	entry->key = key;
	entry->value = value;
	entry->next = table->buckets[index];
	table->buckets[index] = entry;
	++table->entries;

	if (hash_load_factor(table) > 0.75f)
		hash_resize(table, 2*table->size);
}

void* hash_get(struct hash_table* table, const void* key)
{
	size_t index = table->hash_func(key) % table->size;
	for (struct hash_entry* e = table->buckets[index]; e != NULL; e = e->next) {
		if ((table->comp_func == NULL && e->key == key) || (table->comp_func && table->comp_func(e->key, key)))
			return e->value;
	}
	return NULL;
}

void hash_remove(struct hash_table* table, const void* key)
{
	size_t index = table->hash_func(key) % table->size;
	struct hash_entry* prev = NULL;

	for (struct hash_entry* e = table->buckets[index]; e; prev = e, e = e->next) {
		if (e->key == key) {
			if (prev) prev->next = e->next;
			else table->buckets[index] = e->next;
			--table->entries;
			free(e);
		}
	}
}

void hash_resize(struct hash_table* table, size_t new_size)
{
	struct hash_entry** new_buckets = calloc(new_size, sizeof(struct hash_entry*)); 
	for (size_t i = 0; i < table->size; ++i) {
		struct hash_entry* next = NULL;
		for (struct hash_entry* e = table->buckets[i]; e != NULL; e = next) {
			size_t index = table->hash_func(e->key) % new_size;
			next = e->next;
			e->next = new_buckets[index];
			new_buckets[index] = e;
		}
	}
	table->size = new_size;
	free(table->buckets);
	table->buckets = new_buckets;
}

float hash_load_factor(struct hash_table* table)
{
	return (float)table->entries / (float)table->size;
}
