#include "object.h"
#include "main.h"
#include "error.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

List *restrtab;

const char *untranslate(const char *trans) {
  List *node;
  list_foreach(node, restrtab) {
    str_value *v = cast(str_value *, node->data);
    if (!strcmp(v->translated, trans)) {
      return v->origtxt;
    }
  }
  return NULL;
}

void addTranslation(const char *orig, const char *translated) {
  //if (translate(orig)) {
    /* show warning here */
  //}
  str_value *v = malloc(sizeof(str_value));
  v->origtxt = strdup(orig);
  v->translated = strdup(translated);
  list_append(restrtab, cast(Object *, v));
}

FunccallObj *newFunccall(const char *name, Hash *params) {
  FunccallObj *ret = malloc(sizeof(FunccallObj));
  ret->objtype = Obj_Funccall;
  ret->name = name;
  ret->params = params;
  return ret;
}

ArgObj *newArg(const char *name, ExpressionObj *exp) {
  ArgObj *ret = malloc(sizeof(ArgObj));
  ret->objtype = Obj_Arg;
  ret->name = name;
  ret->exp = exp;
  return ret;
}

Hash *general_table;
Hash *mark_table;
Hash *skill_table;
Hash *other_string_table;

int skill_id;
int general_id;
int package_id;
int funcId;
int markId;
int stringId;

ExpressionObj *newExpression(int exptype, long long value, int optype,
                             ExpressionObj *l, ExpressionObj *r) {
  ExpressionObj *ret = malloc(sizeof(ExpressionObj));
  ret->objtype = Obj_Expression;

  ret->exptype = exptype;
  switch (exptype) {
    case ExpCmp:
    case ExpCalc:
    case ExpNum:
      ret->valuetype = TNumber;
      break;
    case ExpStr:
      ret->valuetype = TString;
      break;
    case ExpDict:
      ret->valuetype = TDict;
      break;
    case ExpFunc:
      ret->valuetype = TFunc;
      break;
    default:
      ret->valuetype = -1;
      break;
  }
  ret->value = value;
  ret->optype = optype;
  ret->strvalue = NULL;
  ret->varValue = NULL;
  ret->func = NULL;
  ret->funcdef = NULL;
  ret->array = NULL;
  ret->dict = NULL;
  ret->oprand1 = l;
  ret->oprand2 = r;
  ret->bracketed = false;
  ret->param_name = NULL;

  return ret;
}

VarObj *newVar(const char *name, ExpressionObj *obj) {
  VarObj *ret = malloc(sizeof(VarObj));
  ret->objtype = Obj_Var;
  ret->name = name;
  ret->obj = obj;
  ret->index = NULL;

  if (!ret->obj) {
    if (!ret->name) {
      ret->type = TNone;
      return ret;
    }
    symtab_item *i = sym_lookup(ret->name);
    if (i) {
      ret->type = i->type;
    } else {
      ret->type = TNone;
    }
  } else {
    ret->type = TNone;  /* determine type later */
  }

  return ret;
}

AssignObj *newAssign(VarObj *var, ExpressionObj *e) {
  AssignObj *ret = malloc(sizeof(AssignObj));
  ret->objtype = Obj_Assign;
  ret->var = var;
  ret->value = e;
  ret->custom_type = TNone;
  return ret;
}

IfObj *newIf(ExpressionObj *cond, BlockObj *then, List *elif, BlockObj *el) {
  IfObj *ret = malloc(sizeof(IfObj));
  ret->objtype = Obj_If;
  ret->cond = cond;
  ret->then = then;
  ret->elif = elif;
  ret->el = el;
  return ret;
}

LoopObj *newLoop(BlockObj *body, ExpressionObj *cond) {
  LoopObj *ret = malloc(sizeof(LoopObj));
  ret->objtype = Obj_Loop;
  ret->cond = cond;
  ret->body = body;
  ret->type = 0;

  return ret;
}

TraverseObj *newTraverse(ExpressionObj *array, const char *expname,
                         BlockObj *body) {
  TraverseObj *ret = malloc(sizeof(TraverseObj));
  ret->objtype = Obj_Traverse;
  ret->array = array;
  ret->expname = expname;
  ret->body = body;
  return ret;
}

BlockObj *newBlock(List *stats, ExpressionObj *e) {
  BlockObj *ret = malloc(sizeof(BlockObj));
  ret->objtype = Obj_Block;
  ret->statements = stats;
  ret->ret = e;
  return ret;
}

