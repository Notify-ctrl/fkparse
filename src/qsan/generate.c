#include "object.h"
#include "enums.h"
#include "main.h"
#include <stdarg.h>
#include "error.h"
#include "qsan.h"

static PackageObj *currentpack;
static int indent_level = 0;

static void print_indent() {
  for (int i = 0; i < indent_level; i++)
    fprintf(yyout, "  ");
}

static void writestr(const char *msg, ...) {
  va_list ap;
  va_start(ap, msg);

  vfprintf(yyout, msg, ap);
  va_end(ap);
}

static void writeline(const char *msg, ...) {
  print_indent();
  va_list ap;
  va_start(ap, msg);

  vfprintf(yyout, msg, ap);
  fprintf(yyout, "\n");
  va_end(ap);
}

static void analyzeVar(VarObj *v);
static void analyzeBlock(BlockObj *bl);
static void analyzeFunccall(FunccallObj *f);
static void analyzeFuncdef(FuncdefObj *f);

static void analyzeExp(ExpressionObj *e) {
  if (e->bracketed) writestr("(");
  ExpVType t = TNone;
  List *node;
  ExpressionObj *array_item;
  char buf[64];
  const char *origtext;

  if ((e->exptype == ExpCalc || e->exptype == ExpCmp) && e->optype != 0) {
    if (e->optype == 3 || e->optype == 4) {
      if (e->varValue && strstr(e->varValue->name, "移动")
      && strstr(e->varValue->name, "原因")) {
        writestr("bit32.band(");
        analyzeExp(e->oprand1);
        writestr(", ");
        analyzeExp(e->oprand2);
        writestr(")");
      } else {
        writestr("(");
        analyzeExp(e->oprand1);
        if (e->oprand1->valuetype == TPlayer) {
          writestr(" and ");
          analyzeExp(e->oprand1);
          writestr(":objectName()");
        } else if (e->oprand1->valuetype == TCard) {
          writestr(" and ");
          analyzeExp(e->oprand1);
          writestr(":getId()");
        }
        writestr(")");

        switch (e->optype) {
          case 3: writestr(" ~= "); break;
          case 4: writestr(" == "); break;
        }

        writestr("(");
        analyzeExp(e->oprand2);
        if (e->oprand2->valuetype == TPlayer) {
          writestr(" and ");
          analyzeExp(e->oprand2);
          writestr(":objectName()");
        } else if (e->oprand2->valuetype == TCard) {
          writestr(" and ");
          analyzeExp(e->oprand2);
          writestr(":getId()");
        }

        writestr(")");
      }
      t = TBool;
    } else {
      analyzeExp(e->oprand1);
      checktype(e->oprand1, e->oprand1->valuetype, TNumber);
      switch (e->optype) {
        case 1: writestr(" > "); t = TBool; break;
        case 2: writestr(" < "); t = TBool; break;
        case 5: writestr(" >= "); t = TBool; break;
        case 6: writestr(" <= "); t = TBool; break;
        default: writestr(" %c ", e->optype); t = TNumber; break;
      }
      analyzeExp(e->oprand2);
      checktype(e->oprand2, e->oprand2->valuetype, TNumber);
    }
  } else if (e->exptype == ExpLogic) {
    analyzeExp(e->oprand1);
    switch (e->optype) {
      case 7: writestr(" and "); break;
      case 8: writestr(" or "); break;
    }
    analyzeExp(e->oprand2);
    t = TBool;
  } else if (e->exptype == ExpArray) {
    writestr("fkp.newlist{");
    list_foreach(node, e->array) {
      array_item = cast(ExpressionObj *, node->data);
      analyzeExp(array_item);
      writestr(", ");
      if (t != TNone && t != array_item->valuetype) {
        yyerror(cast(YYLTYPE *, e), "数组中不能有类别不同的元素(预期类型%d，实际得到%d)", t, array_item->valuetype);
        t = TNone;
        break;
      }
      t = array_item->valuetype;
    }
    writestr("}");
    switch (t) {
      case TPlayer: t = TPlayerList; break;
      case TCard: t = TCardList; break;
      case TNumber: t = TNumberList; break;
      case TString: t = TStringList; break;
      case TNone: t = TEmptyList; break;
      default:
        yyerror(cast(YYLTYPE *, e), "未知的数组元素类型%d\n", t);
        break;
    }
  } else if (e->exptype == ExpFuncdef) {
    free((void *)(e->funcdef->funcname));
    e->funcdef->funcname = strdup("");
    analyzeFuncdef(e->funcdef);
    t = TFunc;
  } else if (e->exptype == ExpDict) {
    writestr("{\n");
    indent_level++;

    Hash *h = e->dict;
    for (size_t i = 0; i < h->capacity; i++) {
      const char *s = h->entries[i].key;
      if (s) {
        print_indent();
        writestr("['%s'] = ", s);
        analyzeExp(h->entries[i].value);
        writestr(",\n");
      }
    }

    indent_level--;
    print_indent();
    writestr("}");
    t = TDict;
  } else switch (e->exptype) {
    case ExpNum:
      writestr("%lld", e->value);
      t = TNumber;
      break;
    case ExpBool:
      writestr("%s" ,e->value == 0 ? "false" : "true");
      t = TBool;
      break;
    case ExpStr:
      if (e->param_name != NULL) {
        if (strstr(e->param_name, "原因") || strstr(e->param_name, "技能")) {
          if (strlen(e->strvalue) == 0 && !strcmp(e->param_name, "技能名"))
            writestr("self:objectName()");
          else {
            origtext = hash_get(skill_table, e->strvalue);
            if (origtext) {
              writestr("'%s'", origtext);
            } else {
              writestr("'%s'", e->strvalue);
            }
          }
        } else if (!strcmp(e->param_name, "标记")) {
          origtext = hash_get(mark_table, e->strvalue);
          if (!origtext) {
            sprintf(buf, "@%s_mark_%d", readfile_name, markId);
            markId++;
            hash_set(mark_table, e->strvalue, strdup(buf));
            addTranslation(buf, e->strvalue);
            origtext = hash_get(mark_table, e->strvalue);
          }
          writestr("'%s'", origtext);
        } else if (!strcmp(e->param_name, "文本") || !strcmp(e->param_name, "value")
          || !strcmp(e->param_name, "牌堆名")) {
          origtext = hash_get(other_string_table, e->strvalue);
          if (!origtext) {
            sprintf(buf, "%s_str_%d", readfile_name, stringId);
            stringId++;
            hash_set(other_string_table, e->strvalue, strdup(buf));
            addTranslation(buf, e->strvalue);
            origtext = hash_get(other_string_table, e->strvalue);
          }
          writestr("'%s'", origtext);
        } else if (!strcmp(e->param_name, "战报")) {
          origtext = hash_get(other_string_table, e->strvalue);
          if (!origtext) {
            sprintf(buf, "#%s_str_%d", readfile_name, stringId);
            stringId++;
            hash_set(other_string_table, e->strvalue, strdup(buf));
            addTranslation(buf, e->strvalue);
            origtext = hash_get(other_string_table, e->strvalue);
          }
          writestr("'%s'", origtext);
        } else {
          writestr("'%s'", e->strvalue);
        }
      } else {
        origtext = untranslate(e->strvalue);
        //origtext = hash_get(other_string_table, e->strvalue);
        if (!origtext) {
          sprintf(buf, "%s_str_%d", readfile_name, stringId);
          stringId++;
          //hash_set(other_string_table, e->strvalue, strdup(buf));
          addTranslation(buf, e->strvalue);
          origtext = untranslate(e->strvalue);
        }
        writestr("'%s'", origtext);
      }
      t = TString;
      break;
    case ExpVar:
      analyzeVar(e->varValue);
      t = e->varValue->type;
      break;
    case ExpFunc:
      analyzeFunccall(e->func);
      t = e->func->rettype;
      break;
    default:
      yyerror(cast(YYLTYPE *, e), "unknown exptype %d\n", e->exptype);
      break;
  }

  if (e->bracketed) writestr(")");
  e->valuetype = t;
}

