%{
#include "main.h"
#include "enums.h"
#include "ast.h"
#include "object.h"
#include "grammar.h"
#include "error.h"

/* For travering List in switch-case. */
static List *iter;
#define YYDEBUG 1
int yydebug = 0;

#define YYPARSE_PARAM scanner
#define YYLEX_PARAM scanner

int yylex(YYSTYPE *yylval_param, YYLTYPE *yylloc_param);
static void yycopyloc(void *p, YYLTYPE *loc) {
  Object *dst = p;
  dst->first_line = loc->first_line;
  dst->first_column = loc->first_column;
  dst->last_line = loc->last_line;
  dst->last_column = loc->last_column;
}
%}

%union {
  struct ast *a;
  int enum_v;
  long long i;
  char *s;

  List *list;
  Hash *hash;
  Object *any;

  ExtensionObj *extension;
  PackageObj *package;
  GeneralObj *general;
  SkillObj *skill;
  TriggerSpecObj *trigger_spec;

  BlockObj *block;
  ExpressionObj *exp;
  VarObj *var;
  DefargObj *defarg;
  FuncdefObj *func_def;
  IfObj *if_stat;
  LoopObj *loop;
  TraverseObj *traverse;
  AssignObj *assign;
  FunccallObj *func_call;
}

%token <i> NUMBER
%token <s> IDENTIFIER
%token <s> STRING
%token <s> INTERID
%token <s> FREQUENCY
%token <s> GENDER
%token <s> KINGDOM
%token PKGSTART
%token TRIGGER EVENTI COND EFFECT
%token FUNCDEF
%token <enum_v> EVENT
%token LET EQ IF THEN ELSE END REPEAT UNTIL
%token <enum_v> TYPE
%token RETURN CALL
%token IN EVERY TOWARD PREPEND APPEND DELETE DI GE ELEMENT
%left <enum_v> LOGICOP
%left '+' '-' '*' '/'
%nonassoc <enum_v> CMP
%token FIELD RET
%token FALSE TRUE BREAK
%token DRAW ZHANG CARD LOSE DIAN HP MAX
%token TO CAUSE DAMAGE INFLICT RECOVER ACQUIRE SKILL
%token MEI MARK HIDDEN COUNT
%token FROM SELECT ANITEM ANPLAYER
%token INVOKE HAVE

%type <list> funcdefList defargs defarglist skillList packageList generalList
%type <list> stringList skillspecs triggerSkill triggerspecs
%type <list> statements arglist explist array
%type <hash> args

%type <defarg> defarg
%type <func_def> funcdef
%type <package> package
%type <general> general
%type <skill> skill
%type <a> skillspec
%type <trigger_spec> triggerspec

%type <block> block
%type <any> statement
%type <exp> retstat
%type <traverse> traverse_stat
%type <assign> assign_stat
%type <if_stat> if_stat
%type <loop> loop_stat
%type <a> arg
%type <func_call> action_stat action
%type <func_call> drawCards loseHp causeDamage inflictDamage recoverHp
%type <func_call> acquireSkill detachSkill
%type <func_call> addMark loseMark getMark
%type <func_call> askForChoice askForChoosePlayer
%type <func_call> askForSkillInvoke obtainCard hasSkill
%type <func_call> arrayPrepend arrayAppend arrayRemoveOne arrayAt
%type <func_call> loseMaxHp recoverMaxHp

%type <exp> exp prefixexp opexp
%type <var> var
%type <func_call> func_call

%start extension
%define parse.error custom
// %verbose
%define parse.trace
%define api.pure full
%locations

%%

extension : funcdefList skillList packageList
              {
                extension = newExtension($1, $2, $3);
                yycopyloc(extension, &@$);
                YYACCEPT;
              }
          ;

funcdefList : %empty { $$ = list_new(); }
            | funcdefList funcdef {
                $$ = $1;
                list_append($$, cast(Object *, $2));
              }
            ;

funcdef : FUNCDEF IDENTIFIER defargs block
          { $$ = newFuncdef($2, $3, TNone, $4); yycopyloc($$, &@$); }
        | FUNCDEF IDENTIFIER defargs RETURN TYPE block
          { $$ = newFuncdef($2, $3, $5, $6); yycopyloc($$, &@$); }
        ;

defargs : '{' defarglist '}' { $$ = $2; }
        | '{' '}' { $$ = list_new(); }
        ;

