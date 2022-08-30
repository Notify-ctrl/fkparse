#include "error.h"
#include "main.h"
#include <stdarg.h>

static struct {
  char *orig;
  char *dst;
} yysymtab[] = {
  {"end of file",   "<EOF>"},
  {"error",         "<error>"},
  {"invalid token", "invalid token"},
  {"NUMBER",        "<数字>"},
  {"IDENTIFIER",    "<标识符>"},
  {"STRING",        "<字符串>"},
  {"INTERID",       "<内部标识符>"},
  {"FREQUENCY",     "<技能频率>"},
  {"GENDER",        "<性别>"},
  {"KINGDOM",       "<势力>"},
  {"PKGSTART",      "‘拓展包’"},
  {"TRIGGER",       "‘触发技’"},
  {"EVENTI",        "‘时机:’"},
  {"COND",          "‘条件:’"},
  {"EFFECT",        "‘效果:’"},
  {"ACTIVE",        "‘主动技’"},
  {"CARD_FILTER",   "‘选牌规则:’"},
  {"TARGET_FILTER", "‘选目标规则:’"},
  {"FEASIBLE",      "‘可以点确定:’"},
  {"ON_USE",        "‘使用后:’"},
  {"FUNCDEF",       "‘定义函数’"},
  {"EVENT",         "<触发时机>"},
  {"LET",           "‘令’"},
  {"EQ",            "‘为’"},
  {"IF",            "‘若’"},
  {"THEN",          "‘则’"},
  {"ELSE",          "‘否则’"},
  {"END",           "‘以上’"},
  {"REPEAT",        "‘重复此流程:’"},
  {"UNTIL",         "‘直到’"},
  {"TYPE",          "<类型标识符>"},
  {"RETURN",        "‘返回’"},
  {"CALL",          "‘调用’"},
  {"IN",            "‘中’"},
  {"EVERY",         "‘每个’"},
  {"TOWARD",        "‘向’"},
  {"PREPEND",       "‘插入’"},
  {"APPEND",        "‘追加’"},
  {"DELETE",        "‘删除’"},
  {"DI",            "‘第’"},
  {"GE",            "‘个’"},
  {"ELEMENT",       "‘元素’"},
  {"LOGICOP",       "<逻辑运算符>"},
  {"'+'",           "'+'"},
  {"'-'",           "'-'"},
  {"'*'",           "'*'"},
  {"'/'",           "'/'"},
  {"CMP",           "<比较运算符>"},
  {"FIELD",         "‘的’"},
  {"RET",           "‘返回’"},
  {"FALSE",         "‘假’"},
  {"TRUE",          "‘真’"},
  {"BREAK",         "‘中止此流程’"},
  {"DRAW",          "‘摸’"},
  {"ZHANG",         "‘张’"},
  {"CARD",          "‘牌’"},
  {"LOSE",          "‘失去’"},
  {"DIAN",          "‘点’"},
  {"HP",            "‘体力’"},
  {"MAX",           "‘上限’"},
  {"TO",            "‘对’"},
  {"CAUSE",         "‘造成’"},
  {"DAMAGE",        "‘伤害’"},
  {"INFLICT",       "‘受到’"},
  {"RECOVER",       "‘回复’"},
  {"ACQUIRE",       "‘获得’"},
  {"SKILL",         "‘技能’"},
  {"MEI",           "‘枚’"},
  {"MARK",          "‘标记’"},
  {"HIDDEN",        "‘隐藏’"},
  {"COUNT",         "‘数量’"},
  {"FROM",          "‘从’"},
  {"SELECT",        "‘选择’"},
  {"ANITEM",        "‘一项’"},
  {"ANPLAYER",      "‘一名角色’"},
  {"INVOKE",        "‘发动’"},
  {"HAVE",          "‘拥有’"},
  {"BECAUSE",       "‘因’"},
  {"THROW",         "‘弃置’"},
  {"TIMES",         "‘次数’"},
  {"'{'",           "'{'"},
  {"'}'",           "'}'"},
  {"','",           "','"},
  {"':'",           "':'"},
  {"'='",           "'='"},
  {"'$'",           "'$'"},
  {"'('",           "'('"},
  {"')'",           "')'"},
  {"'['",           "'['"},
  {"']'",           "']'"},
  {"'#'",           "'#'"},
  {"$accept",       "$accept"},
  {"extension",     "<拓展文件>"},
  {"funcdefList",   "<函数定义列表>"},
  {"funcdef",       "<函数定义>"},
  {"defargs",       "<定义时参数表>"},
  {"defarglist",    "<定义时参数列表>"},
  {"defarg",        "<定义时参数>"},
  {"skillList",     "<技能列表>"},
  {"skill",         "<技能>"},
  {"skillspecs",    "<技能描述符列表>"},
  {"skillspec",     "<技能描述符>"},
  {"triggerSkill",  "<触发技>"},
  {"triggerspecs",  "<触发技元组列表>"},
  {"triggerspec",   "<触发技元组>"},
  {"block",         "<代码块>"},
  {"statements",    "<语句列表>"},
  {"statement",     "<语句>"},
  {"assign_stat",   "<赋值语句>"},
  {"if_stat",       "<判断语句>"},
  {"loop_stat",     "<循环语句>"},
  {"traverse_stat", "<遍历语句>"},
  {"func_call",     "<函数调用>"},
  {"args",          "<参数表>"},
  {"arglist",       "<参数列表>"},
  {"arg",           "<参数>"},
  {"exp",           "<表达式>"},
  {"prefixexp",     "<前缀表达式>"},
  {"opexp",         "<二元表达式>"},
  {"explist",       "<表达式列表>"},
  {"array",         "<数组构造式>"},
  {"var",           "<变量>"},
  {"retstat",       "<返回语句>"},
  {"packageList",   "<拓展包列表>"},
  {"package",       "<拓展包>"},
  {"generalList",   "<武将列表>"},
  {"general",       "<武将>"},
  {"stringList",    "<字符串列表>"},
  {"action_stat",   "<动作语句>"},
  {"action",        "<动作>"},
  {"drawCards",     "摸牌"},
  {"loseHp",        "失去体力"},
  {"loseMaxHp",     "失去体力上限"},
  {"causeDamage",   "造成伤害"},
  {"inflictDamage", "受到伤害"},
  {"recoverHp",     "回复体力"},
  {"recoverMaxHp",  "回复体力上限"},
  {"acquireSkill",  "获得技能"},
  {"detachSkill",   "失去技能"},
  {"addMark",       "获得标记"},
  {"loseMark",      "失去标记"},
  {"getMark",       "统计标记数量"},
  {"askForChoice",  "询问选择选项"},
  {"askForChoosePlayer",  "询问选择玩家"},
  {"askForSkillInvoke",   "询问发动技能"},
  {"obtainCard",    "获得卡牌"},
  {"arrayPrepend",  "插入元素"},
  {"arrayAppend",   "追加元素"},
  {"arrayRemoveOne",      "删除元素"},
  {"arrayAt",       "获得数组的信息"},
  {"hasSkill",      "拥有技能"},
  {"throwCardsBySkill",   "因发动技能而弃牌"},
  {"getUsedTimes",        "主动技的发动次数"},

  {NULL, NULL},
};