static void analyzeVar(VarObj *v) {
  ExpVType t = TNone;

  /* Handle index of an array first */
  if (!v->name) {
    analyzeExp(v->obj);
    ExpressionObj *array = v->obj;
    switch (array->valuetype) {
    case TNumberList:
      t = TNumber;
      break;
    case TStringList:
      t = TString;
      break;
    case TPlayerList:
      t = TPlayer;
      break;
    case TCardList:
      t = TCard;
      break;
    default:
      yyerror(cast(YYLTYPE *, v->obj), "试图对不是数组或者空数组根据下标取值");
      break;
    }
    v->type = t;
    writestr(":at(");
    analyzeExp(v->index);
    checktype(v->index, v->index->valuetype, TNumber);
    writestr(" - 1)");
    return;
  }

  const char *name = v->name;
  if (v->obj) {
    if (!strcmp(name, "所在位置")) {
      /* 对于不是直接调成员函数的，得区别对待 */
      /* 在客户端用这个属性的人还是后果自负罢 */
      writestr("room:getCardPlace(");
      analyzeExp(v->obj);
      checktype(v->obj, v->obj->valuetype, TCard);
      writestr(":getId())");
      t = TNumber;
    } else if (!strcmp(name, "持有者")) {
      writestr("room:getCardOwner(");
      analyzeExp(v->obj);
      checktype(v->obj, v->obj->valuetype, TCard);
      writestr(":getId())");
      t = TPlayer;
    } else {
      analyzeExp(v->obj);
      switch (v->obj->valuetype) {
        case TPlayer:
          if (!strcmp(name, "体力值")) {
            writestr(":getHp()");
            t = TNumber;
          } else if (!strcmp(name, "手牌数")) {
            writestr(":getHandcardNum()");
            t = TNumber;
          } else if (!strcmp(name, "体力上限")) {
            writestr(":getMaxHp()");
            t = TNumber;
          } else if (!strcmp(name, "当前阶段")) {
            writestr(":getPhase()");
            t = TNumber;
          } else if (!strcmp(name, "身份")) {
            writestr(":getRoleEnum()");
            t = TNumber;
          } else if (!strcmp(name, "性别")) {
            writestr(":getGender()");
            t = TNumber;
          } else if (!strcmp(name, "手牌上限")) {
            writestr(":getMaxCards()");
            t = TNumber;
          } else if (!strcmp(name, "势力")) {
            writestr(":getKingdom()");
            t = TString;
          } else if (!strcmp(name, "座位号")) {
            writestr(":getSeat()");
            t = TNumber;
          } else if (!strcmp(name, "攻击距离")) {
            writestr(":getAttackRange()");
            t = TNumber;
          } else if (!strcmp(name, "存活状态")) {
            writestr(":isAlive()");
            t = TBool;
          } else if (!strcmp(name, "死亡状态")) {
            writestr(":isDead()");
            t = TBool;
          } else {
            yyerror(cast(YYLTYPE *, v), "无法获取 玩家 的属性 '%s'\n", name);
            t = TNone;
          }
          break;
        case TCard:
          if (!strcmp(name, "点数")) {
            writestr(":getNumber()");
            t = TNumber;
          } else if (!strcmp(name, "花色")) {
            writestr(":getSuitString()");
            t = TString;
          } else if (!strcmp(name, "类别")) {
            writestr(":getType()");
            t = TString;
          } else if (!strcmp(name, "牌名")) {
            writestr(":objectName()");
            t = TString;
          } else if (!strcmp(name, "编号")) {
            writestr(":getId()");
            t = TNumber;
          } else if (!strcmp(name, "是否被装备")) {
            writestr(":isEquipped()");
            t = TBool;
          } else {
            yyerror(cast(YYLTYPE *, v), "无法获取 卡牌 的属性 '%s'\n", name);
            t = TNone;
          }
          break;
        case TCardList:
        case TNumberList:
        case TStringList:
        case TPlayerList:
          if (!strcmp(name, "长度")) {
            writestr(":length()");
            t = TNumber;
          } else {
            yyerror(cast(YYLTYPE *, v), "无法获取 数组 的属性 '%s'\n", name);
            t = TNone;
          }
          break;
        case TPindian:
          if (!strcmp(name, "来源")) {
            writestr(".from");
            t = TPlayer;
          } else if (!strcmp(name, "目标")) {
            writestr(".to");
            t = TPlayer;
          } else if (!strcmp(name, "获胜者")) {
            writestr(".winner");
            t = TPlayer;
          } else if (!strcmp(name, "来源卡牌")) {
            writestr(".from_card");
            t = TCard;
          } else if (!strcmp(name, "目标卡牌")) {
            writestr(".to_card");
            t = TCard;
          } else if (!strcmp(name, "来源点数")) {
            writestr(".from_number");
            t = TCard;
          } else if (!strcmp(name, "目标点数")) {
            writestr(".to_number");
            t = TCard;
          } else if (!strcmp(name, "原因")) {
            writestr(".reason");
            t = TString;
          } else {
            yyerror(cast(YYLTYPE *, v), "无法获取 拼点结果 的属性 '%s'\n", name);
            t = TNone;
          }
          break;
        case TDict:
        case TAny:
          writestr("[\"%s\"]", name);
          t = TAny;
          break;
        default:
          yyerror(cast(YYLTYPE *, v), "不能获取类型为%d的对象的属性\n", v->obj->valuetype);
          t = TNone;
      }
    }
    v->type = t;
  } else {
    symtab_item *i = sym_lookup(name);
    if (!i || i->type == TNone) {
      yyerror(cast(YYLTYPE *, v), "标识符'%s'尚未定义", name);
    } else {
      v->type = i->type;
      if (i->origtext)
        writestr("%s", i->origtext);
      else
        writestr("locals['%s']", name);
    }
  }
}

static void analyzeAssign(AssignObj *a) {
  print_indent();

  if (a->var->obj && a->var->index) {
    ExpressionObj *arr = a->var->obj;
    ExpVType t = TNone;
    analyzeExp(arr);
    switch (arr->valuetype) {
    case TNumberList:
      t = TNumber;
      break;
    case TStringList:
      t = TString;
      break;
    case TPlayerList:
      t = TPlayer;
      break;
    case TCardList:
      t = TCard;
      break;
    default:
      yyerror(cast(YYLTYPE *, arr), "试图对不是数组或者空数组根据下标取左值");
      break;
    }
    writestr(":replace(");
    analyzeExp(a->var->index);
    checktype(a->var->index, a->var->index->valuetype, TNumber);
    writestr(" - 1, ");
    analyzeExp(a->value);
    checktype(a->value, a->value->valuetype, t);
    writestr(")\n");;
    return;
  }

  /* pre-analyze var for symtab */
  if (!a->var->obj && !a->var->index && !sym_lookup(a->var->name)) {
    sym_new_entry(a->var->name, TNone, NULL, false);
    sym_lookup(a->var->name)->type = TAny;
  }

  analyzeVar(a->var);
  writestr(" = ");
  analyzeExp(a->value);
  writestr("\n");

  if (!a->var->obj && !a->var->index){
    symtab_item *i = sym_lookup(a->var->name);
    if (i) {
      if (i->reserved) {
        yyerror(cast(YYLTYPE *, a->var), "不允许重定义标识符 '%s'", a->var->name);
      } else {
        if (a->value->exptype == ExpFuncdef) {
          if (i->type != TFunc) free((void *)i->origtext);
          FuncdefObj *f = a->value->funcdef;
          free((void *)f->funcname); /* should be "" */
          f->funcname = strdup(a->var->name);
          i->origtext = cast(const char *, f);
        }
        if (a->custom_type == TNone)
          i->type = a->value->valuetype;
        else
          i->type = a->custom_type;
      }
    }
  }
}

