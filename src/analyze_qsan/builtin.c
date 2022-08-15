#include "builtin.h"

static void loadfuncdef(Proto *p) {
  FuncdefObj *def = malloc(sizeof(FuncdefObj));
  def->objtype = Obj_Funcdef;
  def->funcname = strdup(p->src);
  sym_new_entry(p->dst, TFunc, cast(const char *, def), true);
  def->rettype = p->rettype;
  def->funcbody = NULL;

  List *l = list_new();
  for (int i = 0; i < p->argcount; i++) {
    struct ProtoArg *arg = &p->args[i];
    DefargObj *defarg = malloc(sizeof(DefargObj));
    defarg->first_line = -1;
    defarg->objtype = Obj_Defarg;
    defarg->name = strdup(arg->name);
    defarg->type = arg->argtype;

    if (!arg->have_default) {
      defarg->d = NULL;
    } else {
      ExpressionObj *e = malloc(sizeof(ExpressionObj));
      e->objtype = Obj_Expression;
      e->valuetype = arg->argtype;
      e->optype = 0;
      e->strvalue = NULL;
      e->varValue = NULL;
      e->func = NULL;
      e->array = NULL;
      e->oprand1 = NULL;
      e->oprand2 = NULL;
      e->bracketed = false;
      e->param_name = strdup(arg->name);

      VarObj *v;
      switch (arg->argtype) {
      case TNumber:
        e->exptype = ExpNum;
        e->value = arg->d.n;
        break;
      case TString:
        e->exptype = ExpStr;
        e->strvalue = strdup(arg->d.s);
        break;
      case TBool:
        e->exptype = ExpBool;
        e->value = arg->d.n;
        break;
      default:
        e->exptype = ExpVar;
        v = malloc(sizeof(VarObj));
        v->objtype = Obj_Var;
        v->name = strdup(arg->d.s);
        v->type = arg->argtype;
        v->obj = NULL;
        e->varValue = v;
        break;
      }
      defarg->d = e;
    }
    list_append(l, cast(Object *, defarg));
  }
  def->params = l;
}

static void loadvar(BuiltinVar *v) {
  sym_new_entry(v->dst, v->type, v->src, true);
}

void loadmodule(Proto *ps, BuiltinVar *vs) {
  if (ps != NULL)
    for (int i = 0; ; i++) {
      Proto *p = &ps[i];
      if (p->dst == NULL) break;
      loadfuncdef(p);
    }

  if (vs != NULL)
    for (int i = 0; ; i++) {
      if (vs[i].dst == NULL) break;
      loadvar(&vs[i]);
    }
}

typedef void (*InitFunc)();
static InitFunc init_funcs[] = {
  load_builtin_action,
  load_builtin_cards,
  load_builtin_enum,
  load_builtin_func,
  load_builtin_getter,
  load_builtin_interaction,
  load_builtin_util,
  NULL
};

void sym_init() {
  if (builtin_symtab != NULL) return;
  builtin_symtab = hash_new();
  symtab_stack = stack_new();
  stack_push(symtab_stack, cast(Object *, builtin_symtab));
  current_tab = builtin_symtab;
  last_lookup_tab = NULL;

  for (int i = 0; ; i++) {
    if (init_funcs[i] == NULL)
      break;
    init_funcs[i]();
  }
}
