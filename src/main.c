#include "main.h"

struct ast *newast(int nodetype, struct ast *l, struct ast *r) {
  struct ast *a = malloc(sizeof(struct ast));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = nodetype;
  a->l = l;
  a->r = r;
  return a;
}

struct ast *newnum(long long n) {
  struct numval *a = malloc(sizeof(struct numval));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = Num;
  a->n = n;
  return (struct ast *)a;
}

void loadTranslations() {}

void returnPackages() {exit(0);}

Value analyzeTree(struct ast *a) {
  Value v;

  if (!a) {
    yyerror("attempt to analyze NULL tree");
    exit(0);
  }

  switch (a->nodetype) {
    case Extension:
      analyzeTree(a->l);
      analyzeTree(a->r);
      loadTranslations();
      returnPackages();
      break;

    // 0 or more
    case Skills:
    case Generals:
      if (a->l) {
        analyzeTree(a->l);
        analyzeTree(a->r);
      }
      break;

    // 1 or more
    case Packages:
      if (a->l) {
        analyzeTree(a->l);
      }
      analyzeTree(a->r);
      break;

    case Package:
      analyzeTree(a->r);
      break;

    default:
      break;
  }

  return v;
}

int main() {
  yyparse();
}
