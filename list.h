#pragma once

#include <stddef.h>
#include <stdbool.h>

struct list_node
{
	struct list_node* next;
	void* data;
};

struct list
{
	struct list_node* first;
	struct list_node* last;
};

#define LIST_EMPTY (struct list) { .first = NULL, .last = NULL }

void list_append(struct list* l, void* data);
size_t list_size(struct list* l);
void* list_at(struct list* l, size_t i);
void list_delete(struct list* l, void* who, bool free_data);
void list_delete_nodes(struct list_node* n, bool free_data);
void list_flush(struct list* l, bool free_data);
struct list_node* list_find(struct list* l, void* data);
