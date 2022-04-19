#ifndef _LINKLIST_H
#define _LINKLIST_H

// asts like stringlist can be considered as linklist with prev & data
#include "ast.h"

int foreach(struct ast *list, struct ast *parent, Callback f);

#endif  // _LINKLIST_H
