#ifndef __HS_LIST_H__
#define __HS_LIST_H__

#include <stdlib.h>
#include <stdbool.h>

struct hs_list_node {
	void* data;
	struct hs_list_node* next;
	struct hs_list_node* prev;
};

struct hs_list {
	struct hs_list_node* head;
	struct hs_list_node* tail;
	size_t elem_num;
};

struct hs_list*
hs_list_init(struct hs_list* list);

struct hs_list*
hs_list_create();

void
hs_list_destroy(struct hs_list* list);

void*
hs_list_head_data(struct hs_list* list) {
	if (!list)
		return NULL;
	if (!list->head)
		return NULL;
	return list->head->data;
}

void*
hs_list_tail_data(struct hs_list* list) {
	if (!list)
		return NULL;
	if (!list->tail)
		return NULL;
	return list->tail->data;
}

size_t
hs_list_elem_num(struct hs_list* list) {
	if (!list)
		return 0;
	return list->elem_num;
}

void
hs_list_push_head(struct hs_list* list, void* data);

void
hs_list_push_tail(struct hs_list* list, void* data);

bool
hs_list_delete_first(struct hs_list* list, void* data);

size_t
hs_list_delete_elems(struct hs_list* list, void* data);

void
hs_list_clear_all(struct hs_list* list);


struct hs_list_iter {
	struct hs_list* list;
	struct hs_list_node* curr_node;
};

struct hs_list_iter
hs_list_iter_init(struct hs_list* list);

void*
hs_list_iter_data(struct hs_list_iter* iter);

bool
hs_list_iter_next(struct hs_list_iter* iter);

#endif
