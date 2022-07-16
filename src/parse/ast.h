#ifndef _AST_H
#define _AST_H

#include "structs.h"

enum NodeType {
  N_TriggerSkill,
  N_Arg,
};

typedef enum NodeType NodeType;

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

void checktype(ExpVType a, ExpVType t);

typedef struct ast {
  NodeType nodetype;
  struct ast *l;
  struct ast *r;
} ast;

struct ast *newast(NodeType nodetype, struct ast *l, struct ast *r);

#endif  // _AST_H