TriggerSpecObj *newTriggerSpec(int event, BlockObj *cond, BlockObj *effect) {
  TriggerSpecObj *ret = malloc(sizeof(TriggerSpecObj));
  ret->objtype = Obj_TriggerSpec;
  ret->event = event;
  ret->can_trigger = cond;
  ret->on_trigger = effect;
  ret->on_refresh = NULL; /* TODO */
  ret->on_cost = NULL;

  return ret;
}

ActiveSpecObj *newActiveSpec(BlockObj *cond, BlockObj *card_filter,
                             BlockObj *target_filter, BlockObj *feasible,
                             BlockObj *on_use, BlockObj *on_effect)
{
  ActiveSpecObj *ret = malloc(sizeof(ActiveSpecObj));
  ret->objtype = Obj_ActiveSpec;
  ret->cond = cond;
  ret->card_filter = card_filter;
  ret->target_filter = target_filter;
  ret->feasible = feasible;
  ret->on_use = on_use;
  ret->on_effect = on_effect;
  return ret;
}

ViewAsSpecObj *newViewAsSpec(BlockObj *cond, BlockObj *card_filter,
                             BlockObj *feasible, BlockObj *view_as)
{
  ViewAsSpecObj *ret = malloc(sizeof(ViewAsSpecObj));
  ret->objtype = Obj_ViewAsSpec;
  ret->cond = cond;
  ret->card_filter = card_filter;
  ret->feasible = feasible;
  ret->view_as = view_as;
  ret->can_response = NULL;
  ret->responsable = NULL;
  return ret;
}

StatusSpecObj *newStatusSpec() {
  StatusSpecObj *ret = malloc(sizeof(StatusSpecObj));
  memset(ret, 0, sizeof(StatusSpecObj));  // NULL
  ret->objtype = Obj_StatusSpec;
  return ret;
}

SkillObj *newSkill(const char *id, const char *desc, const char *frequency,
                   const char *interid, List *specs) {
  SkillObj *ret = malloc(sizeof(SkillObj));
  ret->objtype = Obj_Skill;
  ret->internal_id = skill_id++;

  char buf[64];
  sprintf(buf, "%s_s_%d", readfile_name, ret->internal_id);
  if (!interid) {
    interid = strdup(buf);
  }

  ret->id = id;
  ret->description = desc;
  ret->frequency = frequency ? frequency : strdup("NotFrequent");
  ret->interid = interid;
  addTranslation(interid, id);
  hash_set(skill_table, id, strdup(interid));

  sprintf(buf, ":%s", ret->interid);
  addTranslation(buf, desc);

  ret->triggerSpecs = NULL;
  ret->activeSpec = NULL;
  ret->vsSpec = NULL;
  ret->statusSpec = NULL;

  List *iter;
  list_foreach(iter, specs) {
    SkillSpecObj *data = cast(SkillSpecObj *, iter->data);
    switch (data->type) {
    case Spec_TriggerSkill:
      ret->triggerSpecs = cast(List *, data->obj);
      free(data);
      break;
    case Spec_ActiveSkill:
      if (ret->activeSpec != NULL) {
        yyerror(cast(YYLTYPE *, data->obj), "不允许一个技能下同时存在多个主动技");
        freeObject(ret->activeSpec);
      }
      ret->activeSpec = cast(ActiveSpecObj *, data->obj);
      free(data);
      break;
    case Spec_ViewAsSkill:
      if (ret->vsSpec != NULL) {
        yyerror(cast(YYLTYPE *, data->obj), "不允许一个技能下同时存在多个视为技");
        freeObject(ret->vsSpec);
      }
      ret->vsSpec = cast(ViewAsSpecObj *, data->obj);
      free(data);
      break;
    case Spec_StatusSkill:
      if (ret->statusSpec != NULL) {
        yyerror(cast(YYLTYPE *, data->obj), "不允许一个技能下同时存在多个状态技");
        freeObject(ret->statusSpec);
      }
      ret->statusSpec = cast(StatusSpecObj *, data->obj);
      free(data);
      break;
    }
  }

  list_free(specs, NULL);

  return ret;
}

