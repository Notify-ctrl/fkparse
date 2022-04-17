%{
#include "main.h"
int yylex();
void yyerror(char *msg) {
  fprintf(stderr, "error: %s\n", msg);
}
%}

%union {
  struct ast *a;
  int enum_v;
  long long i;
  char *s;
}

%token <i> Number
%token <s> Identifier
%token <s> StringLiteral
%token PackageStart

%type <a> extension
%type <a> skillList skill
%type <a> packageList package
%type <a> generalList general

%start extension

%%

extension : skillList packageList
              {
                $$ = newast(Extension, $1, $2);
                analyzeTree($$);
              }
          ;

skillList : %empty
              { $$ = newast(Skills, NULL, NULL); }
          | skillList skill
              { $$ = newast(Skills, $1, $2); }
          ;

skill     : '$' Identifier
              {
                $$ = newast(Skill, NULL, NULL);
                printf("read new skill %s\n", $2);
              }
          ;

packageList : package
                { $$ = newast(Packages, NULL, $1); }
            | packageList package
                { $$ = newast(Packages, $1, $2); }
            ;

package     : PackageStart Identifier generalList
                { $$ = newast(Package, NULL, $3); printf("read new pack %s\n", $2); } ;

generalList : %empty
                { $$ = newast(Generals, NULL, NULL); }
            | generalList general
                { $$ = newast(Generals, $1, $2); }
            ;

general     : '#' Identifier
                { $$ = newast(General, NULL, NULL); printf("read new general %s\n", $2);}
            ;

%%
