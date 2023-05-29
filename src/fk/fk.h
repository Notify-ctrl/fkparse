#ifndef _FK_H
#define _FK_H

#include "builtin.h"

void fk_load_action();
void fk_load_cards();
void fk_load_enum();
void fk_load_func();
void fk_load_getter();
void fk_load_interaction();
void fk_load_util();

void fk_load();

extern char *fk_event_table[];

#endif
