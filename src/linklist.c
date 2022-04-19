#include "linklist.h"

int foreach(struct ast* list, struct ast *parent, Callback f)
{
  int ret = 0;
  if (list->l)  // struct ast *prev
    ret = foreach(list->l, parent, f);

  if (list->r)  // struct ast *data
    ret = ret | f(list->r, parent);

  return ret;
}
