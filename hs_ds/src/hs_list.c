#include "hs_list.h"

struct hs_list*
hs_list_init(struct hs_list* list)
{
	if (!list)
		return NULL;

	list->head = list->tail = NULL;
	list->elem_num = 0;
	return list;
}

struct hs_list*
hs_list_create()
{
	struct hs_list* list = (struct hs_list*)malloc(sizeof(struct hs_list));
	return hs_list_init(list);
}

void
hs_list_destroy(struct hs_list* list)
{
	if (!list)
		return;

	struct hs_list_node* t = NULL;
	struct hs_list_node* n = list->head;
	while (n) {
		t = n->next;
		free(n);
		n = t;
	}
}

void
hs_list_push_head(struct hs_list* list, void* data)
{
	if (!list)
		return;

	struct hs_list_node* n = (struct hs_list_node*)malloc(sizeof(struct hs_list_node));
	n->data = data;

	if (!list->head) {
		list->head = list->tail = n;
		n->next = NULL;
	} else {
		n->next = list->head;
		list->head = n->next;
	}
}

void
hs_list_push_tail(struct hs_list* list, void* data)
{
	if (!list)
		return;

	struct hs_list_node* n = (struct hs_list_node*)malloc(sizeof(struct hs_list_node));
	n->data = data;
	n->next = NULL;
	if (!list->tail) {
		list->head = n;
	} else {
		list->tail->next = n;
	}
	list->tail = n;
}

bool
hs_list_delete_first(struct hs_list* list, void* data)
{
	if (!list)
		return false;

	if (!list->head)
		return false;

	struct hs_list_node* n = NULL;
	if (list->head->data == data) {
		n = list->head;
		if (list->head == list->tail) {
			list->head = list->tail = NULL;
		} else {
			list->head = list->head->next;
		}
	}

	n = list->head;
	while (n->next) {
		if (n->next->data == data) {
			n = n->next;
			n->next = n->next->next;
			break;
		}
	}

	free(n);

	list->elem_num -= 1;

	return true;
}

size_t
hs_list_delete_elems(struct hs_list* list, void* data)
{
	if (!list)
		return 0;

	size_t s = 0;
	struct hs_list_node* p = NULL;
	struct hs_list_node* n = list->head;
	while (n) {
		if (n->data == data) {
			if (n == list->head) {
				if (list->head != list->tail)
					list->head = n->next;
				else
					list->head = list->tail = NULL;
			} else {
				p->next = n->next;
				if (n == list->tail)
					list->tail = p;
			}
			free(n);
			list->elem_num -= 1;
			s += 1;
		}
		p = n;
		n = n->next;
	}

	return s;
}

void
hs_list_clear_all(struct hs_list* list)
{
	if (!list)
		return;

	struct hs_list_node* nn = NULL;
	struct hs_list_node* n = list->head;
	while (n) {
		nn = n->next;
		free(n);
		n = nn;
	}
	list->head = list->tail = NULL;
	list->elem_num = 0;
}


struct hs_list_iter
hs_list_iter_init(struct hs_list* list)
{
	struct hs_list_iter iter;
	iter.list = list;
	if (!list)
		iter.curr_node = NULL;
	else
		iter.curr_node = list->head;
	return iter;
}

void*
hs_list_iter_data(struct hs_list_iter* iter)
{
	if (!iter)
		return NULL;

	if (!iter->curr_node)
		return NULL;

	return iter->curr_node->data;
}

bool
hs_list_iter_next(struct hs_list_iter* iter)
{
	if (!iter)
		return false;

	if (!iter->curr_node)
		return false;

	if (!iter->curr_node->next)
		return false;

	iter->curr_node = iter->curr_node->next;

	return true;
}
