%code requires {

typedef struct {
  int tag;
  BlockObj *block;
} StatusFunc;

}

%{
#include "main.h"
#include "enums.h"
#include "object.h"
#include "grammar.h"
#include "error.h"

/* For travering List in switch-case. */
static List *iter;

static ExpressionObj *tempExp;

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

static StatusFunc *newStatusFunc(int tag, BlockObj *block) {
  StatusFunc *ret = malloc(sizeof(StatusFunc));
  ret->tag = tag;
  ret->block = block;
  return ret;
}

%}

%union {
  int enum_v;
  long long i;
  char *s;

  List *list;
  Hash *hash;
  Object *any;

  ExtensionObj *extension;
  PackageObj *package;
  GeneralObj *general;
  CardObj *card;
  SkillObj *skill;
  SkillSpecObj *skillspec;
  TriggerSpecObj *trigger_spec;
  ActiveSpecObj *active_spec;
  ViewAsSpecObj *vs_spec;
  StatusSpecObj *status_spec;
  StatusFunc *status_func;

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
  ArgObj *arg;

  void *tmp[2];
}

%token <i> NUMBER
%token <s> IDENTIFIER
%token <s> STRING
%token <s> FREQUENCY
%token <s> GENDER
%token <s> KINGDOM
%token <s> CARDTYPE
%token PKGSTART
%token TRIGGER EVENTI COND COST EFFECT HOWCOST DOCOST REFRESH
%token ACTIVE CARD_FILTER TARGET_FILTER FEASIBLE ON_USE
%token VIEWAS VSRULE RESPONSECOND RESPONSABLE
%token STATUSSKILL IS_PROHIBITED DISTANCE_CORRECT MAX_EXTRA MAX_FIXED
%token TMD_RESIDUE TMD_DISTANCE TMD_EXTARGET ATKRANGE_EXTRA ATKRANGE_FIXED
%token FUNCDEF
%token <enum_v> EVENT
%token LET EQ IF THEN ELSEIF ELSE END REPEAT UNTIL WHILE
%token <enum_v> TYPE
%token RETURN
%token IN EVERY TOWARD PREPEND APPEND DELETE DI GE ELEMENT
%left <enum_v> LOGICOP
%nonassoc <enum_v> CMP
%left '+' '-' '*' '/'
%token FIELD RET
%token FALSE TRUE BREAK
%token DRAW ZHANG CARD LOSE DIAN HP MAX
%token TO CAUSE DAMAGE INFLICT RECOVER ACQUIRE SKILL
%token MEI MARK HIDDEN COUNT
%token FROM SELECT ANITEM ANPLAYER
%token INVOKE HAVE
%token BECAUSE THROW TIMES
%token SPEAK ACT_LINE WASH CHANGEGENERAL CHANGESEAT YU
%token EXEC JUDGE
%token GUANXING PILETOP
%token JIANG RESULT FIX SELF AZHANG USE RESPOND
%token SENDLOG
%token GIVE PINDIAN SWAPCARD
%token TURNOVER EXTRATURN SKIP
%token DAO DISTANCE ATTACK INSIDE AT RANGE ADJACENT
%token EXPECT OTHERPLAYER
%token DE AS
%token PUT PILE
%token THISROUND THISTURN THISPHASE INVOKED
%token SHAN SHA NEED RESPONSE

%type <list> eliflist
%type <list> defargs defarglist 
%type <list> skillspecs triggerSkill triggerspecs
%type <list> statements root_stats arglist explist array
%type <hash> args

%type <defarg> defarg
%type <func_def> funcdef anon_funcdef
%type <package> package
%type <general> general
%type <card> card
%type <skill> skill
%type <skillspec> skillspec
%type <trigger_spec> triggerspec
%type <tmp> cost
%type <active_spec> activespec
%type <vs_spec> vsspec
%type <status_spec> statusspec
%type <list> statusfuncs
%type <status_func> statusfunc

