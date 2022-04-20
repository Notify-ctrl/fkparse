#include "analyzer.h"

#define NHASH 9997
static struct symbol symtab[NHASH];

/* symbol table */
/* hash a symbol */
static unsigned
symhash(char *sym)
{
  unsigned int hash = 0;
  unsigned c;

  while ((c = *(sym++))) hash = hash * 9 ^ c;

  return hash;
}

struct symbol *lookup(char *sym)
{
  struct symbol *sp = &symtab[symhash(sym) % NHASH];
  int scount = NHASH;		/* how many have we looked at */

  while(--scount >= 0) {
    if(sp->name && !strcmp(sp->name, sym)) { return sp; }

    if(!sp->name) { /* new entry */
      sp->name = sym;
      sp->type = TNone;
      return sp;
    }

    if(++sp >= symtab+NHASH) sp = symtab; /* try the next entry */
  }
  yyerror("symbol table overflow\n");
  abort(); /* tried them all, table is full */

}

static struct {
  char *dst;
  char *src;
  int type;
} reserved[] = {
  {"你", "player", TPlayer},

  {"魏", "\"wei\"", TNumber},
  {"蜀", "\"shu\"", TNumber},
  {"吴", "\"wu\"", TNumber},
  {"群", "\"qun\"", TNumber},
  {"神", "\"god\"", TNumber},

  {"黑桃", "sgs.Card_Spade", TNumber},
  {"红桃", "sgs.Card_Heart", TNumber},
  {"梅花", "sgs.Card_Club", TNumber},
  {"方块", "sgs.Card_Diamond", TNumber},

  {NULL, NULL, TNone}
};

int isReserved(char *k) {
  int ret = 0;
  for (int i = 0; ; i++) {
    if (reserved[i].dst == NULL) break;
    if (!strcmp(k, reserved[i].dst)) {
      ret = 1;
      break;
    }
  }
  return ret;
}

int analyzeReserved(char *k) {
  int ret = TNone;
  for (int i = 0; ; i++) {
    if (reserved[i].dst == NULL) break;
    if (!strcmp(k, reserved[i].dst)) {
      fprintf(yyout, "%s", reserved[i].src);
      ret = reserved[i].type;
      return ret;
    }
  }
  fprintf(stderr, "错误：未在预定义符号表中发现\"%s\"，请检查\n", k);
  exit(1);
}

