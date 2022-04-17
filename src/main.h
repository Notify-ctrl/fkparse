#ifndef _MAIN_H
#define _MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include "grammar.h"
void yyerror(char *msg);

typedef YYSTYPE Value;

enum NodeType {
  Extension, // ast
  Packages, // ast
  Package,  // ast, l->Id, r->Generals
  Skills, // ast
  Skill,
  Generals, // ast
  General,

  Num,
  Str,
  Id,
};

struct ast {
  int nodetype;
  struct ast *l;
  struct ast *r;
};

struct numval {
  int nodetype;
  long long n;
};

struct strval {
  int nodetype;
  char *str;
};

struct ast *newast(int nodetype, struct ast *l, struct ast *r);
struct ast *newnum(long long n);

Value analyzeTree(struct ast *a);

#endif // _MAIN_H
