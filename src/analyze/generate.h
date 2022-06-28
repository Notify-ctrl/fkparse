#ifndef _GENERATE_H
#define _GENERATE_H

extern int indent_level;
void print_indent();
void writestr(const char *msg, ...);
void writeline(const char *msg, ...);
void outputError(const char *msg, ...);

void analyzeExp(ExpressionObj *e);

#endif
