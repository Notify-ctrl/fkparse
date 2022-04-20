%option noyywrap nodefault yylineno

%{
#include "grammar.h"
#include "main.h"
#include "enums.h"
%}

%%

\'[^\']+\' {
    yylval.s = strdup(yytext + 1);
    yylval.s[strlen(yylval.s) - 1] = 0;
    return IDENTIFIER;
    }

 /* Frequently used IDENTIFIER */
"你" |
"X" |
"其"    {
    yylval.s = strdup(yytext);
    return IDENTIFIER;
    }

\"[^\"]*\" {
    yylval.s = strdup(yytext + 1);
    yylval.s[strlen(yylval.s) - 1] = 0;
    return STRING;
    }
[0-9]+    { yylval.i = atoi(yytext); return NUMBER; }

"拓展包" { return PKGSTART; }
"触发技" { return TRIGGER; }
"时机:" { return EVENTI; }
"条件:" { return COND; }
"效果:" { return EFFECT; }
"令"    { return LET; }
"为"    { return EQ; }
"若"    { return IF; }
"则"    { return THEN; }
"否则"  { return ELSE; }
"以上"  { return END; }
"重复此流程:"   { return REPEAT; }
"直到"  { return UNTIL; }
"的"    { return FIELD; }
"返回"  { return RET; }
"假"  { return FALSE; }
"真"  { return TRUE; }
"中止此流程"    { return BREAK; }

"摸"    { return DRAW; }
"张"    { return ZHANG; }
"牌"    { return CARD; }
"失去"  { return LOSE; }
"点"    { return DIAN; }
"体力"  { return HP; }
"对"    { return TO; }
"造成"  { return CAUSE; }
"伤害"  { return DAMAGE; }
"受到"  { return INFLICT; }
"回复"  { return RECOVER; }
"获得"  { return ACQUIRE; }
"技能"  { return SKILL; }

"造成伤害时"    { yylval.enum_v = DamageCaused; return EVENT; }
"受到伤害时"    { yylval.enum_v = DamageInflicted; return EVENT; }
"造成伤害后"    { yylval.enum_v = Damage; return EVENT; }
"受到伤害后"    { yylval.enum_v = Damaged; return EVENT; }
"造成伤害结算完成后" { yylval.enum_v = DamageComplete; return EVENT; }

"$" |
"#" |
"[" |
"]" |
"(" |
")" |
";" |
"+" |
"-" |
"*" |
"/" { return yytext[0]; }

"大于"     { yylval.enum_v = 1; return CMP; }
"小于"     { yylval.enum_v = 2; return CMP; }
"不是"    { yylval.enum_v = 3; return CMP; }
"是"    { yylval.enum_v = 4; return CMP; }
"不小于"    { yylval.enum_v = 5; return CMP; }
"不大于"    { yylval.enum_v = 6; return CMP; }

 /* Below are ignored */
"注：".*
"然后"
"立即"
"，"
"。"
[ \t\r\n\,\.]
. { yyerror("unrecognized char"); }

%%
