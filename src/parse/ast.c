#include "ast.h"
#include "main.h"

void checktype(ExpVType a, ExpVType t) {
  if (a != t && a != TAny && t != TAny) {
    fprintf(error_output, "Type error: expect %d, got %d\n", t, a);
    while(1);
  }
}

struct ast *newast(NodeType nodetype, struct ast *l, struct ast *r) {
  struct ast *a = malloc(sizeof(struct ast));
  a->nodetype = nodetype;
  a->l = l;
  a->r = r;
  return a;
}