static void analyzeIf(IfObj *i) {
  print_indent();
  writestr("if ");
  analyzeExp(i->cond);
  writestr(" then\n");
  indent_level++;
  analyzeBlock(i->then);
  
  List *node;
  list_foreach(node, i->elif) {
    IfObj *i2 = cast(IfObj *, node->data);
    indent_level--;
    print_indent();
    writestr("elseif ");
    analyzeExp(i2->cond);
    writestr(" then\n");
    indent_level++;
    analyzeBlock(i2->then);
  }
  if (i->el) {
    indent_level--;
    writeline("else");
    indent_level++;
    analyzeBlock(i->el);
  }
  indent_level--;
  writeline("end");
}

static void analyzeLoop(LoopObj *l) {
  if (l->type == 0) {
    writeline("repeat");
    indent_level++;
    analyzeBlock(l->body);
    indent_level--;
    print_indent();
    writestr("until ");
    analyzeExp(l->cond);
    writestr("\n");
  } else {
    print_indent();
    writestr("while ");
    analyzeExp(l->cond);
    writestr(" do\n");
    indent_level++;
    analyzeBlock(l->body);
    indent_level--;
    writeline("end");
  }
}

static void analyzeTraverse(TraverseObj *t) {
  static int iter_index = 0;
  print_indent();
  writestr("for _, iter%d in sgs.list(", iter_index);
  analyzeExp(t->array);
  ExpVType type = t->array->valuetype;
  if (type != TCardList && type != TNumberList
    && type != TPlayerList && type != TStringList && type != TEmptyList
  ) {
    yyerror(cast(YYLTYPE *, t->array), "只能对数组进行遍历操作");
    return;
  }

  writestr(") do\n");

  char buf[16];
  sprintf(buf, "iter%d", iter_index);
  char *s = strdup(buf);
  ExpVType vtype = TNone;
  switch (type) {
    case TCardList: vtype = TCard; break;
    case TNumberList: vtype = TNumber; break;
    case TPlayerList: vtype = TPlayer; break;
    case TStringList: vtype = TString; break;
    case TEmptyList: yyerror(cast(YYLTYPE *, t->array), "不允许遍历空数组"); break;
    default: break;
  }
  sym_new_entry(t->expname, vtype, s, false);

  Hash *symtab = hash_new();
  current_tab = symtab;
  stack_push(symtab_stack, cast(Object *, current_tab));

  iter_index++;
  indent_level++;
  analyzeBlock(t->body);
  indent_level--;
  iter_index--;

  stack_pop(symtab_stack);
  sym_free(symtab);
  current_tab = cast(Hash *, stack_gettop(symtab_stack));

  free(s);

  writeline("end");
}

static void analyzeFunccall(FunccallObj *f) {
  symtab_item *i = sym_lookup(f->name);
  if (!i) {
    yyerror(cast(YYLTYPE *, f), "调用了未定义的函数“%s”", f->name);
    return;
  }
  FuncdefObj *d = cast(FuncdefObj *, i->origtext);
  if (!strcmp(f->name, "__prepend") || !strcmp(f->name, "__append")) {
    /* if is empty list, create new list here */
    /* fkp.functions.append wont affect empty list */
    analyzeExp(hash_get(f->params, "array"));
    writestr(" = ");
  }

  char buf[64];
  sprintf(buf, "%s_func_", readfile_name);
  if (strstr(d->funcname, buf) || i->reserved)
    writestr("%s(", d->funcname);
  else
    writestr("locals['%s'](", d->funcname);

  List *node;
  bool start = true;
  bool errored = false;
  list_foreach(node, d->params) {
    if (!start) {
      writestr(", ");
    } else {
      start = false;
    }

    DefargObj *a = cast(DefargObj *, node->data);
    const char *name = a->name;
    ExpressionObj *e = hash_get(f->params, a->name);
    if (!e) {
      if (!a->d) {
        yyerror(cast(YYLTYPE *, f), "调用函数“%s”时：", f->name);
        yyerror(cast(YYLTYPE *, a), "函数“%s”的参数“%s”没有默认值，调用时必须传入值", f->name, name);
        errored = true;
      } else {
        analyzeExp(a->d);
      }
    } else {
      analyzeExp(e);
      checktype(e, e->valuetype, a->type);
    }
  }

  writestr(")");
  f->rettype = d->rettype;

  /* 因为没有函数重载，所以要对数组操作耦合一下
     TAny用多了容易翻车 */
  if (errored) return;
  if (!strcmp(f->name, "__prepend") || !strcmp(f->name, "__append")) {
    ExpressionObj *array = hash_get(f->params, "array");
    ExpressionObj *value = hash_get(f->params, "value");

    if (array->valuetype == TEmptyList) {
      switch (value->valuetype) {
      case TNumber:
        array->valuetype = TNumberList;
        break;
      case TString:
        array->valuetype = TStringList;
        break;
      case TCard:
        array->valuetype = TCardList;
        break;
      case TPlayer:
        array->valuetype = TPlayerList;
        break;
      default:
        yyerror(cast(YYLTYPE *, f), "不能把类型 %d 添加到数组中");
        break;
      }
      if (array->exptype == ExpVar && array->varValue) {
        symtab_item *i = sym_lookup(array->varValue->name);
        if (i) i->type = array->valuetype;
      }
    } else {
      ExpVType t = TNone;
      switch (array->valuetype) {
      case TNumberList:
        t = TNumber;
        break;
      case TStringList:
        t = TString;
        break;
      case TPlayerList:
        t = TPlayer;
        break;
      case TCardList:
        t = TCard;
        break;
      case TEmptyList:
        t = TAny;
        break;
      default:
        yyerror(cast(YYLTYPE *, f), "未知的数组类型");
        break;
      }

      checktype(value, value->valuetype, t);
    }

  }
}

static void defineLocal(char *k, char *v, int type) {
  writeline("locals['%s'] = %s", k, v);
  if (!sym_lookup(k)) {
    sym_new_entry(k, TNone, NULL, false);
  }
  sym_lookup(k)->type = type;
}

