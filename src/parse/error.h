#ifndef _ERROR_H
#define _ERROR_H

#include "object.h"
#include "grammar.h"
#include "main.h"

const char *yytr(const char *orig);

void yyerror(YYLTYPE *loc, const char *msg, ...);

#endif
