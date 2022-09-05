%option noyywrap nodefault yylineno
%option bison-bridge bison-locations

%{
#include "object.h"
#include "grammar.h"
#include "main.h"
#include "enums.h"

static int yycolumn = 1;
#define YY_USER_ACTION do { \
    yylloc->first_line = yylloc->last_line = yylineno; \
    yylloc->first_column = yycolumn; yylloc->last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng; \
    } while(0);
%}

%%

\'[^\'\n]+\' {
    yylval->s = strdup(yytext + 1);
    yylval->s[strlen(yylval->s) - 1] = 0;
    return IDENTIFIER;
    }

 /* ‘[^’]+’ */
"‘"([^\xe2]|\xe2[^\x80]|\xe2\x80[^\x99])+"’" {
    yylval->s = strdup(yytext + 3);
    yylval->s[strlen(yylval->s) - 3] = 0;
    return IDENTIFIER;
    }

 /* Frequently used IDENTIFIER */
"你" |
"其" |
"不存在的" {
    yylval->s = strdup(yytext);
    return IDENTIFIER;
    }

\"[^\"\n]*\" {
    yylval->s = strdup(yytext + 1);
    yylval->s[strlen(yylval->s) - 1] = 0;
    return STRING;
    }

 /* “[^”]+” */
"“"([^\xe2]|\xe2[^\x80]|\xe2\x80[^\x9d])+"”" {
    yylval->s = strdup(yytext + 3);
    yylval->s[strlen(yylval->s) - 3] = 0;
    return STRING;
    }

-?[0-9]+    { yylval->i = atoi(yytext); return NUMBER; }

"拓展包" |
"Package"       { return PKGSTART; }
"触发技" |
"TriggerSkill"  { return TRIGGER; }
"时机:" |
"时机："   |
"Event:"        { return EVENTI; }
"条件:" |
"条件："   |
"Condition:"    { return COND; }
"消耗:" |
"消耗："  |
"Cost:"         { return COST; }
"效果:" |
"效果："   |
"Effect:"       { return EFFECT; }
"令" |
"let"           { return LET; }
"为" |
"=" |
"equal"         { return EQ; }
"若" |
"if"            { return IF; }
"则" |
"then"          { return THEN; }
"否则若" |
"elseif"        { return ELSEIF; }
"否则" |
"else"          { return ELSE; }
"以上" |
"end"           { return END; }
"重复此流程:" |
"重复此流程：" |
"repeat"        { return REPEAT; }
"直到" |
"until"         { return UNTIL; }
"当" |
"while"         { return WHILE; }
"的属性" |
"."             { return FIELD; }
"返回" |
"return"        { return RET; }
"->"            { return RETURN; }
"假" |
"false"         { return FALSE; }
"真" |
"true"          { return TRUE; }
"中止此流程" |
"break"         { return BREAK; }
"定义函数" |
"def"           { return FUNCDEF; }

"摸"    { return DRAW; }
"张"    { return ZHANG; }
"牌"    { return CARD; }
"失去"  { return LOSE; }
"点"    { return DIAN; }
"体力"  { return HP; }
"上限"    { return MAX; }
"对"    { return TO; }
"中"     { return IN; }
"每个"    { return EVERY; }
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
"发动"  { return INVOKE; }
"向" { return TOWARD; }
"插入" { return PREPEND; }
"追加" { return APPEND; }
"删除" { return DELETE; }
"第" { return DI; }
"个" { return GE; }
"的" { return DE; }
"元素" { return ELEMENT; }
"拥有"  { return HAVE; }
"洗牌" { return WASH; }
"变身为" { return CHANGEGENERAL; }
"交换座位" { return CHANGESEAT; }
"与" { return YU; }
"观星" { return GUANXING; }
"牌堆顶" { return PILETOP; }
"将" { return JIANG; }
"结果" { return RESULT; }
"修改" { return FIX; }
"自己" { return SELF; }
"一张" { return AZHANG; }
"使用" { return USE; }
"打出" { return RESPOND; }
"距离" { return DISTANCE; }
"范围" { return RANGE; }
"攻击" { return ATTACK; }
"内" { return INSIDE; }
"在" { return AT; }
"到" { return DAO; }
"相邻" { return ADJACENT; }
"之外的"  { return EXPECT; }
"其他角色"  { return OTHERPLAYER; }
"作为"  { return AS; }
"闪" { return SHAN; }
"杀" { return SHA; }
"需" { return NEED; }
"响应" { return RESPONSE; }

