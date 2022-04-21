%{
#include "main.h"
#include "enums.h"
#include "ast.h"
#include "analyzer.h"
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
%token PKGSTART
%token TRIGGER EVENTI COND EFFECT
%token <enum_v> EVENT
%token LET EQ IF THEN ELSE END REPEAT UNTIL
%nonassoc <enum_v> CMP
%left '+' '-'
%left '*' '/'
%token FIELD RET
%token FALSE TRUE BREAK
%token DRAW ZHANG CARD LOSE DIAN HP 
%token TO CAUSE DAMAGE INFLICT RECOVER ACQUIRE SKILL

%type <a> extension
%type <a> skillList skill
%type <a> packageList package
%type <a> generalList general
%type <a> stringList
%type <a> skillspecs skillspec
%type <a> triggerSkill triggerspecs triggerspec

%type <a> block statements statement retstat
%type <a> assign_stat if_stat loop_stat action_stat
%type <a> drawCards loseHp causeDamage inflictDamage recoverHp acquireSkill detachSkill
%type <a> exp prefixexp opexp var

%start extension
%define parse.error detailed
%define parse.trace

%%

extension : skillList packageList
              {
                $$ = newast(N_Extension, $1, $2);
                analyzeExtension($$);
              }
          ;

skillList : %empty  { $$ = newast(N_Skills, NULL, NULL); }
          | skillList skill { $$ = newast(N_Skills, $1, $2); }
          ;

skill     : '$' IDENTIFIER STRING STRING skillspecs
              {
                $$ = newskill($2, $3, $4, $5);
                free($2); free($3); free($4);
              }
          | '$' IDENTIFIER STRING skillspecs
              {
                $$ = newskill($2, $3, NULL, $4);
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

action_stat : drawCards { $$ = newaction(ActionDrawcard, $1); }
            | loseHp { $$ = newaction(ActionLosehp, $1); }
            | causeDamage { $$ = newaction(ActionDamage, $1); }
            | inflictDamage { $$ = newaction(ActionDamage, $1); }
            | recoverHp { $$ = newaction(ActionRecover, $1); }
            | acquireSkill { $$ = newaction(ActionAcquireSkill, $1); }
            | detachSkill { $$ = newaction(ActionDetachSkill, $1); }
            ;

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

exp : FALSE { $$ = newexp(ExpBool, 0, 0, NULL, NULL); }
    | TRUE { $$ = newexp(ExpBool, 1, 0, NULL, NULL); }
    | NUMBER { $$ = newexp(ExpNum, $1, 0, NULL, NULL); }
    | STRING { $$ = newexp(ExpStr, 0, 0, (struct astExp *)newstr($1), NULL); free($1); }
    | prefixexp { $$ = $1; }
    | opexp { $$ = $1; }
    ;

opexp : exp CMP exp { $$ = newexp(ExpCmp, 0, $2, (struct astExp *)$1, (struct astExp *)$3); }
      | exp '+' exp { $$ = newexp(ExpCalc, 0, '+', (struct astExp *)$1, (struct astExp *)$3); }
      | exp '-' exp { $$ = newexp(ExpCalc, 0, '-', (struct astExp *)$1, (struct astExp *)$3); }
      | exp '*' exp { $$ = newexp(ExpCalc, 0, '*', (struct astExp *)$1, (struct astExp *)$3); }
      | exp '/' exp { $$ = newexp(ExpCalc, 0, '/', (struct astExp *)$1, (struct astExp *)$3); }
      ;

prefixexp : var { $$ = newexp(ExpVar, 0, 0, (struct astExp *)$1, NULL); }
          | '(' action_stat ')' { $$ = newexp(ExpAction, 0, 0, (struct astExp *)$2, NULL); }
          | '(' exp ')' { $$ = $2; }
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

general     : '#' IDENTIFIER STRING NUMBER STRING '[' stringList ']'
                {
                  $$ = newgeneral($2, $3, $4, $5, $7);
                  free($2); free($3); free($5);
                }
            ;

stringList  : %empty  { $$ = newast(N_Strs, NULL, NULL); }
            | stringList STRING { $$ = newast(N_Strs, $1, newstr($2)); free($2); }
            ;

%%
