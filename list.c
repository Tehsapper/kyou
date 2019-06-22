#include "list.h"

#include <stdlib.h>

void list_append(struct list* l, void* data)
{
	struct list_node* n = malloc(sizeof(struct list_node));

	n->next = NULL;
	n->data = data;

	if (l->last) l->last->next = n;
	else l->first = n;

	l->last = n;
}

size_t list_size(struct list* l)
{
	size_t i = 0;
	for (struct list_node *n = l->first; n; n = n->next)
		i++;
	return i;
}

void* list_at(struct list* l, size_t a)
{
	// if it goes out of bounds = oops
	// returning NULL is bad because it can be a valid list node value
	// TODO: maybe return list_node instead ?
	struct list_node *n = l->first;
	for (size_t i = 0; i < a; ++i) {
		n = n->next;
	}

	return n->data;
}

void list_delete(struct list* l, void* who, bool free_data)
{
	struct list_node *prev = NULL;
	for (struct list_node *n = l->first; n; prev = n, n = n->next) {
		if (n->data == who) {
			if (prev != NULL) {
				prev->next = n->next;
				if (prev->next == NULL) l->last = prev;
			} else {
				l->first = n->next;
				if (n->next == NULL) l->last = NULL;
			}

			if (free_data) free(n->data);
			free(n);

			return;
		}
	}
}

void list_delete_nodes(struct list_node* n, bool free_data)
{
	if (n == NULL) return;

	list_delete_nodes(n->next, free_data);
	if (free_data) free(n->data);
	free(n);
}

void list_flush(struct list* l, bool free_data)
{
	list_delete_nodes(l->first, free_data);

	l->first = NULL;
	l->last = NULL;
}

struct list_node* list_find(struct list* l, void* data)
{
	for (struct list_node *n = l->first; n; n = n->next)
		if (n->data == data)
			return n;
	return NULL;
}
