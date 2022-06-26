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

\{[a-zA-Z][_a-zA-Z0-9]*\} {
    yylval.s = strdup(yytext + 1);
    yylval.s[strlen(yylval.s) - 1] = 0;
    return INTERID;
    }

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
"枚"    { return MEI; }
"标记"  { return MARK; }
"隐藏"  { return HIDDEN; }
"数量"  { return COUNT; }
"从"    { return FROM; }
"选择"  { return SELECT; }
"一项"  { return ANITEM; }
"一名角色" { return ANPLAYER; }

"游戏开始时"     { yylval.enum_v = GameStart; return EVENT; }
"回合即将开始时"   { yylval.enum_v = TurnStart; return EVENT; }
"阶段开始时"     { yylval.enum_v = EventPhaseStart; return EVENT; }
"阶段结束时"     { yylval.enum_v = EventPhaseEnd; return EVENT; }
"阶段被跳过时"    { yylval.enum_v = EventPhaseSkipping; return EVENT; }
"摸牌阶段摸牌时"   { yylval.enum_v = DrawNCards; return EVENT; }
"摸牌阶段摸牌后"   { yylval.enum_v = AfterDrawNCards; return EVENT; }
"分发起始手牌时"   { yylval.enum_v = DrawInitialCards; return EVENT; }
"分发起始手牌后"   { yylval.enum_v = AfterDrawInitialCards; return EVENT; }
"回复体力时"     { yylval.enum_v = PreHpRecover; return EVENT; }
"回复体力后"     { yylval.enum_v = HpRecover; return EVENT; }
"失去体力前"     { yylval.enum_v = PreHpLost; return EVENT; }
"失去体力后"     { yylval.enum_v = HpLost; return EVENT; }
"体力变化后"     { yylval.enum_v = HpChanged; return EVENT; }
"体力上限变化后"   { yylval.enum_v = MaxHpChanged; return EVENT; }
"失去技能后"     { yylval.enum_v = EventLoseSkill; return EVENT; }
"获得技能后"     { yylval.enum_v = EventAcquireSkill; return EVENT; }
"开始判定时"     { yylval.enum_v = StartJudge; return EVENT; }
"改判前"       { yylval.enum_v = AskForRetrial; return EVENT; }
"改判后"       { yylval.enum_v = FinishRetrial; return EVENT; }
"判定完成后"     { yylval.enum_v = FinishJudge; return EVENT; }
"拼点时"       { yylval.enum_v = PindianVerifying; return EVENT; }
"拼点后"       { yylval.enum_v = Pindian; return EVENT; }
"翻面后"       { yylval.enum_v = TurnedOver; return EVENT; }
"连环状态改变后"   { yylval.enum_v = ChainStateChanged; return EVENT; }
"造成伤害时"    { yylval.enum_v = DamageCaused; return EVENT; }
"受到伤害时"    { yylval.enum_v = DamageInflicted; return EVENT; }
"造成伤害后"    { yylval.enum_v = Damage; return EVENT; }
"受到伤害后"    { yylval.enum_v = Damaged; return EVENT; }
"造成伤害结算完成后" { yylval.enum_v = DamageComplete; return EVENT; }
"进入濒死时"     { yylval.enum_v = EnterDying; return EVENT; }
"进入濒死后"     { yylval.enum_v = Dying; return EVENT; }
"脱离濒死后"     { yylval.enum_v = QuitDying; return EVENT; }
"求桃开始时"     { yylval.enum_v = AskForPeaches; return EVENT; }
"求桃完成后"     { yylval.enum_v = AskForPeachesDone; return EVENT; }
"角色阵亡时"     { yylval.enum_v = Death; return EVENT; }
"阵亡后判定胜负前"  { yylval.enum_v = BeforeGameOverJudge; return EVENT; }
"阵亡后判定胜负时"  { yylval.enum_v = GameOverJudge; return EVENT; }
"打出卡牌时"     { yylval.enum_v = PreCardResponded; return EVENT; }
"打出卡牌后"     { yylval.enum_v = CardResponded; return EVENT; }
"卡牌移动前"     { yylval.enum_v = BeforeCardsMove; return EVENT; }
"卡牌移动后"     { yylval.enum_v = CardsMoveOneTime; return EVENT; }
"卡牌使用后"     { yylval.enum_v = CardUsed; return EVENT; }
"指定目标时"     { yylval.enum_v = TargetSpecifying; return EVENT; }
"确定目标时"     { yylval.enum_v = TargetConfirming; return EVENT; }
"指定目标后"     { yylval.enum_v = TargetSpecified; return EVENT; }
"确定目标后"     { yylval.enum_v = TargetConfirmed; return EVENT; }
"卡牌生效后"     { yylval.enum_v = CardEffected; return EVENT; }
"卡牌结算完成后"   { yylval.enum_v = CardFinished; return EVENT; }

"锁定技" |
"普通技" |
"默认技" |
"觉醒技" |
"限定技" {
    yylval.s = strdup(yytext);
    return FREQUENCY;
    }

"男性" |
"女性" |
"中性" {
    yylval.s = strdup(yytext);
    return GENDER;
    }

"魏" |
"蜀" |
"吴" |
"群" |
"神" {
    yylval.s = strdup(yytext);
    return KINGDOM;
    }

"$" |
"#" |
"[" |
"]" |
"(" |
")" |
"{" |
"}" |
";" |
"+" |
"-" |
"*" |
"/" |
"," |
"." |
":" { return yytext[0]; }

"大于"     { yylval.enum_v = 1; return CMP; }
"小于"     { yylval.enum_v = 2; return CMP; }
"不是"    { yylval.enum_v = 3; return CMP; }
"是"    { yylval.enum_v = 4; return CMP; }
"不小于"    { yylval.enum_v = 5; return CMP; }
"不大于"    { yylval.enum_v = 6; return CMP; }

"且"    { yylval.enum_v = 7; return LOGICOP; }
"或"    { yylval.enum_v = 8; return LOGICOP; }

 /* Below are ignored */
"注：".*
"然后"
"立即"
"，"
"。"
[ \t\r\n]
. { yyerror("unrecognized char"); }

%%
