#include "main.h"
#include <stdarg.h>

char *readfile_name;
FILE *in_file;
FILE *error_output;
int error_occured = 0;
ExtensionObj *extension;

static char *getFileName(char *path, int include_ext) {
  char *retVal = path, *p;
  for (p = path; *p; p++) {
    if (*p == '/' || *p == '\\' || *p == ':') {
      retVal = p + 1;
    }
  }
  retVal = strdup(retVal);
  if (!include_ext)
    retVal[strlen(retVal) - 4] = 0;
  return retVal;
}

static void freeTranslation(void *ptr) {
  str_value *s = ptr;
  free((void *)s->origtxt);
  free((void *)s->translated);
  free(s);
}

void parse(char *filename) {
  if (strlen(filename) > 40) {
    fprintf(stderr, "filename %s is too long, max length 40\n", filename);
    return;
  }
  yyin = fopen(filename, "r");
  if (!yyin) {
    fprintf(stderr, "cannot open file %s\n", filename);
    return;
  }
  in_file = fopen(filename, "r");
  char f[64];
  memset(f, 0, sizeof(f));
  readfile_name = getFileName(filename, 0);
  sprintf(f, "%s.lua", readfile_name);

  global_symtab = hash_new();
  stack_push(symtab_stack, cast(Object *, global_symtab));
  current_tab = global_symtab;
  strtab = hash_new();
  restrtab = list_new();
  mark_table = hash_new();
  skill_table = hash_new();
  other_string_table = hash_new();

  yyout = fopen(f, "w+");

#ifndef FK_DEBUG
  char f2[64];
  memset(f2, 0, sizeof(f2));
  sprintf(f2, "%s-error.txt", readfile_name);
  error_output = fopen(f2, "w+");
#else
  error_output = stderr;
#endif

  if (yyparse() == 0) {
    analyzeExtension(extension);
    freeObject(extension);
  } else {
    fprintf(error_output, "发生不可恢复的语法错误，编译中断。\n");
  }

  fclose(in_file);
  fclose(yyin);
  fclose(yyout);

  if (error_occured) {
    fprintf(error_output, "在编译期间有错误产生，请检查您的输入文件。\n");
#ifndef FK_DEBUG
    fclose(error_output);
#endif
    remove(f);
  } else {
#ifndef FK_DEBUG
    fclose(error_output);
    remove(f2);
#endif
  }

  free(readfile_name);
  stack_pop(symtab_stack);
  sym_free(global_symtab);
  hash_free(strtab, NULL);
  list_free(restrtab, freeTranslation);
  hash_free(other_string_table, free);
  hash_free(mark_table, free);
  hash_free(skill_table, free);

  yylex_destroy();
}

int main(int argc, char **argv) {
  sym_init();
  if (argc > 1) {
    for (int i = 2; i <= argc; i++)
      parse(argv[i - 1]);
    sym_free(builtin_symtab);
    list_free(symtab_stack, NULL);
    return 0;
  } else {
    printf("usage: %s <filename>\n", argv[0]);
    exit(0);
  }
}
