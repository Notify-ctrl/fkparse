#ifndef _OBJECT_H
#define _OBJECT_H

#include "structs.h"

typedef struct {
  ObjectHeader;
  List *stats;
} ExtensionObj;

ExtensionObj *newExtension();
void analyzeExtensionQSan(ExtensionObj *e);

typedef struct {
  ObjectHeader;
  const char *id;
  int internal_id;
} PackageObj;

PackageObj *newPackage(const char *name);

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
  struct ViewAsSpecObj *vsSpec;
  struct StatusSpecObj *statusSpec;
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

enum ExpType {
  ExpCmp,
  ExpLogic,
  ExpCalc,
  ExpStr,
  ExpNum,
  ExpBool,
  ExpVar,
  ExpArray,
  ExpDict,
  ExpFunc,
  ExpFuncdef,
};

typedef enum ExpType ExpType;

enum ExpVType {
  TNone,
  TPackage,
  TSkill,
  TGeneral,
  TNumber,
  TBool,
  TString,
  TPlayer,
  TCard,
  TEmptyList,
  TPlayerList,
  TCardList,
  TNumberList,
  TStringList,
  TMark,
  TDict,
  TFunc,
  TPindian,

  TNotSure = 0xFFFE,
  TAny = 0xFFFF
};

typedef enum ExpVType ExpVType;

typedef struct ExpressionObj {
  ObjectHeader;
  ExpType exptype;
  ExpVType valuetype;
  long long value;
  const char *strvalue;
  struct VarObj *varValue;
  struct FunccallObj *func;
  struct FuncdefObj *funcdef;
  List *array;
  Hash *dict;
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
  ExpressionObj *index; /* used in index of array */
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

typedef struct FuncdefObj {
  ObjectHeader;
  const char *funcname;
  const char *name;
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
  Spec_ViewAsSkill,
  Spec_StatusSkill,
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
  BlockObj *on_cost;
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

typedef struct ViewAsSpecObj {
  ObjectHeader;
  BlockObj *cond;
  BlockObj *card_filter;
  BlockObj *feasible;
  BlockObj *view_as;
  BlockObj *can_response;
  ExpressionObj *responsable;
} ViewAsSpecObj;

ViewAsSpecObj *newViewAsSpec(BlockObj *cond, BlockObj *card_filter,
                             BlockObj *feasible, BlockObj *view_as);

typedef struct StatusSpecObj {
  ObjectHeader;
  BlockObj *is_prohibited;
  BlockObj *card_filter;
  BlockObj *vsrule;
  BlockObj *distance_correct;
  BlockObj *max_extra;
  BlockObj *max_fixed;
  BlockObj *tmd_residue;
  BlockObj *tmd_distance;
  BlockObj *tmd_extarget;
  BlockObj *atkrange_extra;
  BlockObj *atkrange_fixed;
} StatusSpecObj;

StatusSpecObj *newStatusSpec();

typedef struct {
  ObjectHeader;
  ExpressionObj *cond;
  BlockObj *then;
  List *elif;
  BlockObj *el; /* maybe NULL */
} IfObj;

IfObj *newIf(ExpressionObj *cond, BlockObj *then, List *elif, BlockObj *el);

typedef struct {
  ObjectHeader;
  BlockObj *body;
  ExpressionObj *cond;
  int type;
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
  ExpVType custom_type;
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

extern int skill_id;
extern int general_id;
extern int package_id;
extern int funcId;
extern int markId;
extern int stringId;

#endif