%type <block> block
%type <any> statement root_stat
%type <exp> retstat
%type <traverse> traverse_stat
%type <assign> assign_stat
%type <if_stat> if_stat
%type <loop> loop_stat while_stat
%type <arg> arg
%type <func_call> action_stat action
%type <func_call> drawCards loseHp causeDamage inflictDamage recoverHp
%type <func_call> acquireSkill detachSkill
%type <func_call> addMark loseMark getMark
%type <func_call> askForChoice askForChoosePlayer
%type <func_call> askForSkillInvoke obtainCard hasSkill
%type <func_call> arrayPrepend arrayAppend arrayRemoveOne
%type <func_call> loseMaxHp recoverMaxHp
%type <func_call> throwCardsBySkill getUsedTimes
%type <func_call> broadcastSkillInvoke
%type <func_call> askForDiscard
%type <func_call> swapPile
%type <func_call> changeHero
%type <func_call> swapSeat
%type <func_call> judge
%type <func_call> askForGuanxing
%type <func_call> getNCards
%type <func_call> retrial
%type <func_call> askChooseForCard
%type <func_call> askUseForCard
%type <func_call> askResponseForCard
%type <func_call> askForCardChosen
%type <func_call> chat sendlog
%type <func_call> throwCards giveCards pindian swapCards
%type <func_call> turnOver playExtraTurn skipPhase
%type <func_call> inMyAttackRange distanceTo isAdjacentTo
%type <func_call> getOtherPlayers
%type <func_call> jinknum
%type <func_call> addToPile getPile
%type <func_call> getSkillUsedTimes

%type <exp> exp prefixexp opexp
%type <var> var
%type <func_call> func_call
%type <list> dict_entries
%type <hash> dictionary
%type <arg> dict_entry

%destructor {} <enum_v>
%destructor {} <i>
%destructor {} <tmp>
%destructor { free($$); } <s>
%destructor { list_free($$, freeObject); } <list>
%destructor { hash_free($$, freeObject); } <hash>
%destructor { freeObject($$); } <*>

%start extension
%define parse.error custom
%verbose
%define parse.trace
%define api.pure full
%locations

%%

extension : root_stats {
              extension = newExtension(); 
              extension->stats = $1;
            }
          ;

root_stats  : %empty { $$ = list_new(); }
            | root_stats root_stat { $$ = $1; list_append($$, $2); }
            ;

root_stat : statement
          | skill { $$ = cast(Object *, $1); }
          | package { $$ = cast(Object *, $1); }
          | general { $$ = cast(Object *, $1); }
          | card { $$ = cast(Object *, $1); }
          ;

funcdef : FUNCDEF IDENTIFIER defargs block END
          { $$ = newFuncdef($2, $3, TNone, $4); yycopyloc($$, &@$); }
        | FUNCDEF IDENTIFIER defargs RETURN TYPE block END
          { $$ = newFuncdef($2, $3, $5, $6); yycopyloc($$, &@$); }
        ;

anon_funcdef  : FUNCDEF defargs block END
                { $$ = newFuncdef(strdup(""), $2, TNone, $3); yycopyloc($$, &@$); }
              | FUNCDEF defargs RETURN TYPE block END
                { $$ = newFuncdef(strdup(""), $2, $4, $5); yycopyloc($$, &@$); }
              ;

defargs : '(' defarglist ')' { $$ = $2; }
        | '(' defarglist ',' ')' { $$ = $2; }
        | '(' ')' { $$ = list_new(); }
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
       | IDENTIFIER ':' TYPE EQ exp
         { $$ = newDefarg($1, $3, $5); yycopyloc($$, &@$); }
       ;

skill     : '$' IDENTIFIER STRING FREQUENCY skillspecs END
              {
                $$ = newSkill($2, $3, $4, NULL, $5);
                yycopyloc($$, &@$);
              }
          | '$' IDENTIFIER STRING skillspecs END
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

skillspec   : triggerSkill { $$ = newSkillSpec(Spec_TriggerSkill, $1); }
            | activespec { $$ = newSkillSpec(Spec_ActiveSkill, $1); }
            | vsspec { $$ = newSkillSpec(Spec_ViewAsSkill, $1); }
            | statusspec { $$ = newSkillSpec(Spec_StatusSkill, $1); }
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

triggerspec : EVENTI EVENT cost EFFECT block {
                $$ = newTriggerSpec($2, NULL, $5);
                $$->how_cost = $3[0];
                $$->on_cost = $3[1];
                yycopyloc($$, &@$);
              }
            | EVENTI EVENT COND block cost EFFECT block {
                $$ = newTriggerSpec($2, $4, $7);
                $$->how_cost = $5[0];
                $$->on_cost = $5[1];
                yycopyloc($$, &@$);
              }
            | EVENTI EVENT REFRESH block {
                $$ = newTriggerSpec($2, NULL, $4);
                $$->is_refresh = true;
                yycopyloc($$, &@$);
              }
            | EVENTI EVENT COND block REFRESH block {
                $$ = newTriggerSpec($2, $4, $6);
                $$->is_refresh = true;
                yycopyloc($$, &@$);
              }
            ;

