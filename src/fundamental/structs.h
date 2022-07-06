#ifndef _STRUCTS_H
#define _STRUCTS_H

#define cast(type, o) ((type)(o))
#define unused(o) ((void)(o))
extern char *strdup(const char *s);
#include <stdbool.h>

/**
 * 所有元素的第一个成员都是objtype
 * 这个结构体用来代表所有可能的成员
 **/

enum ObjType {
  Obj_Integer,
  Obj_Double,
  Obj_String,

  Obj_Extension,
  Obj_Defarg,
  Obj_Funcdef,
  Obj_Package,
  Obj_General,
  Obj_Skill,
  Obj_Card,
  Obj_Player,

  Obj_Block,
  Obj_TriggerSpec,
  Obj_If,
  Obj_Loop,
  Obj_Break,
  Obj_Funccall,
  Obj_Return,
  Obj_Assign,
  Obj_Action,
  Obj_ActionBody,

  Obj_Expression,
  Obj_Var
};

typedef enum ObjType ObjType;

typedef struct {
  ObjType objtype;
} Object;

/* ------------------------- */
/* 链表 */

typedef struct List {
  struct List *next;
  Object *data;
} List;

#define list_foreach(node, list) \
  for ((node) = (list)->next; (node); (node) = (node)->next)

List *list_new();
void list_append(List *l, Object *o);
void list_prepend(List *l, Object *o);
int list_indexOf(List *l, Object *o);
#define list_contains(l, o) (list_indexOf(l, o) != -1)
void list_removeOne(List *l, Object *o);
Object *list_at(List *l, int index);
int list_length(List *l);
#define list_empty(l) (list_length(l) == 0)
void list_free(List *l);

/* stack */
typedef List Stack;
#define stack_new list_new
#define stack_push list_prepend
#define stack_pop(l) list_removeOne((l),list_at((l),0))
#define stack_gettop(l) list_at((l), 0)

/* ------------------------- */
/* 哈希表 */

typedef struct {
  const char *key;
  void *value;
} hash_entry;

typedef struct {
  hash_entry *entries;
  int capacity;
  int length;
} Hash;

Hash *hash_new();
void *hash_get(Hash *h, const char* k);
void hash_set(Hash *h, const char* k, void *v);
void hash_free(Hash *h);

/* ------------------------- */

typedef struct {
  int type;
  const char *origtext; /* text displayed in generated lua */
  bool reserved;
} symtab_item;

extern Hash *global_symtab;
extern Hash *current_tab;
extern Hash *last_lookup_tab;
extern Stack *symtab_stack;
void sym_init();
symtab_item *sym_lookup(const char *k);
void sym_new_entry(const char *k, int type, const char *origtext, bool reserved);
void sym_set(const char *k, symtab_item *v);
void sym_free(Hash *h);

enum StrType {
  Str_Package,
  Str_General,
  Str_Skill,
  Str_Mark,
  Str_Pile
};

typedef struct {
  int type;
  const char *origtxt;
  const char *translated;
} str_value;
extern Hash *strtab;
extern List *restrtab;
const char *translate(const char *orig);
void addTranslation(const char *orig, const char *translated);

extern char *event_table[];

#endif
