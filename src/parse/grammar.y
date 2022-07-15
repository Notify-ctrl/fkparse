%{
#include "main.h"
#include "enums.h"
#include "ast.h"
#include "object.h"
%}

%union {
  struct ast *a;
  int enum_v;
  long long i;
  char *s;
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

%type <a> extension
%type <a> funcdefList funcdef defargs defarglist defarg
%type <a> skillList skill
%type <a> packageList package
%type <a> generalList general
%type <a> stringList
%type <a> skillspecs skillspec
%type <a> triggerSkill triggerspecs triggerspec

%type <a> block statements statement retstat traverse_stat
%type <a> assign_stat if_stat loop_stat action_stat action args arglist arg
%type <a> drawCards loseHp causeDamage inflictDamage recoverHp
%type <a> acquireSkill detachSkill
%type <a> addMark loseMark getMark
%type <a> askForChoice askForChoosePlayer
%type <a> askForSkillInvoke obtainCard hasSkill
%type <a> arrayPrepend arrayAppend arrayRemoveOne arrayAt
%type <a> loseMaxHp recoverMaxHp

%type <a> exp prefixexp opexp var
%type <a> explist array func_call

%start extension
%define parse.error detailed
%define parse.trace

%%

extension : funcdefList skillList packageList
              {
                $$ = newextension($1, $2, $3);
                analyzeExtension(newExtension($$));
              }
          ;

funcdefList : %empty { $$ = newast(N_Funcdefs, NULL, NULL); }
            | funcdefList funcdef { $$ = newast(N_Funcdefs, $1, $2); }
            ;

funcdef : FUNCDEF IDENTIFIER defargs block
          { $$ = newfuncdef(newstr($2), $3, TNone, $4); }
        | FUNCDEF IDENTIFIER defargs RETURN TYPE block
          { $$ = newfuncdef(newstr($2), $3, $5, $6); }
        ;

defargs : '{' defarglist '}' { $$ = $2; }
        | '{' '}' { $$ = NULL; }
        ;

defarglist : defarglist ',' defarg { $$ = newast(N_Defargs, $1, $3); }
           | defarg {$$ = newast(N_Defargs, NULL, $1); }
           ;

defarg : IDENTIFIER ':' TYPE
         { $$ = newdefarg(newstr($1), $3, NULL); }
       | IDENTIFIER ':' TYPE '=' exp
         { $$ = newdefarg(newstr($1), $3, $5); }
       ;

skillList : %empty  { $$ = newast(N_Skills, NULL, NULL); }
          | skillList skill { $$ = newast(N_Skills, $1, $2); }
          ;

skill     : '$' IDENTIFIER STRING FREQUENCY INTERID skillspecs
              {
                $$ = newskill($2, $3, $4, $5, $6);
                free($2); free($3); free($4); free($5);
              }
          | '$' IDENTIFIER STRING FREQUENCY skillspecs
              {
                $$ = newskill($2, $3, $4, NULL, $5);
                free($2); free($3); free($4);
              }
          | '$' IDENTIFIER STRING skillspecs
              {
                $$ = newskill($2, $3, NULL, NULL, $4);
                free($2); free($3);
              }
          ;

skillspecs  : %empty { $$ = newast(N_SkillSpecs, NULL, NULL); }
            | skillspecs skillspec  { $$ = newast(N_SkillSpecs, $1, $2); }
            ;

skillspec   : triggerSkill  { $$ = $1; }
            ;

triggerSkill  : TRIGGER triggerspecs  { $$ = newast(N_TriggerSkill, $2, NULL); }
              ;

triggerspecs  : triggerspec { $$ = newast(N_TriggerSpecs, NULL, $1); }
              | triggerspecs triggerspec  { $$ = newast(N_TriggerSpecs, $1, $2); }
              ;

triggerspec   : EVENTI EVENT EFFECT block { $$ = newtriggerspec($2, NULL, $4); }
              | EVENTI EVENT COND block EFFECT block  { $$ = newtriggerspec($2, $4, $6); }
              ;

block   : statements  { $$ = newast(N_Block, $1, NULL); }
        | statements retstat  { $$ = newast(N_Block, $1, $2); }
        ;

statements  : %empty  { $$ = newast(N_Stats, NULL, NULL); }
            | statements statement  { $$ = newast(N_Stats, $1, $2); }
            ;

statement   : ';' { $$ = newast(N_Stat_None, NULL, NULL); }
            | assign_stat { $$ = $1; }
            | if_stat { $$ = $1; }
            | loop_stat { $$ = $1; }
            | traverse_stat { $$ = $1; }
            | BREAK { $$ = newast(N_Stat_Break, NULL, NULL); }
            | func_call { $$ = $1; }
            | action_stat { $$ = $1; }
            ;

assign_stat : LET var EQ exp
              {
                $$ = newast(N_Stat_Assign, $2, $4);
              }
            ;

if_stat : IF exp THEN block END { $$ = newif($2, $4, NULL); }
        | IF exp THEN block ELSE block END  { $$ = newif($2, $4, $6); }
        ;

loop_stat : REPEAT block UNTIL exp { $$ = newast(N_Stat_Loop, $2, $4); }
          ;

traverse_stat : TO exp IN EVERY IDENTIFIER REPEAT block END
                { $$ = newtraverse($2, newstr($5), $7); }
              ;

func_call : CALL IDENTIFIER args  { $$ = newast(N_Stat_Funccall, newstr($2), $3); }
          ;

args : '{' arglist '}' { $$ = $2; }
     | '{' '}' { $$ = NULL; }
     ;

arglist : arglist ',' arg { $$ = newast(N_Args, $1, $3); }
        | arg {$$ = newast(N_Args, NULL, $1); }
        ;

arg : IDENTIFIER ':' exp { $$ = newast(N_Arg, newstr($1), $3); }
    ;

exp : FALSE { $$ = newexp(ExpBool, 0, 0, NULL, NULL); }
    | TRUE { $$ = newexp(ExpBool, 1, 0, NULL, NULL); }
    | NUMBER { $$ = newexp(ExpNum, $1, 0, NULL, NULL); }
    | STRING { $$ = newexp(ExpStr, 0, 0, (struct astExp *)newstr($1), NULL); free($1); }
    | prefixexp { $$ = $1; }
    | opexp { $$ = $1; }
    | '(' action_stat ')'
      {
        $$ = newexp(ExpAction, 0, 0, (struct astExp *)$2, NULL);
        ((struct astAction *)($2->l))->standalone = false;
      }
    | array { $$ = $1; }
    ;

opexp : exp CMP exp { $$ = newexp(ExpCmp, 0, $2, (struct astExp *)$1, (struct astExp *)$3); }
      | exp LOGICOP exp { $$ = newexp(ExpLogic, 0, $2, (struct astExp *)$1, (struct astExp *)$3); }
      | exp '+' exp { $$ = newexp(ExpCalc, 0, '+', (struct astExp *)$1, (struct astExp *)$3); }
      | exp '-' exp { $$ = newexp(ExpCalc, 0, '-', (struct astExp *)$1, (struct astExp *)$3); }
      | exp '*' exp { $$ = newexp(ExpCalc, 0, '*', (struct astExp *)$1, (struct astExp *)$3); }
      | exp '/' exp { $$ = newexp(ExpCalc, 0, '/', (struct astExp *)$1, (struct astExp *)$3); }
      ;

prefixexp : var { $$ = newexp(ExpVar, 0, 0, (struct astExp *)$1, NULL); }
          | '(' func_call ')'
              { $$ = newexp(ExpFunc, 0, 0, (struct astExp *)$2, NULL); }
          | '(' exp ')' { $$ = $2; ((struct astExp *)$2)->bracketed = 1; }
          ;

explist : exp { $$ = newast(N_Exps, NULL, $1); }
        | explist ',' exp { $$ = newast(N_Exps, $1, $3); }
        ;

array : '[' ']' { $$ = newexp(ExpArray, 0, 0, NULL, NULL); }
      | '[' explist ']' { $$ = newexp(ExpArray, 0, 0, (struct astExp *)$2, NULL); }
      ;

var : IDENTIFIER { $$ = newast(N_Var, newstr($1), NULL); free($1); }
    | prefixexp FIELD STRING { $$ = newast(N_Var, newstr($3), $1); free($3); }
    ;

retstat : RET exp
          { $$ = newast(N_Stat_Ret, $2, NULL); }
        ;

packageList : package { $$ = newast(N_Packages, NULL, $1); }
            | packageList package { $$ = newast(N_Packages, $1, $2); }
            ;

package     : PKGSTART IDENTIFIER generalList {
                $$ = newpackage($2, $3);
                free($2);
              }
            ;

generalList : %empty { $$ = newast(N_Generals, NULL, NULL); }
            | generalList general { $$ = newast(N_Generals, $1, $2); }
            ;

general     : '#' KINGDOM STRING IDENTIFIER NUMBER GENDER INTERID '[' stringList ']'
                {
                  $$ = newgeneral($4, $2, $5, $3, $6, $7, $9);
                  free($2); free($3); free($4); free($6); free($7);
                }
            | '#' KINGDOM STRING IDENTIFIER NUMBER GENDER '[' stringList ']'
                {
                  $$ = newgeneral($4, $2, $5, $3, $6, NULL, $8);
                  free($2); free($3); free($4); free($6);
                }
            | '#' KINGDOM STRING IDENTIFIER NUMBER '[' stringList ']'
                {
                  $$ = newgeneral($4, $2, $5, $3, NULL, NULL, $7);
                  free($2); free($3); free($4);
                }
            ;

stringList  : %empty  { $$ = newast(N_Strs, NULL, NULL); }
            | stringList STRING { $$ = newast(N_Strs, $1, newstr($2)); free($2); }
            ;

/* special function calls */

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

%%