cost :
    HOWCOST block COST block
    { $$[0] = $2; $$[1] = $4; }
  | HOWCOST block
    { $$[0] = $2; $$[1] = NULL; }
  | COST block
    { $$[0] = NULL; $$[1] = $2; }
  | %empty
    { $$[0] = NULL; $$[1] = NULL; }
    ;

activespec  : ACTIVE COND block CARD_FILTER block TARGET_FILTER block FEASIBLE block ON_USE block EFFECT block
              { $$ = newActiveSpec($3, $5, $7, $9, $11, $13); yycopyloc($$, &@$); }
            | ACTIVE COND block CARD_FILTER block TARGET_FILTER block FEASIBLE block ON_USE block
              { $$ = newActiveSpec($3, $5, $7, $9, $11, NULL); yycopyloc($$, &@$); }
            ;

vsspec  : VIEWAS COND block CARD_FILTER block FEASIBLE block VSRULE block
          { $$ = newViewAsSpec($3, $5, $7, $9); yycopyloc($$, &@$); }
        | VIEWAS COND block CARD_FILTER block FEASIBLE block VSRULE block RESPONSECOND block RESPONSABLE array
          {
            $$ = newViewAsSpec($3, $5, $7, $9);
            $$->can_response = $11;
            tempExp = newExpression(ExpArray, 0, 0, NULL, NULL);
            tempExp->array = $13;
            yycopyloc(tempExp, &@13);
            $$->responsable = tempExp;
            yycopyloc($$, &@$);
          }
        ;

statusspec  : STATUSSKILL statusfuncs
              {
                $$ = newStatusSpec();
                list_foreach(iter, $2) {
                  StatusFunc *f = cast(StatusFunc *, iter->data);
                  switch (f->tag) {
                  case IS_PROHIBITED:
                    freeObject($$->is_prohibited);
                    $$->is_prohibited = f->block;
                    break;
                  case CARD_FILTER:
                    freeObject($$->card_filter);
                    $$->card_filter = f->block;
                    break;
                  case VSRULE:
                    freeObject($$->vsrule);
                    $$->vsrule = f->block;
                    break;
                  case DISTANCE_CORRECT:
                    freeObject($$->distance_correct);
                    $$->distance_correct = f->block;
                    break;
                  case MAX_EXTRA:
                    freeObject($$->max_extra);
                    $$->max_extra = f->block;
                    break;
                  case MAX_FIXED:
                    freeObject($$->max_fixed);
                    $$->max_fixed = f->block;
                    break;
                  case TMD_RESIDUE:
                    freeObject($$->tmd_residue);
                    $$->tmd_residue = f->block;
                    break;
                  case TMD_DISTANCE:
                    freeObject($$->tmd_distance);
                    $$->tmd_distance = f->block;
                    break;
                  case TMD_EXTARGET:
                    freeObject($$->tmd_extarget);
                    $$->tmd_extarget = f->block;
                    break;
                  case ATKRANGE_EXTRA:
                    freeObject($$->atkrange_extra);
                    $$->atkrange_extra = f->block;
                    break;
                  case ATKRANGE_FIXED:
                    freeObject($$->atkrange_fixed);
                    $$->atkrange_fixed = f->block;
                    break;
                  default:
                    break;
                  }
                }
                list_free($2, free);
              }
            ;

statusfuncs : statusfunc
              {
                $$ = list_new();
                list_append($$, cast(Object *, $1));
              }
            | statusfuncs statusfunc
              {
                $$ = $1;
                list_append($$, cast(Object *, $2));
              }
            ;

