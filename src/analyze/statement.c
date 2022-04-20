#include "analyzer.h"

static int getField(int objtype, char *field) {
  switch (objtype) {
    case TPlayer:
      if (!strcmp(field, "体力值")) {
        fprintf(yyout, ":getHp()");
        return TNumber;
      } else if (!strcmp(field, "手牌数")) {
        fprintf(yyout, ":getHandcardNum()");
        return TNumber;
      } else if (!strcmp(field, "体力上限")) {
        fprintf(yyout, ":getMaxHp()");
        return TNumber;
      } else {
        fprintf(stderr, "无法获取 玩家 的属性 \"%s\"\n", field);
        exit(1);
      }
    case TCard:
      if (!strcmp(field, "点数")) {
        fprintf(yyout, ":getNumber()");
        return TNumber;
      } else if (!strcmp(field, "花色")) {
        fprintf(yyout, ":getSuit()");
        return TNumber;
      } else {
        fprintf(stderr, "无法获取 卡牌 的属性 \"%s\"\n", field);
        exit(1);
      }
    default:
      fprintf(stderr, "错误：不能获取类型为%d的对象的属性\n", objtype);
      exit(1);
  }
}

int analyzeVar(struct ast *a) {
  checktype(a->nodetype, N_Var);

  int ret = TAny;
  struct astVar *v = (struct astVar *)a;
  int t;

  char *s = v->name->str;

  if (v->obj) {
    t = analyzeExp((struct ast *)(v->obj));
    return getField(t, s);
  } else {
    if (isReserved(s)) {
      t = analyzeReserved(s);
    } else {
      fprintf(yyout, "locals[\"%s\"]", v->name->str);
      t = lookup(v->name->str)->type;
    }
    if (t == TNone) {
      fprintf(stderr, "错误：标识符\"%s\"尚未定义\n", v->name->str);
      exit(1);
    }
    return t;
  }

  return ret;
}

/* return the type of exp */
int analyzeExp(struct ast *a) {
  checktype(a->nodetype, N_Exp);

  int ret = TNone;
  int t;

  struct astExp *e = (struct astExp *)a;
  if ((e->exptype == ExpCalc || e->exptype == ExpCmp) && e->optype != 0) {
    t = analyzeExp((struct ast *)(e->l));
    checktype(t, TNumber);
    switch (e->optype) {
      case 1: fprintf(yyout, " > "); break;
      case 2: fprintf(yyout, " < "); break;
      case 3: fprintf(yyout, " ~= "); break;
      case 4: fprintf(yyout, " == "); break;
      case 5: fprintf(yyout, " >= "); break;
      case 6: fprintf(yyout, " <= "); break;
      default: fprintf(yyout, " %c " ,e->optype); break;
    }
    t = analyzeExp((struct ast *)(e->r));
    checktype(t, TNumber);
    return TNumber;
  }

  switch (e->exptype) {
    case ExpNum: fprintf(yyout, "%lld" ,e->value); return TNumber;
    case ExpBool: fprintf(yyout, "%s" ,e->value == 0 ? "false" : "true"); return TNumber;
    case ExpStr: fprintf(yyout, "'%s'", ((struct aststr *)(e->l))->str); return TString;
    case ExpVar: return analyzeVar((struct ast *)(e->l));
    case ExpAction: return analyzeAction((struct ast *)(e->l));
    default: fprintf(stderr, "unknown exptype %d\n", e->exptype); exit(1);
  }

  return ret;
}

void analyzeBlock(struct ast *a) {
  checktype(a->nodetype, N_Block);

  analyzeStats(a->l);
}

void analyzeStats(struct ast *a) {
  checktype(a->nodetype, N_Stats);

  struct ast *stat;

  if (a->l) {
    analyzeStats(a->l);
    stat = a->r;
    switch (stat->nodetype) {
      case N_Stat_None: writeline(";"); break;
      case N_Stat_Assign: analyzeStatAssign(stat); break;
      case N_Stat_If: analyzeIf(stat); break;
      case N_Stat_Loop: analyzeLoop(stat); break;
      case N_Stat_Break: writeline("break"); break;
      case N_Stat_Ret: break;
      case N_Stat_Action: analyzeAction(stat); break;
      default: fprintf(stderr, "unexpected statement type %d\n", stat->nodetype); break;
    }
  }
}

