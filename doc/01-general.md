# 武将

> [fkparse](./index.md) > 武将

___

## 创建武将的格式

创建武将的格式如下：

```
# <势力> <字符串> <标识符> <数字> [<性别> [<内部标识符>] ] '[' {<字符串>} ']'
```

这一行可以创建一名武将。[势力](./lexical.md#势力)和[性别](./lexical.md#性别)只能从固定的几个词语中选。[字符串](./lexical.md#字符串)表示这个武将的称号，[标识符](./lexical.md#标识符)则是这个武将的名字。[数字]是武将的体力上限。[内部标识符](./lexical.md#内部标识符)是武将在Lua中的内部名称，填写此项可以让制图工作变得方便。

内部标识符和性别可以不填。不填性别的话默认为男性，不填内部标识符的话系统会进行自动分配。

内部标识符后是一对包含了许多字符串的花括号，这个表示这名武将拥有哪些[技能](./02-skill.md)。这些技能必须只能是同一个文件内定义过的。

___

## 系统自动分配的标识符

在上述格式中，若不填标识符则系统会分配如下格式的标识符：

```
<字符串>g<数字>
```

这其中字符串是你整个拓展文件的文件名，数字是这个武将由编译器分发的id，从0开始。例如test.txt的第一名武将被分配的名称为"testg0"。

___

## 太阳神三国杀武将的制图

TODO: 简要描述一下制图

___

上一篇：[基本格式](./00-basic.md)
下一篇：[技能](./02-skill.md)
