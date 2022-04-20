#include "main.h"
#include "ast.h"

/* simple symtab of fixed size */
#define NHASH 9997
static struct symbol symtab[NHASH];

/* hash a symbol */
static unsigned symhash(char *sym) {
  unsigned int hash = 0;
  unsigned c;

  while(c = *sym++) hash = hash*9 ^ c;

  return hash;
}

struct symbol *lookup(char* sym) {
  struct symbol *sp = &symtab[symhash(sym)%NHASH];
  int scount = NHASH;		/* how many have we looked at */

  while(--scount >= 0) {
    if(sp->name && !strcmp(sp->name, sym)) { return sp; }

    if(!sp->name) {		/* new entry */
      sp->name = strdup(sym);
      sp->type = TNumber;
      sp->value.i = 0;
      return sp;
    }

    if(++sp >= symtab+NHASH) sp = symtab; /* try the next entry */
  }
  yyerror("symbol table overflow\n");
  abort(); /* tried them all, table is full */

}

char *readfile_name;

static char *getFileName(char *path) {
  char *retVal = path, *p;
  for (p = path; *p; p++) {
    if (*p == '/' || *p == '\\' || *p == ':') {
      retVal = p + 1;
    }
  }
  retVal[strlen(retVal) - 4] = 0;
  return retVal;
}

int main(int argc, char **argv) {
  if (argc > 1) {
    yyin = fopen(argv[1], "r");
    if (!yyin) {
      fprintf(stderr, "cannot open file %s\n", argv[1]);
      exit(-1);
    }
  }
  int MAXSIZE = 0xFFF;
  char proclnk[0xFFF];
  char filename[0xFFF];
  int fno = fileno(yyin);
  ssize_t r;
  sprintf(proclnk, "/proc/self/fd/%d", fno);
  r = readlink(proclnk, filename, MAXSIZE);
  if (r < 0) {
    printf("failed to readlink\n");
    exit(1);
  }
  filename[r] = '\0';
  readfile_name = getFileName(filename);
  sprintf(filename, "%s.lua\0", readfile_name);
  yyout = fopen(filename, "w+");
  yyparse();
}
