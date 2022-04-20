#ifndef _MAIN_H
#define _MAIN_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "grammar.h"

extern int yylineno;
extern FILE *yyin;
extern FILE *yyout;
int yylex();
void yyerror(const char *msg, ...);

extern char *readfile_name;

typedef YYSTYPE Value;

extern int package_index;
extern int general_index;
extern int skill_index;

void defPackage(char *name, struct ast *a);
void defSkill(char *name, struct ast *a);
void defGeneral(char *name, struct ast *a);
Value analyzeTree(struct ast *a);

#endif // _MAIN_H

