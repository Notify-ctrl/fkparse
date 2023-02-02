#include "structs.h"
#include "object.h"
#include <stdlib.h>
#include "error.h"

Hash *builtin_symtab = NULL;
Hash *global_symtab = NULL;
Hash *current_tab = NULL;
Hash *last_lookup_tab = NULL;
Stack *symtab_stack = NULL;

symtab_item *sym_lookup(const char *k) {
  void *v = NULL;
  Stack *node;
  last_lookup_tab = NULL;
  list_foreach(node, symtab_stack) {
    Hash *h = cast(Hash *, node->data);
    v = hash_get(h, k);
    if (v) {
      last_lookup_tab = h;
      break;
    }
  }
  return cast(symtab_item *, v);
}

void sym_set(const char *k, symtab_item *v) {
  symtab_item *i = sym_lookup(k);
  if (i && i->reserved) {
    fprintf(error_output, "错误：不能修改预定义的标识符 %s", k);
  } else {
    if (i) checktype(NULL, i->type, v->type);
    hash_set(last_lookup_tab ? last_lookup_tab : current_tab, k, (void *)v);
  }
}

void sym_new_entry(const char *k, int type, const char *origtext, bool reserved) {
  symtab_item *i = sym_lookup(k);
  symtab_item *v;
  if (i) {
    if (i->type != TNone)
      fprintf(error_output, "错误：%s 已经存在于表中，类型为 %d", k, i->type);
    else {
      i->type = type;
      i->origtext = origtext;
      i->funcdef = NULL;
      i->reserved = reserved;
    }
  } else {
    v = malloc(sizeof(symtab_item));
    v->type = type;
    if (origtext)
      v->origtext = strdup(origtext);
    v->funcdef = NULL;
    v->reserved = reserved;
    sym_set(k, cast(void *, v));
  }
}

void sym_free(Hash *h) {
  for (size_t i = 0; i < h->capacity; i++) {
    if (h->entries[i].key) {
      free((void*)h->entries[i].key);
      symtab_item *item = h->entries[i].value;
      if (item->origtext) {
        free((void *)item->origtext);
      } else if (h == builtin_symtab) {
        freeFuncdef(item->funcdef);
      }
      free(item);
    }
  }

  free(h->entries);
  free(h);
}

