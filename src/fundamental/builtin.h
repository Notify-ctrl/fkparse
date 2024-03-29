#ifndef _BUILTIN_H
#define _BUILTIN_H

#include "object.h"
#include "main.h"
#include "fkparse.h"

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
void sym_init(fkp_analyze_type parse_type);

#endif
