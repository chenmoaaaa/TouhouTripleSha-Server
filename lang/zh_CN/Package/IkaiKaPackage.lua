-- translation for IkaiHiPackage

return {
	["ikai-ka"] = "异世界的火种",
	
--wind
	["#wind025"] = "计测万端",
	["wind025"] = "白",--风 - 空 - 3血
	["ikzhiju"] = "智局",
	[":ikzhiju"] = "出牌阶段限一次，你可以弃置场上的一张牌，若如此做，此回合的弃牌阶段开始时，你须将两张手牌置于牌堆顶。<br />※操作:这两张牌将以与你点击顺序相反的顺序置于牌堆顶",
	["@ikzhiju"] = "请弃置场上的一张牌",
	["ikyingqi"] = "影契",
	[":ikyingqi"] = "一名角色的出牌阶段开始时，若其手牌数不大于体力值，你可以令其摸一张牌。",

	["#wind033"] = "朝阳的游击士",
	["wind033"] = "艾丝蒂尔•布莱特",--风 - 空 - 4血
	["ikjilun"] = "极轮",
	[":ikjilun"] = "出牌阶段限一次，你可弃置一张手牌并指定一名装备区内有牌的其他角色，你令该角色获得其装备区的一张牌，然后视为你对其使用一张无视距离的【杀】（此【杀】不计入每阶段的使用限制）。",

	["#wind034"] = "人型电脑天使心",
	["wind034"] = "艾露妲",--风 - 空 - 4血
	["ikjiqiao"] = "机巧",
	[":ikjiqiao"] = "摸牌阶段结束时，若你的手牌数小于体力上限，你可以弃置一张牌，若弃置的牌的种类为：<br />1.基本牌，视为你对一名其他角色使用一张无视距离的【杀】<br />2.装备牌，你令一名其他角色摸两张牌<br />3.锦囊牌，你令一名其他角色回复1点体力",
	["@ikjiqiao"] = "你可以发动“机巧”",
	["@ikjiqiao-basic"] = "请选择一名其他角色，视为对其使用一张【杀】",
	["@ikjiqiao-equip"] = "请选择一名其他角色，令其摸两张牌",
	["@ikjiqiao-trick"] = "请选择一名其他角色，令其回复1点体力",

	["#wind035"] = "赤珠之锁",
	["wind035"] = "摩尔迦娜",--风 - 空 - 4血
	["ikkangjin"] = "亢劲",
	[":ikkangjin"] = "出牌阶段，你可以将一张手牌交给一名其他角色（须与你此阶段上一次发动“亢劲”交给一名其他角色的牌颜色不同），视为你对其使用一张【碎月绮幕】；受到该【碎月绮幕】造成的伤害的角色摸等同于其已损失体力值数量的牌。",
	["#ikkangjin"] = "亢劲（摸牌）",

	["#wind036"] = "蠕动之混沌",
	["wind036"] = "奈亚拉托提普",--风 - 空 - 4血
	["ikhunkao"] = "魂铐",
	[":ikhunkao"] = "出牌阶段限两次，你可以展示全部的手牌，并弃置其中一种花色全部的牌，然后你指定至多等同于弃置的牌的数量的其他角色，并令她们依次选择一项：交给你一张该花色的牌；或视为你对其使用一张无视距离的【杀】。",

	["#wind045"] = "讨魔的暗杀者",
	["wind045"] = "雪风•帕尼托尼",--风 - 空 - 4血
	["ikhudie"] = "狐谍",
	[":ikhudie"] = "每当你受到一次伤害时，若你与伤害来源的势力属性不同，你可以将之改变为与伤害来源相同。",
	["ikyinsha"] = "隐杀",
	[":ikyinsha"] = "锁定技，当你计算与势力属性相同的角色距离时，无视除该角色外的其他角色及场上的坐骑牌。",
	["ikhualan"] = "花岚",
	[":ikhualan"] = "每当你对势力属性不同的角色造成一次伤害后，你可以摸一张牌，或将势力属性改变为风势力。",
	["ikhualan:draw"] = "摸一张牌",
	["ikhualan:change"] = "将势力属性改变为风势力",

--bloom
	["#bloom032"] = "灭牙的元神灵",
	["bloom032"] = "狮子神黑",--花 - 空 - 4血
	["ikfengxing"] = "风行",
	[":ikfengxing"] = "准备阶段开始时，你可以展示全部的手牌，若其中没有【杀】，你摸一张牌。你可以重复此流程，直到你展示的牌中有【杀】为止。",

	["#bloom034"] = "乐土的和声",
	["bloom034"] = "莉特丝•特尔提娜＆萨露莎•特尔提娜",--花 - 空 - 4血
	["ikqizhong"] = "祈钟",
	[":ikqizhong"] = "当你于出牌阶段使用牌时，若此牌与你此阶段使用的上一张牌颜色不同，你可以亮出牌堆顶的一张牌，若亮出的牌与你使用的牌颜色不同，你获得之；否则，你可以用一张手牌替换之，或将该牌置入弃牌堆。",
	["@ikqizhong-exchange"] = "你可以用一张手牌替换亮出的【%arg】",

	["#bloom035"] = "天光的勇者",
	["bloom035"] = "游佐恵美",--花 - 空 - 4血
	["ikduduan"] = "独断",
	[":ikduduan"] = "准备阶段开始时，你可以摸一张牌或弃置你或你攻击范围内的一名角色的一张牌。若如此做，且你于此回合的出牌阶段没有造成伤害，你的手牌上限-2，直到回合结束。",
	["@ikduduan"] = "你发动了“独断”，可以弃置你或你攻击范围内的一名角色的一张牌，或点“取消”摸一张牌",

	["#bloom036"] = "弁天号舰长",
	["bloom036"] = "加藤茉莉香",--花 - 空 - 4血
	["ikpaomu"] = "炮幕",
	[":ikpaomu"] = "限定技，出牌阶段，你可以令一名其他角色获得一枚“猎标”标记。你计算与拥有“猎标”标记的角色的距离时无视除该角色外的其他角色及场上的坐骑牌；你对其使用黑色【杀】在结算后，可以令所有攻击范围内有该角色的其他角色依次选择一项：对其使用一张【杀】，或令你摸一张牌。该角色死亡时，你将场上的“猎标”标记转移给一名其他角色。",
	["@liebiao"] = "猎标",
	["@ikpaomu"] = "你可以发动“炮幕”",
	["@ikpaomu-slash"] = "受“炮幕”影响，请对 %dest 使用一张【杀】，或点“取消”令 %src 摸一张牌",

	["#bloom045"] = "鸣露的策士",
	["bloom045"] = "新子憧",--花 - 幻 - 4血
	["ikdengpo"] = "登破",
	[":ikdengpo"] = "出牌阶段限一次，你可以弃置X张牌并令一名牌数不少于X的其他角色弃置等量的牌（至多弃置四张），若如此做，此阶段结束时，你与其各摸X张牌。若你以此法弃置了你装备区的最后一张牌，你无视与该角色的距离，直到回合结束。",

--snow
	["#snow031"] = "异国的公主",
	["snow031"] = "梅露露琳丝•蕾蒂•阿鲁兹",--雪 - 空 - 3血
	["iklingyun"] = "灵运",
	[":iklingyun"] = "你使用的基本牌在结算后置入弃牌堆时，你可以将之置于牌堆顶。",
	["ikmiyao"] = "秘药",
	[":ikmiyao"] = "一名其他角色的回合结束时，你可以将手牌补至或弃置至等同于其手牌数的张数。若你以此法获得了两张或更多的牌，你失去1点体力；若你以此法弃置了两张或更多的牌，你回复1点体力。",
	
	["#snow034"] = "恶趣的女仆",
	["snow034"] = "汐王寺茉莉花",--雪 - 空 - 4血
	["ikshidao"] = "侍道",
	[":ikshidao"] = "当你需要使用一张基本牌时，你可以将两张不同类别的牌交给一名其他角色，视为你使用了一张该名称的基本牌。<br />※操作提示：给牌的目标在点击“确定”后才选择，即“确定”确定前只需要选择此基本牌的目标",
	["ikshidao_saveself"] = "侍道自救",
	["ikshidao_slash"] = "侍道出杀",
	["@ikshidao"] = "请将这两张牌交给一名其他角色",
	
	["#snow035"] = "绮语的采女",
	["snow035"] = "琴羽",--雪 - 空 - 4血
	["iktaoxiao"] = "桃枭",
	[":iktaoxiao"] = "出牌阶段，你可以将一张【桃】当【神惠雨降】使用。",
	["ikyushenyu"] = "愈神",
	[":ikyushenyu"] = "在你的回合，每当一名角色回复1点体力后，你可以摸一张牌或弃置该角色的一张牌。",
	["ikyushenyu:draw"] = "摸一张牌",
	["ikyushenyu:discard"] = "弃置该角色的一张牌",
	
	["#snow036"] = "天赐的艺术家",
	["snow036"] = "椎名真白",--雪 - 空 - 3血
	--ikmitu
	["iklinghui"] = "灵绘",
	[":iklinghui"] = "出牌阶段，你可以令一名其他角色摸一张牌，然后其须亮出一张手牌，你可以弃置任意张颜色相同的手牌，并令等量的角色依次摸两张牌。",

	["#snow045"] = "电子的妖精",
	["snow045"] = "星野琉璃",--雪 - 空 - 4血
	["ikhuaiji"] = "怀计",
	[":ikhuaiji"] = "一名角色的出牌阶段结束时，若此阶段进入弃牌堆的牌数大于你的体力值，你可以将牌堆顶的三张牌置入弃牌堆，若其中有红桃牌，你可以令一名角色回复1点体力。",
	["ikdianyan"] = "电衍",
	[":ikdianyan"] = "出牌阶段限一次，你可以将一张手牌面朝上置于牌堆底。一名角色获得该牌时，你可以令其摸两张牌；或失去1点体力。",

--luna
	["#luna030"] = "病娇的白蔷薇",
	["luna030"] = "雪华绮晶",--月 - 空 - 4血
	["iklingcu"] = "凌簇",
	[":iklingcu"] = "你可以跳过出牌阶段并将你的人物牌翻面，然后进行一次判定，判定牌生效后将其置于你的人物牌上。你须重复从判定开始的流程，直到你的人物牌上出现花色相同的牌为止，然后你的人物牌上每有一张牌，视为你对你攻击范围内的一名角色使用一张【杀】，在结算后，将你的人物牌上的全部的牌置入弃牌堆。",

	["#luna031"] = "灵心的炼金师",
	["luna031"] = "托托莉雅•赫尔默德",--月 - 空 - 3血
	["ikqisi"] = "奇思",
	[":ikqisi"] = "当一名手牌数不小于你的其他角色使用一张非延时类锦囊牌生效前，若你的手牌数为奇数，你可以摸一张牌，若如此做，视为你使用了一张【三滴天粒】。",
	["ikmiaoxiang"] = "妙想",
	[":ikmiaoxiang"] = "当一名手牌数不大于你的其他角色受到一次伤害后，若你的手牌数为偶数，你可以弃置一张非基本牌，然后令其回复1点体力。",

	["#luna032"] = "星咏的歌姬",
	["luna032"] = "初音未来",--月 - 空 - 3血
	["ikjichang"] = "激唱",
	[":ikjichang"] = "准备阶段开始时，若你没有手牌，你可以摸四张牌；否则你可以展示你全部的手牌，每少一种花色，你摸一张牌。若你于本阶段获得了两张或更多的牌，跳过你的判定阶段和摸牌阶段。",
	["ikmanwu"] = "曼舞",
	[":ikmanwu"] = "出牌阶段限一次，你可以与一名其他角色拼点。拼点赢的角色将手牌补至等同于其体力值的张数。",

	["#luna033"] = "赤猫的琴手",
	["luna033"] = "中野梓",--月 - 空 - 4血
	["ikxianlv"] = "弦律",
	[":ikxianlv"] = "准备阶段或结束阶段开始时，你可以进行一次判定：若结果与你人物牌上任意一张牌花色均不同，你须将判定牌置于你的人物牌上，称为“乐”。一名角色的摸牌阶段开始时，你可以选择任意张“乐”并令其获得之，然后你摸一张牌，若如此做，该角色摸牌时须少摸一张牌。",

	["#luna036"] = "神降的姬样",
	["luna036"] = "神代小莳",--月 - 空 - 4血
	["iktianwu"] = "天巫",
	[":iktianwu"] = "准备阶段开始时，你可以失去1点体力或弃置一张装备牌，然后弃置一名其他角色的一张牌。若此牌为锦囊牌，你可以令至多X名角色摸一张牌；若此牌为基本牌，你计算与其他角色的距离时，始终-X，直到回合结束；若此牌为装备牌，你于此回合的出牌阶段可额外使用X张【杀】（X为你已损失的体力值，且至少为1）。",

	["#luna037"] = "天仪的孤风",
	["luna037"] = "岛风",--月 - 空 - 4血
	["ikmoshan"] = "魔闪",
	[":ikmoshan"] = "锁定技，你的【闪】均视为【净琉璃镜】。每当你即将失去装备区的【净琉璃镜】时或失去技能“魔闪”时，须将装备区的防具牌交给一名其他角色，然后弃置一名其他角色的一张牌。",

	["#luna045"] = "赤色之绊",
	["luna045"] = "千堂瑛里华",--月 - 空 - 4血
	["ikxieke"] = "血刻",
	[":ikxieke"] = "出牌阶段，若你的人物牌竖置，你可视为你使用一张基本牌或【三滴天粒】，然后你将人物牌横置。",
	["ikyunmai"] = "运脉",
	[":ikyunmai"] = "回合结束时，若人物牌横置的角色数不少于X，你可以重置一名角色的人物牌；若人物牌竖置的角色数不少于X，你可以横置一名角色的人物牌（X为存活角色的数量的一半）。",
}