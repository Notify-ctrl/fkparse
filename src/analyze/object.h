#ifndef _OBJECT_H
#define _OBJECT_H

#include "structs.h"
#include "ast.h"

typedef struct {
  ObjectHeader;
  List *funcdefs;
  List *skills;
  List *packages;
} ExtensionObj;

ExtensionObj *newExtension(List *funcs, List *skills, List *packs);
void analyzeExtension(ExtensionObj *e);

typedef struct {
  ObjectHeader;
  const char *id;
  int internal_id;
  List *generals;
} PackageObj;

PackageObj *newPackage(const char *name, List *generals);

typedef struct {
  ObjectHeader;
  const char *id;
  const char *kingdom;
  long long hp;
  const char *nickname;
  const char *gender;
  int internal_id;
  List *skills;
} GeneralObj;

GeneralObj *newGeneral(const char *id, const char *kingdom, long long hp,
                       const char *nickname, const char *gender,
                       const char *interid, List *skills);

typedef struct {
  ObjectHeader;
  const char *id;
  const char *description;
  const char *frequency;
  const char *interid;
  int internal_id;

  List *triggerSpecs;
  struct ActiveSpecObj *activeSpec;
  /* TODO: other skill specs */
} SkillObj;

SkillObj *newSkill(const char *id, const char *desc, const char *frequency,
                   const char *interid, List *specs);

/* ------------------------ */

typedef struct {
  ObjectHeader;
  List *statements; /* maybe empty */
  struct ExpressionObj *ret;
} BlockObj;

BlockObj *newBlock(List *stats, struct ExpressionObj *e);

typedef struct ExpressionObj {
  ObjectHeader;
  ExpType exptype;
  ExpVType valuetype;
  long long value;
  const char *strvalue;
  struct VarObj *varValue;
  struct FunccallObj *func;
  List *array;
  int optype;
  struct ExpressionObj *oprand1; /* maybe NULL */
  struct ExpressionObj *oprand2; /* maybe NULL */
  bool bracketed;
  const char *param_name;
} ExpressionObj;

ExpressionObj *newExpression(int exptype, long long value, int optype,
                             ExpressionObj *l, ExpressionObj *r);

typedef struct VarObj {
  ObjectHeader;
  const char *name; /* or field */
  ExpressionObj *obj; /* maybe NULL */
  ExpVType type;
} VarObj;

VarObj *newVar(const char *name, ExpressionObj *obj);

typedef struct {
  ObjectHeader;
  const char *name;
  ExpVType type;
  ExpressionObj *d;
} DefargObj;

DefargObj *newDefarg(const char *name, int type, ExpressionObj *d);

typedef struct {
  ObjectHeader;
  const char *funcname;
  List *params;
  int rettype;
  BlockObj *funcbody;
} FuncdefObj;

FuncdefObj *newFuncdef(const char *name, List *params, int rettype,
                       BlockObj *funcbody);
void freeFuncdef(void *ptr);

typedef enum {
  Spec_TriggerSkill,
  Spec_ActiveSkill,
} SpecType;

typedef struct {
  ObjectHeader;
  SpecType type;
  void *obj;
} SkillSpecObj;

SkillSpecObj *newSkillSpec(SpecType type, void *obj);

typedef struct {
  ObjectHeader;
  int event;
  BlockObj *can_trigger;
  BlockObj *on_trigger;
  BlockObj *on_refresh;
} TriggerSpecObj;

TriggerSpecObj *newTriggerSpec(int event, BlockObj *cond, BlockObj *effect);

typedef struct ActiveSpecObj {
  ObjectHeader;
  BlockObj *cond;
  BlockObj *card_filter;
  BlockObj *target_filter;
  BlockObj *feasible;
  BlockObj *on_use;
  BlockObj *on_effect;
} ActiveSpecObj;

ActiveSpecObj *newActiveSpec(BlockObj *cond, BlockObj *card_filter,
                             BlockObj *target_filter, BlockObj *feasible,
                             BlockObj *on_use, BlockObj *on_effect);

typedef struct {
  ObjectHeader;
  ExpressionObj *cond;
  BlockObj *then;
  BlockObj *el; /* maybe NULL */
} IfObj;

IfObj *newIf(ExpressionObj *cond, BlockObj *then, BlockObj *el);

typedef struct {
  ObjectHeader;
  BlockObj *body;
  ExpressionObj *cond;
} LoopObj;

LoopObj *newLoop(BlockObj *body, ExpressionObj *cond);

typedef struct {
  ObjectHeader;
  ExpressionObj *array;
  const char *expname;
  BlockObj *body;
} TraverseObj;

TraverseObj *newTraverse(ExpressionObj *array, const char *expname,
                         BlockObj *body);

typedef struct {
  ObjectHeader;
  VarObj *var;
  ExpressionObj *value;
} AssignObj;

AssignObj *newAssign(VarObj *var, ExpressionObj *e);

typedef struct FunccallObj {
  ObjectHeader;
  ExpVType rettype;
  const char *name;
  Hash *params;
} FunccallObj;

FunccallObj *newFunccall(const char *name, Hash *params);

typedef struct {
  ObjectHeader;
  const char *name;
  ExpressionObj *exp;
} ArgObj;

ArgObj *newArg(const char *name, ExpressionObj *exp);

/* e.g. newParams(2, "xxx", e: exp, "xxx2", e2, ...) */
Hash *newParams(int param_count, ...);

void freeObject(void *p);

#endif
