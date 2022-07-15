#ifndef _OBJECT_H
#define _OBJECT_H

#include "structs.h"
#include "ast.h"

typedef struct {
  ObjType objtype;
  long value;
} IntegerObj;

typedef struct {
  ObjType objtype;
  double value;
} DoubleObj;

typedef struct {
  ObjType objtype;
  const char *value;
} StringObj;

/* ------------------------ */

typedef struct {
  ObjType objtype;
  List *funcdefs;
  List *skills;
  List *packages;
} ExtensionObj;

ExtensionObj *newExtension(struct ast *a);
void analyzeExtension(ExtensionObj *e);

typedef struct {
  ObjType objtype;
  const char *id;
  int internal_id;
  List *generals;
} PackageObj;

typedef struct {
  ObjType objtype;
  const char *id;
  const char *kingdom;
  long long hp;
  const char *nickname;
  const char *gender;
  int internal_id;
  List *skills;
} GeneralObj;

typedef struct {
  ObjType objtype;
  const char *id;
  const char *description;
  const char *frequency;
  const char *interid;
  int internal_id;

  List *triggerSpecs;
  /* TODO: other skill specs */
} SkillObj;

/* ------------------------ */

typedef struct {
  ObjType objtype;
  List *statements; /* maybe empty */
  struct ExpressionObj *ret;
} BlockObj;

typedef struct ExpressionObj {
  ObjType objtype;
  ExpType exptype;
  ExpVType valuetype;
  long long value;
  const char *strvalue;
  struct VarObj *varValue;
  struct ActionObj *action;
  struct FunccallObj *func;
  List *array;
  int optype;
  struct ExpressionObj *oprand1; /* maybe NULL */
  struct ExpressionObj *oprand2; /* maybe NULL */
  bool bracketed;
} ExpressionObj;

ExpressionObj *newExpression(struct ast *a);

typedef struct VarObj {
  ObjType objtype;
  const char *name; /* or field */
  ExpressionObj *obj; /* maybe NULL */
  ExpVType type;
} VarObj;

VarObj *newVar(struct ast *a);

typedef struct {
  ObjType objtype;
  const char *name;
  ExpVType type;
  ExpressionObj *d;
} DefargObj;

typedef struct {
  ObjType objtype;
  const char *funcname;
  List *params;
  int rettype;
  BlockObj *funcbody;
} FuncdefObj;

typedef struct {
  ObjType objtype;
  int event;
  BlockObj *can_trigger;
  BlockObj *on_trigger;
  BlockObj *on_refresh;
} TriggerSpecObj;

typedef struct {
  ObjType objtype;
  ExpressionObj *cond;
  BlockObj *then;
  BlockObj *el; /* maybe NULL */
} IfObj;

typedef struct {
  ObjType objtype;
  BlockObj *body;
  ExpressionObj *cond;
} LoopObj;

typedef struct {
  ObjType objtype;
  ExpressionObj *array;
  const char *expname;
  BlockObj *body;
} TraverseObj;

typedef struct {
  ObjType objtype;
  VarObj *var;
  ExpressionObj *value;
} AssignObj;

typedef struct FunccallObj {
  ObjType objtype;
  ExpVType rettype;
  const char *name;
  Hash *params;
} FunccallObj;

typedef struct ActionObj {
  ObjType objtype;
  ActionType actiontype;
  ExpVType valuetype;
  Object *action;
  bool standalone;
} ActionObj;

/* e.g. newParams(2, "xxx", e: exp, "xxx2", e2, ...) */
Hash *newParams(int param_count, ...);

#endif
