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
      } else if (!strcmp(field, "当前阶段")) {
        fprintf(yyout, ":getPhase()");
        return TNumber;
      } else if (!strcmp(field, "身份")) {
        fprintf(yyout, ":getRoleEnum()");
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
      } else if (!strcmp(field, "类别")) {
        fprintf(yyout, ":getTypeId()");
        return TNumber;
      } else if (!strcmp(field, "牌名")) {
        fprintf(yyout, ":objectName()");
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

static int analyzeArray(struct ast *a) {
  if (!a) return TNone;
  checktype(a->nodetype, N_Exps);
  int ret = TNone;

  /* determine array type first */
  ret = analyzeArray(a->l);
  int arraytype = analyzeExp(a->r);
  fprintf(yyout, ", ");
  if (ret != TNone && arraytype != ret) {
    fprintf(stderr, "错误：数组中不能有类别不同的元素(预期类型%d，实际得到%d)\n", ret, arraytype);
    exit(1);
  }
  ret = arraytype;
  return ret;
}

/* return the type of exp */
int analyzeExp(struct ast *a) {
  checktype(a->nodetype, N_Exp);

  int ret = TNone;
  int t;

  struct astExp *e = (struct astExp *)a;
  if (e->bracketed) fprintf(yyout, "(");

  if ((e->exptype == ExpCalc || e->exptype == ExpCmp) && e->optype != 0) {
    if (e->optype == 3 || e->optype == 4) {
      t = analyzeExp((struct ast *)(e->l));
      if (t == TPlayer) {
        fprintf(yyout, ":objectName()");
      }
      switch (e->optype) {
        case 3: fprintf(yyout, " ~= "); break;
        case 4: fprintf(yyout, " == "); break;
      }
      t = analyzeExp((struct ast *)(e->r));
      if (t == TPlayer) {
        fprintf(yyout, ":objectName()");
      }
      ret = TBool;
    } else {
      t = analyzeExp((struct ast *)(e->l));
      checktype(t, TNumber);
      switch (e->optype) {
        case 1: fprintf(yyout, " > "); ret = TBool; break;
        case 2: fprintf(yyout, " < "); ret = TBool; break;
        case 5: fprintf(yyout, " >= "); ret = TBool; break;
        case 6: fprintf(yyout, " <= "); ret = TBool; break;
        default: fprintf(yyout, " %c ", e->optype); ret = TNumber; break;
      }
      t = analyzeExp((struct ast *)(e->r));
      checktype(t, TNumber);
    }
  } else if (e->exptype == ExpLogic) {
    analyzeExp((struct ast *)(e->l));
    switch (e->optype) {
      case 7: fprintf(yyout, " and "); break;
      case 8: fprintf(yyout, " or "); break;
    }
    analyzeExp((struct ast *)(e->r));
    ret = TBool;
  } else if (e->exptype == ExpArray) {
    fprintf(yyout, "fkp.newlist{");
    t = analyzeArray((struct ast *)(e->l));
    fprintf(yyout, "}");
    switch (t) {
      case TPlayer: ret = TPlayerList; break;
      case TCard: ret = TCardList; break;
      case TNumber: ret = TNumberList; break;
      case TString: ret = TStringList; break;
      default:
        fprintf(stderr, "错误：未知的数组元素类型%d\n", t);
        exit(1);
    }
  } else switch (e->exptype) {
    case ExpNum: fprintf(yyout, "%lld" ,e->value); ret = TNumber; break;
    case ExpBool: fprintf(yyout, "%s" ,e->value == 0 ? "false" : "true"); ret = TBool; break;
    case ExpStr: fprintf(yyout, "'%s'", ((struct aststr *)(e->l))->str); ret = TString; break;
    case ExpVar: ret = analyzeVar((struct ast *)(e->l)); break;
    case ExpAction: ret = analyzeAction((struct ast *)(e->l)); break;
    default: fprintf(stderr, "unknown exptype %d\n", e->exptype); exit(1);
  }

  if (e->bracketed) fprintf(yyout, ")");

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

static struct {
  char *markname;
  char *markid;
  int hidden;
} MarkTable[999];

static int MarkId = 0;

static char *getMarkId(char *k, int hidden) {
  for (int i = 0; i < MarkId; i++) {
    if (MarkTable[i].hidden == hidden && !strcmp(MarkTable[i].markname, k)) {
      return MarkTable[i].markid;
    }
  }
  char buf[64];
  sprintf(buf, "%c%s_mark%d", hidden ? '_' : '@', readfile_name, MarkId);
  MarkTable[MarkId].markname = strdup(k);
  MarkTable[MarkId].markid = strdup(buf);
  if (!hidden) addTranslation(buf, k);
  return MarkTable[MarkId++].markid;
}

static int analyzeActionMark(struct actionMark *m) {
  int ret = TNone;
  int t;
  char *internal_id = getMarkId(m->name->str, m->hidden);
  switch (m->optype) {
    case 1:
      if (m->hidden) {
        fprintf(yyout, "room:addPlayerMark(");
        t = analyzeExp(m->player); checktype(t, TPlayer);
        fprintf(yyout, ", \"%s\", ", internal_id);
        t = analyzeExp(m->num); checktype(t, TNumber);
        fprintf(yyout, ")");
      } else {
        t = analyzeExp(m->player); checktype(t, TPlayer);
        fprintf(yyout, ":gainMark(\"%s\", ", internal_id);
        t = analyzeExp(m->num); checktype(t, TNumber);
        fprintf(yyout, ")");
      }
      break;
    case 2:
      if (m->hidden) {
        fprintf(yyout, "room:removePlayerMark(");
        t = analyzeExp(m->player); checktype(t, TPlayer);
        fprintf(yyout, ", \"%s\", ", internal_id);
        t = analyzeExp(m->num); checktype(t, TNumber);
        fprintf(yyout, ")");
      } else {
        t = analyzeExp(m->player); checktype(t, TPlayer);
        fprintf(yyout, ":loseMark(\"%s\", ", internal_id);
        t = analyzeExp(m->num); checktype(t, TNumber);
        fprintf(yyout, ")");
      }
      break;
    case 3:
      t = analyzeExp(m->player); checktype(t, TPlayer);
      fprintf(yyout, ":getMark(\"%s\")", internal_id);
      ret = TNumber;
      break;
    default:
      break;
  }
  return ret;
}

int analyzeAction(struct ast *a) {
  checktype(a->nodetype, N_Stat_Action);

  int ret = TNone;
  int t;
  char buf[1024];

  struct astAction *s = (struct astAction *)a;
  struct ast *action = s->action;
  struct actionMark *m;

  if (s->standalone) print_indent();

  switch (s->actiontype) {
    case ActionDrawcard:
      t = analyzeExp(action->l); checktype(t, TPlayer);
      fprintf(yyout, ":drawCards(");
      t = analyzeExp(action->r); checktype(t, TNumber);
      fprintf(yyout, ", self:objectName())");
      break;
    case ActionLosehp:
      fprintf(yyout, "room:loseHp(");
      t = analyzeExp(action->l); checktype(t, TPlayer);
      fprintf(yyout, ", ");
      t = analyzeExp(action->r); checktype(t, TNumber);
      fprintf(yyout, ")");
      break;
    case ActionDamage:
      fprintf(yyout, "room:damage(sgs.DamageStruct(self:objectName(), ");
      struct actionDamage *d = (struct actionDamage *)action;
      if (d->src) {
        t = analyzeExp(d->src); checktype(t, TPlayer);
      } else fprintf(yyout, "nil");
      fprintf(yyout, ", ");
      t = analyzeExp(d->dst); checktype(t, TPlayer);
      fprintf(yyout, ", ");
      t = analyzeExp(d->num); checktype(t, TNumber);
      fprintf(yyout, "))");
      break;
    case ActionRecover:
      fprintf(yyout, "room:recover(");
      t = analyzeExp(action->l); checktype(t, TPlayer);
      fprintf(yyout, ", sgs.RecoverStruct(nil, nil, ");
      t = analyzeExp(action->r); checktype(t, TNumber);
      fprintf(yyout, "))");
      break;
    case ActionAcquireSkill:
      fprintf(yyout, "room:acquireSkill(");
      t = analyzeExp(action->l); checktype(t, TPlayer);
      fprintf(yyout, ", ");
      t = analyzeExp(action->r); checktype(t, TString);
      fprintf(yyout, ")");
      break;
    case ActionDetachSkill:
      fprintf(yyout, "room:detachSkillFromPlayer(");
      t = analyzeExp(action->l); checktype(t, TPlayer);
      fprintf(yyout, ", ");
      t = analyzeExp(action->r); checktype(t, TString);
      fprintf(yyout, ")");
      break;
    case ActionMark:
      m = (struct actionMark *)action;
      ret = analyzeActionMark(m);
      break;
    case ActionAskForChoice:
      fprintf(yyout, "room:askForChoice(");
      t = analyzeExp(action->l); checktype(t, TPlayer);
      fprintf(yyout, ", self:objectName(), table.concat(");
      t = analyzeExp(action->r); checktype(t, TStringList);
      fprintf(yyout, ", \"+\"), data)");
      ret = TString;
      break;
    case ActionAskForPlayerChosen:
      fprintf(yyout, "room:askForPlayerChosen(");
      t = analyzeExp(action->l); checktype(t, TPlayer);
      fprintf(yyout, ", ");
      t = analyzeExp(action->r); checktype(t, TPlayerList);
      fprintf(yyout, ", self:objectName(), nil, false, true)");
      ret = TPlayer;
      break;
    default:
      fprintf(stderr, "unexpected action type %d\n", s->nodetype); break;
  }

  if (s->standalone) fprintf(yyout, "\n");

  return ret;
}
