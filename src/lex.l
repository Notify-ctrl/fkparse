%option noyywrap nodefault

%{
#include "main.h"
%}

%%

\'[^\']*\' { yylval.s = yytext; return Identifier; }
\"[^\"]*\" { yylval.s = yytext; return StringLiteral; }
[0-9]+    { yylval.i = atoi(yytext); return Number; }

"拓展包" { return PackageStart; }

 /* magic char */

"$" |
"#" { return yytext[0]; }

 /* Below are ignored */
[ \t\r\n\,\.]
.

%%
