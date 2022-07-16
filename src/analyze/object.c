#include "object.h"
#include "main.h"
#include "generate.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

Hash *global_symtab;
Hash *current_tab;
Hash *last_lookup_tab;
Stack *symtab_stack;

symtab_item *sym_lookup(const char *k) {
  void *v = NULL;
  Stack *node;
  last_lookup_tab = NULL;
  list_foreach(node, symtab_stack) {
    Hash *h = cast(Hash *, node->data);
    v = hash_get(h, k);
    if (v) {
      last_lookup_tab = h;
      break;
    }
  }
  return cast(symtab_item *, v);
}

void sym_set(const char *k, symtab_item *v) {
  symtab_item *i = sym_lookup(k);
  if (i && i->reserved) {
    outputError("不能修改预定义的标识符 %s", k);
  } else {
    if (i) checktype(i->type, v->type);
    hash_set(last_lookup_tab ? last_lookup_tab : current_tab, k, (void *)v);
  }
}

void sym_new_entry(const char *k, int type, const char *origtext, bool reserved) {
  symtab_item *i = sym_lookup(k);
  symtab_item *v;
  if (i) {
    if (i->type != TNone)
      outputError("%s 已经存在于表中，类型为 %d", k, i->type);
    else {
      i->type = type;
      i->origtext = origtext;
      i->reserved = reserved;
    }
  } else {
    v = malloc(sizeof(symtab_item));
    v->type = type;
    v->origtext = origtext;
    v->reserved = reserved;
    sym_set(k, cast(void *, v));
  }
}

void sym_free(Hash *h) {
  for (size_t i = 0; i < h->capacity; i++) {
    free((void*)h->entries[i].key);
    free((void*)h->entries[i].value);
  }

  free(h->entries);
  free(h);
}

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
  v->origtxt = orig;
  v->translated = translated;
  list_append(restrtab, cast(Object *, v));
}

FunccallObj *newFunccall(const char *name, Hash *params) {
  FunccallObj *ret = malloc(sizeof(FunccallObj));
  ret->objtype = Obj_Funccall;
  ret->name = name;
  ret->params = params;
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
      /* sym_new_entry(ret->name, TNone, NULL, false); */
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
  addTranslation(strdup(interid), id);
  hash_set(skill_table, id, strdup(interid));

  sprintf(buf, ":%s", ret->interid);
  addTranslation(strdup(buf), desc);

  List *iter;
  list_foreach(iter, specs) {
    struct ast *data = cast(struct ast *, iter->data);
    switch (data->nodetype) {
    case N_TriggerSkill:
      ret->triggerSpecs = cast(List *, data->r);
      break;
    default:
      break;
    }
  }

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
  addTranslation(strdup(buf), nickname);

  ret->skills = skills;

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
  sprintf(buf, "%sfunc%d", readfile_name, funcId);
  funcId++;

  ret->funcname = strdup(buf);
  sym_new_entry(name, TFunc, cast(const char *, ret), false);
  ret->params = params;
  ret->rettype = rettype;
  ret->funcbody = funcbody;

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
