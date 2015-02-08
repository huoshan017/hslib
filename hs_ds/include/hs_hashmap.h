#ifndef __HS_HASHMAP_H__
#define __HS_HASHMAP_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

struct hs_hashmap {
	void** data_ptr;
	size_t elem_num;
	size_t size;
};

struct hs_hashmap*
hs_hashmap_create(size_t s);

struct hs_hashmap*
hs_hashmap_init(struct hs_hashmap* hm, size_t s);

void
hs_hashmap_destroy(struct hs_hashmap* hm);

size_t
hs_hashmap_elem_num(struct hs_hashmap* hm) {
	if (!hm) return 0;
	return hm->elem_num;
}

size_t
hs_hashmap_size(struct hs_hashmap* hm) {
	if (!hm) return 0;
	return hm->size;
}

bool
hs_hashmap_insert(struct hs_hashmap* hm, int key, void* value);

bool
hs_hashmap_find(struct hs_hashmap* hm, int key, void** value);

bool
hs_hashmap_delete(struct hs_hashmap* hm, int key, void** value);


struct hs_hashmap_iter {
	struct hs_hashmap* hashmap;
	void* curr_data1;
	void* curr_data2;
};

struct hs_hashmap_iter
hs_hashmap_iter_init(struct hs_hashmap* hm);

bool
hs_hashmap_iter_data(struct hs_hashmap_iter* iter, void** data);

bool
hs_hashmap_iter_next(struct hs_hashmap_iter* iter);

#endif
