fkp = {}

sgs.LoadTranslationTable{
  ["#Discard"] = "请弃置 %arg 张手牌",
  ["#DiscardWithMin"] = "请弃置 %arg 张手牌，至少弃置 %arg2 张",
  ["#DiscardWithEquip"] = "请弃置 %arg 张牌（包括装备区）",
  ["#DiscardWithEquipMin"] = "请弃置 %arg 张牌，至少弃置 %arg2 张（包括装备区）",
}

local string2suit = {
  spade = sgs.Card_Spade,
  club = sgs.Card_Club,
  heart = sgs.Card_Heart,
  diamond = sgs.Card_Diamond,
  no_suit = sgs.Card_NoSuit,
  no_suit_black = sgs.Card_NoSuitBlack,
  no_suit_red = sgs.Card_NoSuitRed,
}

fkp.functions = {
  prepend = function(arr, e)
    arr:prepend(e)
  end,

  append = function(arr, e)
    arr:append(e)
  end,

  removeOne = function(arr, e)
    arr:removeOne(e)
  end,

  at = function(arr, i)
    return arr:at(i - 1)
  end,
---------------------------------
  drawCards = function(p, n)
    p:drawCards(n)
  end,

  loseHp = function(p, n)
    p:getRoom():loseHp(p, n)
  end,

  loseMaxHp = function(p, n)
    p:getRoom():loseMaxHp(p, n)
  end,

  damage = function(from, to, n, nature, card, reason)
    local damage = sgs.DamageStruct()
    damage.from = from
    damage.to = to
    damage.damage = n
    damage.nature = nature
    damage.card = card
    damage.reason = reason
    to:getRoom():damage(damage)
  end,

  recover = function(player, int, who, card)
    local recover = sgs.RecoverStruct()
    recover.recover = int
    recover.who = who
    recover.card = card
    player:getRoom():recover(player, recover)
  end,

  recoverMaxHp = function(player, int)
    local room = player:getRoom()
    local msg = sgs.LogMessage()
    local mhp = sgs.QVariant()
    room:setPlayerProperty(player, "maxhp", sgs.QVariant(player:getMaxHp() + int))
    msg.type = "#GainMaxHp"
    msg.from = player
    msg.arg = int
    room:sendLog(msg)
  end,

  acquireSkill = function(player, skill)
    player:getRoom():acquireSkill(player, skill)
  end,

  loseSkill = function(player, skill)
    player:getRoom():detachSkillFromPlayer(player, skill)
  end,

  addMark = function(player, mark, count, hidden)
    local room = player:getRoom()
    if hidden then
      mark = string.gsub(mark, "@", "_")
    end

    if hidden then
      room:addPlayerMark(player, mark, count)
    else
      player:gainMark(mark, count)
    end
  end,

  loseMark = function(player, mark, count, hidden)
    local room = player:getRoom()
    if hidden then
      mark = string.gsub(mark, "@", "_")
    end

    if hidden then
      room:removePlayerMark(player, mark, count)
    else
      player:loseMark(mark, count)
    end
  end,

  getMark = function(player, mark, hidden)
    if hidden then
      mark = string.gsub(mark, "@", "_")
    end

    return player:getMark(mark)
  end,

  askForChoice = function(player, choices, reason)
    return player:getRoom():askForChoice(player, reason, table.concat(choices, "+"))
  end,

  askForPlayerChosen = function(player, targets, reason, prompt, optional, notify)
    return player:getRoom():askForPlayerChosen(player, targets, reason, prompt, optional, notify)
  end,

  askForSkillInvoke = function(player, skill)
    return player:askForSkillInvoke(skill)
  end,

  obtainCard = function(player, card, open)
    player:getRoom():obtainCard(player, card, open)
  end,

  hasSkill = function(player, skill)
    return player:hasSkill(skill)
  end,

  throwCardsBySkill = function(player, cards, skill_name)
    local room = player:getRoom()
    local reason = sgs.CardMoveReason(sgs.CardMoveReason_S_REASON_THROW, player:objectName(), "", skill_name, "")
    local moves = sgs.CardsMoveList()
    for _, cd in sgs.list(cards) do
      local move = sgs.CardsMoveStruct(cd:getId(), nil, sgs.Player_DiscardPile, reason)
      moves:append(move)
    end
    room:moveCardsAtomic(moves, true)
  end,

  getUsedTimes = function(player, skill)
    return player:usedTimes('#' .. skill)
  end,

  broadcastSkillInvoke = function(player, skill, index)
    player:getRoom():broadcastSkillInvoke(skill, index)
  end,

  askForDiscard = function(target, skill, discard_num, min_num, optional, include_equip, prompt, pattern)
    if min_num == -1 then
      min_num = discard_num
    end

    if prompt == "" then
      if include_equip then
        if min_num < discard_num then
          prompt = string.format("#DiscardWithEquipMin:::%d:%d", discard_num, min_num);
        else
          prompt = "#DiscardWithEquip"
        end
      else
        if min_num < discard_num then
          prompt = string.format("#DiscardWithMin:::%d:%d", discard_num, min_num);
        else
          prompt = "#Discard"
        end
      end
    end

    local room = target:getRoom()
    local card = room:askForExchange(target, skill, discard_num, min_num, include_equip, prompt, optional, pattern)
    local ret = sgs.CardList()
    if card then
      local mreason = sgs.CardMoveReason(sgs.CardMoveReason_S_REASON_THROW, target:objectName(), "", card:getSkillName(), skill)
      room:throwCard(card, mreason, target)

      local idlist = card:getSubcards()
      for _, id in sgs.list(idlist) do
        ret:append(sgs.Sanguosha:getCard(id))
      end
      card:deleteLater()
    end
    return ret
  end,

  swapPile = function(player)
    player:getRoom():swapPile()
  end,

  changeHero = function(player, new_general, full_state, invokeStart, isSecondaryHero, sendLog)
    player:getRoom():changeHero(player, new_general, full_state, invokeStart, isSecondaryHero, sendLog)
  end,

  swapSeat = function(playerA, playerB)
    playerA:getRoom():swapSeat(playerA, playerB)
  end,

  askForGuanxing = function(player, cards, guanxing_type)
    local idlist = sgs.IntList()
    for _,card in sgs.list(cards) do
      idlist:append(card:getId())
    end
    player:getRoom():askForGuanxing(player, idlist, guanxing_type)
  end,
  
  getNCards = function(player, card_number, update_pile_number)
    local room = player:getRoom()
    local idlist = room:getNCards(card_number, update_pile_number)
    local cdl = sgs.CardList()
    for _, id in sgs.list(idlist) do
      cdl:append(sgs.Sanguosha:getCard(id))
    end
    return cdl
  end,

  retrial = function(card, player, judge, skill_name, exchange)
    local room = player:getRoom()
    return room:retrial(card, player, judge, skill_name, exchange)
  end

}