statusfunc  : IS_PROHIBITED block { $$ = newStatusFunc(IS_PROHIBITED, $2); }
            | CARD_FILTER block { $$ = newStatusFunc(CARD_FILTER, $2); }
            | VSRULE block { $$ = newStatusFunc(VSRULE, $2); }
            | DISTANCE_CORRECT block { $$ = newStatusFunc(DISTANCE_CORRECT, $2); }
            | MAX_EXTRA block { $$ = newStatusFunc(MAX_EXTRA, $2); }
            | MAX_FIXED block { $$ = newStatusFunc(MAX_FIXED, $2); }
            | TMD_RESIDUE block { $$ = newStatusFunc(TMD_RESIDUE, $2); }
            | TMD_DISTANCE block { $$ = newStatusFunc(TMD_DISTANCE, $2); }
            | TMD_EXTARGET block { $$ = newStatusFunc(TMD_EXTARGET, $2); }
            | ATKRANGE_EXTRA block { $$ = newStatusFunc(ATKRANGE_EXTRA, $2); }
            | ATKRANGE_FIXED block { $$ = newStatusFunc(ATKRANGE_FIXED, $2); }
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
            | while_stat { $$ = cast(Object *, $1); }
            | traverse_stat { $$ = cast(Object *, $1); }
            | BREAK { $$ = malloc(sizeof(Object)); $$->objtype = Obj_Break; }
            | DOCOST { $$ = malloc(sizeof(Object)); $$->objtype = Obj_Docost; }
            | func_call ';' { $$ = cast(Object *, $1); }
            | action_stat { $$ = cast(Object *, $1); }
            | funcdef { $$ = cast(Object *, $1); }
            | error { $$ = NULL; }
            ;

assign_stat : var EQ exp { $$ = newAssign($1, $3); yycopyloc($$, &@$); }
            | LET var EQ exp { $$ = newAssign($2, $4); yycopyloc($$, &@$); }
            | var EQ exp AS TYPE { $$ = newAssign($1, $3); $$->custom_type = $5; yycopyloc($$, &@$); }
            | LET var EQ exp AS TYPE { $$ = newAssign($2, $4); $$->custom_type = $6; yycopyloc($$, &@$); }
            ;

if_stat : IF exp THEN block eliflist END { $$ = newIf($2, $4, $5, NULL); yycopyloc($$, &@$); }
        | IF exp THEN block eliflist ELSE block END { $$ = newIf($2, $4, $5, $7); yycopyloc($$, &@$); }
        ;

eliflist  : %empty  { $$ = list_new(); }
          | eliflist ELSEIF exp THEN block {
              $$ = $1;
              list_append($$, cast(Object *, newIf($3, $5, list_new(), NULL)));
            }
          ;

loop_stat : REPEAT block UNTIL exp { $$ = newLoop($2, $4); yycopyloc($$, &@$); }
          ;

while_stat  : WHILE exp REPEAT block END { $$ = newLoop($4, $2); $$->type = 1; yycopyloc($$, &@$); }
            ;

traverse_stat : TO exp IN EVERY IDENTIFIER REPEAT block END
                { $$ = newTraverse($2, $5, $7); yycopyloc($$, &@$); }
              ;

func_call : IDENTIFIER args { $$ = newFunccall($1, $2); yycopyloc($$, &@$); }
          | IDENTIFIER '(' explist ')' { $$ = newFunccall($1, NULL); $$->param_list = $3; yycopyloc($$, &@$); }
          ;

args : '(' arglist ')' {
          $$ = hash_new();
          list_foreach(iter, $2) {
            ArgObj *a = cast(ArgObj *, iter->data);
            hash_set($$, a->name, a->exp);
            free((void *)a->name);
            free(a);
          }
          list_free($2, NULL);
        }
     | '(' arglist ',' ')' {
          $$ = hash_new();
          list_foreach(iter, $2) {
            ArgObj *a = cast(ArgObj *, iter->data);
            hash_set($$, a->name, a->exp);
            free((void *)a->name);
            free(a);
          }
          list_free($2, NULL);
        }
     | '(' ')' { $$ = hash_new(); }
     ;

arglist : arglist ',' arg { $$ = $1; list_append($$, cast(Object *, $3)); }
        | arg { $$ = list_new(); list_append($$, cast(Object *, $1)); }
        ;

arg : IDENTIFIER ':' exp {
        $3->param_name = strdup($1);
        $$ = newArg($1, $3);
      }
    ;

