#include "main.h"
#include "fkparse.h"
#include "builtin.h"
#include <stdarg.h>

char *readfile_name;
FILE *in_file;
FILE *error_output;
int error_occured = 0;
ExtensionObj *extension;

static char *getFileName(const char *path, int include_ext) {
  const char *p;
  for (p = path; *p; p++) {
    if (*p == '/' || *p == '\\' || *p == ':') {
      path = p + 1;
    }
  }
  char *retVal = strdup(path);
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

void parse(const char *filename, fkp_analyze_type type) {
  yyin = fopen(filename, "r");
  if (!yyin) {
    error_occured = 1;
    fprintf(error_output, "cannot open file %s\n", filename);
    return;
  }
  in_file = fopen(filename, "r");
  char f[64];
  memset(f, 0, sizeof(f));
  readfile_name = getFileName(filename, 0);
  if (strlen(readfile_name) > 40) {
    error_occured = 1;
    fprintf(error_output, "filename %s is too long, max length 40\n", readfile_name);
    free(readfile_name);
    return;
  }
  sprintf(f, "%s.lua", readfile_name);

  yyout = fopen(f, "w+");

// #ifndef FK_DEBUG
  char f2[64];
  memset(f2, 0, sizeof(f2));
  sprintf(f2, "%s-error.txt", readfile_name);
  error_output = fopen(f2, "w+");
// #else
//   error_output = stderr;
// #endif

  if (yyparse() == 0) {
    switch (type) {
    case FKP_QSAN_LUA:
      analyzeExtensionQSan(extension);
      break;
    case FKP_NONAME_JS:
      analyzeExtensionNoname(extension);
      break;
    default:
      error_occured = 1;
      fprintf(error_output, "此编译功能未完成，敬请期待\n");
      break;
    }
    freeObject(extension);
  } else {
    fprintf(error_output, "发生不可恢复的语法错误，编译中断。\n");
  }

  fclose(in_file);
  fclose(yyin);
  fclose(yyout);

  if (error_occured) {
    fprintf(error_output, "一共检测到%d条错误。\n", error_occured);
    fprintf(error_output, "在编译期间有错误产生，请检查您的输入文件。\n");
// #ifndef FK_DEBUG
    fclose(error_output);
// #endif
    remove(f);
  } else {
// #ifndef FK_DEBUG
    fclose(error_output);
    remove(f2);
// #endif
  }

  free(readfile_name);

  yylex_destroy();
}

fkp_parser *fkp_new_parser() {
  fkp_parser *ret = malloc(sizeof(fkp_parser));
  ret->generals = (fkp_hash *)hash_new();
  ret->skills = (fkp_hash *)hash_new();
  ret->marks = (fkp_hash *)hash_new();
  return ret;
}

static void fkp_reset(fkp_parser *p) {
  hash_free((Hash *)p->generals, free);
  hash_free((Hash *)p->skills, free);
  hash_free((Hash *)p->marks, free);
  p->generals = (fkp_hash *)hash_new();
  p->skills = (fkp_hash *)hash_new();
  p->marks = (fkp_hash *)hash_new();
}

int fkp_parse(fkp_parser *p, const char *filename, fkp_analyze_type type) {
  error_occured = 0;
  sym_init(type);
  global_symtab = hash_new();
  stack_push(symtab_stack, cast(Object *, global_symtab));
  current_tab = global_symtab;
  restrtab = list_new();
  general_table = hash_new();
  mark_table = hash_new();
  skill_table = hash_new();
  other_string_table = hash_new();

  skill_id = 0;
  general_id = 0;
  package_id = 0;
  funcId = 0;
  markId = 0;
  stringId = 0;

  parse(filename, type);

  if (!error_occured) {
    fkp_reset(p);
    hash_copy_all((Hash *)p->generals, general_table);
    hash_copy_all((Hash *)p->skills, skill_table);
    hash_copy_all((Hash *)p->marks, mark_table);
  }

  stack_pop(symtab_stack);
  sym_free(global_symtab);
  list_free(restrtab, freeTranslation);
  hash_free(other_string_table, free);
  hash_free(mark_table, free);
  hash_free(skill_table, free);
  hash_free(general_table, free);
  sym_free(builtin_symtab);
  builtin_symtab = NULL;
  list_free(symtab_stack, NULL);
  return error_occured;
}

void fkp_close(fkp_parser *p) {
  hash_free((Hash *)p->generals, free);
  hash_free((Hash *)p->skills, free);
  hash_free((Hash *)p->marks, free);
  free(p);
}

int main(int argc, char **argv) {
  fkp_parser *p = fkp_new_parser();
  if (argc > 1) {
    for (int i = 2; i <= argc; i++) {
      fkp_parse(p, argv[i - 1], FKP_QSAN_LUA);
    }
    fkp_close(p);
    return 0;
  } else {
    printf("usage: %s <filename>\n", argv[0]);
    fkp_close(p);
    exit(0);
  }
}
