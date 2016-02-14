--缄魔：其他角色的出牌阶段开始时，若其手牌数不小于体力上限，你可令其选择一项：摸一张牌，且此阶段不能使用或打出【杀】；或此阶段使用【杀】时需弃置一张牌，否则此【杀】无效。
sgs.ai_skill_invoke.thjianmo = function(self, data)
	local target = data:toPlayer()
	if self:isEnemy(target) then
		return true
	end
	return false
end

sgs.ai_skill_choice.thjianmo = function(self, choices)
	if self:getCardsNum("Slash") < 1 then
		return "jian"
	end
	if self:getOverflow() < -1 then
		return "jian"
	end
	return "mo"
end

sgs.ai_skill_discard.thjianmo = function(self, discard_num, min_num, optional, include_equip)
	local ret = self:askForDiscard("", 1, 1, false, true)
	if #ret ~= 0 then
		self.room:writeToConsole(ret[1])
		if isCard("Peach", ret[1], self.player) then
			return {}
		else
			return ret
		end
	else
		return {}
	end
end

sgs.ai_choicemade_filter.skillInvoke.thjianmo = function(self, player, promptlist)
	if promptlist[#promptlist] == "yes" then
		local target = self.room:findPlayer(promptlist[#promptlist - 1])
		if target then
			sgs.updateIntention(player, target, 30)
		end
	end
end

--二重:觉醒技，若你回合外的两个连续的回合内，当前回合角色均未使用【杀】，且第二个回合的回合结束时，若你已受伤，你须减少1点体力上限，并获得技能“幻法”和“祝祭”。
--无

--春度：君主技，每当其他雪势力角色使用的红桃基本牌结算后置入弃牌堆时，你可弃置一张手牌获得之。
sgs.ai_skill_cardask["@thchundu"] = function(self, data, pattern)
	local card = data:toMoveOneTime().reason.m_extraData:toCard()
	if card then
		local h_cards = sgs.QList2Table(self.player:getHandcards())
		self:sortByKeepValue(h_cards)
		if self:getKeepValue(card) > self:getKeepValue(h_cards[1]) then
			return "$" .. h_cards[1]
		end
	end
	return "."
end

--醉觞：你的回合内，所有角色可以将两张牌当【酒】使用（你的回合内，所有角色使用的【酒】不计入使用限制）。
local thzuishang_skill = {}
thzuishang_skill.name = "thzuishang"
table.insert(sgs.ai_skills, thzuishang_skill)
thzuishang_skill.getTurnUseCard = function(self, inclusive)
	local cards = self.player:getCards("he")
	for _, id in sgs.qlist(self.player:getPile("wooden_ox")) do
		cards:prepend(sgs.Sanguosha:getCard(id))
	end
	cards = sgs.QList2Table(cards)
	for _, acard in ipairs(cards) do
		if isCard("Analeptic", acard, self.player) then return end
	end
	self:sortByUseValue(cards)
	local newcards = {}
	local has_slash = false
	for _, card in ipairs(cards) do
		if self:getCardsNum("Slash") == 1 and isCard("Slash", card, self.player) then
			continue
		end
		if self:getCardsNum("Slash") == 2 and isCard("Slash", card, self.player) and has_slash then
			continue
		end
		if not isCard("Analeptic", card, self.player) and not isCard("Peach", card, self.player) and not (isCard("ExNihilo", card, self.player) and self.player:getPhase() == sgs.Player_Play) then
			if isCard("Slash", card, self.player) then
				has_slash = true
			end
			table.insert(newcards, card)
		end
	end
	if #newcards <= self.player:getHp() - 1 and self.player:getHp() <= 4 and self:needKongcheng()
		and not (self.player:hasSkill("ikshengtian") and self.player:getMark("@shengtian") == 0) then return end
	if #newcards < 2 then return end

	local card_id1 = newcards[1]:getEffectiveId()
	local card_id2 = newcards[2]:getEffectiveId()

	local card_str = ("analeptic:%s[%s:%s]=%d+%d"):format("thzuishang", "to_be_decided", 0, card_id1, card_id2)
	local analeptic = sgs.Card_Parse(card_str)
	return analeptic
end

function cardsView_thzuishang(self, player)
	local cards = player:getCards("he")
	for _, id in sgs.qlist(player:getPile("wooden_ox")) do
		cards:prepend(sgs.Sanguosha:getCard(id))
	end
	cards = sgs.QList2Table(cards)
	for _, acard in ipairs(cards) do
		if isCard("Analeptic", acard, player) then return end
	end
	local newcards = {}
	for _, card in ipairs(cards) do
		if not isCard("Analeptic", card, player) and not isCard("Peach", card, player) and not (isCard("ExNihilo", card, player) and player:getPhase() == sgs.Player_Play) then
			table.insert(newcards, card)
		end
	end
	if #newcards < 2 then return end
	sgs.ais[player:objectName()]:sortByKeepValue(newcards)
	
	local card_id1 = newcards[1]:getEffectiveId()
	local card_id2 = newcards[2]:getEffectiveId()
	
	local card_str = ("analeptic:%s[%s:%s]=%d+%d"):format("thzuishang", "to_be_decided", 0, card_id1, card_id2)
	return card_str
end

function sgs.ai_cardsview.thzuishang(self, class_name, player)
	if class_name == "Analeptic" and player:getPhase() ~= sgs.Player_NotActive then
		return cardsView_thzuishang(self, player)
	end
end

function sgs.ai_cardsview.thzuishangv(self, class_name, player)
	if class_name == "Analeptic" then
		local obj_name = player:property("zhouhua_source"):toString()
		local splayer = self.room:findPlayer(obj_name)
		if splayer and splayer:hasSkill("thzuishang") then
			return cardsView_thzhouhua(self, player)
		end
	end
end

sgs.ai_skill_playerchosen.thxugu = sgs.ai_skill_playerchosen.zero_card_as_slash

sgs.ai_skill_use.analeptic = function(self, prompt, method)
	local list = prompt:split(":")
	local from = self.room:findPlayer(list[#list])
	if not from then
		return "."
	end
	if self:getCardsNum("Jink") > 1 then
		return "."
	end
	if self:getCardsNum("Jink") > 0 and not from:canSlash(self.player) then
		return "."
	end
	if getCardsNum("Slash", from, self.player) > 1 and from:canSlash(self.player) then
		local analeptic = self:getCardId("Analeptic")
		if analeptic then
			local card = sgs.Card_Parse(analeptic)
			for _, id in sgs.qlist(card:getSubcards()) do
				if isCard("Jink", id, self.player) then
					return "."
				end
			end
			return analeptic
		end
	end
	return "."
end

sgs.ai_playerchosen_intention.thxugu = 50

--慈航：当你使用的【杀】被目标角色的【闪】抵消时，你可以选择一项，以令此【杀】依然造成伤害：弃置等同于目标角色已损失的体力值数量的牌（不足则全弃）；或令目标角色摸等同于其体力值数量的牌（至多摸五张）。
sgs.thcihang_choice = ""

sgs.ai_skill_invoke.thcihang = function(self, data)
	local effect = data:toSlashEffect()
	local target = effect.to
	if self:isFriend(target) then
		if target:getHp() > 2 and not self:hasHeavySlashDamage(target, effect.slash) then
			sgs.thcihang_choice = "draw"
			return true
		end
		return false
	elseif self:isEnemy(target) then
		if not self:slashIsEffective(effect.slash, target, self.player) then
			return false
		end
		if self:willSkipPlayPhase(target) then
			sgs.thcihang_choice = "draw"
			return true
		end
		local losthp = target:getLostHp()
		if losthp < 2 or self.player:getCardCount() < 2 then
			sgs.thcihang_choice = "discard"
			return true
		end
		local hp = target:getHp()
		if hp < 2 or (hp < 4 and self:hasHeavySlashDamage(target, effect.slash)) then
			sgs.thcihang_choice = "draw"
			return true
		end
	end
	return false
end

sgs.ai_skill_choice.thcihang = function(self, choices, data)
	if sgs.thcihang_choice ~= "" then
		return sgs.thcihang_choice
	end
	local target = data:toPlayer()
	if self:isFriend(target) then
		return "draw"
	end
	return math.random(1, 3) == 1 and "discard" or "draw"
end

sgs.ai_choicemade_filter.skillChoice.thcihang = function(self, player, promptlist)
	local effect = player:getTag("ThCihangData"):toSlashEffect()
	local target = effect.to
	if target then
		if promptlist[#promptlist] == "discard" then
			sgs.updateIntention(player, target, 50)
		end
	end
end

--战操：你的回合外，当你或你的攻击范围内的一名角色成为【杀】的目标时，你可选择一项使该【杀】对其无效：失去1点体力，且该【杀】在结算后置入弃牌堆时，你获得之；或弃置一张非基本牌。
sgs.ai_skill_invoke.thzhancao = function(self, data)
	sgs.thzhancao_throw = nil
	local target = data:toPlayer()
	if self:isFriend(target) then
		local use = self.player:getTag("ThZhancaoData"):toCardUse()
		local slash = use.card
		local need_lost = 0
		if not slash:hasFlag("thzhancao") then
			if slash:isVirtualCard() then
				for _, id in sgs.qlist(slash:getSubcards()) do
					if isCard("Peach", id, self.player) then
						need_lost = 1
						break
					end
				end
			elseif isCard("Peach", slash, self.player) then
				need_lost = 1
			end
			if need_lost > 0 and (self:getHp() > 1 or self:getCardsNum({ "Peach", "Analeptic" }) >= 1) then
				sgs.thzhancao_throw = nil
				return true
			end
		else
			need_lost = -1
		end

		local not_basics = {}
		for _, c in sgs.qlist(self.player:getCards("he")) do
			if c:isKindOf("EquipCard") or c:isKindOf("TrickCard") then
				table.insert(not_basics, c)
			end
		end
		if #not_basics > 0 then
			self:sortByKeepValue(not_basics)
			sgs.thzhancao_throw = not_basics[1]:getEffectiveId()
		end
		if sgs.thzhancao_throw and isCard("Peach", sgs.thzhancao_throw, self.player) then
			sgs.thzhancao_throw = nil
		end
		if self:isWeak(target) and not self:isWeak(self.player) then
			return true
		end
		if sgs.getDefense(target) < sgs.getDefense(self.player) then
			if not slash:hasFlag("thzhancao") and self:getCardsNum("Slash") < 1 and self.player:getHp() > 2 then
				sgs.thzhancao_throw = nil
				return true
			end
			return true
		end
		if self:getOverflow() > 0 and sgs.thzhancao_throw then
			return true
		end
	end
	return false
end

sgs.ai_skill_cardask["@thzhancao"] = function(self, data, pattern)
	if sgs.thzhancao_throw then
		return "$" .. sgs.thzhancao_throw
	else
		return "."
	end
end

sgs.ai_cardneed.thzhancao = function(to, card, self)
	return card:getTypeId() ~= sgs.Card_TypeBasic
end

sgs.ai_choicemade_filter.skillInvoke.thzhancao = function(self, player, promptlist)
	if promptlist[#promptlist] == "yes" then
		local target = self.room:findPlayer(promptlist[#promptlist - 1])
		if target then
			sgs.updateIntention(player, target, -50)
		end
	end
end

sgs.thzhancao_keep_value = {
	Weapon = 6,
	EquipCard = 5,
	TrickCard = 5,
}

--【遁甲】ai
sgs.ai_skill_invoke.thdunjia = true
sgs.ai_skill_choice.thdunjia = function(self, choices, data)	
	local target = self.player:getTag("ThDunjiaTarget"):toPlayer()
	if not target or not self:isEnemy(target) then return "draw" end
	if choices:match("discard") then
		local x = math.abs(self.player:getEquips():length()- target:getEquips():length())
		if target:getCards("he"):length() >= x then
			return "discard"
		end
		-- 目前无脑拆
		--其实判断最好加入对于敌人装备值得衡量和自身需要过牌的考虑，而不是无脑发动 = =
		--高阶ai aoe时需要队友会卖血
	end
	return "draw"
end
--sgs.ai_skill_cardchosen.thdunjia = function(self, who, flags)
--交给smart-ai的askForCardChosen去选择,应该没有特别要注意的


sgs.ai_skill_invoke.thlingya = true
sgs.ai_skill_choice.thlingya = function(self, choices, data)	
	local yukari = self.player:getTag("ThLingyaSource"):toPlayer()
	if yukari and choices:match("discard") then 
		if self:isFriend(yukari) and self.player:hasSkills(sgs.lose_equip_skill) then
			return "discard"
		elseif self:isEnemy(yukari) then
			local LetDiscard = false
			--高级ai 应该对letdiscard做更详细的评估
			if not yukari:canDiscard(self.player,"h") and self:hasSkills(sgs.lose_equip_skill) then
				LetDiscard  = true
			elseif  self:needKongcheng(p) and self:getHandcardNum()==1  then 
				if not yukari:canDiscard(self.player,"e") or self:hasSkills(sgs.lose_equip_skill) then
					LetDiscard  = true
				end
			end
			if LetDiscard then  return "discard" end
		end
	end
	return "letdraw"
end


--黑幕的存在使得carduse本身就有变化。。。。比如可以故意作死地去决斗敌人。。。反正会转移使用者当一个离间。。。
-- 一般ai使用决斗不会这么做,这个功能需要改usecard的底层ai本身 = =
sgs.ai_skill_playerchosen.thheimu = function(self, targets)
	local cardUse = self.player:getTag("ThHeimuCardUse"):toCardUse()
	local isRed = cardUse.card:isRed()
	
	--case1  灵压敌人
	local goodLingyaCard = "god_salvation|amazing_grace|iron_chain" 
	--|slash|thunder_slash|fire_slash
	local isGoodLingyaCard =  goodLingyaCard:match(cardUse.card:objectName())
	if self.player:hasSkill("thlingya") and isGoodLingyaCard and isRed then
		if #self.enemies > 0 then
			self:sort(self.enemies, "defense")
			return self.enemies[1]
		end
	end
	
	--case2 助队友收反或使主公杀忠掉牌
	local isDamageCard =  sgs.dynamic_value.damage_card[cardUse.card:getClassName()]
	if isDamageCard then
		local lord = self.room:getLord()
		if self:isFriend(lord) then
			local weakRebel 
			for _,p in sgs.qlist(cardUse.to) do
				if p:getHp()<=1 and self:isEnemy(p) then
					weakRebel  = p
					continue
				end
			end
			if weakRebel  then
				for _, p in sgs.qlist(targets) do
					if self:isFriend(p) and p:hasSkills(sgs.cardneed_skill) then
						if (cardUse.card:isKindOf("TrickCard") and self:hasTrickEffective(cardUse.card, weakRebel, p)) 
						or (cardUse.card:isKindOf("Slash") and self:slashIsEffective(cardUse.card, weakRebel, p)) then
							return p
						end
					end
				end
			end
		else
			local weakLoyalist
			for _,p in sgs.qlist(cardUse.to) do
				if p:getHp()<=1 and self:isEnemy(p) then
					weakLoyalist  = p
					continue
				end
			end
			if weakLoyalist then
				for _, p in sgs.qlist(targets) do
					if p:isLord(p) then
						if (cardUse.card:isKindOf("TrickCard") and self:hasTrickEffective(cardUse.card, weakLoyalist, p)) 
						or (cardUse.card:isKindOf("Slash") and self:slashIsEffective(cardUse.card, weakLoyalist, p)) then
							return p
						end
					end
				end
			end
		end
	end
	--case3  一般灵压 针对队友
	if isRed and self.player:hasSkill("thlingya")  then
		for _, p in sgs.qlist(targets) do
			if self:isFriend(p) then
				return p
			end
		end
	end
	return nil
end



--【冬末】ai
sgs.ai_skill_use["@@thdongmo"] = function(self, prompt)
	local targetNames={}
	--need a sort method... 
	for _,p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if (self:isFriend(p) and not p:faceUp()) 
		or (self:isEnemy(p) and  p:faceUp()) then
			table.insert(targetNames,p:objectName())
		end
		if #targetNames>=self.player:getLostHp() then
			break
		end
	end
	if #targetNames ==0 then 
		return "."	 
	elseif #targetNames < self.player:getLostHp() then
		local can=false
		if not self.player:faceUp() then
			can =true
		elseif self.player:getLostHp() -#targetNames<2 then
			can =true
		end
		if can then
			return "@ThDongmoCard=.->" .. table.concat(targetNames, "+")
		end
	else
		return "@ThDongmoCard=.->" .. table.concat(targetNames, "+")
	end
	return "."
end
sgs.ai_card_intention.ThDongmoCard = function(self, card, from, tos)
	for _,to in pairs (tos) do
		if to:faceUp() then
			sgs.updateIntention(from, to, 50)
		else
			sgs.updateIntention(from, to, -50)	
		end
	end
end

--【凛寒】ai
sgs.ai_skill_invoke.thlinhan = true


--【骚葬】ai
--要确保弃牌发动技能，ai都是不用牌不舒服斯基的主
--这个属于修改基础用牌ai 暂时不弄了。。。
sgs.ai_skill_playerchosen.thsaozang = function(self, targets)
	--self:sort(self.enemies, "defense")
	if #self.enemies>0 then
		self:sort(self.enemies, "handcard")
		for _,p in pairs (self.enemies) do
			if not self.player:canDiscard(p, "h") then
				continue
			elseif self:needKongcheng(p) and p:getHandcardNum()==1 then
				continue
			else
				return p
			end
		end
	end
	return nil
end
--【骚葬】的仇恨：比起playerchosen，可能cardchosen更精确
sgs.ai_choicemade_filter.cardChosen.thsaozang = sgs.ai_choicemade_filter.cardChosen.dismantlement

--【絮曲】ai
sgs.ai_skill_playerchosen.thxuqu = function(self, targets)
	local target = self:findPlayerToDraw(false, 1)
	if target then return target end
	return nil
end
sgs.ai_playerchosen_intention.thxuqu = -30

--【苦戒】ai
local thkujiev_skill = {}
thkujiev_skill.name = "thkujiev"
table.insert(sgs.ai_skills, thkujiev_skill)
thkujiev_skill.getTurnUseCard = function(self)
	if self.player:hasFlag("ForbidThKujie") then return nil end
	local reds={}
	for _,c in sgs.qlist(self.player:getCards("he")) do
		if c:isRed() and c:isKindOf("BasicCard") then
			table.insert(reds,c)
		end
	end
	if #reds==0 then return nil end
	self:sortByKeepValue(reds)
	return sgs.Card_Parse("@ThKujieCard=" .. reds[1]:getEffectiveId())
end
sgs.ai_skill_use_func.ThKujieCard = function(card, use, self)
	local targets ={}
	for _,p in sgs.qlist(self.room:findPlayersBySkillName("thkujie")) do
		if  self.player:inMyAttackRange(p) and not p:hasFlag("ThKujieInvoked") then
			table.insert(targets,p)
		end
	end
	if #targets==0 then return nil end
	self:sort(targets, "hp")
	local good_target 
	for _,p in pairs (targets) do
		if self:isEnemy(p) then
			if p:getHp()==1 and self:getAllPeachNum(p)<=0 then
				good_target = p
				break
			end
		elseif self:isFriend(p) then
			if p:isWounded() and p:getHp()>1 then
				good_target = p
				break
			end
		end
	end
	if good_target then
		use.card = card
		if use.to then
			use.to:append(good_target)
			return
		end
	end
end
sgs.ai_card_intention.ThKujieCard = function(self, card, from, tos)
	for _, to in pairs(tos) do
		if to:getHp()<=1 then
			sgs.updateIntention(from, to, 80)
		else
			sgs.updateIntention(from, to, -20)
		end
	end
end

--【廕庇】ai
sgs.ai_skill_invoke.thyinbi = function(self, data)
	local damage = data:toDamage()
	if self:isFriend(damage.to) then
		if damage.to:getLostHp() >= damage.damage and  self.player:getHp() > damage.damage then
			local isSlash 
			if damage.card and damage.card:isKindOf("Slash") then
				isSlash= true
			end
			return not self:needToLoseHp(damage.to, damage.from, isSlash, true)
		end
	end
	return false
end
sgs.ai_choicemade_filter.skillInvoke.thyinbi = function(self, player, promptlist)
	local to=player:getTag("thyinbiDamage"):toDamage().to
	if to and promptlist[#promptlist] == "yes" then
		sgs.updateIntention(player, to, -80)
	end
end




--【灵蝶】ai
local function countKnownCards(target)
		local count=0
		for _, card in sgs.qlist(target:getHandcards()) do
			--flag的情况其实可以不要。。。
			local flag = string.format("%s_%s_%s", "visible", global_room:getCurrent():objectName(), target:objectName())
			if  card:hasFlag("visible") or card:hasFlag(flag) then	
				count=count+1
			end
		end
		return count
	end
local lingdieCompare_func = function(a, b)
	return	countKnownCards(a)>countKnownCards(b)
end
	
local thlingdie_skill = {}
thlingdie_skill.name = "thlingdie"
table.insert(sgs.ai_skills, thlingdie_skill)
thlingdie_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("ThLingdieCard") then return nil end
	if #self.friends_noself ==0 then return nil end
	local cards =sgs.QList2Table(self.player:getCards("he"))
	if #cards==0 then return nil end
	
	--没补牌技能时，防御太虚
	if not self.player:hasSkill("thwushou") and #cards <3 then
	--sgs.getDefense(self.player, gameProcess)
		return nil 
	end

	self:sortByKeepValue(cards)
	return sgs.Card_Parse("@ThLingdieCard=" .. cards[1]:getEffectiveId())
	--目前敌友都可以属于看牌目标，也就不预先检测目标了
end
sgs.ai_skill_use_func.ThLingdieCard = function(card, use, self)

	local good_enemy
	if #self.enemies>0 then
		table.sort(self.enemies, lingdieCompare_func)
		good_enemy=self.enemies[1]
	end
	local good_friend
	if good_enemy then
		for _,p in pairs (self.friends_noself) do
			--考虑急火？ canslash？
			if p:inMyAttackRange(good_enemy)  then
				good_friend =p
				break
			end
		
		end
	end
	use.card = card
	if use.to then
		if good_friend then
			use.to:append(good_friend)
		else
			use.to:append(self.friends_noself[math.random(1,#self.friends_noself)])
		end
		return
	end
end
sgs.ai_card_intention.ThLingdieCard = -50
sgs.ai_skill_playerchosen.thlingdie = function(self, targets)
	if #self.enemies>0 then
		table.sort(self.enemies, lingdieCompare_func)
		good_enemy=self.enemies[1]
	end
	if good_enemy and not good_enemy:isKongcheng() then
		return good_enemy
	end
	return targets:first()
end
sgs.ai_playerchosen_intention.thlingdie =function(self, from, to)
	if not self:isFriend(from,to) then
		sgs.updateIntention(from, to, 20)
	end
end
--灵蝶优先度应该很低。。。

--【无寿】ai
sgs.ai_need_damaged.thwushou = function(self, attacker, player)
	--卖血条件：体力值大于1，且能补3张以上
	if attacker and attacker:hasSkill("ikxuwu") then return false end
	local num = 4 - player:getHp()
	return num >= 2
end

--【浮月】ai
local thfuyuev_skill = {}
thfuyuev_skill.name = "thfuyuev"
table.insert(sgs.ai_skills, thfuyuev_skill)
thfuyuev_skill.getTurnUseCard = function(self)
	if self.player:getKingdom()~="yuki" then return nil end
	if self.player:isKongcheng() or self:needBear()  or self.player:hasFlag("ForbidThFuyue") then return nil end
	return sgs.Card_Parse("@ThFuyueCard=.")
end
sgs.ai_skill_use_func.ThFuyueCard = function(card, use, self)
	local lord
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player:hasLordSkill("thfuyue") and not player:isKongcheng() 
		and not player:hasFlag("ThFuyueInvoked") and player:isWounded()
		then
			lord=player
			break
		end
	end
	if not lord then return nil end
	
	--暂时不考虑反贼获利
	if self:isEnemy(lord)  then
		return nil
	end
	
	
	
	
	local cards = self.player:getHandcards()
	local max_num = 0, max_card
	local min_num = 14, min_card
		for _, hcard in sgs.qlist(cards) do
			if hcard:isKindOf("Peach") then continue end
			if hcard:getNumber() > max_num then
				max_num = hcard:getNumber()
				max_card = hcard
			end

			if hcard:getNumber() <= min_num then
				if hcard:getNumber() == min_num then
					if min_card and self:getKeepValue(hcard) > self:getKeepValue(min_card) then
						min_num = hcard:getNumber()
						min_card = hcard
					end
				else
					min_num = hcard:getNumber()
					min_card = hcard
				end
			end
		end
	if not min_card then return nil end
	
	--很大概率主公赢不了
	if min_card:getNumber()>=12 then return nil end
	if min_card:getNumber()>9 and lord:getHandcardNum()<=4 then
		local lord_card = self:getMaxCard(lord)
		if not lord_card or lord_card:getNumber() < min_card:getNumber() then
			return nil
		end
	end
	if self:isFriend(lord) then
		self.thfuyue_card = min_card:getEffectiveId()
		use.card = card
		if use.to then use.to:append(lord) end
		return
	end
end
--响应拼点者的应对
function sgs.ai_skill_pindian.thfuyue(minusecard, self, requestor, maxcard)
	return self:getMaxCard()
end

sgs.ai_choicemade_filter.pindian.thfuyue = function(self, from, promptlist)
	local number = sgs.Sanguosha:getCard(tonumber(promptlist[4])):getNumber()
	local lord = findPlayerByObjectName(self.room, promptlist[5])
	if not lord then return end
	
	if number < 6 then sgs.updateIntention(from, lord, -60)
	elseif number > 8 and lord:getHandcardNum()<=4 and self:isEnemy(lord,from) then 
	--反贼拼点？
		sgs.updateIntention(from, lord, 60) 
	end
end
