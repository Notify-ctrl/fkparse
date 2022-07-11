fkp = {}

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
