#ifndef _MAIN_H
#define _MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "object.h"

extern int yylineno;
extern FILE *yyin;
extern FILE *yyout;
int yylex();
int yylex_destroy();
void yyerror(const char *msg, ...);
int yyparse();

extern char *readfile_name;
extern FILE *error_output;
extern ExtensionObj *extension;

#endif // _MAIN_H

