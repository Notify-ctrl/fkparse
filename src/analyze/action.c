#include "action.h"
#include "main.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static DrawcardAct *newDrawcard(struct astAction *a, struct ast *params) {
  checktype(a->actiontype, ActionDrawcard);

  DrawcardAct *ret = malloc(sizeof(DrawcardAct));
  ret->objtype = Obj_ActionBody;
  ret->player = newExpression(a->action->l);
  ret->number = newExpression(a->action->r);
  ret->reason = NULL;
  unused(params);

  return ret;
}

static LoseHpAct *newLoseHp(struct astAction *a) {
  checktype(a->actiontype, ActionLosehp);

  LoseHpAct *ret = malloc(sizeof(LoseHpAct));
  ret->objtype = Obj_ActionBody;
  ret->player = newExpression(a->action->l);
  ret->number = newExpression(a->action->r);
  return ret;
}

static DamageAct *newDamage(struct astAction *a, struct ast *params) {
  checktype(a->actiontype, ActionDamage);

  DamageAct *ret = malloc(sizeof(DamageAct));
  ret->objtype = Obj_ActionBody;
  struct actionDamage *d = cast(struct actionDamage *, a->action);
  ret->from = newExpression(d->src);
  ret->to = newExpression(d->dst);
  ret->damage = newExpression(d->num);
  ret->reason = NULL;
  ret->nature = NULL;
  unused(params);

  return ret;
}

static RecoverAct *newRecover(struct astAction *a, struct ast *params) {
  checktype(a->actiontype, ActionRecover);

  RecoverAct *ret = malloc(sizeof(RecoverAct));
  ret->objtype = Obj_ActionBody;
  ret->player = newExpression(a->action->l);
  ret->recover = newExpression(a->action->r);
  ret->who = NULL;
  ret->card = NULL;
  unused(params);

  return ret;
}

static AcquireDetachSkillAct *newAcquireDetachSkill(
  struct astAction *a,
  int isAcquire
) {
  AcquireDetachSkillAct *ret = malloc(sizeof(AcquireDetachSkillAct));
  ret->objtype = Obj_ActionBody;
  ret->isAcquire = isAcquire;
  ret->player = newExpression(a->action->l);
  ret->skill = newExpression(a->action->r);

  return ret;
}

static MarkAct *newMark(struct astAction *a) {
  static int markId = 0;
  checktype(a->actiontype, ActionMark);

  MarkAct *ret = malloc(sizeof(MarkAct));
  ret->objtype = Obj_ActionBody;
  struct actionMark *m = cast(struct actionMark *, a->action);
  ret->hidden = m->hidden;
  ret->name = m->name->str;
  ret->optype = m->optype;

  symtab_item *i = sym_lookup(ret->name);
  if (!i) {
    char buf[64];
    sprintf(buf, "@%s_mark%d", readfile_name, markId);
    addTranslation(strdup(buf), ret->name);
    sym_new_entry(ret->name, TMark, strdup(buf), false);
  } else if (i->type != TMark) {
    /* error */
  }

  ret->number = newExpression(m->num);
  ret->player = newExpression(m->player);

  return ret;
}

static AskForChoiceAct *newAskForChoice(struct astAction *a) {
  checktype(a->actiontype, ActionAskForChoice);

  AskForChoiceAct *ret = malloc(sizeof(AskForChoiceAct));
  ret->player = newExpression(a->action->l);
  ret->choices = newExpression(a->action->r);

  return ret;
}

static AskForChoosePlayerAct *newAskForChoosePlayer(struct astAction *a) {
  checktype(a->actiontype, ActionAskForPlayerChosen);

  AskForChoosePlayerAct *ret = malloc(sizeof(AskForChoosePlayerAct));
  ret->player = newExpression(a->action->l);
  ret->targets = newExpression(a->action->r);

  return ret;
}

static AskForSkillInvokeAct *newAskForSkillInvoke(struct astAction *a) {
  checktype(a->actiontype, ActionAskForSkillInvoke);

  AskForSkillInvokeAct *ret = malloc(sizeof(AskForSkillInvokeAct));
  ret->player = newExpression(a->action->l);
  ret->skill_name = cast(struct aststr *, a->action->r)->str;

  return ret;
}