"主动技" |
"ActiveSkill"   { return ACTIVE; }
"选牌规则:" |
"选牌规则：" |
"CardFilter:"   { return CARD_FILTER; }
"选目标规则:" |
"选目标规则：" |
"TargetFilter:" { return TARGET_FILTER; }
"可以点确定:" |
"可以点确定：" |
"Feasible:"     { return FEASIBLE; }
"使用后:" |
"使用后：" |
"AfterUse:"     { return ON_USE; }
"视为技" |
"ViewAsSkill"   { return VIEWAS; }
"视为规则:" |
"视为规则：" |
"ViewAsRule:"   { return VSRULE; }
"响应条件:" |
"响应条件：" |
"Responsable:"  { return RESPONSECOND; }
"响应规则:" |
"响应规则：" |
"ResponseRule:" { return RESPONSABLE; }
"状态技" |
"StatusSkill"   { return STATUSSKILL; }
"禁止指定目标:" |
"禁止指定目标：" |
"Prohibited:"   { return IS_PROHIBITED; }
"距离修正:" |
"距离修正：" |
"DistanceCorrect:"  { return DISTANCE_CORRECT; }
"手牌上限修正:" |
"手牌上限修正：" |
"Maxcards Extra:"   { return MAX_EXTRA; }
"手牌上限固定:" |
"手牌上限固定：" |
"Maxcards Fixed:"   { return MAX_FIXED; }
"出牌次数修正:" |
"出牌次数修正：" |
"Residue:"      { return TMD_RESIDUE; }
"出牌距离修正:" |
"出牌距离修正：" |
"DistanceLimit:"    { return TMD_DISTANCE; }
"出牌目标数量修正:" |
"出牌目标数量修正：" |
"ExtraTarget:"      { return TMD_EXTARGET; }
"攻击范围修正:" |
"攻击范围修正：" |
"AtkRange Extra:"   { return ATKRANGE_EXTRA; }
"攻击范围固定:" |
"攻击范围固定：" |
"AtkRange Fixed:"   { return ATKRANGE_FIXED; }

"因" { return BECAUSE; }
"弃置"    { return THROW; }
"次数"    { return TIMES; }

"说出"    { return SPEAK; }
"台词"    { return ACT_LINE; }
"发送战报"  { return SENDLOG; }
"进行"    { return EXEC; }
"判定"    { return JUDGE; }
"交给"    { return GIVE; }
"拼点"    { return PINDIAN; }
"换牌"    { return SWAPCARD; }
"翻面"    { return TURNOVER; }
"额外回合"  { return EXTRATURN; }
"跳过"    { return SKIP; }
"私人牌堆"  { return PILE; }
"置于"    { return PUT; }
"本轮"    { return THISROUND; }
"本回合"  { return THISTURN;  }
"本阶段"  { return THISPHASE; }
"发动过"  { return INVOKED;   }