exp : FALSE { $$ = newExpression(ExpBool, 0, 0, NULL, NULL); yycopyloc($$, &@$); }
    | TRUE { $$ = newExpression(ExpBool, 1, 0, NULL, NULL); yycopyloc($$, &@$); }
    | NUMBER { $$ = newExpression(ExpNum, $1, 0, NULL, NULL); yycopyloc($$, &@$); }
    | STRING { $$ = newExpression(ExpStr, 0, 0, NULL, NULL); $$->strvalue = $1; yycopyloc($$, &@$); }
    | prefixexp { $$ = $1; }
    | opexp { $$ = $1; }
    | array { $$ = newExpression(ExpArray, 0, 0, NULL, NULL); $$->array = $1; yycopyloc($$, &@$); }
    | dictionary { $$ = newExpression(ExpDict, 0, 0, NULL, NULL); $$->dict = $1; yycopyloc($$, &@$); }
    | anon_funcdef { $$ = newExpression(ExpFuncdef, 0, 0, NULL, NULL); $$->funcdef = $1; yycopyloc($$, &@$); }
    ;

prefixexp : var { $$ = newExpression(ExpVar, 0, 0, NULL, NULL); $$->varValue = $1; yycopyloc($$, &@$); }
      | func_call
          { $$ = newExpression(ExpFunc, 0, 0, NULL, NULL); $$->func = $1; yycopyloc($$, &@$); }
      | '(' action_stat ')'
        {
          $$ = newExpression(ExpFunc, 0, 0, NULL, NULL);
          $$->func = $2;
          yycopyloc($$, &@$);
        }
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
      | '[' explist ',' ']' { $$ = $2; }
      ;

dict_entries  : dict_entries ',' dict_entry { $$ = $1; list_append($$, cast(Object *, $3)); }
              | dict_entry { $$ = list_new(); list_append($$, cast(Object *, $1)); }
              ;

dict_entry : STRING ':' exp {
               $3->param_name = strdup($1);
               $$ = newArg($1, $3);
             }
            ;

dictionary  : '{' dict_entries '}' {
              $$ = hash_new();
              list_foreach(iter, $2) {
                ArgObj *a = cast(ArgObj *, iter->data);
                hash_set($$, a->name, a->exp);
                free((void *)a->name);
                free(a);
              }
              list_free($2, NULL);
            }
            | '{' dict_entries ',' '}' {
              $$ = hash_new();
              list_foreach(iter, $2) {
                ArgObj *a = cast(ArgObj *, iter->data);
                hash_set($$, a->name, a->exp);
                free((void *)a->name);
                free(a);
              }
              list_free($2, NULL);
            }
            | '{' '}' { $$ = hash_new(); }
            ;

var : IDENTIFIER { $$ = newVar($1, NULL); yycopyloc($$, &@$); }
    | prefixexp FIELD STRING { $$ = newVar($3, $1); yycopyloc($$, &@$); }
    | prefixexp DI exp GE ELEMENT { $$ = newVar(NULL, $1); $$->index = $3; yycopyloc($$, &@$); }
    | prefixexp '[' exp ']' { $$ = newVar(NULL, $1); $$->index = $3; yycopyloc($$, &@$); }
    ;

retstat : RET exp { $$ = $2; }
        ;

package     : PKGSTART IDENTIFIER { $$ = newPackage($2); yycopyloc($$, &@$); }
            ;

general     : '#' KINGDOM STRING IDENTIFIER NUMBER GENDER array
                {
                  $$ = newGeneral($4, $2, $5, $3, $6, NULL, $7);
                  yycopyloc($$, &@$);
                }
            | '#' KINGDOM STRING IDENTIFIER NUMBER array
                {
                  $$ = newGeneral($4, $2, $5, $3, NULL, NULL, $6);
                  yycopyloc($$, &@$);
                }
            ;

card  : '%' IDENTIFIER STRING CARDTYPE STRING
        { $$ = newCard($2, $3, $4, $5); }
      ;

/* special function calls */
action_stat : action { $$ = $1; }
            | action ':' args {
                $$ = $1;
                hash_copy($$->params, $3);
                hash_free($3, NULL);
              }
            ;

action      : drawCards
            | loseHp
            | loseMaxHp
            | causeDamage
            | inflictDamage
            | recoverHp
            | recoverMaxHp
            | acquireSkill
            | detachSkill
            | addMark
            | loseMark
            | getMark
            | askForChoice
            | askForChoosePlayer
            | askForSkillInvoke
            | obtainCard
            | arrayPrepend
            | arrayAppend
            | arrayRemoveOne
            | hasSkill
            | throwCardsBySkill
            | getUsedTimes
            | broadcastSkillInvoke
            | askForDiscard
            | swapPile
            | changeHero
            | swapSeat
            | judge
            | askForGuanxing
            | getNCards
            | retrial
            | askChooseForCard
            | askUseForCard
            | askResponseForCard
            | askForCardChosen
            | chat
            | sendlog
            | throwCards
            | giveCards
            | pindian
            | swapCards
            | turnOver
            | playExtraTurn
            | skipPhase
            | inMyAttackRange
            | distanceTo
            | isAdjacentTo
            | getOtherPlayers
            | addToPile
            | getPile
            | getSkillUsedTimes
            | jinknum
              { $$ = $1; }
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

