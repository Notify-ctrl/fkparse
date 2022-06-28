#include "object.h"
#include "action.h"
#include "main.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Hash *symtab;

symtab_item *sym_lookup(const char *k) {
  return (symtab_item *)hash_get(symtab, k);
}

void sym_set(const char *k, symtab_item *v) {
  symtab_item *i = sym_lookup(k);
  if (i && i->reserved) {
    /* error */
  } else {
    hash_set(symtab, k, (void *)v);
  }
}

void sym_new_entry(const char *k, int type, const char *origtext, bool reserved) {
  symtab_item *i = sym_lookup(k);
  symtab_item *v;
  if (i) {
    /* error */
  } else {
    v = malloc(sizeof(symtab_item));
    v->type = type;
    v->origtext = origtext;
    v->reserved = reserved;
    sym_set(k, cast(void *, v));
  }
}

Hash *strtab;
List *restrtab;
static BlockObj *newBlock(struct ast *a);

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

void addTransWithType(const char *orig, const char *translated, int type) {
  addTranslation(orig, translated);

  str_value *v = malloc(sizeof(str_value));
  v->type = type;
  v->origtxt = orig;
  v->translated = translated;

  List *node;
  str_value *p;
  list_foreach(node, restrtab) {
    p = cast(str_value *, node->data);
    if (!strcmp(v->origtxt, p->origtxt)) {
      /* show warning here */
      p->translated = v->translated;
      p->type = v->type;
      free(v);
      return;
    } else if (!strcmp(v->translated, p->translated)) {
      if (v->type == p->type) {
        free(v);
        /* error */
        return;
      }
    }
  }
  list_append(restrtab, cast(Object *, v));
}

static IntegerObj *newInteger(long a) {
  IntegerObj *ret = malloc(sizeof(IntegerObj));
  ret->objtype = Obj_Integer;
  ret->value = a;
  return ret;
}

static DoubleObj *newDouble(double a) {
  DoubleObj *ret = malloc(sizeof(DoubleObj));
  ret->objtype = Obj_Double;
  ret->value = a;
  return ret;
}

static StringObj *newString(const char *a) {
  StringObj *ret = malloc(sizeof(StringObj));
  ret->objtype = Obj_String;
  ret->value = a;
  return ret;
}

ExpressionObj *newExpression(struct ast *a) {
  if (!a) return NULL;
  checktype(a->nodetype, N_Exp);

  ExpressionObj *ret = malloc(sizeof(ExpressionObj));
  ret->objtype = Obj_Expression;
  struct astExp *e = cast(struct astExp *, a);
  ret->exptype = e->exptype;
  ret->valuetype = e->valuetype;
  ret->value = e->value;
  ret->optype = e->optype;
  ret->bracketed = e->bracketed;

  struct ast *iter;
  ExpressionObj *exp = NULL;
  switch (e->exptype) {
  case ExpCmp:
  case ExpLogic:
  case ExpCalc:
    ret->oprand1 = newExpression(cast(struct ast *, e->l));
    ret->oprand2 = newExpression(cast(struct ast *, e->r));
    break;
  case ExpStr:
    ret->strvalue = cast(struct aststr *, e->l)->str;
    break;
  case ExpNum:
  case ExpBool:
    break;
  case ExpVar:
    ret->varValue = newVar(cast(struct ast *, e->l));
    break;
  case ExpArray:
    ret->array = list_new();
    iter = cast(struct ast *, e->l);
    checktype(iter->nodetype, N_Exps);
    while (iter && iter->r) {
      exp = newExpression(iter->r);
      if (exp) list_prepend(ret->array, cast(Object *, exp));
      iter = iter->l;
    }
    break;
  case ExpAction:
    ret->action = newAction(cast(struct ast *, e->l));
    break;
  }

  return ret;
}