fkp.functions.buildPrompt = function(base, src, dest, arg, arg2)
  if src == nil then
    src = ""
  else
    src = src:objectName()
  end
  if dest == nil then
    dest = ""
  else
    dest = dest:objectName()
  end
  if arg == nil then arg = "" end
  if arg2 == nil then arg2 = "" end

  local prompt_tab = {src, dest, arg, arg2}
  if arg2 == "" then
    table.remove(prompt_tab, 4)
    if arg == "" then
      table.remove(prompt_tab, 3)
      if dest == "" then
        table.remove(prompt_tab, 2)
        if src == "" then
          table.remove(prompt_tab, 1)
        end
      end
    end
  end

  for _, str in ipairs(prompt_tab) do
    base = base .. ":" .. str
  end

  return base
end

fkp.functions.judge = function(player, reason, pattern, good, play_animation)
  local judge = sgs.JudgeStruct()
  judge.who = player
  judge.reason = reason
  judge.pattern = pattern
  judge.good = good
  judge.play_animation = play_animation
  player:getRoom():judge(judge)
  return judge.card
end

fkp.functions.buildPattern = function(names, suits, numbers)
  if not names then names = {"."} end
  if not suits then suits = {"."} end
  if not numbers then numbers = {"."} end

  names = table.concat(names, ",")
  -- FIXME: write getters in lua
  names = string.gsub(names, "basic,", "BasicCard,")
  names = string.gsub(names, "trick,", "TrickCard,")
  names = string.gsub(names, "equip", "EquipCard")

  suits = table.concat(suits, ",")
  numbers = table.concat(numbers, ",")
  return string.format("%s|%s|%s|.", names, suits, numbers)
