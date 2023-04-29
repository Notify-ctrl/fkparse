#ifndef _ERROR_H
#define _ERROR_H

#include "object.h"
#include "grammar.h"
#include "main.h"

const char *yytr(const char *orig);
void checktype(void *o, ExpVType a, ExpVType t);
void fkp_yyerror(FKP_YYLTYPE *loc, const char *msg, ...);

#endif
