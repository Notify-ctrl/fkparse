#ifndef _ANALYZER_H
#define _ANALYZER_H

#include "main.h"

enum ExpVType {
  TNone,
  TPackage,
  TSkill,
  TGeneral,
  TNumber,
  TString,
  TPlayer,
  TCard,

  TAny = 0xFF
};

void analyzeExtension(struct ast *a);

void analyzeSkillList(struct ast *a);
void analyzeSkill(struct ast *a);
void analyzeSkillspecs(struct ast *a);
void analyzeTriggerSkill(struct ast *a);
void analyzeTriggerspecs(struct ast *a);
void analyzeTriggerspec(struct ast *a);

void analyzeBlock(struct ast *a);
void analyzeStats(struct ast *a);
void analyzeStatAssign(struct ast *a);
void analyzeIf(struct ast *a);
void analyzeLoop(struct ast *a);
int analyzeAction(struct ast *a);

void analyzePackageList(struct ast *a);
void analyzePackage(struct ast *a);
void analyzeGeneralList(struct ast *a);
void analyzeGeneral(struct ast *a);
void analyzeStringList(struct ast *a);
void analyzeString(struct ast *a);

#endif // _ANALYZER_H