defarglist  : defarglist ',' defarg {
                $$ = $1;
                list_append($$, cast(Object *, $3));
              }
            | defarg {
                $$ = list_new();
                list_append($$, cast(Object *, $1));
              }
            ;

defarg : IDENTIFIER ':' TYPE
         { $$ = newDefarg($1, $3, NULL); yycopyloc($$, &@$); }
       | IDENTIFIER ':' TYPE '=' exp
         { $$ = newDefarg($1, $3, $5); yycopyloc($$, &@$); }
       ;

skillList : %empty { $$ = list_new(); }
          | skillList skill {
              $$ = $1;
              list_append($$, cast(Object *, $2));
            }
          ;

skill     : '$' IDENTIFIER STRING FREQUENCY INTERID skillspecs
              {
                $$ = newSkill($2, $3, $4, $5, $6);
                yycopyloc($$, &@$);
              }
          | '$' IDENTIFIER STRING FREQUENCY skillspecs
              {
                $$ = newSkill($2, $3, $4, NULL, $5);
                yycopyloc($$, &@$);
              }
          | '$' IDENTIFIER STRING skillspecs
              {
                $$ = newSkill($2, $3, NULL, NULL, $4);
                yycopyloc($$, &@$);
              }
          ;

skillspecs  : %empty { $$ = list_new(); }
            | skillspecs skillspec {
                $$ = $1;
                list_append($$, cast(Object *, $2));
              }
            ;

skillspec   : triggerSkill { $$ = newast(N_TriggerSkill, NULL, cast(struct ast *, $1)); }
            ;

triggerSkill  : TRIGGER triggerspecs  { $$ = $2; }
              ;

triggerspecs  : triggerspec {
                  $$ = list_new();
                  list_append($$, cast(Object *, $1));
                }
              | triggerspecs triggerspec {
                  $$ = $1;
                  list_append($$, cast(Object *, $2));
                }
              ;

triggerspec : EVENTI EVENT EFFECT block {
                $$ = newTriggerSpec($2, NULL, $4);
                yycopyloc($$, &@$);
              }
            | EVENTI EVENT COND block EFFECT block {
                $$ = newTriggerSpec($2, $4, $6);
                yycopyloc($$, &@$);
              }
            ;

block   : statements  { $$ = newBlock($1, NULL); yycopyloc($$, &@$); }
        | statements retstat  { $$ = newBlock($1, $2); yycopyloc($$, &@$); }
        ;

statements  : %empty { $$ = list_new(); }
            | statements statement { $$ = $1; list_append($$, cast(Object *, $2)); }
            ;

statement   : assign_stat { $$ = cast(Object *, $1); }
            | if_stat { $$ = cast(Object *, $1); }
            | loop_stat { $$ = cast(Object *, $1); }
            | traverse_stat { $$ = cast(Object *, $1); }
            | BREAK { $$ = malloc(sizeof(Object)); $$->objtype = Obj_Break; }
            | func_call { $$ = cast(Object *, $1); }
            | action_stat { $$ = cast(Object *, $1); }
            | error { $$ = malloc(sizeof(Object)); $$->objtype = Obj_Break; }
            ;

assign_stat : LET var EQ exp { $$ = newAssign($2, $4); yycopyloc($$, &@$); }
            ;

if_stat : IF exp THEN block END { $$ = newIf($2, $4, NULL); yycopyloc($$, &@$); }
        | IF exp THEN block ELSE block END { $$ = newIf($2, $4, $6); yycopyloc($$, &@$); }
        ;

loop_stat : REPEAT block UNTIL exp { $$ = newLoop($2, $4); yycopyloc($$, &@$); }
          ;

traverse_stat : TO exp IN EVERY IDENTIFIER REPEAT block END
                { $$ = newTraverse($2, $5, $7); yycopyloc($$, &@$); }
              ;

func_call : CALL IDENTIFIER args { $$ = newFunccall($2, $3); yycopyloc($$, &@$); }
          ;

args : '{' arglist '}' {
          $$ = hash_new();
          list_foreach(iter, $2) {
            hash_set($$, cast(const char *, cast(struct ast *, iter->data)->l),
                    cast(void *, cast(struct ast *, iter->data)->r));
            free(iter->data);
          }
          list_free($2, NULL);
        }
     | '{' '}' { $$ = hash_new(); }
     ;

