#ifndef _ERROR_H
#define _ERROR_H

#include "object.h"
#include "grammar.h"
void yyerror(YYLTYPE *loc, const char *msg, ...);

#endif
