#ifndef _LINKLIST_H
#define _LINKLIST_H

// asts like stringlist can be considered as linklist with prev & data
#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct translations {
  struct translations *next;
  char *src;
  char *dest;
};

extern struct translations *Translations;

void addTranslation(char *src, char *dest);
void loadTranslations();

int foreach(struct ast *list, struct ast *parent, Callback f);

#endif  // _LINKLIST_H