end

fkp.functions.newVirtualCard = function(number, suit, name, subcards, skill)
  if not subcards then subcards = sgs.CardList() end
  local ret = sgs.Sanguosha:cloneCard(name, string2suit[suit], number)
  if not ret then
    ret = sgs.Sanguosha:cloneCard("slash", string2suit[suit], number)
  end
  ret:setSkillName(skill)
  ret:addSubcards(subcards)
  return ret
end

local function table_filter(tab, func)
  local ret = {}
  for index, item in ipairs(tab) do
    if func(item) then
      ret[index] = item
    end
  end
  return ret
end

fkp.functions.patternMatch = function(pattern1, pattern2)
  local pattern_tab1 = pattern1:split('#')[1]:split('|')
  local pattern_tab2 = pattern2:split('#')[1]:split('|')
  for i = 1, 4 - #pattern_tab1 do
    table.insert(pattern_tab1, '.')
  end
  for i = 1, 4 - #pattern_tab2 do
    table.insert(pattern_tab2, '.')
  end
  for i = 1, 4 do
    pattern_tab1[i] = pattern_tab1[i]:split(',')
    pattern_tab2[i] = pattern_tab2[i]:split(',')
    if #table_filter(pattern_tab1[i], function(item)
      -- TODO: handle 'BasicCard', etc.
      return item == '.'
          or table.contains(pattern_tab2[i], item)
          or table.contains(pattern_tab2[i], '.')
    end) == 0 then
      return false
    end
  end
  return true
end

function fkp.newlist(t)
  local element_type = swig_type(t[1])
  local ret
  if element_type == "ServerPlayer *" then
    ret = sgs.SPlayerList()
  elseif element_type == "Player *" then
    ret = sgs.PlayerList()
  elseif element_type == "Card *" then
    ret = sgs.CardList()
  elseif element_type == "number" then
    ret = sgs.IntList()
  elseif element_type == "string" then
    ret = {
      length = function(self)
        return #self
      end,

      prepend = function(self, element)
        if type(self[1]) ~= type(element) then return end
        for i = #self, 1, -1 do
          self[i + 1] = self[i]
        end
        self[1] = element
      end,

      append = function(self, element)
        if type(self[1]) ~= type(element) then return end
        table.insert(self, element)
      end,

      removeOne = function(self, element)
        if #self == 0 or type(self[1]) ~= type(element) then return false end

        for i = 1, #self do
          if self[i] == element then
            table.remove(self, i)
            return true
          end
        end
        return false
      end,

      at = function(self, index)
        return self[index + 1]
      end,
    }
  end

  for _, t in ipairs(t) do
    if swig_type(ret) == "table" then
      table.insert(ret, t)
    else
      ret:append(t)
    end
  end
  return ret
end

function fkp.CreateTriggerSkill(spec)
  assert(type(spec.name) == "string")

  local freq = spec.frequency or sgs.Skill_NotFrequent
  local limit = spec.limit_mark or ""
  local specs = spec.specs or {}
  local eve = {}
  for event, _ in pairs(specs) do
    table.insert(eve, event)
  end

  return sgs.CreateTriggerSkill{
    name = spec.name,
    frequency = freq,
    limit_mark = limit,
    view_as_skill = spec.view_as_skill,
    events = eve,
    on_trigger = function(self, event, player, data)
      local room = player:getRoom()
      if not specs[event] then return end
      for _, p in sgs.qlist(room:getAlivePlayers()) do
        if specs[event][1](self, player, p, data) then
          return specs[event][2](self, player, p, data)
        end
      end
    end,
    can_trigger = function(self, target)
      return target
    end
  }

end

