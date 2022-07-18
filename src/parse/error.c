#include "error.h"
#include "main.h"
#include <stdarg.h>

/*
 * fkparse处理的文本基本都是汉字
 * 一个汉字在UTF-8中是三个字节，但是输出到屏幕上面时候只有两个字母的宽度
 * 所以在定位报错信息时候不能简单的看传来的值，需要去考虑到底输出多少空格
 */
typedef struct {
  int start_column;
  int end_column;
  int start_display_column;
  int end_display_column;
} Bound;

static char *getLineOfSource(int lineno, size_t *length) {
  char *ret = NULL;
  int i = 1;
  while (getline(&ret, length, in_file) != -1) {
    if (i++ == lineno) {
      fseek(in_file, 0, 0);
      return ret;
    }
  }

  free(ret);
  fseek(in_file, 0, 0);
  return NULL;
}

static char *getPrintPos(char *src, YYLTYPE *pos, YYLTYPE *newPos) {
  newPos->first_line = pos->first_line;
  if (strlen(src) < 120) {
    newPos->first_column = 1;
    newPos->last_column = strlen(src);
    src[newPos->last_column - 1] = 0;
    return src;
  } else {
    if (pos->first_column > 60) {
      newPos->first_column = pos->first_column - 30;
    } else {
      newPos->first_column = 1;
    }
    if (pos->last_line == pos->first_line
      && pos->last_column - pos->first_column < 72
    ) {
      newPos->last_column = pos->last_column;
    } else if (strlen(src) - pos->first_column > 90) {
      newPos->last_column = newPos->first_column + 72;
    } else {
      newPos->last_column = strlen(src);
    }
    src[newPos->last_column - 1] = 0;
    return src + newPos->first_column;
  }
}

/*
 * 判断一串字符（UTF-8）中的实际占屏幕宽度
 */
static void getBoundOfString(const char *s, YYLTYPE *err_loc,
                             YYLTYPE *print_loc, Bound *bound) {
  bound->start_column = err_loc->first_column - print_loc->first_column;
#define min(a,b) (a>b?b:a)
  bound->end_column = min(print_loc->last_column, err_loc->last_column);
  if (err_loc->first_line != err_loc->last_line)
    bound->end_column = print_loc->last_column;
#undef min
  bound->start_display_column = 0;
  bound->end_display_column = 0;
  int display_pos = 1;
  int pos = 1;
  while (*s != 0) {
    if (pos >= bound->start_column && display_pos != 0
      && bound->start_display_column == 0
    ) {
      bound->start_display_column = display_pos;
    }
    if (pos >= bound->end_column && bound->start_display_column != 0
      && bound->end_display_column == 0
    ) {
      bound->end_display_column = display_pos;
    }

    unsigned char uc = *s;
    if (uc >= 0xf0 && uc <= 0xf7) {   /* emoji */
      pos += 4;
      display_pos += 2;
      s += 4;
    } else if (uc >= 0xe0 && uc <= 0xef) {  /* Asian char */
      pos += 3;
      display_pos += 2;
      s += 3;
    } else if (uc >= 0xc0 && uc <= 0xdf) {  /* latin char */
      pos += 2;
      display_pos += 1;
      s += 2;
    } else {    /* ascii */
      pos += 1;
      display_pos += 1;
      s += 1;
    }
  }
  if (bound->end_display_column == 0)
    bound->end_display_column = display_pos;
}

static void printPosAnnonation(YYLTYPE *loc, Bound *bound) {
  char buf[64];
  sprintf(buf, "%d", loc->first_line);
  for (int i = 0; i < strlen(buf); i++) {
    fprintf(error_output, " ");
  }
  fprintf(error_output, "  | ");
  for (int i = 1; i < bound->start_display_column; i++) {
    fprintf(error_output, " ");
  }
  fprintf(error_output, "^");
  for (int i = bound->start_display_column;
       i < bound->end_display_column; i++) {
    fprintf(error_output, "~");
  }
  fprintf(error_output, "\n\n");
}

void yyerror(YYLTYPE *loc, const char *msg, ...) {
  size_t source_line_length;
  char *source_line = getLineOfSource(loc->first_line, &source_line_length);
  if (!source_line)
    return;

  YYLTYPE newloc;   /* 对于过长的行只输出一部分，这个保存输出时开始的行列 */
  char *best_print_pos = getPrintPos(source_line, loc, &newloc);

  va_list ap;
  va_start(ap, msg);
  fprintf(error_output, "%s.txt:%d:%d: ", readfile_name, loc->first_line,
          loc->first_column);
  vfprintf(error_output, msg, ap);
  fprintf(error_output, "\n");
  fprintf(error_output, " %d | %s\n", newloc.first_line, best_print_pos);
  Bound bound;
  getBoundOfString(best_print_pos, loc, &newloc, &bound);
  printPosAnnonation(&newloc, &bound);
  free(source_line);
}
