fkp = {}

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
    return arr:at(i)
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

  obtainCard = function(player, card, reason, open)
    player:getRoom():obtainCard(player, card, reason, open)
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
}

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
      for _, p in sgs.qlist(room:findPlayersBySkillName(self:objectName())) do
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
      local plist = sgs.SPlayerList()
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
      local plist = sgs.SPlayerList()
      for _, p in ipairs(targets) do
        plist:append(p)
      end
      return spec.target_filter(self, plist, to_select)
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