static void initData(int event) {
  //writeline("-- get datas for this trigger event");
  switch (event) {
    case DrawNCards:
    case AfterDrawNCards:
    case DrawInitialCards:
    case AfterDrawInitialCards:
      writeline("local draw = data:toInt()");
      defineLocal("摸牌数量", "draw", TNumber);
      break;

    case PreHpRecover:
    case HpRecover:
      writeline("local recover = data:toRecover()");
      defineLocal("回复来源", "recover.who", TPlayer);
      defineLocal("回复的牌", "recover.card", TCard);
      defineLocal("回复值", "recover.recover", TNumber);
      break;

    case PreHpLost:
    case HpLost:
      writeline("local lost = data:toInt()");
      defineLocal("失去值", "lost", TNumber);
      break;

    case EventLoseSkill:
    case EventAcquireSkill:
      writeline("local skill = data:toString()");
      defineLocal("技能", "skill", TString);
      break;

    case StartJudge:
    case AskForRetrial:
    case FinishRetrial:
    case FinishJudge:
      writeline("local judge = data:toJudge()");
      defineLocal("判定效果是负收益", "judge.negative", TBool);
      defineLocal("播放判定动画", "judge.play_animation", TBool);
      defineLocal("判定角色", "judge.who", TPlayer);
      defineLocal("判定牌", "judge.card", TCard);
      defineLocal("判定类型", "judge.pattern", TNumber);
      defineLocal("判定结果", "judge.good", TBool);
      defineLocal("判定原因", "judge.reason", TString);
      defineLocal("判定时间延迟", "judge.time_consuming", TBool);
      defineLocal("可改判角色", "judge.retrial_by_response", TPlayer);
      break;

    case PindianVerifying:
    case Pindian:
      writeline("local pindian = data:toPindian()");
      defineLocal("拼点来源", "pindian.from", TPlayer);
      defineLocal("拼点目标", "pindian.to", TPlayer);
      defineLocal("拼点来源的牌", "pindian.from_card", TCard);
      defineLocal("拼点目标的牌", "pindian.to_card", TCard);
      defineLocal("拼点来源的点数", "pindian.from_number", TNumber);
      defineLocal("拼点目标的点数", "pindian.to_number", TNumber);
      defineLocal("拼点原因", "pindian.reason", TString);
      defineLocal("拼点成功", "pindian.success", TBool);
      break;

    case ConfirmDamage:
    case Predamage:
    case DamageForseen:
    case DamageCaused:
    case DamageInflicted:
    case PreDamageDone:
    case DamageDone:
    case Damage:
    case Damaged:
    case DamageComplete:
      writeline("local damage = data:toDamage()");
      defineLocal("伤害来源", "damage.from", TPlayer);
      defineLocal("伤害目标", "damage.to", TPlayer);
      defineLocal("造成伤害的牌", "damage.card", TCard);
      defineLocal("伤害值", "damage.damage", TNumber);
      defineLocal("伤害属性", "damage.nature", TNumber);
      defineLocal("伤害是传导伤害", "damage.chain", TBool);
      defineLocal("伤害是转移伤害", "damage.transfer", TBool);
      defineLocal("是目标角色", "damage.by_user", TBool);
      defineLocal("造成伤害的原因", "damage.reason", TString);
      defineLocal("伤害传导的原因", "damage.transfer_reason", TString);
      defineLocal("伤害被防止", "damage.prevented", TBool);
      break;

    case EventPhaseChanging:
      writeline("local change = data:toPhaseChange()");
      defineLocal("上个阶段", "change.from", TNumber);
      defineLocal("下个阶段", "change.to", TNumber);
      break;

    case EnterDying:
    case Dying:
    case QuitDying:
    case AskForPeaches:
    case AskForPeachesDone:
      writeline("local dying = data:toDying()");
      defineLocal("濒死的角色", "dying.who", TPlayer);
      defineLocal("濒死的伤害来源", "dying.damage.from", TPlayer);
      defineLocal("濒死的伤害目标", "dying.damage.to", TPlayer);
      defineLocal("造成濒死的伤害的牌", "dying.damage.card", TCard);
      defineLocal("濒死的伤害值", "dying.damage.damage", TNumber);
      defineLocal("濒死的伤害属性", "dying.damage.nature", TNumber);
      defineLocal("濒死的伤害是传导伤害", "dying.damage.chain", TBool);
      defineLocal("濒死的伤害是转移伤害", "dying.damage.transfer", TBool);
      defineLocal("濒死的角色是目标角色", "dying.damage.by_user", TBool);
      defineLocal("造成濒死的伤害的原因", "dying.damage.reason", TString);
      defineLocal("濒死的伤害传导的原因", "dying.damage.transfer_reason", TString);
      defineLocal("濒死的伤害被防止", "dying.damage.prevented", TBool);
      break;

    case Death:
    case BuryVictim:
    case BeforeGameOverJudge:
    case GameOverJudge:
      writeline("local death = data:toDeath()");
      defineLocal("死亡的角色", "death.who", TPlayer);
      defineLocal("死亡的伤害来源", "death.damage.from", TPlayer);
      defineLocal("死亡的伤害目标", "death.damage.to", TPlayer);
      defineLocal("造成死亡的伤害的牌", "death.damage.card", TCard);
      defineLocal("死亡的伤害值", "death.damage.damage", TNumber);
      defineLocal("死亡的伤害属性", "death.damage.nature", TNumber);
      defineLocal("死亡的伤害是传导伤害", "death.damage.chain", TBool);
      defineLocal("死亡的伤害是转移伤害", "death.damage.transfer", TBool);
      defineLocal("死亡的角色是目标角色", "death.damage.by_user", TBool);
      defineLocal("造成死亡的伤害的原因", "death.damage.reason", TString);
      defineLocal("死亡的伤害传导的原因", "death.damage.transfer_reason", TString);
      defineLocal("死亡的伤害被防止", "death.damage.prevented", TBool);
      break;

    case PreCardResponded:
    case CardResponded:
      writeline("local resp = data:toCardResponse()");
      defineLocal("响应的牌", "resp.m_card", TCard);
      defineLocal("响应的目标", "resp.m_who", TPlayer);
      defineLocal("响应的牌被使用", "resp.m_isUse", TBool);
      defineLocal("响应的牌被改判", "resp.m_isRetrial", TBool);
      defineLocal("响应的牌是手牌", "resp.m_isHandcard", TBool);
      break;

    case BeforeCardsMove:
    case CardsMoveOneTime:
      writeline("local move = data:toMoveOneTime()");
      defineLocal("移动的牌", "move.card_ids", TNumber);
      defineLocal("移动来源地", "move.from_place", TNumber);
      defineLocal("移动目的地", "move.to_place", TNumber);
      defineLocal("移动的来源的名字", "move.from_player_name", TString);
      defineLocal("移动的目标的名字", "move.to_player_name", TString);
      defineLocal("移动的牌堆", "move.from_pile_name", TString);
      defineLocal("目的地牌堆", "move.to_pile_name", TString);
      defineLocal("移动的来源", "move.from", TPlayer);
      defineLocal("移动的目标", "move.to", TPlayer);
      defineLocal("移动的原因", "move.reason.m_reason", TString);
      defineLocal("是打开的", "move.open", TBool);
      defineLocal("是最后的手牌", "move.is_last_handcard", TBool);
      break;

    case CardUsed:
    case TargetSpecifying:
    case TargetConfirming:
    case TargetSpecified:
    case TargetConfirmed:
    case CardFinished:
      writeline("local use = data:toCardUse()");
      defineLocal("使用的牌", "use.card", TCard);
      defineLocal("使用者", "use.from", TPlayer);
      defineLocal("目标", "use.to", TPlayerList);
      defineLocal("持有者是使用者", "use.m_isOwnerUse", TBool);
      defineLocal("计入使用次数", "use.m_addHistory", TBool);
      defineLocal("是手牌", "use.m_isHandcard", TBool);
      defineLocal("无效的目标", "use.nullified_list", TNumber);
      break;

    case CardEffected:
      writeline("local effect = data:toCardEffect()");
      defineLocal("生效的牌", "effect.card", TCard);
      defineLocal("使用者", "effect.from", TPlayer);
      defineLocal("目标", "effect.to", TPlayer);
      defineLocal("是否生效", "effect.nullified", TBool);
      break;

    default:
      break;
  }
  writestr("\n");
}

static void clearLocal(char *k, char *v, int rewrite) {
  sym_lookup(k)->type = TNone;
  if (rewrite) {
    writeline("%s = locals['%s']", v, k);
  }
}

