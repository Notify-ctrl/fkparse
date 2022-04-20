#include "main.h"
#include "ast.h"

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
  } else {
    printf("usage: %s <filename>\n", argv[0]);
    exit(0);
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
