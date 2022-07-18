#include "ast.h"
#include "main.h"
#include "error.h"

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
  "函数类型"
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
    //while(1);
  }
}

struct ast *newast(NodeType nodetype, struct ast *l, struct ast *r) {
  struct ast *a = malloc(sizeof(struct ast));
  a->nodetype = nodetype;
  a->l = l;
  a->r = r;
  return a;
}
