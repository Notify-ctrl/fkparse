#include "structs.h"
#include "main.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static unsigned prime_table[29] = {
  13, 31, 53, 97, 193, 389, 769, 1543, 3079, 6151, 12289,
  24593, 49157, 98317, 196613, 393241, 786433, 1572869, 3145739,
  6291469, 12582917, 25165843, 50331653, 100663319, 201326611,
  402653189, 805306457, 1610612741, 0
};

Hash *hash_new() {
  Hash* table = malloc(sizeof(Hash));
  table->length = 0;
  table->capacity_level = 0;
  table->capacity = prime_table[table->capacity_level];
  table->entries = calloc(table->capacity, sizeof(hash_entry));
  return table;
}

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

/* https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function */
static unsigned hash_key(const char* key) {
  size_t hash = FNV_OFFSET;
  for (const char* p = key; *p; p++) {
    hash ^= (size_t)(unsigned char)(*p);
    hash *= FNV_PRIME;
  }
  return (unsigned)hash;
}

#undef FNV_OFFSET
#undef FNV_PRIME

void *hash_get(Hash *h, const char* k) {
  unsigned hash = hash_key(k);
  unsigned index = hash % h->capacity;

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

static void hash_setentry(hash_entry *entries, unsigned capacity, const char *k,
                          void *v, unsigned *length)
{
  unsigned hash = hash_key(k);
  unsigned index = hash % capacity;

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
  unsigned new_capacity = prime_table[h->capacity_level + 1];
  if (new_capacity < h->capacity) {
    fprintf(error_output, "%s: hash capacity overflow\n", __FUNCTION__);
    exit(-1);
  }
  hash_entry *new_entries = calloc(new_capacity, sizeof(hash_entry));
  if (!new_entries) {
    fprintf(error_output, "%s: no memory\n", __FUNCTION__);
    exit(-1);
  }

  /* 转移数据, 需要考虑到哈希值改变 */
  for (unsigned i = 0; i < h->capacity; i++) {
    hash_entry e = h->entries[i];
    if (e.key != NULL) {
      hash_setentry(new_entries, new_capacity, e.key, e.value, NULL);
    }
  }

  free(h->entries);
  h->entries = new_entries;
  h->capacity = new_capacity;
  h->capacity_level++;
}

void hash_set(Hash *h, const char* k, void *v) {
  if (!v) return;

  /* 由于搜索的时候是循环往复搜索，所以一定要确保空位足够 */
  if (h->length >= h->capacity / 2) {
    hash_expand(h);
  }

  hash_setentry(h->entries, h->capacity, k, v, &h->length);
}

void hash_copy(Hash *dst, Hash *src) {
  for (int i = 0; i < src->capacity; i++) {
    if (src->entries[i].key != NULL) {
      hash_set(dst, src->entries[i].key, src->entries[i].value);
    }
  }
}

void hash_copy_all(Hash *dst, Hash *src) {
  for (int i = 0; i < src->capacity; i++) {
    if (src->entries[i].key != NULL) {
      hash_set(dst, src->entries[i].key, strdup(src->entries[i].value));
    }
  }
}

void hash_free(Hash *h, void (*freefunc)(void *)) {
  for (size_t i = 0; i < h->capacity; i++) {
    free((void *)h->entries[i].key);
    if (freefunc && h->entries[i].value)
      freefunc(h->entries[i].value);
  }

  free(h->entries);
  free(h);
}