const char *yytr(const char *orig) {
  for (int i = 0; yysymtab[i].orig; i++) {
    if (!strcmp(orig, yysymtab[i].orig)) {
      if (!strcmp(yysymtab[i].dst, "")) {
        return orig;
      } else {
        return yysymtab[i].dst;
      }
    }
  }
  return orig;
}

static const char *type_table[] = {
  "无类型",
  "拓展包类型",
  "技能类型",
  "武将类型",
  "数字类型",
  "布尔类型",
  "字符串类型",
  "玩家类型",
  "卡牌类型",
  "空数组",
  "玩家数组",
  "卡牌数组",
  "数字数组",
  "字符串数组",
  "标记类型",
  "字典类型",
  "函数类型",
  "拼点结果",
};

void checktype(void *o, ExpVType a, ExpVType t) {
  if (a != t && a != TAny && t != TAny) {
    if (!o) {
      fprintf(error_output, "Type error: expect %d, got %d\n", t, a);
    } else {
      YYLTYPE *obj = o;
      yyerror(obj, "类型不匹配：需要 %s，但得到的是 %s",
              type_table[t], type_table[a]);
    }
  }
}

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

/* 辣鸡Windows不给用getline函数 */
static char *getLineOfSource(int lineno) {
  char ch;
  int startpos = 1, length = 0;
  int i = 1;
  while ((ch = fgetc(in_file)) != EOF) {
    if (ch == '\n') {
#ifdef FK_WIN32
      startpos++;   /* windows下的fseek会把'\r'也算进去 */
#endif
      if (i == lineno) {
        break;
      } else {
        i++;
        length = 0;
      }
    }
    if (i != lineno) startpos++;
    length++;
  }

  if (i != lineno) {
    fseek(in_file, 0, 0);
    return NULL;
  } else {
    fseek(in_file, startpos, 0);
    char *ret = malloc(length + 1);
    fgets(ret, length + 1, in_file);
    fseek(in_file, 0, 0);
    return ret;
  }
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
  error_occured = 1;

  /* built-in error report */
  if (loc->first_line == -1) {
    va_list ap;
    va_start(ap, msg);
    fprintf(error_output, "<内置函数>: ");
    vfprintf(error_output, msg, ap);
    fprintf(error_output, "\n");
    va_end(ap);
    return;
  }

  char *source_line = getLineOfSource(loc->first_line);
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
  va_end(ap);
}