static void clearData(int event) {
  int rewrite = 0;
  switch (event) {
    case DrawNCards:
    case DrawInitialCards:
    case PreHpRecover:
    case PreHpLost:
    case PindianVerifying:
    case PreCardResponded:
    case BeforeCardsMove:
    case TargetSpecifying:
    case TargetConfirming:
    case ConfirmDamage:
    case Predamage:
    case DamageForseen:
    case DamageCaused:
    case DamageInflicted:
      rewrite = 1;

    default:
      break;
  }

  switch (event) {
    case DrawNCards:
    case AfterDrawNCards:
    case DrawInitialCards:
    case AfterDrawInitialCards:
      clearLocal("摸牌数量", "draw", rewrite);
      if (rewrite) writeline("data:setValue(draw)");
      break;

    case PreHpRecover:
    case HpRecover:
      if (rewrite) writestr("\n");
      clearLocal("回复来源", "recover.who", rewrite);
      clearLocal("回复的牌", "recover.card", rewrite);
      clearLocal("回复值", "recover.recover", rewrite);
      if (rewrite) writeline("data:setValue(recover)");
      break;

    case PreHpLost:
    case HpLost:
      clearLocal("失去值", "lost", rewrite);
      if (rewrite) writeline("data:setValue(lost)");
      break;

    case EventLoseSkill:
    case EventAcquireSkill:
      clearLocal("技能", "skill", rewrite);
      if (rewrite) writeline("data:setValue(skill)");
      break;

    case StartJudge:
    case AskForRetrial:
    case FinishRetrial:
    case FinishJudge:
      if (rewrite) writestr("\n");
      clearLocal("判定效果是负收益", "judge.negative", rewrite);
      clearLocal("播放判定动画", "judge.play_animation", rewrite);
      clearLocal("判定角色", "judge.who", rewrite);
      clearLocal("判定牌", "judge.card", rewrite);
      clearLocal("判定类型", "judge.pattern", rewrite);
      clearLocal("判定结果", "judge.good", rewrite);
      clearLocal("判定原因", "judge.reason", rewrite);
      clearLocal("判定时间延迟", "judge.time_consuming", rewrite);
      clearLocal("可改判角色", "judge.retrial_by_response", rewrite);
      if (rewrite) writeline("data:setValue(judge)");
      break;

    case PindianVerifying:
    case Pindian:
      if (rewrite) writestr("\n");
      clearLocal("拼点来源", "pindian.from", rewrite);
      clearLocal("拼点目标", "pindian.to", rewrite);
      clearLocal("拼点来源的牌", "pindian.from_card", rewrite);
      clearLocal("拼点目标的牌", "pindian.to_card", rewrite);
      clearLocal("拼点来源的点数", "pindian.from_number", rewrite);
      clearLocal("拼点目标的点数", "pindian.to_number", rewrite);
      clearLocal("拼点原因", "pindian.reason", rewrite);
      clearLocal("拼点成功", "pindian.success", rewrite);
      if (rewrite) writeline("data:setValue(pindian)");
      break;

    case ConfirmDamage:
    case Predamage:
    case DamageForseen:
    case DamageCaused:
    case DamageInflicted:
    case PreDamageDone:
    case DamageDone:
    case Damage:
    case Damaged:
    case DamageComplete:
      if (rewrite) writestr("\n");
      clearLocal("伤害来源", "damage.from", rewrite);
      clearLocal("伤害目标", "damage.to", rewrite);
      clearLocal("造成伤害的牌", "damage.card", rewrite);
      clearLocal("伤害值", "damage.damage", rewrite);
      clearLocal("伤害属性", "damage.nature", rewrite);
      clearLocal("伤害是传导伤害", "damage.chain", rewrite);
      clearLocal("伤害是转移伤害", "damage.transfer", rewrite);
      clearLocal("是目标角色", "damage.by_user", rewrite);
      clearLocal("造成伤害的原因", "damage.reason", rewrite);
      clearLocal("伤害传导的原因", "damage.transfer_reason", rewrite);
      clearLocal("伤害被防止", "damage.prevented", rewrite);
      if (rewrite) writeline("data:setValue(damage)");
      break;

    case EventPhaseChanging:
      if (rewrite) writestr("\n");
      clearLocal("上个阶段", "change.from", rewrite);
      clearLocal("下个阶段", "change.to", rewrite);
      if (rewrite) writeline("data:setValue(change)");
      break;

    case EnterDying:
    case Dying:
    case QuitDying:
    case AskForPeaches:
    case AskForPeachesDone:
      if (rewrite) writestr("\n");
      clearLocal("濒死的角色", "dying.who", rewrite);
      clearLocal("濒死的伤害来源", "dying.damage.from", rewrite);
      clearLocal("濒死的伤害目标", "dying.damage.to", rewrite);
      clearLocal("造成濒死的伤害的牌", "dying.damage.card", rewrite);
      clearLocal("濒死的伤害值", "dying.damage.damage", rewrite);
      clearLocal("濒死的伤害属性", "dying.damage.nature", rewrite);
      clearLocal("濒死的伤害是传导伤害", "dying.damage.chain", rewrite);
      clearLocal("濒死的伤害是转移伤害", "dying.damage.transfer", rewrite);
      clearLocal("濒死的角色是目标角色", "dying.damage.by_user", rewrite);
      clearLocal("造成濒死的伤害的原因", "dying.damage.reason", rewrite);
      clearLocal("濒死的伤害传导的原因", "dying.damage.transfer_reason", rewrite);
      clearLocal("濒死的伤害被防止", "dying.damage.prevented", rewrite);
      if (rewrite) writeline("data:setValue(dying)");
      break;

    case Death:
    case BuryVictim:
    case BeforeGameOverJudge:
    case GameOverJudge:
      if (rewrite) writestr("\n");
      clearLocal("死亡的角色", "death.who", rewrite);
      clearLocal("死亡的伤害来源", "death.damage.from", rewrite);
      clearLocal("死亡的伤害目标", "death.damage.to", rewrite);
      clearLocal("造成死亡的伤害的牌", "death.damage.card", rewrite);
      clearLocal("死亡的伤害值", "death.damage.damage", rewrite);
      clearLocal("死亡的伤害属性", "death.damage.nature", rewrite);
      clearLocal("死亡的伤害是传导伤害", "death.damage.chain", rewrite);
      clearLocal("死亡的伤害是转移伤害", "death.damage.transfer", rewrite);
      clearLocal("死亡的角色是目标角色", "death.damage.by_user", rewrite);
      clearLocal("造成死亡的伤害的原因", "death.damage.reason", rewrite);
      clearLocal("死亡的伤害传导的原因", "death.damage.transfer_reason", rewrite);
      clearLocal("死亡的伤害被防止", "death.damage.prevented", rewrite);
      if (rewrite) writeline("data:setValue(death)");
      break;

    case PreCardResponded:
    case CardResponded:
      if (rewrite) writestr("\n");
      clearLocal("响应的牌", "resp.m_card", rewrite);
      clearLocal("响应的目标", "resp.m_who", rewrite);
      clearLocal("响应的牌被使用", "resp.m_isUse", rewrite);
      clearLocal("响应的牌被改判", "resp.m_isRetrial", rewrite);
      clearLocal("响应的牌是手牌", "resp.m_isHandcard", rewrite);
      if (rewrite) writeline("data:setValue(resp)");
      break;

    case BeforeCardsMove:
    case CardsMoveOneTime:
      if (rewrite) writestr("\n");
      clearLocal("移动的牌", "move.card_ids", rewrite);
      clearLocal("移动来源地", "move.from_place", rewrite);
      clearLocal("移动目的地", "move.to_place", rewrite);
      clearLocal("移动的来源的名字", "move.from_player_name", rewrite);
      clearLocal("移动的目标的名字", "move.to_player_name", rewrite);
      clearLocal("移动的牌堆", "move.from_pile_name", rewrite);
      clearLocal("目的地牌堆", "move.to_pile_name", rewrite);
      clearLocal("移动的来源", "move.from", rewrite);
      clearLocal("移动的目标", "move.to", rewrite);
      clearLocal("移动的原因", "move.reason.m_reason", rewrite);
      clearLocal("是打开的", "move.open", rewrite);
      clearLocal("是最后的手牌", "move.is_last_handcard", rewrite);
      if (rewrite) writeline("data:setValue(move)");
      break;

    case CardUsed:
    case TargetSpecifying:
    case TargetConfirming:
    case TargetSpecified:
    case TargetConfirmed:
    case CardFinished:
      if (rewrite) writestr("\n");
      clearLocal("使用的牌", "use.card", rewrite);
      clearLocal("使用者", "use.from", rewrite);
      clearLocal("目标", "use.to", rewrite);
      clearLocal("持有者是使用者", "use.m_isOwnerUse", rewrite);
      clearLocal("计入使用次数", "use.m_addHistory", rewrite);
      clearLocal("是手牌", "use.m_isHandcard", rewrite);
      clearLocal("无效的目标", "use.nullified_list", rewrite);
      if (rewrite) writeline("data:setValue(use)");
      break;

    case CardEffected:
      if (rewrite) writestr("\n");
      clearLocal("生效的牌", "effect.card", rewrite);
      clearLocal("使用者", "effect.from", rewrite);
      clearLocal("目标", "effect.to", rewrite);
      clearLocal("是否生效", "effect.nullified", rewrite);
      if (rewrite) writeline("data:setValue(effect)");
      break;

    default:
      break;
  }
}

