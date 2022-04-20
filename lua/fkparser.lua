fkp = {}
function fkp.CreateTriggerSkill(spec)
  assert(type(spec.name) == "string")

  local frequency = spec.frequency or sgs.Skill_NotFrequent
  local limit_mark = spec.limit_mark or ""
  local skill = sgs.LuaTriggerSkill(spec.name, frequency, limit_mark)

  local specs = spec.specs or {}

  for event, _ in pairs(specs) do
    skill:addEvent(event)
  end

  skill.on_trigger = function(self, event, player, data)
    local room = player:getRoom()
    if not specs[event] then return end
    for _, p in sgs.qlist(room:findPlayersBySkillName(self:objectName())) do
      if specs[event][1](self, player, p, data) then
        return specs[event][2](self, player, p, data)
      end
    end
  end

  skill.can_trigger = function(self, target)
    return target
  end

  return skill
end

dummyobj = {
  objectName = function() return "" end
}
