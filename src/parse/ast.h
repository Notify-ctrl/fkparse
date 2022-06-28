#ifndef _AST_H
#define _AST_H

#include "structs.h"
#include "main.h"

#define checktype(a, t) do { \
  if (a != t && a != TAny) { \
    fprintf(error_output, "Type error: expect %d, got %d\n", t, a); \
    exit(1); \
  } \
} while(0)

enum NodeType {
  N_Extension,
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
  N_Stat_Break,
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
  ActionDamage,
  ActionRecover,
  ActionAcquireSkill,
  ActionDetachSkill,
  ActionMark,
  ActionAskForChoice,
  ActionAskForPlayerChosen
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
  ExpAction
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

  TNotSure = 0xFFFE,
  TAny = 0xFFFF
};

typedef enum ExpVType ExpVType;

typedef struct ast {
  NodeType nodetype;
  struct ast *l;
  struct ast *r;
} ast;

typedef int (*Callback)(struct ast *list, struct ast *parent);

struct ast *newast(NodeType nodetype, struct ast *l, struct ast *r);

struct numval {
  NodeType nodetype;
  long long n;
};

struct ast *newnum(long long n);

struct aststr {
  NodeType nodetype;
  char *str;
};

struct ast *newstr(char *s);

struct astpackage {
  NodeType nodetype;
  struct ast *id;
  struct ast *generals;
  int uid;
};

struct ast *newpackage(char *id, struct ast *generals);

struct astgeneral {
  NodeType nodetype;
  struct aststr *id;
  struct aststr *kingdom;
  long long hp;
  struct aststr *nickname;
  struct aststr *gender;
  struct aststr *interid;
  struct ast *skills;
  int uid;
};

struct ast *newgeneral(char *id, char *kingdom, long long hp,
                        char *nickname, char *gender, char *interid, struct ast *skills);

struct astskill {
  NodeType nodetype;
  struct aststr *id;
  struct aststr *description;
  struct aststr *frequency;
  struct aststr *interid;
  struct ast *skillspecs;
  int uid;
};

struct ast *newskill(char *id, char *desc, char *frequency, char *interid, struct ast *spec);

struct astTriggerSkill {
  NodeType nodetype;
  struct ast *specs;
};

struct astTriggerSpec {
  NodeType nodetype;
  int event;
  struct ast *cond;
  struct ast *effect;
};

struct ast *newtriggerspec(int event, struct ast *cond, struct ast *effect);

struct astAssignStat {
  NodeType nodetype;
  struct ast *lval;
  struct ast *rval;
};

struct astIf {
  NodeType nodetype;
  struct ast *cond;
  struct ast *then;
  struct ast *el;
};

struct ast *newif(struct ast *cond, struct ast *then, struct ast *el);

struct astLoop {
  NodeType nodetype;
  struct ast *body;
  struct ast *cond;
}; 

struct astAction {
  NodeType nodetype;
  ActionType actiontype;
  struct ast *action;
  bool standalone;
};

struct ast *newaction(int type, struct ast *action);

/* actions with 2 or less operator(s) use struct ast instead */

struct actionDamage {
  NodeType nodetype;
  struct ast *src;
  struct ast *dst;
  struct ast *num;
};

struct ast *newdamage(struct ast *src, struct ast *dst, struct ast *num);

struct actionMark {
  NodeType nodetype;
  struct ast *player;
  struct aststr *name;
  struct ast *num;
  bool hidden;
  int optype; /* 1 = add, 2 = lose, 3 = count */
};

struct ast *newmark(struct ast *player, char *name, struct ast *num, int hidden, int optype);

struct astExp {
  NodeType nodetype;
  ExpType exptype;
  ExpVType valuetype;
  long long value;
  int optype;
  struct astExp *l;
  struct astExp *r;
  bool bracketed;
};

struct ast *newexp(int exptype, long long value, int optype, struct astExp *l, struct astExp *r);

struct astVar {
  NodeType nodetype;
  struct aststr *name;  /* or field */
  struct astExp *obj;
  ExpVType type;
};

#endif  // _AST_H