static void startDef(int params, ...) {
  va_list ap;
  va_start(ap, params);

  Hash *param_symtab = hash_new();
  current_tab = param_symtab;
  stack_push(symtab_stack, cast(Object *, param_symtab));

  const char *src;
  const char *dst;
  ExpVType type;
  int analyzed = 0;

  while (analyzed < params) {
    src = va_arg(ap, const char *);
    dst = va_arg(ap, const char *);
    type = va_arg(ap, ExpVType);
    sym_new_entry(src, type, dst, true);
    analyzed++;
  }

  va_end(ap);
}

static void endDef() {
  sym_free(cast(Hash *, stack_gettop(symtab_stack)));
  stack_pop(symtab_stack);
}

#define START_DEF \
  indent_level++; \
  writeline("local locals = {}"); \
  writeline("global_self = self")

#define END_DEF \
  indent_level--; \
  writeline("end,")

static void analyzeTriggerSpec(TriggerSpecObj *t) {
  startDef(2,
    "你", "player", TPlayer,
    "当前目标", "target", TPlayer
  );

  writeline("[%s] = {", qsan_event_table[t->event]);
  indent_level++;
  //writeline("-- can_trigger");
  if (t->can_trigger) {
    writeline("function (self, target, player, data)");
    START_DEF;
    writeline("local room = player:getRoom()");
    initData(t->event);
    analyzeBlock(t->can_trigger);
    if (t->can_trigger->ret == NULL) clearData(t->event);
    END_DEF;
  } else {
    writeline("nil,");
  }

  //writeline("-- on effect");
  writeline("function (self, target, player, data)");
  START_DEF;
  writeline("local room = player:getRoom()");
  initData(t->event);
  analyzeBlock(t->on_trigger);
  if (t->on_trigger->ret == NULL) clearData(t->event);
  END_DEF;

  //writeline("-- on_cost");
  if (t->on_cost) {
    writeline("function (self, target, player, data)");
    START_DEF;
    writeline("local room = player:getRoom()");
    initData(t->event);
    analyzeBlock(t->on_cost);
    if (t->on_cost->ret == NULL) clearData(t->event);
    END_DEF;
  } else {
    writeline("nil,");
  }

  //writeline("-- how to cost");
  if (t->how_cost) {
    writeline("function (self, funcs, target, player, data)");
    START_DEF;
    writeline("local room = player:getRoom()");
    initData(t->event);
    analyzeBlock(t->how_cost);
    if (t->how_cost->ret == NULL) clearData(t->event);
    END_DEF;
  } else {
    writeline("nil,");
  }

  indent_level--;
  writeline("},");

  endDef();
  current_tab = cast(Hash *, stack_gettop(symtab_stack));
}

static void analyzeActiveSpec(ActiveSpecObj *a) {
  startDef(1, "你", "player", TPlayer);
  writeline("can_use = function(self, player)");
  START_DEF;
  analyzeBlock(a->cond);
  END_DEF;
  endDef();

  startDef(3,
    "你", "sgs.Self", TPlayer,
    "已选卡牌", "selected", TCardList,
    "备选卡牌", "to_select", TCard
  );
  writeline("card_filter = function(self, selected, to_select)");
  START_DEF;
  analyzeBlock(a->card_filter);
  END_DEF;
  endDef();

  startDef(4,
    "你", "sgs.Self", TPlayer,
    "已选目标", "targets", TPlayerList,
    "备选目标", "to_select", TPlayer,
    "已选卡牌", "selected", TCardList
  );
  writeline("target_filter = function(self, targets, to_select, selected)");
  START_DEF;
  analyzeBlock(a->target_filter);
  END_DEF;
  endDef();

  startDef(3,
    "你", "sgs.Self", TPlayer,
    "已选目标", "targets", TPlayerList,
    "已选卡牌", "cards", TCardList
  );
  writeline("feasible = function(self, targets, cards)");
  START_DEF;
  analyzeBlock(a->feasible);
  END_DEF;
  endDef();

  startDef(3,
    "你", "player", TPlayer,
    "选择的目标", "targets", TPlayerList,
    "选择的卡牌", "cards", TCardList
  );
  writeline("on_use = function(self, player, targets, cards)");
  START_DEF;
  writeline("local room = player:getRoom()");
  analyzeBlock(a->on_use);
  END_DEF;
  endDef();

  if (a->on_effect) {
    startDef(3,
      "你", "effect.from", TPlayer,
      "目标", "effect.to", TPlayer,
      "卡牌列表", "self:getSubCards()", TCardList
    );
    writeline("on_effect = function(self, effect)");
    START_DEF;
    writeline("local room = effect.from:getRoom()");
    analyzeBlock(a->on_effect);
    END_DEF;
    endDef();
  }

  current_tab = cast(Hash *, stack_gettop(symtab_stack));
}

static void analyzeViewAsSpec(ViewAsSpecObj *v) {
  startDef(1, "你", "player", TPlayer);
  writeline("can_use = function(self, player)");
  START_DEF;
  analyzeBlock(v->cond);
  END_DEF;
  endDef();

  startDef(3,
    "你", "sgs.Self", TPlayer,
    "已选卡牌", "selected", TCardList,
    "备选卡牌", "to_select", TCard
  );
  writeline("card_filter = function(self, selected, to_select)");
  START_DEF;
  analyzeBlock(v->card_filter);
  END_DEF;
  endDef();

  startDef(2,
    "你", "sgs.Self", TPlayer,
    "已选卡牌", "cards", TCardList
  );
  writeline("feasible = function(self, cards)");
  START_DEF;
  analyzeBlock(v->feasible);
  END_DEF;
  endDef();

  startDef(2,
    "你", "sgs.Self", TPlayer,
    "选择的卡牌", "cards", TCardList
  );
  writeline("view_as = function(self, cards)");
  START_DEF;
  analyzeBlock(v->view_as);
  END_DEF;
  endDef();

  if (v->can_response) {
    startDef(1, "你", "player", TPlayer);
    writeline("can_response = function(self, player)");
    START_DEF;
    analyzeBlock(v->can_response);
    END_DEF;
    endDef();
  }

  if (v->responsable) {
    print_indent();
    writestr("response_patterns = ");
    analyzeExp(v->responsable);
    checktype(v->responsable, v->responsable->valuetype, TStringList);
    writestr("\n");
  }

  current_tab = cast(Hash *, stack_gettop(symtab_stack));
}