static ObtainCardAct *newObtainCard(struct astAction *a) {
  checktype(a->actiontype, ActionObtainCard);

  ObtainCardAct *ret = malloc(sizeof(ObtainCardAct));
  ret->player = newExpression(a->action->l);
  ret->card = newExpression(a->action->r);

  return ret;
}

ActionObj *newAction(struct ast *a) {
  checktype(a->nodetype, N_Stat_Action);

  ActionObj *ret = malloc(sizeof(ActionObj));
  ret->objtype = Obj_Action;
  struct astAction *as = cast(struct astAction *, a->l);
  ret->actiontype = as->actiontype;
  ret->standalone = as->standalone;

  switch (as->actiontype) {
  case ActionDrawcard:
    ret->action = cast(Object *, newDrawcard(as, a->r));
    ret->valuetype = TNone;
    break;
  case ActionLosehp:
    ret->action = cast(Object *, newLoseHp(as));
    ret->valuetype = TNone;
    break;
  case ActionDamage:
    ret->action = cast(Object *, newDamage(as, a->r));
    ret->valuetype = TNone;
    break;
  case ActionRecover:
    ret->action = cast(Object *, newRecover(as, a->r));
    ret->valuetype = TNone;
    break;
  case ActionAcquireSkill:
    ret->action = cast(Object *, newAcquireDetachSkill(as, 1));
    ret->valuetype = TNone;
    break;
  case ActionDetachSkill:
    ret->action = cast(Object *, newAcquireDetachSkill(as, 0));
    ret->valuetype = TNone;
    break;
  case ActionMark:
    ret->action = cast(Object *, newMark(as));
    ret->valuetype = cast(MarkAct *, ret->action)->optype == 3 ? TNumber : TNone;
    break;
  case ActionAskForChoice:
    ret->action = cast(Object *, newAskForChoice(as));
    ret->valuetype = TString;
    break;
  case ActionAskForPlayerChosen:
    ret->action = cast(Object *, newAskForChoosePlayer(as));
    ret->valuetype = TPlayer;
    break;
  case ActionAskForSkillInvoke:
    ret->action = cast(Object *, newAskForSkillInvoke(as));
    ret->valuetype = TBool;
    break;
  case ActionObtainCard:
    ret->action = cast(Object *, newObtainCard(as));
    ret->valuetype = TNone;
    break;
  }

  return ret;
}

/* -------------------------------- */

#include "generate.h"

static void actDrawcard(Object *o) {
  DrawcardAct *d = cast(DrawcardAct *, o);
  analyzeExp(d->player);
  checktype(d->player->valuetype, TPlayer);
  writestr(":drawCards(");
  analyzeExp(d->number);
  checktype(d->number->valuetype, TNumber);
  writestr(", self:objectName())");
}

static void actLosehp(Object *o) {
  LoseHpAct *d = cast(LoseHpAct *, o);
  writestr("room:loseHp(");
  analyzeExp(d->player);
  checktype(d->player->valuetype, TPlayer);
  writestr(", ");
  analyzeExp(d->number);
  checktype(d->number->valuetype, TNumber);
  writestr(")");
}

static void actDamage(Object *o) {
  writestr("room:damage(sgs.DamageStruct(self:objectName(), ");
  DamageAct *d = cast(DamageAct *, o);
  if (d->from) {
    analyzeExp(d->from); checktype(d->from->valuetype, TPlayer);
  } else writestr("nil");
  writestr(", ");
  analyzeExp(d->to); checktype(d->to->valuetype, TPlayer);
  writestr(", ");
  analyzeExp(d->damage); checktype(d->damage->valuetype, TNumber);
  writestr("))");
}

static void actRecover(Object *o) {
  RecoverAct *r = cast(RecoverAct *, o);
  writestr("room:recover(");
  analyzeExp(r->player); checktype(r->player->valuetype, TPlayer);
  writestr(", sgs.RecoverStruct(nil, nil, ");
  analyzeExp(r->recover); checktype(r->recover->valuetype, TNumber);
  writestr("))");
}

static void actAcquireSkill(Object *o) {
  AcquireDetachSkillAct *a = cast(AcquireDetachSkillAct *, o);
  writestr("room:acquireSkill(");
  analyzeExp(a->player); checktype(a->player->valuetype, TPlayer);
  writestr(", ");
  analyzeExp(a->skill); checktype(a->skill->valuetype, TString);
  writestr(")");
}

