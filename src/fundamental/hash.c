#include "structs.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Hash *hash_new() {
  Hash* table = malloc(sizeof(Hash));
  table->length = 0;
  table->capacity = 256;
  table->entries = calloc(table->capacity, sizeof(hash_entry));
  return table;
}

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

/* https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function */
static unsigned long hash_key(const char* key) {
  unsigned long hash = FNV_OFFSET;
  for (const char* p = key; *p; p++) {
    hash ^= (unsigned long)(unsigned char)(*p);
    hash *= FNV_PRIME;
  }
  return hash;
}

#undef FNV_OFFSET
#undef FNV_PRIME

void *hash_get(Hash *h, const char* k) {
  unsigned long hash = hash_key(k);
  int index = (int)(hash % (unsigned long)(h->capacity));

  while (h->entries[index].key != NULL) {
    if (!strcmp(k, h->entries[index].key)) {
      return h->entries[index].value;
    }
    /* 键不在这个位置，往后搜索 */
    index++;
    if (index >= h->capacity) {
      index = 0;
    }
  }
  return NULL;
}

static void hash_setentry(hash_entry *entries, int capacity, const char *k,
                          void *v, int *length)
{
  unsigned long hash = hash_key(k);
  int index = (int)(hash % (long)(capacity));

  while (entries[index].key) {
    if (!strcmp(k, entries[index].key)) {
      entries[index].value = v;
      return;
    }

    index++;
    if (index >= capacity) {
      index = 0;
    }
  }

  if (length != NULL) {
    k = strdup(k);
    (*length)++;
  }
  entries[index].key = k;
  entries[index].value = v;
}

static void hash_expand(Hash *h) {
  int new_capacity = h->capacity * 2;
  if (new_capacity < h->capacity) {
    fprintf(stderr, "%s: integer overflow\n", __FUNCTION__);
    exit(-1);
  }
  hash_entry *new_entries = calloc(new_capacity, sizeof(hash_entry));
  if (!new_entries) {
    fprintf(stderr, "%s: no memory\n", __FUNCTION__);
    exit(-1);
  }

  /* 转移数据, 需要考虑到哈希值改变 */
  for (int i = 0; i < h->capacity; i++) {
    hash_entry e = h->entries[i];
    if (e.key != NULL) {
      hash_setentry(new_entries, new_capacity, e.key, e.value, NULL);
    }
  }

  free(h->entries);
  h->entries = new_entries;
  h->capacity = new_capacity;
}

void hash_set(Hash *h, const char* k, void *v) {
  if (!v) return;

  /* 由于搜索的时候是循环往复搜索，所以一定要确保空位足够 */
  if (h->length >= h->capacity / 2) {
    hash_expand(h);
  }

  hash_setentry(h->entries, h->capacity, k, v, &h->length);
}

void hash_free(Hash *h) {
  for (size_t i = 0; i < h->capacity; i++) {
    free((void*)h->entries[i].key);
  }

  free(h->entries);
  free(h);
}
