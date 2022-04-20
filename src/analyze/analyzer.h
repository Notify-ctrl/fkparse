#ifndef _ANALYZER_H
#define _ANALYZER_H

#include "main.h"
#include "ast.h"
#include <stdlib.h>
#include <stdarg.h>

#define checktype(a, t) do { \
  if (a != t && a != TAny) { \
    fprintf(stderr, "Type error: expect %d, got %d\n", t, a); \
    exit(1); \
  } \
} while(0)

extern struct ast *all_skills;
extern struct astpackage *currentpack;
extern struct astgeneral *currentgeneral;
extern struct astskill *currentskill;
extern int indent_level;

void print_indent();
void writeline(const char *msg, ...);

struct translations {
  struct translations *next;
  char *src;
  char *dest;
};

extern struct translations *Translations;

void addTranslation(char *src, char *dest);
void loadTranslations();

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

/* analyzer for each grammar rule */

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

int analyzeExp(struct ast *);

void analyzePackageList(struct ast *a);
void analyzePackage(struct ast *a);
void analyzeGeneralList(struct ast *a);
void analyzeGeneral(struct ast *a);

#endif // _ANALYZER_H