GeneralObj *newGeneral(const char *id, const char *kingdom, long long hp,
                       const char *nickname, const char *gender,
                       const char *interid, List *skills) {
  GeneralObj *ret = malloc(sizeof(GeneralObj));
  ret->objtype = Obj_General;
  ret->internal_id = general_id++;

  ret->id = id;
  ret->kingdom = kingdom;
  ret->hp = hp;
  ret->nickname = nickname;
  ret->gender = gender ? gender : strdup("Male");

  char buf[64];
  sprintf(buf, "%s_g_%d", readfile_name, ret->internal_id);
  if (!interid) {
    interid = strdup(buf);
  }

  addTranslation(interid, id);
  sym_new_entry(ret->id, TGeneral, interid, false);
  hash_set(general_table, id, strdup(interid));

  sprintf(buf, "#%s", interid);
  addTranslation(buf, nickname);

  ret->skills = skills;

  free((void *)interid);
  return ret;
}

PackageObj *newPackage(const char *name) {
  PackageObj *ret = malloc(sizeof(PackageObj));
  ret->objtype = Obj_Package;
  ret->internal_id = package_id++;

  char buf[64];
  sprintf(buf, "%s_p_%d", readfile_name, ret->internal_id);
  ret->id = strdup(buf);
  addTranslation(ret->id, name);
  sym_new_entry(name, TPackage, ret->id, false);

  free((void *)name);
  return ret;
}

DefargObj *newDefarg(const char *name, int type, ExpressionObj *d) {
  DefargObj *ret = malloc(sizeof(DefargObj));
  ret->objtype = Obj_Defarg;
  ret->name = name;
  ret->type = type;
  ret->d = d;
  return ret;
}

FuncdefObj *newFuncdef(const char *name, List *params, int rettype,
                       BlockObj *funcbody) {
  FuncdefObj *ret = malloc(sizeof(FuncdefObj));
  ret->objtype = Obj_Funcdef;

  char buf[64];
  sprintf(buf, "%s_func_%d", readfile_name, funcId);
  funcId++;

  ret->funcname = strdup(buf);
  ret->name = name;
  ret->params = params;
  ret->rettype = rettype;
  ret->funcbody = funcbody;

  return ret;
}

SkillSpecObj *newSkillSpec(SpecType type, void *obj) {
  SkillSpecObj *ret = malloc(sizeof(SkillSpecObj));
  ret->objtype = Obj_SkillSpec;
  ret->type = type;
  ret->obj = obj;
  return ret;
}

ExtensionObj *newExtension() {
  ExtensionObj *ret = malloc(sizeof(ExtensionObj));
  ret->objtype = Obj_Extension;

  return ret;
}

Hash *newParams(int param_count, ...) {
  va_list ap;
  va_start(ap, param_count);

  const char *s;
  ExpressionObj *e;
  int analyzed = 0;

  Hash *ret = hash_new();
  while (analyzed < param_count) {
    s = va_arg(ap, const char *);
    e = va_arg(ap, ExpressionObj *);
    e->param_name = strdup(s);
    hash_set(ret, s, cast(void *, e));
    analyzed++;
  }

  va_end(ap);
  return ret;
}

/* free functions */

static void freeBlock(void *ptr);
static void freeVar(void *ptr);
static void freeFunccall(void *ptr);

static void freeExp(void *ptr) {
  if (!ptr) return;
  ExpressionObj *e = ptr;
  free((void *)e->strvalue);
  freeObject(e->varValue);
  freeObject(e->func);
  freeObject(e->funcdef);
  if (e->array) list_free(e->array, freeObject);
  if (e->dict) hash_free(e->dict, freeObject);
  freeObject(e->oprand1);
  freeObject(e->oprand2);
  free((void *)e->param_name);
  free(e);
}

static void freeVar(void *ptr) {
  VarObj *v = cast(VarObj *, ptr);
  free((void *)v->name);
  freeObject(v->obj);
  freeObject(v->index);
  free(v);
}

static void freeArg(void *ptr) {
  ArgObj *a = ptr;
  free((void *)a->name);
  freeObject(a->exp);
  free(a);
}

static void freeFunccall(void *ptr) {
  FunccallObj *f = ptr;
  free((void *)f->name);
  hash_free(f->params, freeObject);
  free(f);
}

static void freeAssign(AssignObj *a) {
  freeObject(a->var);
  freeObject(a->value);
  free(a);
}

static void freeIf(IfObj *i) {
  freeObject(i->cond);
  freeObject(i->then);
  list_free(i->elif, freeObject);
  freeObject(i->el);
  free(i);
}

static void freeLoop(LoopObj *l) {
  freeObject(l->body);
  freeObject(l->cond);
  free(l);
}

static void freeTraverse(TraverseObj *t) {
  freeObject(t->array);
  free((void *)t->expname);
  freeObject(t->body);
  free(t);
}