"游戏开始时" |
"GameStart"         { yylval->enum_v = GameStart; return EVENT; }
"回合即将开始时" |
"TurnStart"         { yylval->enum_v = TurnStart; return EVENT; }
"阶段开始时" |
"EventPhaseStart"   { yylval->enum_v = EventPhaseStart; return EVENT; }
"阶段结束时" |
"EventPhaseEnd"     { yylval->enum_v = EventPhaseEnd; return EVENT; }
"阶段被跳过时" |
"EventPhaseSkipping"    { yylval->enum_v = EventPhaseSkipping; return EVENT; }
"摸牌阶段摸牌时" |
"DrawNCards"        { yylval->enum_v = DrawNCards; return EVENT; }
"摸牌阶段摸牌后" |
"AfterDrawNCards"   { yylval->enum_v = AfterDrawNCards; return EVENT; }
"分发起始手牌时" |
"DrawInitialCards"  { yylval->enum_v = DrawInitialCards; return EVENT; }
"分发起始手牌后" |
"AfterDrawInitialCards" { yylval->enum_v = AfterDrawInitialCards; return EVENT; }
"回复体力时" |
"PreHpRecover"      { yylval->enum_v = PreHpRecover; return EVENT; }
"回复体力后" |
"HpRecover"         { yylval->enum_v = HpRecover; return EVENT; }
"失去体力前" |
"PreHpLost"         { yylval->enum_v = PreHpLost; return EVENT; }
"失去体力后" |
"HpLost"            { yylval->enum_v = HpLost; return EVENT; }
"体力变化后" |
"HpChanged"         { yylval->enum_v = HpChanged; return EVENT; }
"体力上限变化后" |
"MaxHpChanged"      { yylval->enum_v = MaxHpChanged; return EVENT; }
"失去技能后" |
"EventLoseSkill"    { yylval->enum_v = EventLoseSkill; return EVENT; }
"获得技能后" |
"EventAcquireSkill" { yylval->enum_v = EventAcquireSkill; return EVENT; }
"开始判定时" |
"StartJudge"        { yylval->enum_v = StartJudge; return EVENT; }
"改判前" |
"AskForRetrial"     { yylval->enum_v = AskForRetrial; return EVENT; }
"改判后" |
"FinishRetrial"     { yylval->enum_v = FinishRetrial; return EVENT; }
"判定完成后" |
"FinishJudge"       { yylval->enum_v = FinishJudge; return EVENT; }
"拼点时" |
"PindianVerifying"  { yylval->enum_v = PindianVerifying; return EVENT; }
"拼点后" |
"Pindian"           { yylval->enum_v = Pindian; return EVENT; }
"翻面后" |
"TurnedOver"        { yylval->enum_v = TurnedOver; return EVENT; }
"连环状态改变后" |
"ChainStateChanged" { yylval->enum_v = ChainStateChanged; return EVENT; }
"造成伤害时" |
"DamageCaused"      { yylval->enum_v = DamageCaused; return EVENT; }
"受到伤害时" |
"DamageInflicted"   { yylval->enum_v = DamageInflicted; return EVENT; }
"造成伤害后" |
"Damage"            { yylval->enum_v = Damage; return EVENT; }
"受到伤害后" |
"Damaged"           { yylval->enum_v = Damaged; return EVENT; }
"造成伤害结算完成后" |
"DamageComplete"    { yylval->enum_v = DamageComplete; return EVENT; }
"进入濒死时" |
"EnterDying"        { yylval->enum_v = EnterDying; return EVENT; }
"进入濒死后" |
"Dying"             { yylval->enum_v = Dying; return EVENT; }
"脱离濒死后" |
"QuitDying"         { yylval->enum_v = QuitDying; return EVENT; }
"求桃开始时" |
"AskForPeaches"     { yylval->enum_v = AskForPeaches; return EVENT; }
"求桃完成后" |
"AskForPeachesDone" { yylval->enum_v = AskForPeachesDone; return EVENT; }
"角色阵亡时" |
"Death"             { yylval->enum_v = Death; return EVENT; }
"阵亡后判定胜负前" |
"BeforeGameOverJudge"   { yylval->enum_v = BeforeGameOverJudge; return EVENT; }
"阵亡后判定胜负时" |
"GameOverJudge"     { yylval->enum_v = GameOverJudge; return EVENT; }
"打出卡牌时" |
"PreCardResponded"  { yylval->enum_v = PreCardResponded; return EVENT; }
"打出卡牌后" |
"CardResponded"     { yylval->enum_v = CardResponded; return EVENT; }
"卡牌移动前" |
"BeforeCardsMove"   { yylval->enum_v = BeforeCardsMove; return EVENT; }
"卡牌移动后" |
"CardsMoveOneTime"  { yylval->enum_v = CardsMoveOneTime; return EVENT; }
"卡牌使用后" |
"CardUsed"          { yylval->enum_v = CardUsed; return EVENT; }
"指定目标时" |
"TargetSpecifying"  { yylval->enum_v = TargetSpecifying; return EVENT; }
"确定目标时" |
"TargetConfirming"  { yylval->enum_v = TargetConfirming; return EVENT; }
"指定目标后" |
"TargetSpecified"   { yylval->enum_v = TargetSpecified; return EVENT; }
"确定目标后" |
"TargetConfirmed"   { yylval->enum_v = TargetConfirmed; return EVENT; }
"卡牌生效后" |
"CardEffected"      { yylval->enum_v = CardEffected; return EVENT; }
"卡牌结算完成后" |
"CardFinished"      { yylval->enum_v = CardFinished; return EVENT; }

