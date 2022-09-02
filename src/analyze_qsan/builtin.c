#include "builtin.h"

static void loadfuncdef(Proto *p) {
  FuncdefObj *def = newFuncdef(strdup(p->src), NULL, p->rettype, NULL);
  sym_new_entry(p->dst, TFunc, cast(const char *, def), true);

  List *l = list_new();
  for (int i = 0; i < p->argcount; i++) {
    struct ProtoArg *arg = &p->args[i];
    DefargObj *defarg = newDefarg(strdup(arg->name), arg->argtype, NULL);

    if (!arg->have_default) {
      defarg->d = NULL;
    } else {
      ExpressionObj *e = newExpression(ExpVar, 0, 0, NULL, NULL);
      e->valuetype = arg->argtype;
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
        v = newVar(strdup(arg->d.s), NULL);
        v->type = arg->argtype;
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