arglist : arglist ',' arg { $$ = $1; list_append($$, cast(Object *, $3)); }
        | arg { $$ = list_new(); list_append($$, cast(Object *, $1)); }
        ;

arg : IDENTIFIER ':' exp {
        $3->param_name = $1;
        $$ = newast(N_Arg, cast(struct ast *, $1), cast(struct ast *, $3));
      }
    ;

exp : FALSE { $$ = newExpression(ExpBool, 0, 0, NULL, NULL); yycopyloc($$, &@$); }
    | TRUE { $$ = newExpression(ExpBool, 1, 0, NULL, NULL); yycopyloc($$, &@$); }
    | NUMBER { $$ = newExpression(ExpNum, $1, 0, NULL, NULL); yycopyloc($$, &@$); }
    | STRING { $$ = newExpression(ExpStr, 0, 0, NULL, NULL); $$->strvalue = $1; yycopyloc($$, &@$); }
    | prefixexp { $$ = $1; }
    | opexp { $$ = $1; }
    | '(' action_stat ')'
      {
        $$ = newExpression(ExpFunc, 0, 0, NULL, NULL);
        $$->func = $2;
        yycopyloc($$, &@$);
      }
    | array { $$ = newExpression(ExpArray, 0, 0, NULL, NULL); $$->array = $1; yycopyloc($$, &@$); }
    ;

prefixexp : var { $$ = newExpression(ExpVar, 0, 0, NULL, NULL); $$->varValue = $1; yycopyloc($$, &@$); }
      | '(' func_call ')'
          { $$ = newExpression(ExpFunc, 0, 0, NULL, NULL); $$->func = $2; yycopyloc($$, &@$); }
      | '(' exp ')' { $$ = $2; $$->bracketed = 1; yycopyloc($$, &@$); }
      ;

opexp : exp CMP exp { $$ = newExpression(ExpCmp, 0, $2, $1, $3); yycopyloc($$, &@$); }
      | exp LOGICOP exp { $$ = newExpression(ExpLogic, 0, $2, $1, $3); yycopyloc($$, &@$); }
      | exp '+' exp { $$ = newExpression(ExpCalc, 0, '+', $1, $3); yycopyloc($$, &@$); }
      | exp '-' exp { $$ = newExpression(ExpCalc, 0, '-', $1, $3); yycopyloc($$, &@$); }
      | exp '*' exp { $$ = newExpression(ExpCalc, 0, '*', $1, $3); yycopyloc($$, &@$); }
      | exp '/' exp { $$ = newExpression(ExpCalc, 0, '/', $1, $3); yycopyloc($$, &@$); }
      ;

explist : exp { $$ = list_new(); list_append($$, cast(Object *, $1)); }
        | explist ',' exp { $$ = $1; list_append($$, cast(Object *, $3)); }
        ;

array : '[' ']' { $$ = list_new(); }
      | '[' explist ']' { $$ = $2; }
      ;

var : IDENTIFIER { $$ = newVar($1, NULL); yycopyloc($$, &@$); }
    | prefixexp FIELD STRING { $$ = newVar($3, $1); yycopyloc($$, &@$); }
    ;

retstat : RET exp { $$ = $2; }
        ;

packageList : package { $$ = list_new(); list_append($$, cast(Object *, $1)); }
            | packageList package {
                $$ = $1;
                list_append($$, cast(Object *, $2));
              }
            ;

package     : PKGSTART IDENTIFIER generalList { $$ = newPackage($2, $3); yycopyloc($$, &@$); }
            ;

generalList : %empty { $$ = list_new(); }
            | generalList general {
                $$ = $1;
                list_append($$, cast(Object *, $2));
              }
            ;

general     : '#' KINGDOM STRING IDENTIFIER NUMBER GENDER INTERID '[' stringList ']'
                {
                  $$ = newGeneral($4, $2, $5, $3, $6, $7, $9);
                  yycopyloc($$, &@$);
                }
            | '#' KINGDOM STRING IDENTIFIER NUMBER GENDER '[' stringList ']'
                {
                  $$ = newGeneral($4, $2, $5, $3, $6, NULL, $8);
                  yycopyloc($$, &@$);
                }
            | '#' KINGDOM STRING IDENTIFIER NUMBER '[' stringList ']'
                {
                  $$ = newGeneral($4, $2, $5, $3, NULL, NULL, $7);
                  yycopyloc($$, &@$);
                }
            ;