static void actDetachSkill(Object *o) {
  AcquireDetachSkillAct *a = cast(AcquireDetachSkillAct *, o);
  writestr("room:detachSkillFromPlayer(");
  analyzeExp(a->player); checktype(a->player->valuetype, TPlayer);
  writestr(", ");
  analyzeExp(a->skill); checktype(a->skill->valuetype, TString);
  writestr(")");
}

static void actMark(Object *o) {
  MarkAct *m = cast(MarkAct *, o);
  char internal_id[64];
  strncpy(internal_id, sym_lookup(m->name)->origtext, 64);
  internal_id[0] = m->hidden ? '%' : '@';
  switch (m->optype) {
    case 1:
      if (m->hidden) {
        writestr("room:addPlayerMark(");
        analyzeExp(m->player); checktype(m->player->valuetype, TPlayer);
        writestr(", \"%s\", ", internal_id);
        analyzeExp(m->number); checktype(m->number->valuetype, TNumber);
        writestr(")");
      } else {
        analyzeExp(m->player); checktype(m->player->valuetype, TPlayer);
        writestr(":gainMark(\"%s\", ", internal_id);
        analyzeExp(m->number); checktype(m->number->valuetype, TNumber);
        writestr(")");
      }
      break;
    case 2:
      if (m->hidden) {
        writestr("room:removePlayerMark(");
        analyzeExp(m->player); checktype(m->player->valuetype, TPlayer);
        writestr(", \"%s\", ", internal_id);
        analyzeExp(m->number); checktype(m->number->valuetype, TNumber);
        writestr(")");
      } else {
        analyzeExp(m->player); checktype(m->player->valuetype, TPlayer);
        writestr(":loseMark(\"%s\", ", internal_id);
        analyzeExp(m->number); checktype(m->number->valuetype, TNumber);
        writestr(")");
      }
      break;
    case 3:
      analyzeExp(m->player); checktype(m->player->valuetype, TPlayer);
      writestr(":getMark(\"%s\")", internal_id);
      break;
    default:
      break;
  }
}

static void actAskForChoice(Object *o) {
  AskForChoiceAct *a = cast(AskForChoiceAct *, o);
  writestr("room:askForChoice(");
  analyzeExp(a->player); checktype(a->player->valuetype, TPlayer);
  writestr(", self:objectName(), table.concat(");
  analyzeExp(a->choices); checktype(a->choices->valuetype, TStringList);
  writestr(", \"+\"), data)");
}

static void actAskForPlayerChosen(Object *o) {
  AskForChoosePlayerAct *a = cast(AskForChoosePlayerAct *, o);
  writestr("room:askForPlayerChosen(");
  analyzeExp(a->player); checktype(a->player->valuetype, TPlayer);
  writestr(", ");
  analyzeExp(a->targets); checktype(a->targets->valuetype, TPlayerList);
  writestr(", self:objectName(), nil, false, true)");
}

static void actAskForSkillInvoke(Object *o) {
  AskForSkillInvokeAct *a = cast(AskForSkillInvokeAct *, o);
  writestr("room:askForSkillInvoke(");
  analyzeExp(a->player); checktype(a->player->valuetype, TPlayer);
  writestr(", \"%s\")", sym_lookup(a->skill_name)->origtext);
}

static void actObtainCard(Object *o) {
  ObtainCardAct *d = cast(ObtainCardAct *, o);
  analyzeExp(d->player);
  checktype(d->player->valuetype, TPlayer);
  writestr(":obtainCard(");
  analyzeExp(d->card);
  checktype(d->card->valuetype, TCard);
  writestr(", false)");
}

typedef void(*ActFunc)(Object *);
static ActFunc act_func_table[] = {
  actDrawcard,
  actLosehp,
  actDamage,
  actRecover,
  actAcquireSkill,
  actDetachSkill,
  actMark,
  actAskForChoice,
  actAskForPlayerChosen,
  actAskForSkillInvoke,
  actObtainCard
};

void analyzeAction(ActionObj *a) {
  if (a->standalone)
    print_indent();

  ActFunc f = act_func_table[a->actiontype];
  (*f)(a->action);

  if (a->standalone)
    writestr("\n");
}

