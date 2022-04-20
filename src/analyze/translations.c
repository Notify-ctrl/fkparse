#include "analyzer.h"

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
  indent_level++;
  while (t) {
    writeline("[\"%s\"] = \"%s\",", t->src, t->dest);
    t = t->next;
  }
  fprintf(yyout, "}\n");
}