stringList  : %empty  { $$ = list_new(); }
            | stringList STRING {
                $$ = $1;
                list_append($$, cast(Object *, $2));
              }
            ;

/* special function calls */
action_stat : action { $$ = $1; }
            | action args { $$ = $1; hash_copy($$->params, $2); }
            ;

action      : drawCards { $$ = $1; yycopyloc($$, &@$); }
            | loseHp { $$ = $1; yycopyloc($$, &@$); }
            | loseMaxHp { $$ = $1; yycopyloc($$, &@$); }
            | causeDamage { $$ = $1; yycopyloc($$, &@$); }
            | inflictDamage { $$ = $1; yycopyloc($$, &@$); }
            | recoverHp { $$ = $1; yycopyloc($$, &@$); }
            | recoverMaxHp { $$ = $1; yycopyloc($$, &@$); }
            | acquireSkill { $$ = $1; yycopyloc($$, &@$); }
            | detachSkill { $$ = $1; yycopyloc($$, &@$); }
            | addMark { $$ = $1; yycopyloc($$, &@$); }
            | loseMark { $$ = $1; yycopyloc($$, &@$); }
            | getMark { $$ = $1; yycopyloc($$, &@$); }
            | askForChoice { $$ = $1; yycopyloc($$, &@$); }
            | askForChoosePlayer { $$ = $1; yycopyloc($$, &@$); }
            | askForSkillInvoke { $$ = $1; yycopyloc($$, &@$); }
            | obtainCard { $$ = $1; yycopyloc($$, &@$); }
            | arrayPrepend { $$ = $1; yycopyloc($$, &@$); }
            | arrayAppend { $$ = $1; yycopyloc($$, &@$); }
            | arrayRemoveOne { $$ = $1; yycopyloc($$, &@$); }
            | arrayAt { $$ = $1; yycopyloc($$, &@$); }
            | hasSkill { $$ = $1; yycopyloc($$, &@$); }
            ;

drawCards : exp DRAW exp ZHANG CARD {
              $$ = newFunccall(
                    strdup("__drawCards"),
                    newParams(2, "玩家", $1, "数量", $3)
                  );
            }
          ;

loseHp  : exp LOSE exp DIAN HP {
            $$ = newFunccall(
                  strdup("__loseHp"),
                  newParams(2, "玩家", $1, "数量", $3)
                );
          }
        ;

loseMaxHp : exp LOSE exp DIAN HP MAX {
              $$ = newFunccall(
                    strdup("__loseMaxHp"),
                    newParams(2, "玩家", $1, "数量", $3)
                  );
            }
          ;

causeDamage : exp TO exp CAUSE exp DIAN DAMAGE {
                $$ = newFunccall(
                      strdup("__damage"),
                      newParams(3, "伤害来源", $1, "伤害目标", $3, "伤害值", $5)
                    );
              }
            ;

inflictDamage : exp INFLICT exp DIAN DAMAGE {
                  $$ = newFunccall(
                        strdup("__damage"),
                        newParams(2, "伤害目标", $1, "伤害值", $3)
                      );
                }
              ;

recoverHp : exp RECOVER exp DIAN HP {
              $$ = newFunccall(
                    strdup("__recover"),
                    newParams(2, "玩家", $1, "数量", $3)
                  );
            }
          ;

recoverMaxHp : exp RECOVER exp DIAN HP MAX {
                $$ = newFunccall(
                      strdup("__recoverMaxHp"),
                      newParams(2, "玩家", $1, "数量", $3)
                    );
              }
             ;

acquireSkill  : exp ACQUIRE SKILL exp {
                  $$ = newFunccall(
                        strdup("__acquireSkill"),
                        newParams(2, "玩家", $1, "技能", $4)
                      );
                }
              ;

detachSkill : exp LOSE SKILL exp {
                $$ = newFunccall(
                      strdup("__loseSkill"),
                      newParams(2, "玩家", $1, "技能", $4)
                    );
              }
            ;

addMark : exp ACQUIRE exp MEI exp MARK {
            $$ = newFunccall(
                  strdup("__addMark"),
                  newParams(3, "玩家", $1, "标记", $5, "数量", $3)
                );
          }
        | exp ACQUIRE exp MEI exp HIDDEN MARK {
            $$ = newFunccall(
                  strdup("__addMark"),
                  newParams(4, "玩家", $1, "标记", $5, "数量", $3,
                            "隐藏", newExpression(ExpBool, 1, 0, NULL, NULL))
                );
          }
        ;

