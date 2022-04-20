# fkparse

借助Flex和Bison，用C语言编写的Lua生成器。

项目仍处于十分早期的阶段，欢迎发起PR。

___

## 构建

首先克隆仓库：

```shell
$ git clone https://github.com/Notify-ctrl/fkparse
```

然后安装编译软件所必需的软件包：

```shell
$ sudo apt install cmake flex bison
```

然后编译即可得到可执行文件。

```shell
$ mkdir build && cd build
$ cmake .. && make
```

Windows用户需配置flex和bison的环境变量。

___

## 使用

使用命令行：

```shell
$ ./fkparse test.txt
```

其中test.txt是输入的文件。执行后fkparse所在的文件夹中会多出一个test.lua。然后将生成的lua文件放入游戏的extensions目录，将项目文件夹的lua/fkparser.lua放入游戏文件夹的lua/lib下，即可开始游玩。

___

## 语法规范

示例：[test/basic.txt](test/basic.txt)

{}表示里面的内容重复0次或者多次，[]表示里面的内容重复0次或者1次。

```BNF
extension ::= {skill} package {package}
skill ::= '$' IDENTIFIER STRING {skillspec}
skillspec ::= triggerskill
triggerskill ::= '触发技' triggerspec {triggerspec}
triggerspec ::= '时机:' EVENT ['条件:' block] '效果:' block
block ::= {statement} [retstat]
statement ::= ';'
            | assign_stat
            | if_stat
            | loop_stat
            | '中止此流程'
            | action_stat
assign_stat ::= '令' var '为' exp
if_stat ::= '若' exp '则' block ['否则' block] '以上'
loop_stat ::= '重复此流程:' block '直到' exp
action_stat ::= drawCards
            | loseHp
            | causeDamage
            | inflictDamage
            | recoverHp
            | acquireSkill
            | detachSkill
drawCards ::= exp '摸' exp '张' '牌'
loseHp ::= exp '失去' exp '点' '体力'
causeDamage ::= exp '对' exp '造成' exp '点' '伤害'
inflictDamage ::= exp '受到' exp '点' '伤害'
recoverHp ::= exp '回复' exp '点' '体力'
acquireSkill ::= exp '获得' '技能' exp
detachSkill ::= exp '失去' '技能' exp
exp ::= '假' | '真' | NUMBER | STRING | prefixexp | opexp
prefixexp ::= var | '(' action_stat ')' | '(' exp ')'
opexp ::= exp OP exp
var ::= IDENTIFIER | prefixexp '的' STRING
retstat ::= '返回' exp
package ::= '拓展包' IDENTIFIER {general}
general ::= '#' IDENTIFIER STRING NUMBER STRING '[' {STRING} ']'
IDENTIFIER ::= '\''(~('\''))+'\'' | '你' | 'X' | '其'
STRING ::= '"'(~('"')*)'"'
NUMBER ::= [0-9]+
OP ::= '大于' | '小于' | '是' | '不是' | '不大于' | '不小于' | '+' | '-' | '*' | '/'
EVENT ::= '造成伤害时' | '受到伤害时' | '造成伤害后' | '受到伤害后' | '造成伤害结算完成后' 
```

下列内容在语法分析时会被忽略掉：

```
'注：'直到这一行结束
'然后'
'立即'
'，'
'。'
空格，Tab，换行，半角的逗号和句号
```

___

## 一些说明

1. 关于时机中可以使用的变量以及哪些时机可以修改值，请参考[doc/event_data.md](doc/event_data.md)。
2. 关于各种类型可以获取的属性，请参考[doc/field.md](doc/field.md)。
3. 关于预先定义好的各大字符串（比如魏蜀吴群神），请查阅[symtab.c](src/analyze/symtab.c)。

___

## FAQ

还没有FAQ

