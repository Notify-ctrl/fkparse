#ifndef _QSAN_H
#define _QSAN_H

#include "builtin.h"

void qsan_load_action();
void qsan_load_cards();
void qsan_load_enum();
void qsan_load_func();
void qsan_load_getter();
void qsan_load_interaction();
void qsan_load_util();

void qsan_load();

extern char *qsan_event_table[];

#endif