static void analyzeStatusSpec(const char *name, const char *trans, StatusSpecObj *s) {
  char buf[64];
  /* prohibit skill */
  if (s->is_prohibited) {
    sprintf(buf, "#%s_pr", name);
    addTranslation(buf, trans);
    writeline("%s_pr = fkp.CreateProhibitSkill{", name);
    indent_level++;
    writeline("name = '#%s_pr',", name);

    startDef(3,
      "来源", "from", TPlayer,
      "目标", "to", TPlayer,
      "卡牌", "card", TCard
    );
    writeline("is_prohibited = function(self, from, to, card)");
    START_DEF;
    analyzeBlock(s->is_prohibited);
    END_DEF;
    endDef();

    indent_level--;
    writeline("}");
    writeline("if not sgs.Sanguosha:getSkill('#%s_pr') then \
all_skills:append(%s_pr) end", name, name);
    writeline("extension0:insertRelatedSkills('%s', '#%s_pr')\n", name, name);
  }

  /* filter skill */
  if (s->card_filter && s->vsrule) {
    sprintf(buf, "#%s_fi", name);
    addTranslation(buf, trans);
    writeline("%s_fi = fkp.CreateFilterSkill{", name);
    indent_level++;
    writeline("name = '#%s_fi',", name);

    startDef(2,
      "你", "sgs.Self", TPlayer,
      "备选卡牌", "to_select", TCard
    );
    writeline("card_filter = function(self, to_select)");
    START_DEF;
    analyzeBlock(s->card_filter);
    END_DEF;
    endDef();

    startDef(2,
      "你", "sgs.Self", TPlayer,
      "备选卡牌", "to_select", TCard
    );
    writeline("view_as = function(self, to_select)");
    START_DEF;
    analyzeBlock(s->vsrule);
    END_DEF;
    endDef();

    indent_level--;
    writeline("}");
    writeline("if not sgs.Sanguosha:getSkill('#%s_fi') then \
all_skills:append(%s_fi) end", name, name);
    writeline("extension0:insertRelatedSkills('%s', '#%s_fi')\n", name, name);
  }

  /* distance skill */
  if (s->distance_correct) {
    sprintf(buf, "#%s_di", name);
    addTranslation(buf, trans);
    writeline("%s_di = fkp.CreateDistanceSkill{", name);
    indent_level++;
    writeline("name = '#%s_di',", name);

    startDef(2,
      "来源", "from", TPlayer,
      "目标", "to", TPlayer
    );
    writeline("correct_func = function(self, from, to)");
    START_DEF;
    analyzeBlock(s->distance_correct);
    END_DEF;
    endDef();

    indent_level--;
    writeline("}");
    writeline("if not sgs.Sanguosha:getSkill('#%s_di') then \
all_skills:append(%s_di) end", name, name);
    writeline("extension0:insertRelatedSkills('%s', '#%s_di')\n", name, name);
  }

  /* maxcard skill */
  if (s->max_extra || s->max_fixed) {
    sprintf(buf, "#%s_ex", name);
    addTranslation(buf, trans);
    writeline("%s_ex = fkp.CreateMaxCardsSkill{", name);
    indent_level++;
    writeline("name = '#%s_ex',", name);

    if (s->max_extra) {
      startDef(1, "玩家", "target", TPlayer);
      writeline("extra_func = function(self, target)");
      START_DEF;
      analyzeBlock(s->max_extra);
      END_DEF;
      endDef();
    }

    if (s->max_fixed) {
      startDef(1, "玩家", "target", TPlayer);
      writeline("fixed_func = function(self, target)");
      START_DEF;
      analyzeBlock(s->max_fixed);
      END_DEF;
      endDef();
    }

    indent_level--;
    writeline("}");
    writeline("if not sgs.Sanguosha:getSkill('#%s_ex') then \
all_skills:append(%s_ex) end", name, name);
    writeline("extension0:insertRelatedSkills('%s', '#%s_ex')\n", name, name);
  }

  /* tmd skill */
  if (s->tmd_distance || s->tmd_extarget || s->tmd_residue) {
    sprintf(buf, "#%s_tm", name);
    addTranslation(buf, trans);
    writeline("%s_tm = fkp.CreateTargetModSkill{", name);
    indent_level++;
    writeline("name = '#%s_tm',", name);
    writeline("pattern = '.',");

    if (s->tmd_distance) {
      startDef(2,
        "玩家", "target", TPlayer,
        "卡牌", "card", TCard
      );
      writeline("distance_limit_func = function(self, target, card)");
      START_DEF;
      analyzeBlock(s->tmd_distance);
      END_DEF;
      endDef();
    }

    if (s->tmd_extarget) {
      startDef(2,
        "玩家", "target", TPlayer,
        "卡牌", "card", TCard
      );
      writeline("extra_target_func = function(self, target, card)");
      START_DEF;
      analyzeBlock(s->tmd_extarget);
      END_DEF;
      endDef();
    }

    if (s->tmd_residue) {
      startDef(2,
        "玩家", "target", TPlayer,
        "卡牌", "card", TCard
      );
      writeline("residue_func = function(self, target, card)");
      START_DEF;
      analyzeBlock(s->tmd_residue);
      END_DEF;
      endDef();
    }

    indent_level--;
    writeline("}");
    writeline("if not sgs.Sanguosha:getSkill('#%s_tm') then \
all_skills:append(%s_tm) end", name, name);
    writeline("extension0:insertRelatedSkills('%s', '#%s_tm')\n", name, name);
  }

  /* atkrange skill */
  if (s->atkrange_extra || s->atkrange_fixed) {
    sprintf(buf, "#%s_at", name);
    addTranslation(buf, trans);
    writeline("%s_at = fkp.CreateAttackRangeSkill{", name);
    indent_level++;
    writeline("name = '#%s_at',", name);

    if (s->atkrange_extra) {
      startDef(1, "玩家", "target", TPlayer);
      writeline("extra_func = function(self, target)");
      START_DEF;
      analyzeBlock(s->atkrange_extra);
      END_DEF;
      endDef();
    }

    if (s->atkrange_fixed) {
      startDef(1, "玩家", "target", TPlayer);
      writeline("fixed_func = function(self, target)");
      START_DEF;
      analyzeBlock(s->atkrange_fixed);
      END_DEF;
      endDef();
    }

    indent_level--;
    writeline("}");
    writeline("if not sgs.Sanguosha:getSkill('#%s_at') then \
all_skills:append(%s_at) end", name, name);
    writeline("extension0:insertRelatedSkills('%s', '#%s_at')\n", name, name);
  }

  current_tab = cast(Hash *, stack_gettop(symtab_stack));
}

