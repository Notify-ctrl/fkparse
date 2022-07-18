#ifndef _MAIN_H
#define _MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "object.h"

extern FILE *in_file;
extern FILE *yyin;
extern FILE *yyout;
int yylex_destroy();
int yyparse();

extern char *readfile_name;
extern FILE *error_output;
extern ExtensionObj *extension;

#endif // _MAIN_H

