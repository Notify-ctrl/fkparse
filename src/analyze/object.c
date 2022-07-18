#include "object.h"
#include "main.h"
#include "generate.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

Hash *strtab;
List *restrtab;

const char *translate(const char *orig) {
  // return (const char *)hash_get(strtab, orig);
  return NULL;
}

void addTranslation(const char *orig, const char *translated) {
  if (translate(orig)) {
    /* show warning here */
  }
  // hash_set(strtab, orig, cast(void *, translated));
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

Hash *mark_table;
Hash *skill_table;

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
    default:
      ret->valuetype = -1;
      break;
  }
  ret->value = value;
  ret->optype = optype;
  ret->strvalue = NULL;
  ret->varValue = NULL;
  ret->func = NULL;
  ret->array = NULL;
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

  if (!ret->obj) {
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
  return ret;
}

IfObj *newIf(ExpressionObj *cond, BlockObj *then, BlockObj *el) {
  IfObj *ret = malloc(sizeof(IfObj));
  ret->objtype = Obj_If;
  ret->cond = cond;
  ret->then = then;
  ret->el = el;
  return ret;
}

LoopObj *newLoop(BlockObj *body, ExpressionObj *cond) {
  LoopObj *ret = malloc(sizeof(LoopObj));
  ret->objtype = Obj_Loop;
  ret->cond = cond;
  ret->body = body;

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

  return ret;
}

