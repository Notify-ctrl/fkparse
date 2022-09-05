vim9script

# type identifiers
syn keyword fkpType TNumber TBool TString TCard TNumberList TStringList TPlayerList TCardList TNone
syn keyword fkpType 数字类型 布尔类型 字符串类型 玩家类型 卡牌类型 数字数组 字符串数组 玩家数组 卡牌数组 无类型

# statements
syn keyword fkpStatement end
syn keyword fkpCondition if then elseif else
syn keyword fkpRepeat while repeat break
syn keyword fkpOperator "+" "-" "*" "/" and or "&&" "\|\|" ">" "<" "==" ">=" "<=" "~=" "!="
syn keyword fkpKeyword def return

syn keyword fkpStatement 以上
syn keyword fkpCondition 若 则 否则若 否则
syn keyword fkpRepeat 当 重复此流程： 重复此流程: 中止此流程
syn keyword fkpOperator 且 或 是 不是 大于 小于 不大于 不小于
syn keyword fkpKeyword 定义函数 返回

# comments
syn region fkpComment start="注：" skip="\\$" end="$"
syn region fkpComment start="注:" skip="\\$" end="$"
syn region fkpComment start="//" skip="\\$" end="$"
syn region fkpComment start="--" skip="\\$" end="$"

# string and id
syn region fkpString start=/"/ end=/"/
syn region fkpString start=/“/ end=/”/
syn region fkpIdentifier start=/'/ end=/'/
syn region fkpIdentifier start=/‘/ end=/’/

hi def link fkpType Type
hi def link fkpStatement Statement
hi def link fkpCondition Conditional
hi def link fkpRepeat Repeat
hi def link fkpOperator Operator
hi def link fkpKeyword Keyword
hi def link fkpComment Comment
hi def link fkpString String
hi def link fkpIdentifier Identifier