getMark : exp DE exp MARK COUNT {
            $$ = newFunccall(
                  strdup("__getMark"),
                  newParams(2, "玩家", $1, "标记", $3)
                );
          }
        | exp DE exp HIDDEN MARK COUNT {
            $$ = newFunccall(
                  strdup("__getMark"),
                  newParams(3, "玩家", $1, "标记", $3,
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

hasSkill : exp HAVE SKILL exp {
            $$ = newFunccall(
                  strdup("__hasSkill"),
                  newParams(2, "玩家", $1, "技能", $4)
                );
          }
         ;

throwCardsBySkill : exp BECAUSE SKILL exp THROW CARD exp {
                      $$ = newFunccall(
                        strdup("__throwCardsBySkill"),
                        newParams(3, "玩家", $1, "卡牌列表", $7, "技能名", $4)
                      );
                    }
                  ;

getUsedTimes  : exp INVOKE ACTIVE STRING DE TIMES {
                  tempExp = newExpression(ExpStr, 0, 0, NULL, NULL);
                  tempExp->strvalue = $4;
                  $$ = newFunccall(
                        strdup("__getUsedTimes"),
                        newParams(2, "玩家", $1, "技能名", tempExp)
                      );
                }
              ;

broadcastSkillInvoke  : exp SPEAK STRING DE ACT_LINE {
                          tempExp = newExpression(ExpStr, 0, 0, NULL, NULL);
                          tempExp->strvalue = $3;
                          $$ = newFunccall(
                                strdup("__broadcastSkillInvoke"),
                                newParams(2, "玩家", $1, "技能名", tempExp)
                              );
                        }
                      ;

askForDiscard : exp THROW exp ZHANG CARD {
                  $$ = newFunccall(
                    strdup("__askForDiscard"),
                    newParams(2, "目标", $1, "要求弃置数量", $3)
                  );
                }
              ;

swapPile : exp WASH {
                  $$ = newFunccall(
                    strdup("__swapPile"),
                    newParams(1, "玩家", $1)
                  );
                };
changeHero : exp CHANGEGENERAL exp {
                  $$ = newFunccall(
                    strdup("__changeHero"),
                    newParams(2, "玩家", $1, "新将领", $3)
                  );
                };
swapSeat : exp YU exp CHANGESEAT {
                $$ = newFunccall(
                  strdup("__swapSeat"),
                  newParams(2, "玩家A", $1, "玩家B", $3)
                );
           };
judge : exp EXEC JUDGE {
          $$ = newFunccall(
            strdup("__judge"),
            newParams(1, "玩家", $1)
          );
        };

askForGuanxing : exp TO exp EXEC GUANXING {
          $$ = newFunccall(
            strdup("__askForGuanxing"),
            newParams(2, "玩家", $1, "参与观星的牌", $3)
          );
        };

getNCards: exp SELECT PILETOP exp ZHANG CARD {
          $$ = newFunccall(
            strdup("__getNCards"),
            newParams(2, "玩家", $1, "获得牌的数量", $4)
          );
        };

retrial: exp JIANG JUDGE RESULT FIX EQ exp {
          $$ = newFunccall(
            strdup("__retrial"),
            newParams(2, "玩家", $1, "改判牌", $7)
          );
        };

askChooseForCard: exp SELECT SELF DE AZHANG CARD {
          $$ = newFunccall(
            strdup("__askForCard"),
            newParams(1, "玩家", $1)
          );
        };

askUseForCard: exp USE AZHANG CARD {
          $$ = newFunccall(
            strdup("__askUseForCard"),
            newParams(1, "玩家", $1)
          );
        };

askResponseForCard: exp RESPOND AZHANG CARD {
           $$ = newFunccall(
             strdup("__askRespondForCard"),
             newParams(1, "玩家", $1)
           );
         };

askForCardChosen: exp SELECT exp AZHANG CARD {
           $$ = newFunccall(
             strdup("__askForCardChosen"),
             newParams(2, "玩家", $1, "被选牌者", $3)
           );
         };

chat  : exp SPEAK exp {
          $$ = newFunccall(
              strdup("__chat"),
              newParams(2, "玩家", $1, "聊天句子", $3)
            );
        }
      ;

sendlog : exp SENDLOG exp {
            $$ = newFunccall(
              strdup("__sendlog"),
              newParams(2, "玩家", $1, "战报", $3)
            );
          }
          ;

throwCards : exp THROW CARD exp {
                $$ = newFunccall(
                  strdup("__throwCards"),
                  newParams(2, "玩家", $1, "卡牌列表", $4)
                );
              }
            ;

giveCards : exp GIVE exp CARD exp {
              $$ = newFunccall(
                strdup("__giveCards"),
                newParams(3, "来源", $1, "目标", $3, "卡牌列表", $5)
              );
            }
          ;

pindian   : exp YU exp PINDIAN {
              $$ = newFunccall(
                strdup("__pindian"),
                newParams(2, "来源", $1, "目标", $3)
              );
            }
          ;

swapCards : exp YU exp SWAPCARD {
              $$ = newFunccall(
                strdup("__swapCards"),
                newParams(2, "来源", $1, "目标", $3)
              );
            }
          ;

turnOver  : exp TURNOVER {
              $$ = newFunccall(
                strdup("__turnOver"),
                newParams(1, "玩家", $1)
              );

            }
          ;

playExtraTurn : exp EXEC EXTRATURN {
                  $$ = newFunccall(
                    strdup("__playExtraTurn"),
                    newParams(1, "玩家", $1)
                  );
                }
              ;

skipPhase : exp SKIP exp {
              $$ = newFunccall(
                strdup("__skipPhase"),
                newParams(2, "玩家", $1, "阶段", $3)
              );
            }
          ;

inMyAttackRange : exp AT exp ATTACK RANGE INSIDE {
                $$ = newFunccall(
                  strdup("__inMyAttackRange"),
                  newParams(2, "玩家", $3, "目标", $1)
                );
             }
            ;

distanceTo : exp DAO exp DISTANCE {
                $$ = newFunccall(
                  strdup("__distanceTo"),
                  newParams(2, "玩家", $1, "目标", $3)
                );
             }
            ;

isAdjacentTo : exp YU exp ADJACENT {
            $$ = newFunccall(
               strdup("__isAdjacentTo"),
               newParams(2, "玩家", $1, "目标", $3)
            );
         }
      ;

getOtherPlayers : exp EXPECT OTHERPLAYER {
          $$ = newFunccall(
            strdup("__getOtherPlayers"),
            newParams(1, "排除的玩家", $1)
          );
        }
      ;

addToPile : JIANG exp PUT exp DE PILE exp IN {
              $$ = newFunccall(
                strdup("__addToPile"),
                newParams(3, "加入的牌", $2, "玩家", $4, "牌堆名", $7)
              );
            }
          ;

getPile : exp DE PILE exp IN DE CARD {
            $$ = newFunccall(
              strdup("__getPile"),
              newParams(2, "玩家", $1, "牌堆名", $4)
            );
          }
        ;

getSkillUsedTimes :
    exp THISROUND INVOKED exp DE TIMES {
      $$ = newFunccall(
        strdup("__getSkillUsedTimes"),
        newParams(3, "玩家", $1, "技能名", $4,
                    "格局", newExpression(ExpNum, 1, 0, NULL, NULL))
      );
    }
  | exp THISTURN INVOKED exp DE TIMES {
      $$ = newFunccall(
        strdup("__getSkillUsedTimes"),
        newParams(3, "玩家", $1, "技能名", $4,
                    "格局", newExpression(ExpNum, 2, 0, NULL, NULL))
      );
    }
  | exp THISPHASE INVOKED exp DE TIMES {
      $$ = newFunccall(
        strdup("__getSkillUsedTimes"),
        newParams(3, "玩家", $1, "技能名", $4,
                    "格局", newExpression(ExpNum, 3, 0, NULL, NULL))
      );
    }
  ;

// # 令 对 # 使用 的 杀 需 # 张 闪 响应
jinknum : exp LET TO exp USE DE SHA NEED exp ZHANG SHAN RESPONSE {
          $$ = newFunccall(
            strdup("__jinknum"),
            newParams(3, "玩家", $1, "目标", $4, "需闪数", $9)
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

