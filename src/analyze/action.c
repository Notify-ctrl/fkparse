#include "action.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static DrawcardAct *newDrawcard(struct astAction *a, struct ast *params) {
  checktype(a->actiontype, ActionDrawcard);

  DrawcardAct *ret = malloc(sizeof(DrawcardAct));
  ret->objtype = Obj_ActionBody;

  ret->player = newExpression(a->action->l);
  checktype(ret->player->valuetype, TPlayer);

  ret->number = newExpression(a->action->r);
  checktype(ret->number->valuetype, TNumber);

  ret->reason = NULL;
  unused(params);

  return ret;
}

static LoseHpAct *newLoseHp(struct astAction *a) {
  checktype(a->actiontype, ActionLosehp);

  LoseHpAct *ret = malloc(sizeof(LoseHpAct));
  ret->objtype = Obj_ActionBody;

  ret->player = newExpression(a->action->l);
  checktype(ret->player->valuetype, TPlayer);

  ret->number = newExpression(a->action->r);
  checktype(ret->number->valuetype, TNumber);

  return ret;
}

static DamageAct *newDamage(struct astAction *a, struct ast *params) {
  checktype(a->actiontype, ActionDamage);

  DamageAct *ret = malloc(sizeof(DamageAct));
  ret->objtype = Obj_ActionBody;
  struct actionDamage *d = cast(struct actionDamage *, a->action);

  ret->from = newExpression(d->src);
  checktype(ret->from->valuetype, TPlayer);

  ret->to = newExpression(d->dst);
  checktype(ret->to->valuetype, TPlayer);

  ret->damage = newExpression(d->num);
  checktype(ret->damage->valuetype, TNumber);

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
  checktype(ret->player->valuetype, TPlayer);

  ret->recover = newExpression(a->action->r);
  checktype(ret->recover->valuetype, TNumber);

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
  checktype(ret->player->valuetype, TPlayer);
  
  ret->skill = newExpression(a->action->r);
  checktype(ret->skill->valuetype, TString);

  return ret;
}

static MarkAct *newMark(struct astAction *a) {
  checktype(a->actiontype, ActionMark);

  MarkAct *ret = malloc(sizeof(MarkAct));
  ret->objtype = Obj_ActionBody;
  struct actionMark *m = cast(struct actionMark *, a->action);
  ret->hidden = m->hidden;
  ret->name = m->name->str;
  ret->optype = m->optype;

  ret->number = newExpression(m->num);
  checktype(ret->number->valuetype, TNumber);

  ret->player = newExpression(m->player);
  checktype(ret->player->valuetype, TPlayer);

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
    break;
  case ActionLosehp:
    ret->action = cast(Object *, newLoseHp(as));
    break;
  case ActionDamage:
    ret->action = cast(Object *, newDamage(as, a->r));
    break;
  case ActionRecover:
    ret->action = cast(Object *, newRecover(as, a->r));
    break;
  case ActionAcquireSkill:
    ret->action = cast(Object *, newAcquireDetachSkill(as, 1));
    break;
  case ActionDetachSkill:
    ret->action = cast(Object *, newAcquireDetachSkill(as, 0));
    break;
  case ActionMark:
    ret->action = cast(Object *, newMark(as));
    break;
  case ActionAskForChoice:
    ret->action = cast(Object *, 0);
    break;
  case ActionAskForPlayerChosen:
    ret->action = cast(Object *, 0);
    break;
  }

  return ret;
}