VarObj *newVar(struct ast *a) {
  checktype(a->nodetype, N_Var);

  VarObj *ret = malloc(sizeof(VarObj));
  ret->objtype = Obj_Var;
  struct astVar *v = cast(struct astVar *, a);
  ret->name = v->name->str;
  ret->obj = newExpression(cast(struct ast *, v->obj));

  if (!ret->obj) {
    symtab_item *i = sym_lookup(ret->name);
    if (i) {
      ret->type = i->type;
    } else {
      ret->type = TNone;
      sym_new_entry(ret->name, TNone, NULL, false);
    }
  } else {
    ret->type = TNone;  /* determine type later */
  }

  return ret;
}

static AssignObj *newAssign(struct ast *a) {
  checktype(a->nodetype, N_Stat_Assign);

  AssignObj *ret = malloc(sizeof(AssignObj));
  ret->objtype = Obj_Assign;
  struct astAssignStat *as = cast(struct astAssignStat *, a);
  ret->var = newVar(as->lval);
  ret->value = newExpression(as->rval);

  return ret;
}

static IfObj *newIf(struct ast *a) {
  checktype(a->nodetype, N_Stat_If);

  IfObj *ret = malloc(sizeof(IfObj));
  ret->objtype = Obj_If;
  struct astIf *i = cast(struct astIf *, a);
  ret->cond = newExpression(i->cond);
  ret->then = newBlock(i->then);
  ret->el = newBlock(i->el);

  return ret;
}

static LoopObj *newLoop(struct ast *a) {
  checktype(a->nodetype, N_Stat_Loop);

  LoopObj *ret = malloc(sizeof(LoopObj));
  ret->objtype = Obj_Loop;
  struct astLoop *l = cast(struct astLoop *, a);
  ret->cond = newExpression(l->cond);
  ret->body = newBlock(l->body);

  return ret;
}

static Object *newBreak(struct ast *a) {
  checktype(a->nodetype, N_Stat_Break);

  Object *ret = malloc(sizeof(Object));
  ret->objtype = Obj_Break;

  return ret;
}

static Object *newReturn(struct ast *a) {
  checktype(a->nodetype, N_Stat_Ret);

  Object *ret = malloc(sizeof(Object));
  ret->objtype = Obj_Return;

  return ret;
}

static Object *newStatement(struct ast *a) {
  Object *ret = NULL;
  switch (a->nodetype) {
  case N_Stat_None:
    break;
  case N_Stat_Assign:
    ret = cast(Object *, newAssign(a));
    break;
  case N_Stat_If:
    ret = cast(Object *, newIf(a));
    break;
  case N_Stat_Loop:
    ret = cast(Object *, newLoop(a));
    break;
  case N_Stat_Break:
    ret = cast(Object *, newBreak(a));
    break;
  case N_Stat_Action:
    ret = cast(Object *, newAction(a));
    break;
  case N_Stat_Ret:
    ret = cast(Object *, newReturn(a));
    break;
  default:
    break;
  }
  return ret;
}

static BlockObj *newBlock(struct ast *a) {
  if (!a) return NULL;
  checktype(a->nodetype, N_Block);

  BlockObj *ret = malloc(sizeof(BlockObj));
  ret->objtype = Obj_Block;
  ret->statements = list_new();
  struct ast *iter = a->l;
  Object *stat;
  checktype(iter->nodetype, N_Stats);
  while (iter && iter->r) {
    stat = newStatement(iter->r);
    if (stat) list_prepend(ret->statements, stat);
    iter = iter->l;
  }

  /* TODO: retstat in block */

  return ret;
}

static TriggerSpecObj *newTriggerSpec(struct ast *a) {
  checktype(a->nodetype, N_TriggerSpec);

  TriggerSpecObj *ret = malloc(sizeof(TriggerSpecObj));
  ret->objtype = Obj_TriggerSpec;
  struct astTriggerSpec *t = cast(struct astTriggerSpec *, a);

  ret->event = t->event;
  ret->can_trigger = newBlock(t->cond);
  ret->on_trigger = newBlock(t->effect);
  ret->on_refresh = NULL; /* TODO */

  return ret;
}