function fkp.CreateActiveSkill(spec)
  assert(type(spec.name) == "string")

  local skill_card = sgs.CreateSkillCard{
    name = spec.name,
    target_fixed = false,
    will_throw = false,
    on_use = function(self, room, source, targets)
      local plist = sgs.SPlayerList()
      for _, p in ipairs(targets) do
        plist:append(p)
      end
      local clist = sgs.CardList()
      for _, id in sgs.list(self:getSubcards()) do
        clist:append(sgs.Sanguosha:getCard(id))
      end
      return spec.on_use(self, source, plist, clist)
    end,
    on_effect = spec.on_effect or function()end,  -- TODO
    feasible = function(self, targets)
      local plist = sgs.PlayerList()
      for _, p in ipairs(targets) do
        plist:append(p)
      end
      local clist = sgs.CardList()
      for _, id in sgs.list(self:getSubcards()) do
        clist:append(sgs.Sanguosha:getCard(id))
      end
      return spec.feasible(self, plist, clist)
    end,
    filter = function(self, targets, to_select)
      local plist = sgs.PlayerList()
      for _, p in ipairs(targets) do
        plist:append(p)
      end
      local clist = sgs.CardList()
      for _, id in sgs.list(self:getSubcards()) do
        clist:append(sgs.Sanguosha:getCard(id))
      end
      return spec.target_filter(self, plist, to_select, clist)
    end,
  }

  local vs_skill = sgs.CreateViewAsSkill{
    name = spec.name,
    n = 996,
    view_filter = function(self, selected, to_select)
      local clist = sgs.CardList()
      for _, c in ipairs(selected) do
        clist:append(c)
      end
      return spec.card_filter(self, clist, to_select)
    end,
    view_as = function(self, cards)
      local card = skill_card:clone()
      for _, c in ipairs(cards) do
        card:addSubcard(c)
      end
      return card
    end,
    enabled_at_play = spec.can_use,
    enabled_at_response = function(self, player, pattern)
      return pattern == "@@" .. spec.name
    end,
  }

  return vs_skill
end

function fkp.CreateViewAsSkill(spec)
  assert(type(spec.name) == "string")
  spec.response_patterns = spec.response_patterns or {}

  local vs_skill = sgs.CreateViewAsSkill{
    name = spec.name,
    n = 996,
    view_filter = function(self, selected, to_select)
      local clist = sgs.CardList()
      for _, c in ipairs(selected) do
        clist:append(c)
      end
      return spec.card_filter(self, clist, to_select)
    end,
    view_as = function(self, cards)
      local clist = sgs.CardList()
      for _, c in ipairs(cards) do
        clist:append(c)
      end
      if spec.feasible(self, clist) then
        return spec.view_as(self, clist)
      end
      return nil
    end,
    enabled_at_play = spec.can_use,
    enabled_at_response = function(self, player, pattern)
      if not (spec.can_response and spec.can_response(self, player)) then return false end

      for _, item in ipairs(spec.response_patterns) do
        if fkp.functions.patternMatch(item, pattern) then
          return true
        end
      end
      return false
    end,
    enabled_at_nullification = function(self, player)
      if not (spec.can_response and spec.can_response(self, player)) then return false end

      for _, item in ipairs(spec.response_patterns) do
        if fkp.functions.patternMatch(item, "nullification") then
          return true
        end
      end
      return false
    end,
  }

  return vs_skill
end

fkp.CreateFilterSkill = function(spec)
  assert(type(spec.name) == "string")
  return sgs.CreateFilterSkill{
    name = spec.name,
    view_filter = spec.card_filter,
    view_as = function(self, to_select)
      local ret = spec.view_as(self, to_select)
      if ret then
        ret:deleteLater()
        local card = sgs.Sanguosha:cloneCard(ret:objectName(), ret:getSuit(), ret:getNumber())
        card:setSkillName(self:objectName())
        local _card = sgs.Sanguosha:getWrappedCard(to_select:getId())
        _card:takeOver(card)
        return _card
      end
    end,
  }
end

fkp.CreateProhibitSkill = sgs.CreateProhibitSkill
fkp.CreateDistanceSkill = sgs.CreateDistanceSkill
fkp.CreateMaxCardsSkill = sgs.CreateMaxCardsSkill
fkp.CreateTargetModSkill = sgs.CreateTargetModSkill
fkp.CreateAttackRangeSkill = sgs.CreateAttackRangeSkill
