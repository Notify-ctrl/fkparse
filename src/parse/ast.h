#ifndef _AST_H
#define _AST_H

#include "structs.h"
#include "main.h"

#define checktype(a, t) do { \
  if (a != t && a != TAny && t != TAny) { \
    fprintf(error_output, "Type error: expect %d, got %d\n", t, a); \
    exit(1); \
  } \
} while(0)

enum NodeType {
  N_Extension,
  N_Funcdefs,
  N_Funcdef,
  N_Defargs,
  N_Defarg,
  N_Packages,
  N_Package,
  N_Skills,
  N_Skill,
  N_SkillSpecs,
  N_TriggerSkill,
  N_TriggerSpecs,
  N_TriggerSpec,

  N_Block,
  N_Stats,
  N_Stat_None,
  N_Stat_Assign,
  N_Stat_If,
  N_Stat_Loop,
  N_Stat_Traverse,
  N_Stat_Break,
  N_Stat_Funccall,
  N_Stat_Ret,

  N_Stat_Action,
  N_Action,

  N_Args,
  N_Arg,

  N_Exps,
  N_Exp,

  N_Var,

  N_Generals,
  N_General,

  N_Num,
  N_Strs,
  N_Str,
  N_Id,
};

typedef enum NodeType NodeType;

enum ActionType {
  ActionDrawcard,
  ActionLosehp,
  ActionLoseMaxHp,
  ActionDamage,
  ActionRecover,
  ActionRecoverMaxHp,
  ActionAcquireSkill,
  ActionDetachSkill,
  ActionMark,
  ActionAskForChoice,
  ActionAskForPlayerChosen,
  ActionAskForSkillInvoke,
  ActionObtainCard,
  ArrayPrepend,
  ArrayAppend,
  ArrayRemoveOne,
  ArrayAt,
  ActionHasSkill
};

typedef enum ActionType ActionType;

enum ExpType {
  ExpCmp,
  ExpLogic,
  ExpCalc,
  ExpStr,
  ExpNum,
  ExpBool,
  ExpVar,
  ExpArray,
  ExpFunc,
//  ExpAction
};

typedef enum ExpType ExpType;

enum ExpVType {
  TNone,
  TPackage,
  TSkill,
  TGeneral,
  TNumber,
  TBool,
  TString,
  TPlayer,
  TCard,
  TEmptyList,
  TPlayerList,
  TCardList,
  TNumberList,
  TStringList,
  TMark,
  TFunc,

  TNotSure = 0xFFFE,
  TAny = 0xFFFF
};

typedef enum ExpVType ExpVType;

typedef struct ast {
  NodeType nodetype;
  struct ast *l;
  struct ast *r;
} ast;

struct ast *newast(NodeType nodetype, struct ast *l, struct ast *r);

#endif  // _AST_H