static SkillObj *newSkill(struct ast *a) {
  checktype(a->nodetype, N_Skill);

  SkillObj *ret = malloc(sizeof(SkillObj));
  ret->objtype = Obj_Skill;
  struct astskill *s = cast(struct astskill *, a);

  ret->id = s->id->str;
  ret->description = s->description->str;
  ret->frequency = s->frequency->str;
  ret->interid = s->interid->str;
  ret->internal_id = s->uid;
  addTranslation(ret->interid, ret->id);
  sym_new_entry(ret->id, TSkill, ret->interid, false);
  char buf[64];
  sprintf(buf, ":%s", ret->interid);
  addTranslation(strdup(buf), ret->description);
  ret->triggerSpecs = list_new();
  struct ast *iter = s->skillspecs;
  struct ast *iter2 = NULL, *iter3 = NULL;
  checktype(iter->nodetype, N_SkillSpecs);
  while (iter && iter->r) {
    switch (iter->r->nodetype) {
      case N_TriggerSkill:
        if (iter2 != NULL) {
          /* error */
        } else {
          iter2 = iter->r->l;
          while (iter2 && iter2->r) {
            list_prepend(ret->triggerSpecs,
                         cast(Object *, newTriggerSpec(iter2->r)));
            iter2 = iter2->l;
          }
        }
      default:
        /* error */
        break;
    }
    iter = iter->l;
  }

  return ret;
}

static GeneralObj *newGeneral(struct ast *a) {
  checktype(a->nodetype, N_General);

  GeneralObj *ret = malloc(sizeof(GeneralObj));
  ret->objtype = Obj_General;
  struct astgeneral *g = cast(struct astgeneral *, a);
  ret->id = g->id->str;
  ret->kingdom = g->kingdom->str;
  ret->hp = g->hp;
  ret->nickname = g->nickname->str;
  ret->gender = g->gender->str;
  ret->internal_id = g->uid;
  addTranslation(g->interid->str, g->id->str);
  sym_new_entry(ret->id, TGeneral, g->interid->str, false);
  char buf[64];
  sprintf(buf, "#%s", g->interid->str);
  addTranslation(strdup(buf), g->nickname->str);

  ret->skills = list_new();
  struct ast *iter = g->skills;
  checktype(iter->nodetype, N_Strs);
  while (iter && iter->r) {
    list_prepend(ret->skills, cast(Object *, cast(struct aststr *, iter->r)->str));
    iter = iter->l;
  }

  return ret;
}

static PackageObj *newPackage(struct ast *a) {
  checktype(a->nodetype, N_Package);

  PackageObj *ret = malloc(sizeof(PackageObj));
  ret->objtype = Obj_Package;

  struct astpackage *p = (struct astpackage *)a;
  char buf[64];
  sprintf(buf, "%sp%d", readfile_name, p->uid);
  ret->id = strdup(buf);
  char *dst = cast(struct aststr *, p->id)->str;
  addTranslation(ret->id, dst);
  sym_new_entry(dst, TPackage, ret->id, false);
  ret->internal_id = p->uid;
  ret->generals = list_new();
  struct ast *iter = p->generals;
  checktype(iter->nodetype, N_Generals);
  while (iter && iter->r) {
    list_prepend(ret->generals, cast(Object *, newGeneral(iter->r)));
    iter = iter->l;
  }

  return ret;
}

/* main */
ExtensionObj *newExtension(struct ast *a) {
  checktype(a->nodetype, N_Extension);

  ExtensionObj *ret = malloc(sizeof(ExtensionObj));
  ret->objtype = Obj_Extension;
  struct ast *iter;

  sym_init();
  strtab = hash_new();
  restrtab = list_new();

  ret->skills = list_new();
  iter = a->l;
  checktype(iter->nodetype, N_Skills);
  while (iter && iter->r) {
    list_prepend(ret->skills, cast(Object *, newSkill(iter->r)));
    iter = iter->l;
  }

  ret->packages = list_new();
  iter = a->r;
  checktype(iter->nodetype, N_Packages);
  while (iter && iter->r) {
    list_prepend(ret->packages, cast(Object *, newPackage(iter->r)));
    iter = iter->l;
  }
  return ret;
}

