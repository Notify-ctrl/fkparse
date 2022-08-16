#include "structs.h"
#include <stdlib.h>

List *list_new() {
  List *l = malloc(sizeof(List));
  l->next = NULL;
  l->data = NULL;
  return l;
}

void list_append(List *l, Object *o) {
  List *node, *p = NULL;
  list_foreach(node, l)
    p = node;
  if (!p) p = l;
  List *new_node = malloc(sizeof(List));
  new_node->next = NULL;
  new_node->data = o;
  p->next = new_node;
}

void list_prepend(List *l, Object *o) {
  List *p = l->next;
  List *new_node = malloc(sizeof(List));
  new_node->next = p;
  new_node->data = o;
  l->next = new_node;
}

int list_indexOf(List *l, Object *o) {
  List *node;
  int ret = -1;
  list_foreach(node, l) {
    ret++;
    if (node->data == o)
      break;
  }
  return ret;
}

void list_removeOne(List *l, Object *o) {
  if (!list_contains(l, o))
    return;
  List *node, *p = l, *p2 = l;
  list_foreach(node, l) {
    p2 = p;
    p = node;
    if (p->data == o) {
      p2->next = p->next;
      free(p);
      return;
    }
  }
}

Object *list_at(List *l, int index) {
  List *node;
  int i = -1;
  list_foreach(node, l) {
    i++;
    if (i == index)
      return node->data;
  }
  return NULL;
}

int list_length(List *l) {
  List *node;
  int i = 0;
  list_foreach(node, l) {
    i++;
  }
  return i;
}

void list_free(List *l, void (*freefunc)(void *)) {
  if (!l) return;
  list_free(l->next, freefunc);
  if (freefunc && l->data)
    freefunc(cast(void *, l->data));
  free(l);
}