SkillObj *newSkill(const char *id, const char *desc, const char *frequency,
                   const char *interid, List *specs) {
  SkillObj *ret = malloc(sizeof(SkillObj));
  ret->objtype = Obj_Skill;
  static int skill_id = 0;
  ret->internal_id = skill_id++;

  char buf[64];
  sprintf(buf, "%s_s_%d", readfile_name, ret->internal_id);
  if (!interid) {
    interid = strdup(buf);
  }

  ret->id = id;
  ret->description = desc;
  ret->frequency = frequency ? frequency : strdup("普通技");
  ret->interid = interid;
  addTranslation(interid, id);
  hash_set(skill_table, id, strdup(interid));

  sprintf(buf, ":%s", ret->interid);
  addTranslation(buf, desc);

  List *iter;
  list_foreach(iter, specs) {
    SkillSpecObj *data = cast(SkillSpecObj *, iter->data);
    switch (data->type) {
    case Spec_TriggerSkill:
      ret->triggerSpecs = cast(List *, data->obj);
      free(data);
      break;
    default:
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
  static int general_id = 0;
  ret->internal_id = general_id++;

  ret->id = id;
  ret->kingdom = kingdom;
  ret->hp = hp;
  ret->nickname = nickname;
  ret->gender = gender ? gender : strdup("男性");

  char buf[64];
  sprintf(buf, "%s_g_%d", readfile_name, ret->internal_id);
  if (!interid) {
    interid = strdup(buf);
  }

  addTranslation(interid, id);
  sym_new_entry(ret->id, TGeneral, interid, false);

  sprintf(buf, "#%s", interid);
  addTranslation(buf, nickname);

  ret->skills = skills;

  free((void *)interid);
  return ret;
}

PackageObj *newPackage(const char *name, List *generals) {
  PackageObj *ret = malloc(sizeof(PackageObj));
  ret->objtype = Obj_Package;
  static int package_id = 0;
  ret->internal_id = package_id++;

  char buf[64];
  sprintf(buf, "%s_p_%d", readfile_name, ret->internal_id);
  ret->id = strdup(buf);
  addTranslation(ret->id, name);
  sym_new_entry(name, TPackage, ret->id, false);
  ret->generals = generals;

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

  static int funcId = 0;
  char buf[64];
  sprintf(buf, "%s_func_%d", readfile_name, funcId);
  funcId++;

  ret->funcname = strdup(buf);
  sym_new_entry(name, TFunc, cast(const char *, ret), false);
  ret->params = params;
  ret->rettype = rettype;
  ret->funcbody = funcbody;

  free((void *)name);
  return ret;
}

SkillSpecObj *newSkillSpec(SpecType type, void *obj) {
  SkillSpecObj *ret = malloc(sizeof(SkillSpecObj));
  ret->objtype = Obj_SkillSpec;
  ret->type = type;
  ret->obj = obj;
  return ret;
}

ExtensionObj *newExtension(List *funcs, List *skills, List *packs) {
  ExtensionObj *ret = malloc(sizeof(ExtensionObj));
  ret->objtype = Obj_Extension;
  ret->funcdefs = funcs;
  ret->skills = skills;
  ret->packages = packs;

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
  if (e->varValue) freeVar(e->varValue);
  if (e->func) freeFunccall(e->func);
  if (e->array) list_free(e->array, freeExp);
  if (e->oprand1) freeExp(e->oprand1);
  if (e->oprand2) freeExp(e->oprand2);
  free((void *)e->param_name);
  free(e);
}

static void freeVar(void *ptr) {
  VarObj *v = cast(VarObj *, ptr);
  free((void *)v->name);
  freeExp(v->obj);
  free(v);
}

static void freeArg(void *ptr) {
  ArgObj *a = ptr;
  free((void *)a->name);
  freeExp(a->exp);
  free(a);
}

static void freeFunccall(void *ptr) {
  FunccallObj *f = ptr;
  free((void *)f->name);
  hash_free(f->params, freeExp);
  free(f);
}

static void freeAssign(AssignObj *a) {
  freeVar(a->var);
  freeExp(a->value);
  free(a);
}

static void freeIf(IfObj *i) {
  freeExp(i->cond);
  freeBlock(i->then);
  freeBlock(i->el);
  free(i);
}

static void freeLoop(LoopObj *l) {
  freeBlock(l->body);
  freeExp(l->cond);
  free(l);
}

static void freeTraverse(TraverseObj *t) {
  freeExp(t->array);
  free((void *)t->expname);
  freeBlock(t->body);
  free(t);
}

static void freeStat(void *ptr) {
  Object *o = ptr;
  switch (o->objtype) {
  case Obj_Assign:
    freeAssign(cast(AssignObj *, o));
    break;
  case Obj_If:
    freeIf(cast(IfObj *, o));
    break;
  case Obj_Loop:
    freeLoop(cast(LoopObj *, o));
    break;
  case Obj_Traverse:
    freeTraverse(cast(TraverseObj *, o));
    break;
  case Obj_Break:
    free(o);
    break;
  case Obj_Funccall:
    freeFunccall(cast(IfObj *, o));
    break;
  default:
    break;
  }
}

static void freeBlock(void *ptr) {
  if (!ptr) return;
  BlockObj *b = ptr;
  list_free(b->statements, freeStat);
  freeExp(b->ret);
  free(b);
}

static void freeTriggerSpec(void *ptr) {
  TriggerSpecObj *t = ptr;
  freeBlock(t->can_trigger);
  freeBlock(t->on_trigger);
  freeBlock(t->on_refresh);
  free(t);
}

static void freeDefarg(void *ptr) {
  DefargObj *d = ptr;
  free((void *)d->name);
  freeExp(d->d);
  free(d);
}

void freeFuncdef(void *ptr) {
  FuncdefObj *d = ptr;
  free((void *)d->funcname);
  list_free(d->params, freeDefarg);
  freeBlock(d->funcbody);
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
  free(s);
}

static void freeGeneral(void *ptr) {
  GeneralObj *g = ptr;
  free((void *)g->id);
  free((void *)g->kingdom);
  free((void *)g->nickname);
  free((void *)g->gender);
  list_free(g->skills, free);
  free(g);
}

static void freePackage(void *ptr) {
  PackageObj *pack = ptr;
  free((void *)pack->id);
  list_free(pack->generals, freeGeneral);
  free(pack);
}

void freeExtension(ExtensionObj *e) {
  list_free(e->funcdefs, freeFuncdef);
  list_free(e->skills, freeSkill);
  list_free(e->packages, freePackage);
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
