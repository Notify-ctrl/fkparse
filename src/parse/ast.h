#ifndef _AST_H
#define _AST_H

#include "structs.h"

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

void checktype(void *o, ExpVType a, ExpVType t);

#endif  // _AST_H