static void freeBlock(void *ptr) {
  if (!ptr) return;
  BlockObj *b = ptr;
  list_free(b->statements, freeObject);
  freeObject(b->ret);
  free(b);
}

static void freeTriggerSpec(void *ptr) {
  TriggerSpecObj *t = ptr;
  freeObject(t->can_trigger);
  freeObject(t->on_trigger);
  freeObject(t->on_refresh);
  freeObject(t->on_cost);
  free(t);
}

static void freeActiveSpec(void *p) {
  ActiveSpecObj *a = p;
  freeObject(a->cond);
  freeObject(a->card_filter);
  freeObject(a->target_filter);
  freeObject(a->feasible);
  freeObject(a->on_use);
  freeObject(a->on_effect);
  free(a);
}

static void freeViewAsSpec(void *p) {
  ViewAsSpecObj *v = p;
  freeObject(v->cond);
  freeObject(v->card_filter);
  freeObject(v->feasible);
  freeObject(v->view_as);
  freeObject(v->can_response);
  freeObject(v->responsable);
  free(v);
}

static void freeStatusSpec(void *p) {
  StatusSpecObj *v = p;
  freeObject(v->is_prohibited);
  freeObject(v->card_filter);
  freeObject(v->vsrule);
  freeObject(v->distance_correct);
  freeObject(v->max_extra);
  freeObject(v->max_fixed);
  freeObject(v->tmd_residue);
  freeObject(v->tmd_distance);
  freeObject(v->tmd_extarget);
  freeObject(v->atkrange_extra);
  freeObject(v->atkrange_fixed);
  free(v);
}

static void freeDefarg(void *ptr) {
  DefargObj *d = ptr;
  free((void *)d->name);
  freeObject(d->d);
  free(d);
}

void freeFuncdef(void *ptr) {
  FuncdefObj *d = ptr;
  free((void *)d->funcname);
  free((void *)d->name);
  list_free(d->params, freeDefarg);
  freeObject(d->funcbody);
  free(d);
}

static void freeSkillSpec(void *ptr) {
  SkillSpecObj *s = ptr;
  if (s->type == Spec_TriggerSkill) {
    list_free(s->obj, freeTriggerSpec);
  } else {
    freeObject(s->obj);
  }
  free(s);
}

static void freeSkill(void *ptr) {
  SkillObj *s = ptr;
  free((void *)s->id);
  free((void *)s->description);
  free((void *)s->frequency);
  free((void *)s->interid);
  list_free(s->triggerSpecs, freeTriggerSpec);
  freeObject(s->activeSpec);
  freeObject(s->vsSpec);
  freeObject(s->statusSpec);
  free(s);
}

static void freeGeneral(void *ptr) {
  GeneralObj *g = ptr;
  free((void *)g->id);
  free((void *)g->kingdom);
  free((void *)g->nickname);
  free((void *)g->gender);
  list_free(g->skills, freeExp);
  free(g);
}

static void freePackage(void *ptr) {
  PackageObj *pack = ptr;
  free((void *)pack->id);
  free(pack);
}

static void freeExtension(ExtensionObj *e) {
  list_free(e->stats, freeObject);
  free(e);
}

void freeObject(void *p) {
  if (!p) return;
  Object *obj = p;
  switch (obj->objtype) {
  case Obj_Extension: freeExtension(p); break;;
  case Obj_Defarg: freeDefarg(p); break;
  case Obj_Funcdef: freeFuncdef(p); break;
  case Obj_Package: freePackage(p); break;
  case Obj_General: freeGeneral(p); break;
  case Obj_Skill: freeSkill(p); break;
  case Obj_SkillSpec: freeSkillSpec(p); break;
  case Obj_Card: break;
  case Obj_Block: freeBlock(p); break;
  case Obj_TriggerSpec: freeTriggerSpec(p); break;
  case Obj_ActiveSpec: freeActiveSpec(p); break;
  case Obj_ViewAsSpec: freeViewAsSpec(p); break;
  case Obj_StatusSpec: freeStatusSpec(p); break;
  case Obj_If: freeIf(p); break;
  case Obj_Loop: freeLoop(p); break;
  case Obj_Traverse: freeTraverse(p); break;
  case Obj_Break: free(p); break;
  case Obj_Funccall: freeFunccall(p); break;
  case Obj_Arg: freeArg(p); break;
  case Obj_Assign: freeAssign(p); break;
  case Obj_Expression: freeExp(p); break;
  case Obj_Var: freeVar(p); break;
  }
}
