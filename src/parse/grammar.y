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
%token <enum_v> EVENT
%token LET EQ IF THEN ELSE END REPEAT UNTIL
%left <enum_v> LOGICOP
%left '+' '-' '*' '/'
%nonassoc <enum_v> CMP
%token FIELD RET
%token FALSE TRUE BREAK
%token DRAW ZHANG CARD LOSE DIAN HP 
%token TO CAUSE DAMAGE INFLICT RECOVER ACQUIRE SKILL
%token MEI MARK HIDDEN COUNT
%token FROM SELECT ANITEM ANPLAYER

%type <a> extension
%type <a> skillList skill
%type <a> packageList package
%type <a> generalList general
%type <a> stringList
%type <a> skillspecs skillspec
%type <a> triggerSkill triggerspecs triggerspec

%type <a> block statements statement retstat
%type <a> assign_stat if_stat loop_stat action_stat action args arglist arg
%type <a> drawCards loseHp causeDamage inflictDamage recoverHp
%type <a> acquireSkill detachSkill
%type <a> addMark loseMark getMark
%type <a> askForChoice askForChoosePlayer

%type <a> exp prefixexp opexp var
%type <a> explist array

%start extension
%define parse.error detailed
%define parse.trace

%%

extension : skillList packageList
              {
                $$ = newast(N_Extension, $1, $2);
                newExtension($$);
              }
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
            | BREAK { $$ = newast(N_Stat_Break, NULL, NULL); }
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

action_stat : action { $$ = newast(N_Stat_Action, $1, NULL); }
            | action args { $$ = newast(N_Stat_Action, $1, $2); }
            ;

action      : drawCards { $$ = newaction(ActionDrawcard, $1); }
            | loseHp { $$ = newaction(ActionLosehp, $1); }
            | causeDamage { $$ = newaction(ActionDamage, $1); }
            | inflictDamage { $$ = newaction(ActionDamage, $1); }
            | recoverHp { $$ = newaction(ActionRecover, $1); }
            | acquireSkill { $$ = newaction(ActionAcquireSkill, $1); }
            | detachSkill { $$ = newaction(ActionDetachSkill, $1); }
            | addMark { $$ = newaction(ActionMark, $1); }
            | loseMark  { $$ = newaction(ActionMark, $1); }
            | getMark { $$ = newaction(ActionMark, $1); }
            | askForChoice { $$ = newaction(ActionAskForChoice, $1); }
            | askForChoosePlayer { $$ = newaction(ActionAskForPlayerChosen, $1); }
            ;

args : '{' arglist '}' { $$ = $2; }
     | '{' '}' { $$ = NULL; }
     ;

arglist : arglist ',' arg { $$ = newast(N_Args, $1, $3); }
        | arg {$$ = newast(N_Args, NULL, $1); }
        ;

arg : IDENTIFIER ':' exp { $$ = newast(N_Arg, newstr($1), $3); };

drawCards : exp DRAW exp ZHANG CARD { $$ = newast(-1, $1, $3); }
          ;

loseHp  : exp LOSE exp DIAN HP { $$ = newast(-1, $1, $3); }
        ;

causeDamage : exp TO exp CAUSE exp DIAN DAMAGE
              { $$ = newdamage($1, $3, $5); }
            ;

inflictDamage : exp INFLICT exp DIAN DAMAGE
                { $$ = newdamage(NULL, $1, $3); }
              ;

recoverHp : exp RECOVER exp DIAN HP { $$ = newast(-1, $1, $3); }
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

exp : FALSE { $$ = newexp(ExpBool, 0, 0, NULL, NULL); }
    | TRUE { $$ = newexp(ExpBool, 1, 0, NULL, NULL); }
    | NUMBER { $$ = newexp(ExpNum, $1, 0, NULL, NULL); }
    | STRING { $$ = newexp(ExpStr, 0, 0, (struct astExp *)newstr($1), NULL); free($1); }
    | prefixexp { $$ = $1; }
    | opexp { $$ = $1; }
    | '(' action_stat ')' 
      {
        $$ = newexp(ExpAction, 0, 0, (struct astExp *)$2, NULL);
        ((struct astAction *)$2)->standalone = 0;
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

%%
