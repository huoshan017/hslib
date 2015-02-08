#include "hs_hashmap.h"

struct _key_value_pair {
	bool valid;
	int key;
	void* value;
};

static const int s_min_hash_size = 128;

static const int s_bucket_size = 8;

struct hs_hashmap*
hs_hashmap_init(struct hs_hashmap* hm, size_t s)
{
	if (!s) return NULL;
	int sh = (s-1) / s_min_hash_size;
	size_t size = s_min_hash_size * (sh+1);
	hm->data_ptr = (void**)malloc(sizeof(void*)*size);
	hm->size = size;
	hm->elem_num = 0;
	return hm;
}

struct hs_hashmap*
hs_hashmap_create(size_t s)
{
	if (!s) return NULL;
	struct hs_hashmap* h = (struct hs_hashmap*)malloc(sizeof(struct hs_hashmap));
	return hs_hashmap_init(h, s);
}

void
hs_hashmap_destroy(struct hs_hashmap* hm)
{
	if (!hm) return;
	size_t i = 0;
	for (; i<hm->size; ++i) {
		void* p = hm->data_ptr[i];
		if (!p) continue;
		free((void*)p);
	}
	free((void*)hm->data_ptr);
}

static void
_rehash(struct hs_hashmap** phm) {
	struct hs_hashmap* hm = *phm;
	size_t new_size = 2 * hm->size;
	struct hs_hashmap* new_map = hs_hashmap_create(new_size);

	// 遍历所有节点
	size_t i = 0;
	for (; i<hm->size; ++i) {
		void* p = hm->data_ptr[i];
		if (!p) continue;
		uint8_t j = 0;
		for (; j<s_bucket_size; ++j) {
			struct _key_value_pair* kvp = &((struct _key_value_pair*)p)[j];
			if (kvp->valid) {
				hs_hashmap_insert(new_map, kvp->key, kvp->value);
			}
		}
	}

	// 销毁
	hs_hashmap_destroy(*phm);
	*phm = new_map;
}

bool
hs_hashmap_insert(struct hs_hashmap* hm, int key, void* value)
{
	if (!hm) return false;

	int h = key % hm->size;
	void* p = hm->data_ptr[h];
	if (!p) {
		p = malloc(s_bucket_size*sizeof(struct _key_value_pair));
		size_t i = 0;
		for (; i<s_bucket_size; ++i) {
			((struct _key_value_pair*)p)[i].valid = false;
		}
		hm->data_ptr[h] = p;
	}

	struct _key_value_pair* kvp = (struct _key_value_pair*)p;
	size_t i = 0;
	for (; i<s_bucket_size; ++i) {
		if (kvp[i].valid && kvp[i].key == key)
			return false;

		if (!kvp[i].valid) {
			kvp[i].key = key;
			kvp[i].value = value;
			kvp[i].valid = true;
			break;
		}
	}

	if (i == s_bucket_size) {
		_rehash(&hm);
		hs_hashmap_insert(hm, key, value);
	}

	hm->elem_num += 1;

	return true;
}

struct _key_value_pair*
_hashmap_get(struct hs_hashmap* hm, int key) {
	int h = key % hm->size;
	struct _key_value_pair* kvp = (struct _key_value_pair*)hm->data_ptr[h];
	if (!kvp) return NULL;

	size_t i = 0;
	for (; i<s_bucket_size; ++i) {
		if (kvp[i].valid) {
			if (kvp[i].key == key) {
				break;
			}
		}
	}

	if (i == s_bucket_size)
		return NULL;

	return &kvp[i];
}

bool
hs_hashmap_find(struct hs_hashmap* hm, int key, void** value)
{
	if (!hm) return false;

	struct _key_value_pair* kvp = _hashmap_get(hm, key);
	if (!kvp) return false;
	if (value) *value = kvp->value;

	return true;
}

bool
hs_hashmap_delete(struct hs_hashmap* hm, int key, void** value)
{
	if (!hm) return false;

	struct _key_value_pair* kvp = _hashmap_get(hm, key);
	if (!kvp) return false;
	kvp->valid = false;

	return true;
}


struct hs_hashmap_iter
hs_hashmap_iter_init(struct hs_hashmap* hm)
{
	struct hs_hashmap_iter iter;
	iter.hashmap = hm;
	iter.curr_data1 = iter.curr_data2 = NULL;
	return iter;
}

bool
hs_hashmap_iter_data(struct hs_hashmap_iter* iter, void** value)
{
	if (!iter) return false;
	if (!iter->hashmap) return false;
	if (!iter->hashmap->data_ptr) return false;
	size_t i = (size_t)iter->curr_data1;
	struct _key_value_pair* p = (struct _key_value_pair*)(iter->hashmap->data_ptr[i]);
	if (!p) return false;
	size_t pi = (size_t)iter->curr_data2;
	if (!p[pi].valid) return false;
	if (value) *value = p[pi].value;
	return true;
}

bool
hs_hashmap_iter_next(struct hs_hashmap_iter* iter)
{
	if (!iter) return false;
	if (!iter->hashmap) return false;

	struct _key_value_pair** kvp = (struct _key_value_pair**)iter->hashmap->data_ptr;
	if (!kvp) return false;

	size_t i = (size_t)iter->curr_data1;
	size_t j = (size_t)iter->curr_data2;
	while (1) {
		if (j >= s_bucket_size-1) {
			if (i >= iter->hashmap->size-1)
				return false;

			i += 1;
			j = 0;
			if (!kvp[i])
				continue;
		} else {
			j += 1;
		}

		if (kvp[i][j].valid)
			break;
	}

	return true;
}
