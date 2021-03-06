#ifndef _ACTION_H
#define _ACTION_H

#include "structs.h"
#include "object.h"
#include "ast.h"

typedef struct {
  ObjType objtype;
  ExpressionObj *player;
  ExpressionObj *number;
  ExpressionObj *reason;
} DrawcardAct;

typedef struct {
  ObjType objtype;
  ExpressionObj *player;
  ExpressionObj *number;
} LoseHpAct;

typedef struct {
  ObjType objtype;
  ExpressionObj *from;
  ExpressionObj *to;
  ExpressionObj *card;
  ExpressionObj *damage;
  ExpressionObj *nature;
  ExpressionObj *chain;
  ExpressionObj *transfer;
  ExpressionObj *by_user;
  ExpressionObj *reason;
  ExpressionObj *transfer_reason;
  ExpressionObj *prevented;
} DamageAct;

typedef struct {
  ObjType objtype;
  ExpressionObj *player;
  ExpressionObj *recover;
  ExpressionObj *who;
  ExpressionObj *card;
} RecoverAct;

typedef struct {
  ObjType objtype;
  bool isAcquire;
  ExpressionObj *skill;
  ExpressionObj *player;
} AcquireDetachSkillAct;

typedef struct {
  ObjType objtype;
  bool hidden;
  int optype; /* 1 = add, 2 = lose, 3 = count */
  ExpressionObj *player;
  const char *name;
  ExpressionObj *number; 
} MarkAct;

typedef struct {
  ObjType objtype;
  ExpressionObj *player;
  ExpressionObj *choices;
} AskForChoiceAct;

typedef struct {
  ObjType objtype;
  ExpressionObj *player;
  ExpressionObj *targets;
} AskForChoosePlayerAct;

typedef struct {
  ObjType objtype;
  ExpressionObj *player;
  const char *skill_name;
} AskForSkillInvokeAct;

typedef struct {
  ObjType objtype;
  ExpressionObj *player;
  ExpressionObj *card;
} ObtainCardAct;

typedef struct {
  ObjType objtype;
  ActionObj *parent;
  ExpressionObj *array;
  ExpressionObj *oprand;
  int op; /* 0,1,2,3 - prepend,append,remove,at */
} ArrayOp;

typedef AskForSkillInvokeAct HasSkillAct;
typedef LoseHpAct LoseMaxHpAct;
typedef LoseHpAct RecoverMaxHpAct;

ActionObj *newAction(struct ast *a);
void analyzeAction(ActionObj *a);

#endif