"锁定技" |
"普通技" |
"默认技" |
"觉醒技" |
"限定技" |
"Compulsory" |
"NotFrequent" |
"Frequent" |
"Wake" |
"Limited" {
    yylval->s = strdup(yytext);
    return FREQUENCY;
    }

"男性" |
"女性" |
"中性" |
"Male" |
"Female" |
"Neuter" {
    yylval->s = strdup(yytext);
    return GENDER;
    }

"魏" |
"蜀" |
"吴" |
"群" |
"神" |
"Wei" |
"Shu" |
"Wu" |
"Qun" |
"God" {
    yylval->s = strdup(yytext);
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
"+" |
"-" |
"*" |
"/" |
"," |
":" { return yytext[0]; }

"（"   { return '('; }
"）"   { return ')'; }
"｛"   { return '{'; }
"｝"   { return '}'; }
"："   { return ':'; }

 /* Type identifiers */
"数字类型"  |
"TNumber"   { yylval->enum_v = TNumber; return TYPE; }

"布尔类型"  |
"TBool"     { yylval->enum_v = TBool; return TYPE; }

"字符串类型"  |
"TString"     { yylval->enum_v = TString; return TYPE; }

"玩家类型"  |
"TPlayer"   { yylval->enum_v = TPlayer; return TYPE; }

"卡牌类型"  |
"TCard"     { yylval->enum_v = TCard; return TYPE; }

"数字数组"    |
"TNumberList" { yylval->enum_v = TNumberList; return TYPE; }

"字符串数组"  |
"TStringList" { yylval->enum_v = TStringList; return TYPE; }

"玩家数组"    |
"TPlayerList" { yylval->enum_v = TPlayerList; return TYPE; }

"卡牌数组"  |
"TCardList" { yylval->enum_v = TCardList; return TYPE; }

"无类型"  |
"TNone" { yylval->enum_v = TNone; return TYPE; }

 /* Logic OP */
"大于"  |
">"     { yylval->enum_v = 1; return CMP; }

"小于"  |
"<"     { yylval->enum_v = 2; return CMP; }

"不是"  |
"~="    |
"!="    { yylval->enum_v = 3; return CMP; }

"是" |
"==" { yylval->enum_v = 4; return CMP; }

"不小于"  |
">="      { yylval->enum_v = 5; return CMP; }

"不大于"  |
"<="      { yylval->enum_v = 6; return CMP; }

"且"  |
"and" |
"&&" { yylval->enum_v = 7; return LOGICOP; }

"或"  |
"or" |
"||"  { yylval->enum_v = 8; return LOGICOP; }

 /* C-style identifier, with lowest priority */
[_a-zA-Z][_a-zA-Z0-9]* {
    yylval->s = strdup(yytext);
    return IDENTIFIER;
    }

 /* Below are ignored */
"注:".*
"注：".*
"//".*
"--".*
"然后"
"立即"
"，"
"。"
[ \t\r]
\n { yycolumn = 1; }
. //{ yyerror("unrecognized char"); }

%%