static void analyzeSkill(SkillObj *s) {
  if (!currentpack) {
    yyerror(cast(YYLTYPE *, s), "必须先添加拓展包才能添加技能");
    return;
  }

  List *node;

  if (s->activeSpec) {
    writeline("%s_ac = fkp.CreateActiveSkill{", s->interid);
    indent_level++;
    writeline("name = '%s',", s->interid);
    analyzeActiveSpec(s->activeSpec);
    indent_level--;
    writeline("}");
  }

  if (s->vsSpec) {
    writeline("%s_vs = fkp.CreateViewAsSkill{", s->interid);
    indent_level++;
    writeline("name = '%s',", s->interid);
    analyzeViewAsSpec(s->vsSpec);
    indent_level--;
    writeline("}");
  }

  if (s->statusSpec) {
    analyzeStatusSpec(s->interid, s->id, s->statusSpec);
  }

  writeline("%s = fkp.CreateTriggerSkill{", s->interid);
  indent_level++;
  writeline("name = '%s',", s->interid);
  writeline("frequency = %s,", sym_lookup(s->frequency)->origtext);
  if (s->activeSpec)
    writeline("view_as_skill = %s_ac,", s->interid);
  else if (s->vsSpec)
    writeline("view_as_skill = %s_vs,", s->interid);

  writeline("specs = {");
  indent_level++;
  if (s->triggerSpecs)
    list_foreach(node, s->triggerSpecs) {
      TriggerSpecObj *t = cast(TriggerSpecObj *, node->data);
      if (!t->is_refresh)
        analyzeTriggerSpec(t);
    }
  indent_level--;
  writeline("},");

  writeline("refresh_specs = {");
  indent_level++;
  if (s->triggerSpecs)
    list_foreach(node, s->triggerSpecs) {
      TriggerSpecObj *t = cast(TriggerSpecObj *, node->data);
      if (t->is_refresh)
        analyzeTriggerSpec(t);
    }
  indent_level--;
  writeline("}");

  indent_level--;
  writeline("}");
  writeline("if not sgs.Sanguosha:getSkill('%s') then \
all_skills:append(%s) end\n", s->interid, s->interid);
}

#undef START_DEF
#undef END_DEF

static void analyzeGeneral(GeneralObj *g) {
  if (!currentpack) {
    yyerror(cast(YYLTYPE *, g), "必须先添加拓展包才能添加武将");
    return;
  }

  const char *orig = sym_lookup(g->id)->origtext;
  writestr("%s = sgs.General(extension%d, '%s', %s, %lld)\n",
           orig, currentpack->internal_id, orig,
           sym_lookup(g->kingdom)->origtext, g->hp);
  writestr("%s:setGender(%s)\n", orig, sym_lookup(g->gender)->origtext);

  List *node;
  list_foreach(node, g->skills) {
    ExpressionObj *e = cast(ExpressionObj *, node->data);
    if (e->exptype != ExpStr) {
      yyerror(cast(YYLTYPE *, g), "给武将添加的技能必须是字符串类型");
      return;
    }
    const char *skill_orig = cast(const char *, e->strvalue);
    const char *skill = hash_get(skill_table, skill_orig);
    if (!skill) {
      /* yyerror(cast(YYLTYPE *, g), "只能为武将添加文件内的自定义技能！"); */
      /* 还是允许添加别的技能算了，假设用户知道内部名字 */
      fprintf(error_output, "警告：为武将“%s”添加的技能 “%s” 不是文件内定义的。如果你\
输入的技能名称不是游戏内部已经自带的技能的内部名称的话，\
游戏将会在开始时崩溃！\n\n", g->id, skill_orig);
      skill = skill_orig;
    }
    writestr("%s:addSkill('%s')\n", orig, skill);
  }
  writestr("\n");
}

static void analyzePackage(PackageObj *p) {
  currentpack = p;
  writestr("local extension%d = sgs.Package('%s')\n",
           p->internal_id, p->id);
  writestr("\n");
}

static void loadTranslations() {
  writestr("sgs.LoadTranslationTable{\n");
  List *node;
  indent_level++;
  list_foreach(node, restrtab) {
    str_value *v = cast(str_value *, node->data);
    writeline("['%s'] = '%s',", v->origtxt, v->translated);
  }
  indent_level--;
  writeline("}\n");
}

static void analyzeFuncdef(FuncdefObj *f) {
  print_indent();
  if (strcmp(f->funcname, ""))
    writestr("local function %s(", f->funcname);
  else
    writestr("function(");

  sym_new_entry(f->name, TFunc, cast(const char *, f), false);

  List *node;
  int argId = 0;
  Hash *param_symtab = hash_new();
  current_tab = param_symtab;
  stack_push(symtab_stack, cast(Object *, param_symtab));
  List *param_gclist = list_new();
  const char *s;
  DefargObj *d;
  char buf[64];
  list_foreach(node, f->params) {
    d = cast(DefargObj *, node->data);
    if (argId != 0) writestr(", ");
    sprintf(buf, "arg%d", argId);
    s = strdup(buf);
    writestr(s);
    sym_new_entry(d->name, d->type, s, false);
    list_append(param_gclist, cast(Object *, s));
    argId++;
  }

  writestr(")\n");
  indent_level++;

  argId = 0;
  bool roomDefined = false;
  list_foreach(node, f->params) {
    /* type check should done by fkparse */
    d = cast(DefargObj *, node->data);
    if (d->d) {
      print_indent();
      writestr("if arg%d == nil then arg%d = ", argId, argId);
      analyzeExp(d->d);
      writestr(" end\n");
    }
    if (d->type == TPlayer && !roomDefined) {
      writeline("local room = arg%d:getRoom()", argId);
      roomDefined = true;
    }
    argId++;
  }
  writeline("local self = global_self");
  writeline("local locals = {}\n");

  analyzeBlock(f->funcbody);
  indent_level--;
  print_indent();
  writestr("end\n\n");

  list_free(param_gclist, free);
  stack_pop(symtab_stack);
  sym_free(param_symtab);
  current_tab = cast(Hash *, stack_gettop(symtab_stack));
}

void analyzeBlock(BlockObj *bl) {
  List *node;
  Hash *symtab = hash_new();
  current_tab = symtab;
  stack_push(symtab_stack, cast(Object *, current_tab));

  list_foreach(node, bl->statements) {
    if (!node->data)  /* maybe error token */
      continue;

    switch (node->data->objtype) {
    case Obj_Assign:
      analyzeAssign(cast(AssignObj *, node->data));
      break;
    case Obj_If:
      analyzeIf(cast(IfObj *, node->data));
      break;
    case Obj_Loop:
      analyzeLoop(cast(LoopObj *, node->data));
      break;
    case Obj_Traverse:
      analyzeTraverse(cast(TraverseObj *, node->data));
      break;
    case Obj_Break:
      writeline("break");
      break;
    case Obj_Docost:
      writeline("if fkp.functions.do_cost(self, funcs, target, player, data) then");
      indent_level++;
      writeline("return true");
      indent_level--;
      writeline("end");
      break;
    case Obj_Funccall:
      print_indent();
      analyzeFunccall(cast(FunccallObj *, node->data));
      writestr("\n");
      break;
    case Obj_Funcdef:
      analyzeFuncdef(cast(FuncdefObj *, node->data));
      break;

    /* for root-statements */
    case Obj_Package:
      analyzePackage(cast(PackageObj *, node->data));
      break;
    case Obj_General:
      analyzeGeneral(cast(GeneralObj *, node->data));
      break;
    case Obj_Skill:
      analyzeSkill(cast(SkillObj *, node->data));
      break;
    default:
      break;
    }
  }

  if (bl->ret) {
    print_indent();
    writestr("return ");
    analyzeExp(bl->ret);
    writestr("\n");
  }

  stack_pop(symtab_stack);
  sym_free(symtab);
  current_tab = cast(Hash *, stack_gettop(symtab_stack));
}

void analyzeExtensionQSan(ExtensionObj *e) {
  writeline("require 'fkparser'\n\nlocal global_self\nlocal locals = {}\n");
  writeline("local all_skills = sgs.SkillList()\n");

  currentpack = NULL;
  BlockObj *b = newBlock(e->stats, NULL);
  analyzeBlock(b);
  free(b);  /* stats will be freed in freeObject. */

  loadTranslations();
  writeline("sgs.Sanguosha:addSkills(all_skills)");

  writestr("return {");
  for (int i = 0; i < package_id; i++) {
    writestr("extension%d, ", i);
  }
  writestr("}\n");
}
