#ifndef _AST_H
#define _AST_H

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

enum ActionType {
  ActionDrawcard,
  ActionLosehp,
  ActionDamage,
  ActionRecover,
  ActionAcquireSkill,
  ActionDetachSkill,
  ActionMark
};

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

enum VarType {
  VarNumber,
  VarStr,
  VarPlayer,
  VarCard
};

struct ast {
  int nodetype;
  struct ast *l;
  struct ast *r;
};

typedef  int (*Callback)(struct ast *list, struct ast *parent);

struct ast *newast(int nodetype, struct ast *l, struct ast *r);

struct numval {
  int nodetype;
  long long n;
};

struct ast *newnum(long long n);

struct aststr {
  int nodetype;
  char *str;
};

struct ast *newstr(char *s);

struct astpackage {
  int nodetype;
  struct ast *id;
  struct ast *generals;
  int uid;
};

struct ast *newpackage(char *id, struct ast *generals);

struct astgeneral {
  int nodetype;
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
  int nodetype;
  struct aststr *id;
  struct aststr *description;
  struct aststr *frequency;
  struct aststr *interid;
  struct ast *skillspec;
  int uid;
};

struct ast *newskill(char *id, char *desc, char *frequency, char *interid, struct ast *spec);

struct astTriggerSkill {
  int nodetype;
  struct ast *specs;
};

struct astTriggerSpec {
  int nodetype;
  int event;
  struct ast *cond;
  struct ast *effect;
};

struct ast *newtriggerspec(int event, struct ast *cond, struct ast *effect);

struct astAssignStat {
  int nodetype;
  struct ast *lval;
  struct ast *rval;
};

struct astIf {
  int nodetype;
  struct ast *cond;
  struct ast *then;
  struct ast *el;
};

struct ast *newif(struct ast *cond, struct ast *then, struct ast *el);

struct astLoop {
  int nodetype;
  struct ast *body;
  struct ast *cond;
}; 

struct astAction {
  int nodetype;
  int actiontype;
  struct ast *action;
  int standalone;
};

struct ast *newaction(int type, struct ast *action);

/* actions with 2 or less operator(s) use struct ast instead */

struct actionDamage {
  int nodetype;
  struct ast *src;
  struct ast *dst;
  struct ast *num;
};

struct ast *newdamage(struct ast *src, struct ast *dst, struct ast *num);

struct actionMark {
  int nodetype;
  struct ast *player;
  struct aststr *name;
  struct ast *num;
  int hidden;
  int optype; /* 1 = add, 2 = lose, 3 = count */
};

struct ast *newmark(struct ast *player, char *name, struct ast *num, int hidden, int optype);

struct astExp {
  int nodetype;
  int exptype;
  int valuetype;
  long long value;
  int optype;
  struct astExp *l;
  struct astExp *r;
  int bracketed;
};

struct ast *newexp(int exptype, long long value, int optype, struct astExp *l, struct astExp *r);

struct astVar {
  int nodetype;
  struct aststr *name;  /* or field */
  struct astExp *obj;
  int type;
};

#endif  // _AST_H
