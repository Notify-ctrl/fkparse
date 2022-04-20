#include "linklist.h"
#include "main.h"

struct translations *Translations = 0;

static struct translations *p = 0;

void addTranslation(char *src, char *dest) {
  if (!Translations) {
    Translations = malloc(sizeof(struct translations));
    p = Translations;
  }

  struct translations *t = malloc(sizeof(struct translations));
  t->next = 0;
  t->src = strdup(src);
  t->dest = strdup(dest);
  p->next = t;
  p = t;
}

void loadTranslations() {
  fprintf(yyout, "sgs.LoadTranslationTable{\n");
  struct translations *t = Translations->next;
  while (t) {
    fprintf(yyout, "  [\"%s\"] = \"%s\",\n", t->src, t->dest);
    t = t->next;
  }
  fprintf(yyout, "}\n");
}

int foreach(struct ast* list, struct ast *parent, Callback f)
{
  int ret = 0;
  if (list->l)  // struct ast *prev
    ret = foreach(list->l, parent, f);

  if (list->r)  // struct ast *data
    ret = ret | f(list->r, parent);

  return ret;
}
