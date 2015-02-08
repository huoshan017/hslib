#ifndef __HS_ARRAY_H__
#define __HS_ARRAY_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

struct hs_array {
	void** data_ptr;
	size_t elem_num;
	size_t capacity;
};

struct hs_array*
hs_array_init(struct hs_array* array, size_t cap);

struct hs_array*
hs_array_create(size_t cap);

void
hs_array_destroy(struct hs_array* array);

size_t
hs_array_elem_num(struct hs_array* array) {
	if (!array) return 0;
	return array->elem_num;
}

size_t
hs_array_capacity(struct hs_array* array) {
	if (!array) return 0;
	return array->capacity;
}

bool
hs_array_push_elem(struct hs_array* array, void* data);

void*
hs_array_get_elem(struct hs_array* array, int index);

int
hs_array_find_first(struct hs_array* array, void* data);

bool
hs_array_delete_first(struct hs_array* array, void* data);

bool
hs_array_delete_byindex(struct hs_array* array, int index);

size_t
hs_array_delete_elems(struct hs_array* array, void* data);

void
hs_array_clear_elems(struct hs_array* array);



struct hs_array_iter {
	struct hs_array* array;
	size_t curr_index;
};

struct hs_array_iter
hs_array_iter_init(struct hs_array* array);

void*
hs_array_iter_data(struct hs_array_iter* iter);

bool
hs_array_iter_next(struct hs_array_iter* iter);

#endif
