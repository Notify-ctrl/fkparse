%{
#include "main.h"
#include "enums.h"
#include "ast.h"
#include "object.h"

/* For travering List in switch-case. */
static List *iter;
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

%initial-action {
  sym_init();
  strtab = hash_new();
  restrtab = list_new();
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

%type <extension> extension
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
/*%type <func_call> action_stat action
%type <func_call> drawCards loseHp causeDamage inflictDamage recoverHp
%type <func_call> acquireSkill detachSkill
%type <func_call> addMark loseMark getMark
%type <func_call> askForChoice askForChoosePlayer
%type <func_call> askForSkillInvoke obtainCard hasSkill
%type <func_call> arrayPrepend arrayAppend arrayRemoveOne arrayAt
%type <func_call> loseMaxHp recoverMaxHp*/

%type <exp> exp prefixexp opexp
%type <var> var
%type <func_call> func_call

%start extension
%define parse.error detailed
%define parse.trace

%%

extension : funcdefList skillList packageList
              {
                $$ = newExtension($1, $2, $3);
                analyzeExtension($$);
              }
          ;

funcdefList : %empty { $$ = list_new(); }
            | funcdefList funcdef {
                $$ = $1;
                list_append($1, cast(Object *, $2));
              }
            ;

funcdef : FUNCDEF IDENTIFIER defargs block
          { $$ = newFuncdef($2, $3, TNone, $4); }
        | FUNCDEF IDENTIFIER defargs RETURN TYPE block
          { $$ = newFuncdef($2, $3, $5, $6); }
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
         { $$ = newDefarg($1, $3, NULL); }
       | IDENTIFIER ':' TYPE '=' exp
         { $$ = newDefarg($1, $3, $5); }
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
              }
          | '$' IDENTIFIER STRING FREQUENCY skillspecs
              {
                $$ = newSkill($2, $3, $4, NULL, $5);
              }
          | '$' IDENTIFIER STRING skillspecs
              {
                $$ = newSkill($2, $3, NULL, NULL, $4);
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

triggerspecs  : triggerspec { $$ = list_new(); }
              | triggerspecs triggerspec {
                  $$ = $1;
                  list_append($$, cast(Object *, $2));
                }
              ;

triggerspec : EVENTI EVENT EFFECT block { $$ = newTriggerSpec($2, NULL, $4); }
            | EVENTI EVENT COND block EFFECT block { $$ = newTriggerSpec($2, $4, $6); }
            ;

block   : statements  { $$ = newBlock($1, NULL); }
        | statements retstat  { $$ = newBlock($1, $2); }
        ;

statements  : %empty { $$ = list_new(); }
            | statements statement { $$ = $1; list_append($1, cast(Object *, $2)); }
            ;

statement   : assign_stat { $$ = cast(Object *, $1); }
            | if_stat { $$ = cast(Object *, $1); }
            | loop_stat { $$ = cast(Object *, $1); }
            | traverse_stat { $$ = cast(Object *, $1); }
            | BREAK { $$ = malloc(sizeof(Object)); $$->objtype = Obj_Break; }
            | func_call { $$ = cast(Object *, $1); }
            //| action_stat { $$ = cast(Object *, $1); }
            ;

assign_stat : LET var EQ exp { $$ = newAssign($2, $4); }
            ;

if_stat : IF exp THEN block END { $$ = newIf($2, $4, NULL); }
        | IF exp THEN block ELSE block END { $$ = newIf($2, $4, $6); }
        ;

loop_stat : REPEAT block UNTIL exp { $$ = newLoop($2, $4); }
          ;

traverse_stat : TO exp IN EVERY IDENTIFIER REPEAT block END
                { $$ = newTraverse($2, $5, $7); }
              ;

func_call : CALL IDENTIFIER args { $$ = newFunccall($2, $3); }
          ;

args : '{' arglist '}' {
          $$ = hash_new();
          list_foreach(iter, $2) {
            hash_set($$, cast(const char *, cast(struct ast *, iter->data)->l),
                    cast(void *, cast(struct ast *, iter->data)->r));
          }
        }
     | '{' '}' { $$ = hash_new(); }
     ;

arglist : arglist ',' arg { $$ = $1; list_append($$, cast(Object *, $3)); }
        | arg { $$ = list_new(); list_append($$, cast(Object *, $1)); }
        ;

arg : IDENTIFIER ':' exp { $$ = newast(N_Arg, cast(struct ast *, $1),
                                      cast(struct ast *, $3)); }
    ;

exp : FALSE { $$ = newExpression(ExpBool, 0, 0, NULL, NULL); }
    | TRUE { $$ = newExpression(ExpBool, 1, 0, NULL, NULL); }
    | NUMBER { $$ = newExpression(ExpNum, $1, 0, NULL, NULL); }
    | STRING { $$ = newExpression(ExpStr, 0, 0, NULL, NULL); $$->strvalue = $1; }
    | prefixexp { $$ = $1; }
    | opexp { $$ = $1; }
    /*| '(' action_stat ')'
      {
        $$ = newexp(ExpAction, 0, 0, (struct astExp *)$2, NULL);
        ((struct astAction *)($2->l))->standalone = false;
      }*/
    | array { $$ = newExpression(ExpArray, 0, 0, NULL, NULL); $$->array = $1; }
    ;

prefixexp : var { $$ = newExpression(ExpVar, 0, 0, NULL, NULL); $$->varValue = $1; }
      | '(' func_call ')'
          { $$ = newExpression(ExpFunc, 0, 0, NULL, NULL); $$->func = $2; }
      | '(' exp ')' { $$ = $2; $$->bracketed = 1; }
      ;

opexp : exp CMP exp { $$ = newExpression(ExpCmp, 0, $2, $1, $3); }
      | exp LOGICOP exp { $$ = newExpression(ExpLogic, 0, $2, $1, $3); }
      | exp '+' exp { $$ = newExpression(ExpCalc, 0, '+', $1, $3); }
      | exp '-' exp { $$ = newExpression(ExpCalc, 0, '-', $1, $3); }
      | exp '*' exp { $$ = newExpression(ExpCalc, 0, '*', $1, $3); }
      | exp '/' exp { $$ = newExpression(ExpCalc, 0, '/', $1, $3); }
      ;

explist : exp { $$ = list_new(); list_append($$, cast(Object *, $1)); }
        | explist ',' exp { $$ = $1; list_append($$, cast(Object *, $3)); }
        ;

array : '[' ']' { $$ = list_new(); }
      | '[' explist ']' { $$ = $2; }
      ;

var : IDENTIFIER { $$ = newVar($1, NULL); }
    | prefixexp FIELD STRING { $$ = newVar($3, $1); }
    ;

retstat : RET exp { $$ = $2; }
        ;

packageList : package { $$ = list_new(); list_append($$, cast(Object *, $1)); }
            | packageList package {
                $$ = $1;
                list_append($$, cast(Object *, $2));
              }
            ;

package     : PKGSTART IDENTIFIER generalList { $$ = newPackage($2, $3); }
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
                }
            | '#' KINGDOM STRING IDENTIFIER NUMBER GENDER '[' stringList ']'
                {
                  $$ = newGeneral($4, $2, $5, $3, $6, NULL, $8);
                }
            | '#' KINGDOM STRING IDENTIFIER NUMBER '[' stringList ']'
                {
                  $$ = newGeneral($4, $2, $5, $3, NULL, NULL, $7);
                }
            ;

stringList  : %empty  { $$ = list_new(); }
            | stringList STRING {
                $$ = $1;
                list_append($$, cast(Object *, $2));
              }
            ;

/* special function calls */
/*
action_stat : action { $$ = $1; } //{ $$ = newast(N_Stat_Action, $1, NULL); }
            | action args { $$ = newast(N_Stat_Action, $1, $2); }
            ;

action      : drawCards { $$ = $1; } //{ $$ = newaction(ActionDrawcard, $1); }
            | loseHp { $$ = newaction(ActionLosehp, $1); }
            | loseMaxHp { $$ = newaction(ActionLoseMaxHp, $1); }
            | causeDamage { $$ = newaction(ActionDamage, $1); }
            | inflictDamage { $$ = newaction(ActionDamage, $1); }
            | recoverHp { $$ = newaction(ActionRecover, $1); }
            | recoverMaxHp { $$ = newaction(ActionRecoverMaxHp, $1); }
            | acquireSkill { $$ = newaction(ActionAcquireSkill, $1); }
            | detachSkill { $$ = newaction(ActionDetachSkill, $1); }
            | addMark { $$ = newaction(ActionMark, $1); }
            | loseMark  { $$ = newaction(ActionMark, $1); }
            | getMark { $$ = newaction(ActionMark, $1); }
            | askForChoice { $$ = newaction(ActionAskForChoice, $1); }
            | askForChoosePlayer { $$ = newaction(ActionAskForPlayerChosen, $1); }
            | askForSkillInvoke { $$ = newaction(ActionAskForSkillInvoke, $1); }
            | obtainCard { $$ = newaction(ActionObtainCard, $1); }
            | arrayPrepend { $$ = newaction(ArrayPrepend, $1); }
            | arrayAppend { $$ = newaction(ArrayAppend, $1); }
            | arrayRemoveOne { $$ = newaction(ArrayRemoveOne, $1); }
            | arrayAt { $$ = newaction(ArrayAt, $1); }
            | hasSkill { $$ = newaction(ActionHasSkill, $1); }
            ;

drawCards : exp DRAW exp ZHANG CARD // { $$ = newast(-1, $1, $3); }
            {
              $$ = newast(N_Stat_Funccall, newstr("__摸牌"),
              newast(N_Args,
                newast(N_Args, NULL,
                  newast(N_Arg, newstr("玩家"), $1)
                ), newast(N_Arg, newstr("摸牌数量"), $3)));
            }
          ;

loseHp  : exp LOSE exp DIAN HP { $$ = newast(-1, $1, $3); }
        ;

loseMaxHp : exp LOSE exp DIAN HP MAX { $$ = newast(-1, $1, $3); }
          ;

causeDamage : exp TO exp CAUSE exp DIAN DAMAGE
              { $$ = newdamage($1, $3, $5); }
            ;

inflictDamage : exp INFLICT exp DIAN DAMAGE
                { $$ = newdamage(NULL, $1, $3); }
              ;

recoverHp : exp RECOVER exp DIAN HP { $$ = newast(-1, $1, $3); }
          ;

recoverMaxHp : exp RECOVER exp DIAN HP MAX { $$ = newast(-1, $1, $3); }
             ;

acquireSkill  : exp ACQUIRE SKILL exp { $$ = newast(-1, $1, $4); }
              ;

detachSkill : exp LOSE SKILL exp { $$ = newast(-1, $1, $4); }
            ;

addMark : exp ACQUIRE exp MEI STRING MARK
          { $$ = newmark($1, $5, $3, 0, 1); free($5); }
        | exp ACQUIRE exp MEI STRING HIDDEN MARK
          { $$ = newmark($1, $5, $3, 1, 1); free($5); }
        ;

loseMark  : exp LOSE exp MEI STRING MARK
            { $$ = newmark($1, $5, $3, 0, 2); free($5); }
          | exp LOSE exp MEI STRING HIDDEN MARK
            { $$ = newmark($1, $5, $3, 1, 2); free($5); }
          ;

getMark : exp STRING MARK COUNT
          { $$ = newmark($1, $2, NULL, 0, 3); free($2); }
        | exp STRING HIDDEN MARK COUNT
          { $$ = newmark($1, $2, NULL, 1, 3); free($2); }
        ;

askForChoice : exp FROM exp SELECT ANITEM
          { $$ = newast(-1, $1, $3); }
          ;

askForChoosePlayer : exp FROM exp SELECT ANPLAYER
          { $$ = newast(-1, $1, $3); }
          ;

askForSkillInvoke : exp SELECT INVOKE STRING
          { $$ = newast(-1, $1, newstr($4)); }
          ;

obtainCard : exp ACQUIRE CARD exp
          { $$ = newast(-1, $1, $4); }
          ;

arrayPrepend : TOWARD exp PREPEND exp
          { $$ = newast(-1, $2, $4); }
          ;

arrayAppend : TOWARD exp APPEND exp
          { $$ = newast(-1, $2, $4); }
          ;

arrayRemoveOne : FROM exp DELETE exp
          { $$ = newast(-1, $2, $4); }
          ;

arrayAt : exp DI exp GE ELEMENT
          { $$ = newast(-1, $1, $3); }
          ;

hasSkill : exp HAVE SKILL STRING
         { $$ = newast(-1, $1, newstr($4)); }
         ;
*/
%%