loseMark  : exp LOSE exp MEI exp MARK {
              $$ = newFunccall(
                  strdup("__loseMark"),
                  newParams(3, "玩家", $1, "标记", $5, "数量", $3)
                );
            }
          | exp LOSE exp MEI exp HIDDEN MARK {
              $$ = newFunccall(
                  strdup("__loseMark"),
                  newParams(4, "玩家", $1, "标记", $5, "数量", $3,
                            "隐藏", newExpression(ExpBool, 1, 0, NULL, NULL))
                );
            }
          ;

getMark : exp exp MARK COUNT {
            $$ = newFunccall(
                  strdup("__getMark"),
                  newParams(2, "玩家", $1, "标记", $2)
                );
          }
        | exp exp HIDDEN MARK COUNT {
            $$ = newFunccall(
                  strdup("__getMark"),
                  newParams(3, "玩家", $1, "标记", $2,
                            "隐藏", newExpression(ExpBool, 1, 0, NULL, NULL))
                );
          }
        ;

askForChoice : exp FROM exp SELECT ANITEM {
                $$ = newFunccall(
                      strdup("__askForChoice"),
                      newParams(2, "玩家", $1, "选项列表", $3)
                    );
              }
            ;

askForChoosePlayer : exp FROM exp SELECT ANPLAYER {
                      $$ = newFunccall(
                            strdup("__askForPlayerChosen"),
                            newParams(2, "玩家", $1, "可选列表", $3)
                          );
                    }
                  ;

askForSkillInvoke : exp SELECT INVOKE exp {
                      $$ = newFunccall(
                            strdup("__askForSkillInvoke"),
                            newParams(2, "玩家", $1, "技能", $4)
                          );
                    }
                  ;

obtainCard : exp ACQUIRE CARD exp {
                $$ = newFunccall(
                      strdup("__obtainCard"),
                      newParams(2, "玩家", $1, "卡牌", $4)
                    );
              }
          ;

arrayPrepend : TOWARD exp PREPEND exp {
                $$ = newFunccall(
                      strdup("__loseSkill"),
                      newParams(2, "array", $2, "value", $4)
                    );
              }
          ;

arrayAppend : TOWARD exp APPEND exp {
                $$ = newFunccall(
                      strdup("__append"),
                      newParams(2, "array", $2, "value", $4)
                    );
              }
          ;

arrayRemoveOne : FROM exp DELETE exp {
                  $$ = newFunccall(
                        strdup("__removeOne"),
                        newParams(2, "array", $2, "value", $4)
                      );
                }
          ;

arrayAt : exp DI exp GE ELEMENT {
            $$ = newFunccall(
                  strdup("__at"),
                  newParams(2, "array", $1, "index", $3)
                );
          }
          ;

hasSkill : exp HAVE SKILL exp {
            $$ = newFunccall(
                  strdup("__hasSkill"),
                  newParams(2, "玩家", $1, "技能", $4)
                );
          }
         ;

%%

static int yyreport_syntax_error(const yypcontext_t *ctx) {
  int res = 0;
  char buf[2048];
  char buf2[1024];
  YYLTYPE *loc = yypcontext_location(ctx);
  sprintf(buf, "语法错误");
  // Report the unexpected token.
  {
    yysymbol_kind_t lookahead = yypcontext_token(ctx);
    if (lookahead != YYSYMBOL_YYEMPTY) {
      sprintf(buf2, ": 未预料到的符号 %s", yytr(yysymbol_name(lookahead)));
      strcat(buf, buf2);
    }
  }
  // Report the tokens expected at this point.
  {
    enum { TOKENMAX = 5 };
    yysymbol_kind_t expected[TOKENMAX];
    int n = yypcontext_expected_tokens(ctx, expected, TOKENMAX);
    if (n < 0)
      // Forward errors to yyparse.
      res = n;
    else
      for (int i = 0; i < n; ++i) {
        sprintf(buf2, "%s %s", i == 0 ? "， 需要" : " 或者",
                yytr(yysymbol_name(expected[i])));
        strcat(buf, buf2);
      }
  }
  yyerror(loc, "%s", buf);
  return res;
}