void analyzeStatAssign(struct ast *a) {
  checktype(a->nodetype, N_Stat_Assign);

  int t;
  print_indent();
  lookup(((struct astVar *)(a->l))->name->str)->type = TAny;
  analyzeVar(a->l);
  fprintf(yyout, " = ");
  t = analyzeExp(a->r);
  struct astVar *v = (struct astVar *)(a->l);
  if (v->obj == NULL) {
    lookup(v->name->str)->type = t;
    if (isReserved(v->name->str)) {
      fprintf(stderr, "错误：不允许重定义标识符 \"%s\"\n", v->name->str);
      exit(1);
    }
  }
  fprintf(yyout, "\n");
}

void analyzeIf(struct ast *a) {
  checktype(a->nodetype, N_Stat_If);

  struct astIf *s = (struct astIf *)a;
  print_indent();
  fprintf(yyout, "if ");
  analyzeExp(s->cond);
  fprintf(yyout, " then\n");
  indent_level++;
  analyzeBlock(s->then);
  if (s->el) {
    indent_level--;
    writeline("else");
    indent_level++;
    analyzeBlock(s->el);
  }
  indent_level--;
  writeline("end");
}

void analyzeLoop(struct ast *a) {
  checktype(a->nodetype, N_Stat_Loop);

  struct astLoop *s = (struct astLoop *)a;
  writeline("repeat");
  indent_level++;
  analyzeBlock(s->body);
  indent_level--;
  print_indent();
  fprintf(yyout, "until ");
  analyzeExp(s->cond);
  fprintf(yyout, "\n");
}

int analyzeAction(struct ast *a) {
  checktype(a->nodetype, N_Stat_Action);

  int ret = TNone;
  int t;

  struct astAction *s = (struct astAction *)a;
  struct ast *action = s->action;
  switch (s->actiontype) {
    case ActionDrawcard:
      print_indent();
      t = analyzeExp(action->l);
      checktype(t, TPlayer);
      fprintf(yyout, ":drawCards(");
      t = analyzeExp(action->r);
      checktype(t, TNumber);
      fprintf(yyout, ", self:objectName())\n");
      break;
    case ActionLosehp:
      print_indent();
      fprintf(yyout, "room:loseHp(");
      t = analyzeExp(action->l);
      checktype(t, TPlayer);
      fprintf(yyout, ", ");
      t = analyzeExp(action->r);
      checktype(t, TNumber);
      fprintf(yyout, ")\n");
      break;
    case ActionDamage:
      print_indent();
      fprintf(yyout, "room:damage(sgs.DamageStruct(self:objectName(), ");
      struct actionDamage *d = (struct actionDamage *)action;
      if (d->src) {
        t = analyzeExp(d->src);
        checktype(t, TPlayer);
      } else fprintf(yyout, "nil");
      fprintf(yyout, ", ");
      t = analyzeExp(d->dst); checktype(t, TPlayer); fprintf(yyout, ", ");
      t = analyzeExp(d->num); checktype(t, TNumber); fprintf(yyout, "))\n");
      break;
    case ActionRecover:
      print_indent();
      fprintf(yyout, "room:recover(");
      t = analyzeExp(action->l);
      checktype(t, TPlayer);
      fprintf(yyout, ", sgs.RecoverStruct(nil, nil, ");
      t = analyzeExp(action->r);
      checktype(t, TNumber);
      fprintf(yyout, "))\n");
      break;
    case ActionAcquireSkill:
      print_indent();
      fprintf(yyout, "room:acquireSkill(");
      t = analyzeExp(action->l);
      checktype(t, TPlayer);
      fprintf(yyout, ", ");
      t = analyzeExp(action->r);
      checktype(t, TString);
      fprintf(yyout, ")\n");
      break;
    case ActionDetachSkill:
      print_indent();
      fprintf(yyout, "room:detachSkillFromPlayer(");
      t = analyzeExp(action->l);
      checktype(t, TPlayer);
      fprintf(yyout, ", ");
      t = analyzeExp(action->r);
      checktype(t, TString);
      fprintf(yyout, ")\n");
      break;
    default:
      fprintf(stderr, "unexpected action type %d\n", s->nodetype); break;
  }

  return ret;
}
