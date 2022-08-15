#ifndef _BUILTIN_H
#define _BUILTIN_H

#include "object.h"
#include "main.h"

struct ProtoArg {
  const char *name;
  ExpVType argtype;
  bool have_default;
  union {
    long long n;
    const char *s;
  } d;
};

typedef struct {
  const char *dst;
  const char *src;
  ExpVType rettype;
  int argcount;
  struct ProtoArg args[10];
} Proto;

typedef struct {
  char *dst;
  char *src;
  int type;
} BuiltinVar;

void loadmodule(Proto *ps, BuiltinVar *vs);

void load_builtin_action();
void load_builtin_cards();
void load_builtin_enum();
void load_builtin_func();
void load_builtin_getter();
void load_builtin_interaction();
void load_builtin_util();

#endif
