#include "hs_array.h"
#include <memory.h>

struct hs_array*
hs_array_init(struct hs_array* array, size_t cap)
{
	if (!array)
		return NULL;

	array->data_ptr = (void**)malloc(sizeof(void*)*cap);
	array->elem_num = 0;
	array->capacity = cap;
	return array;
}

struct hs_array*
hs_array_create(size_t cap)
{
	struct hs_array* array = (struct hs_array*)malloc(sizeof(struct hs_array));
	array->data_ptr = (void**)malloc(sizeof(void*)*cap);
	array->elem_num = 0;
	array->capacity = cap;
	return array;
}

void
hs_array_destroy(struct hs_array* array)
{
	if (array) {
		free((void*)array->data_ptr);
		free(array);
	}
}

static void
_grow_capacity(struct hs_array* array) {
	size_t new_cap = 2 * array->capacity;
	void* new_data = malloc(new_cap*sizeof(void*));
	if (array->elem_num)
		memcpy(new_data, array->data_ptr, array->elem_num*sizeof(void*));
	free((void*)array->data_ptr);
	array->data_ptr = (void**)new_data;
	array->capacity = new_cap;
}

bool
hs_array_push_elem(struct hs_array* array, void* data)
{
	if (!array)
		return false;

	if (array->capacity <= array->elem_num) {
		_grow_capacity(array);
	}

	array->data_ptr[array->elem_num] = data;
	array->elem_num += 1;

	return true;
}

void*
hs_array_get_elem(struct hs_array* array, int index)
{
	if (!array)
		return NULL;

	if (index<0 || index>=array->elem_num)
		return NULL;

	return array->data_ptr[index];
}

int
hs_array_find_first(struct hs_array* array, void* data)
{
	if (!array)
		return -1;

	int index = -1;
	size_t i = 0;
	for (; i<array->elem_num; ++i) {
		if (array->data_ptr[i] == data) {
			index = (int)i;
			break;
		}
	}
	return index;
}

bool
hs_array_delete_first(struct hs_array* array, void* data)
{
	if (!array)
		return false;

	int index = -1;
	size_t i = 0;
	for (; i<array->elem_num; ++i) {
		if (array->data_ptr[i] == data) {
			index = (int)i;
			break;
		}
	}

	if (index < 0)
		return false;

	if (index < array->elem_num-1) {
		memmove(&array->data_ptr[index], &array->data_ptr[index+1], (array->elem_num-index-1)*sizeof(void*));
	}
	
	array->elem_num -= 1;

	return true;
}

bool
hs_array_delete_byindex(struct hs_array* array, int index)
{
	if (!array)
		return false;

	if (array->elem_num == 0)
		return false;

	if (index > array->elem_num-1)
		return false;

	if (index < array->elem_num-1)
		memmove(&array->data_ptr[index], &array->data_ptr[index+1], (array->elem_num-index-1)*sizeof(void*));

	array->elem_num -= 1;

	return true;
}

size_t
hs_array_delete_elems(struct hs_array* array, void* data)
{
	if (!array)
		return 0;

	size_t elems = 0;
	size_t i = 0;
	for (; i<array->elem_num; ) {
		if (array->data_ptr[i] == data) {
			memmove(&array->data_ptr[i], &array->data_ptr[i+1], (array->elem_num-i-1)*sizeof(void*));
			array->elem_num -= 1;
			elems += 1;
		} else {
			i += 1;
		}
	}
	return elems;
}

void
hs_array_clear_elems(struct hs_array* array)
{
	if (!array)
		return;

	array->elem_num = 0;
}

struct hs_array_iter
hs_array_iter_init(struct hs_array* array)
{
	struct hs_array_iter iter;
	if (!array) {
		iter.array = NULL;
	} else {
		iter.array = array;
	}
	iter.curr_index = 0;
	return iter;
}

void*
hs_array_iter_data(struct hs_array_iter* iter)
{
	if (!iter)
		return NULL;

	if (!iter->array)
		return NULL;

	if (!iter->array->data_ptr)
		return NULL;

	if (iter->array->elem_num <= iter->curr_index)
		return NULL;

	return iter->array->data_ptr[iter->curr_index];
}

bool
hs_array_iter_next(struct hs_array_iter* iter)
{
	if (!iter)
		return false;

	if (!iter->array)
		return false;

	if (iter->array->elem_num-1 <= iter->curr_index)
		return false;

	iter->curr_index += 1;

	return true;
}
