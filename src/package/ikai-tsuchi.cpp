#include "ikai-tsuchi.h"

#include "general.h"
#include "skill.h"
#include "standard.h"
#include "engine.h"
#include "client.h"
#include "maneuvering.h"

IkShenaiCard::IkShenaiCard() {
    will_throw = false;
    handling_method = Card::MethodNone;
}

void IkShenaiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();
    int num = 0;
    QList<int> ikshenai_list = source->handCards();
    foreach (int id, getSubcards())
        ikshenai_list.removeOne(id);

    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, source->objectName(), target->objectName(), "ikshenai", QString());
    room->obtainCard(target, this, reason, false);

    num += subcardsLength();

    if (!ikshenai_list.isEmpty() && num < 3) {
        int record = ikshenai_list.length();
        QList<ServerPlayer *> targets = room->getOtherPlayers(source);
        targets.removeOne(target);
        if (!targets.isEmpty())
            room->askForYiji(source, ikshenai_list, "ikshenai", false, false, true, 3 - num,
                             targets, reason, "@ikshenai:" + target->objectName(), false);
        record -= ikshenai_list.length();
        num += record;
    }

    if (num >= 2)
        room->recover(source, RecoverStruct(source));
}

class IkShenai: public ViewAsSkill {
public:
    IkShenai(): ViewAsSkill("ikshenai") {
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        if (selected.length() >= 3)
           return false;
        else
            return !to_select->isEquipped();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("IkShenaiCard") && !player->isKongcheng();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.isEmpty())
            return NULL;

        IkShenaiCard *ikshenai_card = new IkShenaiCard;
        ikshenai_card->addSubcards(cards);
        return ikshenai_card;
    }
};

IkXinqiCard::IkXinqiCard() {
}

bool IkXinqiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    Slash *slash = new Slash(NoSuit, 0);
    slash->deleteLater();
    return slash->targetFilter(targets, to_select, Self);
}

const Card *IkXinqiCard::validate(CardUseStruct &cardUse) const{
    cardUse.m_isOwnerUse = false;
    ServerPlayer *liubei = cardUse.from;
    QList<ServerPlayer *> targets = cardUse.to;
    Room *room = liubei->getRoom();
    liubei->broadcastSkillInvoke(this);
    room->notifySkillInvoked(liubei, "ikxinqi");

    LogMessage log;
    log.from = liubei;
    log.to = targets;
    log.type = "#UseCard";
    log.card_str = toString();
    room->sendLog(log);

    const Card *slash = NULL;

    QList<ServerPlayer *> lieges = room->getLieges("kaze", liubei);
    foreach (ServerPlayer *target, targets)
        target->setFlags("IkXinqiTarget");
    foreach (ServerPlayer *liege, lieges) {
        try {
            slash = room->askForCard(liege, "slash", "@ikxinqi-slash:" + liubei->objectName(),
                                     QVariant(), Card::MethodResponse, liubei, false, QString(), true);
        }
        catch (TriggerEvent triggerEvent) {
            if (triggerEvent == TurnBroken || triggerEvent == StageChange) {
                foreach (ServerPlayer *target, targets)
                    target->setFlags("-IkXinqiTarget");
            }
            throw triggerEvent;
        }

        if (slash) {
            foreach (ServerPlayer *target, targets)
                target->setFlags("-IkXinqiTarget");

            foreach (ServerPlayer *target, targets) {
                if (!liubei->canSlash(target, slash))
                    cardUse.to.removeOne(target);
            }
            if (cardUse.to.length() > 0)
                return slash;
            else {
                delete slash;
                return NULL;
            }
        }
    }
    foreach (ServerPlayer *target, targets)
        target->setFlags("-IkXinqiTarget");
    room->setPlayerFlag(liubei, "Global_IkXinqiFailed");
    return NULL;
}

IkXinqiViewAsSkill::IkXinqiViewAsSkill(): ZeroCardViewAsSkill("ikxinqi$") {
}

bool IkXinqiViewAsSkill::isEnabledAtPlay(const Player *player) const{
    return hasKazeGenerals(player) && !player->hasFlag("Global_IkXinqiFailed") && Slash::IsAvailable(player);
}

bool IkXinqiViewAsSkill::isEnabledAtResponse(const Player *player, const QString &pattern) const{
    return hasKazeGenerals(player)
           && (pattern == "slash" || pattern == "@ikxinqi")
           && Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE
           && !player->hasFlag("Global_IkXinqiFailed");
}

const Card *IkXinqiViewAsSkill::viewAs() const{
    return new IkXinqiCard;
}

bool IkXinqiViewAsSkill::hasKazeGenerals(const Player *player) {
    foreach (const Player *p, player->getAliveSiblings())
        if (p->getKingdom() == "kaze")
            return true;
    return false;
}

class IkXinqi: public TriggerSkill {
public:
    IkXinqi(): TriggerSkill("ikxinqi$") {
        events << CardAsked;
        view_as_skill = new IkXinqiViewAsSkill;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        if (player && player->isAlive() && player->hasLordSkill("ikxinqi")) {
            QString pattern = data.toStringList().first();
            QString prompt = data.toStringList().at(1);
            if (pattern != "slash" || prompt.startsWith("@ikxinqi-slash"))
                return QStringList();
            QList<ServerPlayer *> lieges = room->getLieges("kaze", player);
            if (lieges.isEmpty())
                return QStringList();
            return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *liubei, QVariant &data, ServerPlayer *) const{
        if (liubei->askForSkillInvoke(objectName(), data)) {
            room->broadcastSkillInvoke(objectName(), getEffectIndex(liubei, NULL));
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *liubei, QVariant &data, ServerPlayer *) const{
        QList<ServerPlayer *> lieges = room->getLieges("kaze", liubei);
        foreach (ServerPlayer *liege, lieges) {
            const Card *slash = room->askForCard(liege, "slash", "@ikxinqi-slash:" + liubei->objectName(),
                                                 QVariant(), Card::MethodResponse, liubei, false, QString(), true);
            if (slash) {
                room->provide(slash);
                return true;
            }
        }
        return false;
    }
};

class IkChilian: public OneCardViewAsSkill {
public:
    IkChilian(): OneCardViewAsSkill("ikchilian") {
        response_or_use = true;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "slash";
    }

    virtual bool viewFilter(const Card *card) const{
        if (!card->isRed())
            return false;

        if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_PLAY) {
            Slash *slash = new Slash(Card::SuitToBeDecided, -1);
            slash->addSubcard(card->getEffectiveId());
            slash->deleteLater();
            return slash->isAvailable(Self);
        }
        return true;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Card *slash = new Slash(originalCard->getSuit(), originalCard->getNumber());
        slash->addSubcard(originalCard->getId());
        slash->setSkillName(objectName());
        return slash;
    }
};

class IkZhenhong: public TriggerSkill {
public:
    IkZhenhong(): TriggerSkill("ikzhenhong") {
        events << TargetSpecified;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (TriggerSkill::triggerable(player) && use.card->isKindOf("Slash") && use.card->getSuit() == Card::Diamond)
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = player;
        log.arg = objectName();
        room->sendLog(log);
        room->notifySkillInvoked(player, objectName());
        room->broadcastSkillInvoke(objectName());

        CardUseStruct use = data.value<CardUseStruct>();
        foreach (ServerPlayer *p, use.to.toSet())
            p->addQinggangTag(use.card);
        return false;
    }
};

class IkZhenhongTargetMod: public TargetModSkill {
public:
    IkZhenhongTargetMod(): TargetModSkill("#ikzhenhong-target") {
    }

    virtual int getDistanceLimit(const Player *from, const Card *card) const{
        if (from->hasSkill("ikzhenhong") && card->getSuit() == Card::Heart)
            return 1000;
        else
            return 0;
    }
};

class IkLipao: public TargetModSkill {
public:
    IkLipao(): TargetModSkill("iklipao") {
    }

    virtual int getResidueNum(const Player *from, const Card *) const{
        if (from->hasSkill(objectName()))
            return 1000;
        else
            return 0;
    }
};

class IkJiukuang: public OneCardViewAsSkill {
public:
    IkJiukuang(): OneCardViewAsSkill("ikjiukuang") {
        response_or_use = true;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Analeptic::IsAvailable(player) && player->getPhase() == Player::Play;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern.contains("analeptic") && player->getPhase() == Player::Play;
    }

    virtual bool viewFilter(const Card *card) const{
        if (!(card->isNDTrick() && card->isBlack()) && !card->isKindOf("Weapon"))
            return false;

        if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_PLAY) {
            Analeptic *anal = new Analeptic(Card::SuitToBeDecided, -1);
            anal->addSubcard(card->getEffectiveId());
            anal->deleteLater();
            return anal->isAvailable(Self);
        }
        return true;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Analeptic *anal = new Analeptic(originalCard->getSuit(), originalCard->getNumber());
        anal->addSubcard(originalCard->getId());
        anal->setSkillName(objectName());
        return anal;
    }
};

class IkYuxi: public PhaseChangeSkill {
public:
    IkYuxi(): PhaseChangeSkill("ikyuxi") {
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *player) const {
        return TriggerSkill::triggerable(player)
            && player->getPhase() == Player::Start;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        if (player->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *zhuge) const {
        Room *room = zhuge->getRoom();
        QList<int> guanxing = room->getNCards(getIkYuxiNum(room));

        LogMessage log;
        log.type = "$ViewDrawPile";
        log.from = zhuge;
        log.card_str = IntList2StringList(guanxing).join("+");
        room->sendLog(log, zhuge);

        room->askForGuanxing(zhuge, guanxing);

        return false;
    }

    virtual int getIkYuxiNum(Room *room) const{
        return qMin(5, room->alivePlayerCount());
    }
};

class IkJingyou: public ProhibitSkill {
public:
    IkJingyou(): ProhibitSkill("ikjingyou") {
    }

    virtual bool isProhibited(const Player *, const Player *to, const Card *card, const QList<const Player *> &) const{
        return to->hasSkill(objectName()) && (card->isKindOf("Slash") || card->isKindOf("Duel")) && to->isKongcheng();
    }
};

class IkYufeng: public TriggerSkill {
public:
    IkYufeng(): TriggerSkill("ikyufeng") {
        events << TargetSpecified;
    }
    
    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(player)) return QStringList();
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->isKindOf("Slash")) return QStringList(objectName());
        return QStringList();
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        CardUseStruct use = data.value<CardUseStruct>();
        QVariantList jink_list = player->tag["Jink_" + use.card->toString()].toList();
        int index = 0;
        QList<ServerPlayer *> tos;
        foreach (ServerPlayer *p, use.to) {
            if (!player->isAlive()) break;
            if (player->askForSkillInvoke(objectName(), QVariant::fromValue(p))) {
                room->broadcastSkillInvoke(objectName());
                if (!tos.contains(p)) {
                    p->addMark("ikyufeng");
                    room->addPlayerMark(p, "@skill_invalidity");
                    tos << p;

                    foreach (ServerPlayer *pl, room->getAllPlayers())
                        room->filterCards(pl, pl->getCards("he"), true);
                    Json::Value args;
                    args[0] = QSanProtocol::S_GAME_EVENT_UPDATE_SKILL;
                    room->doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);
                }

                JudgeStruct judge;
                judge.pattern = ".";
                judge.good = true;
                judge.reason = objectName();
                judge.who = player;
                judge.play_animation = false;

                room->judge(judge);

                if (p->isAlive() && !p->canDiscard(p, "he")
                    || !room->askForCard(p, ".|" + judge.pattern, "@ikyufeng-discard:::" + judge.pattern, data, Card::MethodDiscard)) {
                    LogMessage log;
                    log.type = "#NoJink";
                    log.from = p;
                    room->sendLog(log);
                    jink_list.replace(index, QVariant(0));
                }
            }
            index++;
        }
        player->tag["Jink_" + use.card->toString()] = QVariant::fromValue(jink_list);
        return false;
    }
};

class IkYufengClear: public TriggerSkill {
public:
    IkYufengClear(): TriggerSkill("#ikyufeng-clear") {
        events << EventPhaseChanging << Death << FinishJudge;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *target, QVariant &data, ServerPlayer* &) const{
        if (triggerEvent == FinishJudge) {
            JudgeStruct *judge = data.value<JudgeStruct *>();
            if (judge->reason == "ikyufeng")
                judge->pattern = judge->card->getSuitString();
            return QStringList();
        } else if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to != Player::NotActive)
                return QStringList();
        } else if (triggerEvent == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who != target || target != room->getCurrent())
                return QStringList();
        }
        QList<ServerPlayer *> players = room->getAllPlayers();
        foreach (ServerPlayer *player, players) {
            if (player->getMark("ikyufeng") == 0) continue;
            room->removePlayerMark(player, "@skill_invalidity", player->getMark("ikyufeng"));
            player->setMark("ikyufeng", 0);

            foreach (ServerPlayer *p, room->getAllPlayers())
                room->filterCards(p, p->getCards("he"), false);
            Json::Value args;
            args[0] = QSanProtocol::S_GAME_EVENT_UPDATE_SKILL;
            room->doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);
        }
        return QStringList();
    }
};

class NonCompulsoryInvalidity: public InvaliditySkill {
public:
    NonCompulsoryInvalidity(): InvaliditySkill("#non-compulsory-invalidity") {
    }

    virtual bool isSkillValid(const Player *player, const Skill *skill) const{
        return player->getMark("@skill_invalidity") == 0 || skill->getFrequency() == Skill::Compulsory;
    }
};

class IkHuiquan: public TriggerSkill {
public:
    IkHuiquan(): TriggerSkill("ikhuiquan") {
        frequency = Frequent;
        events << CardUsed;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *yueying, QVariant &data, ServerPlayer* &) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (TriggerSkill::triggerable(yueying) && use.card->isKindOf("TrickCard"))
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *yueying, QVariant &, ServerPlayer *) const{
        if (yueying->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *yueying, QVariant &, ServerPlayer *) const{
        yueying->drawCards(1, objectName());
        return false;
    }
};

class IkJiaoman: public MasochismSkill {
public:
    IkJiaoman(): MasochismSkill("ikjiaoman") {
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        DamageStruct damage = data.value<DamageStruct>();
        const Card *card = damage.card;
        if (card) {
            QList<int> ids;
            if (card->isVirtualCard())
                ids = card->getSubcards();
            else
                ids << card->getEffectiveId();
            if (ids.length() > 0) {
                bool all_place_table = true;
                foreach (int id, ids) {
                    if (room->getCardPlace(id) != Player::PlaceTable) {
                        all_place_table = false;
                        break;
                    }
                }
                if (all_place_table) return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        DamageStruct damage = data.value<DamageStruct>();
        QList<ServerPlayer *> targets = damage.from ? room->getOtherPlayers(damage.from) : room->getAlivePlayers();
        ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName(), "@ikjiaoman", true, true);
        if (target) {
            room->broadcastSkillInvoke(objectName());
            player->tag["IkJiaomanTarget"] = QVariant::fromValue(target);
            return true;
        }
        return false;
    }

    virtual void onDamaged(ServerPlayer *caocao, const DamageStruct &damage) const{
        ServerPlayer *target = caocao->tag["IkJiaomanTarget"].value<ServerPlayer *>();
        caocao->tag.remove("IkJiaomanTarget");
        if (target)
            target->obtainCard(damage.card);
    }
};

class IkHuanwei: public TriggerSkill {
public:
    IkHuanwei(): TriggerSkill("ikhuanwei$") {
        events << CardAsked;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        if (player && player->isAlive() && player->hasLordSkill(objectName())) {
            QString pattern = data.toStringList().first();
            QString prompt = data.toStringList().at(1);
            if (pattern != "jink" || prompt.startsWith("@ikhuanwei-jink"))
                return QStringList();

            QList<ServerPlayer *> lieges = room->getLieges("hana", player);
            if (lieges.isEmpty())
                return QStringList();

            return QStringList(objectName());
        }

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        if (player->askForSkillInvoke(objectName(), data)) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        QVariant tohelp = QVariant::fromValue(player);
        foreach (ServerPlayer *liege, room->getLieges("hana", player)) {
            const Card *jink = room->askForCard(liege, "jink", "@ikhuanwei-jink:" + player->objectName(),
                                                tohelp, Card::MethodResponse, player, false, QString(), true);
            if (jink) {
                room->provide(jink);
                return true;
            }
        }

        return false;
    }
};

class IkTiansuo: public TriggerSkill {
public:
    IkTiansuo(): TriggerSkill("iktiansuo") {
        events << AskForRetrial;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return TriggerSkill::triggerable(player)
            && !player->isKongcheng();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        JudgeStruct *judge = data.value<JudgeStruct *>();

        QStringList prompt_list;
        prompt_list << "@iktiansuo-card" << judge->who->objectName()
                    << objectName() << judge->reason << QString::number(judge->card->getEffectiveId());
        QString prompt = prompt_list.join(":");
        const Card *card = room->askForCard(player, "." , prompt, data, Card::MethodResponse, judge->who, true);
        if (card) {
            player->tag["IkTiansuoCard"] = QVariant::fromValue(card);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        const Card *card = player->tag["IkTiansuoCard"].value<const Card *>();
        player->tag.remove("IkTiansuoCard");
        if (card) {
            if (player->hasInnateSkill("iktiansuo") || !player->hasSkill("jilve"))
                room->broadcastSkillInvoke(objectName());
            else
                room->broadcastSkillInvoke("jilve", 1);
            room->retrial(card, player, data.value<JudgeStruct *>(), objectName());
        }

        return false;
    }
};

class IkHuanji: public MasochismSkill {
public:
    IkHuanji(): MasochismSkill("ikhuanji") {
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        QStringList skill;
        if (!TriggerSkill::triggerable(player)) return skill;
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.from && !damage.from->isAllNude())
            skill << objectName();
        return skill;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (player->askForSkillInvoke(objectName(), QVariant::fromValue(damage.from))) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual void onDamaged(ServerPlayer *simayi, const DamageStruct &damage) const{
        int card_id = simayi->getRoom()->askForCardChosen(simayi, damage.from, "hej", "ikhuanji");
        simayi->obtainCard(Sanguosha->getCard(card_id), false);
    }
};

class IkAoli: public TriggerSkill {
public:
    IkAoli(): TriggerSkill("ikaoli") {
        events << Damaged << FinishJudge;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *xiahou, QVariant &data, ServerPlayer* &) const{
        QStringList skill;
        if (triggerEvent == FinishJudge) {
            JudgeStruct *judge = data.value<JudgeStruct *>();
            if (judge->reason != objectName()) return skill;
            judge->pattern = QString::number(int(judge->card->getSuit()));
            return skill;
        }
        if (!TriggerSkill::triggerable(xiahou)) return skill;
        DamageStruct damage = data.value<DamageStruct>();
        for (int i = 0; i < damage.damage; i++)
            skill << objectName();
        return skill;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *xiahou, QVariant &data, ServerPlayer *) const{
        if (xiahou->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *xiahou, QVariant &data, ServerPlayer *) const{
        JudgeStruct judge;
        judge.pattern = ".";
        judge.play_animation = false;
        judge.reason = objectName();
        judge.who = xiahou;

        room->judge(judge);
        DamageStruct damage = data.value<DamageStruct>();
        
        if (!damage.from || damage.from->isDead()) return false;
        Card::Suit suit = (Card::Suit)(judge.pattern.toInt());
        switch (suit) {
        case Card::Heart:
        case Card::Diamond: {
                room->damage(DamageStruct(objectName(), xiahou, damage.from));
                break;
            }
        case Card::Club:
        case Card::Spade: {
                if (xiahou->canDiscard(damage.from, "he")) {
                    int id = room->askForCardChosen(xiahou, damage.from, "he", objectName(), false, Card::MethodDiscard);
                    room->throwCard(id, damage.from, xiahou);
                }
                break;
            }
        default:
                break;
        }

        return false;
    }
};

class IkQingjian: public TriggerSkill {
public:
    IkQingjian(): TriggerSkill("ikqingjian") {
        events << CardsMoveOneTime;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (!room->getTag("FirstRound").toBool() && player->getPhase() != Player::Draw
            && move.to == player && move.to_place == Player::PlaceHand) {
            QList<int> ids;
            foreach (int id, move.card_ids)
                if (room->getCardOwner(id) == player && room->getCardPlace(id) == Player::PlaceHand)
                    return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (!room->getTag("FirstRound").toBool() && player->getPhase() != Player::Draw
            && move.to == player && move.to_place == Player::PlaceHand) {
            QList<int> ids;
            foreach (int id, move.card_ids) {
                if (room->getCardOwner(id) == player && room->getCardPlace(id) == Player::PlaceHand)
                    ids << id;
            }
            if (room->askForYiji(player, ids, objectName(), false, true, true, -1,
                                 QList<ServerPlayer *>(), CardMoveReason(), "@ikqingjian-distribute", true))
                return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        QList<int> ids;
        foreach (int id, move.card_ids) {
            if (room->getCardOwner(id) == player && room->getCardPlace(id) == Player::PlaceHand)
                ids << id;
        }
        if (ids.isEmpty())
            return false;
        while (room->askForYiji(player, ids, objectName(), false, false, true, -1,
                                QList<ServerPlayer *>(), CardMoveReason(), "@ikqingjian-distribute", false)) {
            room->notifySkillInvoked(player, objectName());
            if (player->isDead()) return false;
        }

        return false;
    }
};

IkLianbaoCard::IkLianbaoCard() {
}

bool IkLianbaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (targets.length() >= Self->getMark("iklianbao") || to_select->getHandcardNum() < Self->getHandcardNum() || to_select == Self)
        return false;

    return !to_select->isKongcheng();
}

void IkLianbaoCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->setFlags("IkLianbaoTarget");
}

class IkLianbaoViewAsSkill: public ZeroCardViewAsSkill {
public:
    IkLianbaoViewAsSkill(): ZeroCardViewAsSkill("iklianbao") {
        response_pattern = "@@iklianbao";
    }

    virtual const Card *viewAs() const{
        return new IkLianbaoCard;
    }
};

class IkLianbao: public DrawCardsSkill {
public:
    IkLianbao(): DrawCardsSkill("iklianbao") {
        view_as_skill = new IkLianbaoViewAsSkill;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *zhangliao, QVariant &data, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(zhangliao)) return QStringList();
        QList<ServerPlayer *> targets;
        foreach (ServerPlayer *p, room->getOtherPlayers(zhangliao))
            if (p->getHandcardNum() >= zhangliao->getHandcardNum())
                targets << p;
        int num = qMin(targets.length(), data.toInt());
        foreach (ServerPlayer *p, room->getOtherPlayers(zhangliao))
            p->setFlags("-IkLianbaoTarget");
        if (num > 0)
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *zhangliao, QVariant &data, ServerPlayer *) const{
        QList<ServerPlayer *> targets;
        foreach (ServerPlayer *p, room->getOtherPlayers(zhangliao))
            if (p->getHandcardNum() >= zhangliao->getHandcardNum())
                targets << p;
        int num = qMin(targets.length(), data.toInt());
        room->setPlayerMark(zhangliao, "iklianbao", num);
        if (room->askForUseCard(zhangliao, "@@iklianbao", "@iklianbao-card:::" + QString::number(num))) {
            room->broadcastSkillInvoke(objectName());
            return true;
        } else
            room->setPlayerMark(zhangliao, "iklianbao", 0);
        return false;
    }

    virtual int getDrawNum(ServerPlayer *zhangliao, int n) const{
        Room *room = zhangliao->getRoom();
        int count = 0;
        foreach (ServerPlayer *p, room->getOtherPlayers(zhangliao))
            if (p->hasFlag("IkLianbaoTarget")) count++;
        
        return n - count;
    }
};

class IkLianbaoAct: public TriggerSkill {
public:
    IkLianbaoAct(): TriggerSkill("#iklianbao") {
        events << AfterDrawNCards;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *zhangliao, QVariant &, ServerPlayer* &) const{
        if (zhangliao->getMark("iklianbao") == 0) return QStringList();
        return QStringList(objectName());
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *zhangliao, QVariant &, ServerPlayer *) const{
        room->setPlayerMark(zhangliao, "iklianbao", 0);

        QList<ServerPlayer *> targets;
        foreach (ServerPlayer *p, room->getOtherPlayers(zhangliao)) {
            if (p->hasFlag("IkLianbaoTarget")) {
                p->setFlags("-IkLianbaoTarget");
                targets << p;
            }
        }
        foreach (ServerPlayer *p, targets) {
            if (!zhangliao->isAlive())
                break;
            if (p->isAlive() && !p->isKongcheng()) {
                int card_id = room->askForCardChosen(zhangliao, p, "h", "iklianbao");

                CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, zhangliao->objectName());
                room->obtainCard(zhangliao, Sanguosha->getCard(card_id), reason, false);
            }
        }
        return false;
    }
};

class IkLuoyi: public TriggerSkill {
public:
    IkLuoyi(): TriggerSkill("ikluoyi") {
        events << EventPhaseChanging;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (TriggerSkill::triggerable(player) && change.to == Player::Draw && !player->isSkipped(Player::Draw))
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        if (player->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        player->skip(Player::Draw, true);
        room->setPlayerMark(player, "ikluoyi", 1);

        QList<int> ids = room->getNCards(3, false);
        CardsMoveStruct move(ids, player, Player::PlaceTable,
                             CardMoveReason(CardMoveReason::S_REASON_TURNOVER, player->objectName(), "ikluoyi", QString()));
        room->moveCardsAtomic(move, true);

        room->getThread()->delay();
        room->getThread()->delay();

        QList<int> card_to_throw;
        QList<int> card_to_gotback;
        for (int i = 0; i < 3; i++) {
            const Card *card = Sanguosha->getCard(ids[i]);
            if (card->getTypeId() == Card::TypeBasic || card->isKindOf("Weapon") || card->isKindOf("Duel"))
                card_to_gotback << ids[i];
            else
                card_to_throw << ids[i];
        }
        if (!card_to_throw.isEmpty()) {
            DummyCard *dummy = new DummyCard(card_to_throw);
            CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, player->objectName(), "ikluoyi", QString());
            room->throwCard(dummy, reason, NULL);
            delete dummy;
        }
        if (!card_to_gotback.isEmpty()) {
            DummyCard *dummy = new DummyCard(card_to_gotback);
            room->obtainCard(player, dummy);
            delete dummy;
        }
        return false;
    }
};

class IkLuoyiBuff: public TriggerSkill {
public:
    IkLuoyiBuff(): TriggerSkill("#ikluoyi") {
        events << DamageCaused;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *xuchu, QVariant &data, ServerPlayer* &) const{
        if (!xuchu || xuchu->getMark("ikluoyi") == 0 || xuchu->isDead()) return QStringList();
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.chain || damage.transfer) return QStringList();
        const Card *reason = damage.card;
        if (reason && (reason->isKindOf("Slash") || reason->isKindOf("Duel")))
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *xuchu, QVariant &data, ServerPlayer *) const{
        DamageStruct damage = data.value<DamageStruct>();
        LogMessage log;
        log.type = "#IkLuoyiBuff";
        log.from = xuchu;
        log.to << damage.to;
        log.arg = QString::number(damage.damage);
        log.arg2 = QString::number(++damage.damage);
        room->sendLog(log);

        data = QVariant::fromValue(damage);

        return false;
    }
};

class IkLuoyiClear: public TriggerSkill {
public:
    IkLuoyiClear(): TriggerSkill("#ikluoyi-clear") {
        events << EventPhaseStart;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (player != NULL && player->isAlive() && player->getPhase() == Player::RoundStart && player->getMark("ikluoyi") > 0)
            room->setPlayerMark(player, "ikluoyi", 0);
        return QStringList();
    }
};

class IkTiandu: public TriggerSkill {
public:
    IkTiandu(): TriggerSkill("iktiandu") {
        frequency = Frequent;
        events << FinishJudge;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *guojia, QVariant &data, ServerPlayer* &) const {
        JudgeStruct *judge = data.value<JudgeStruct *>();
        if (TriggerSkill::triggerable(guojia) && room->getCardPlace(judge->card->getEffectiveId()) == Player::PlaceJudge)
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *guojia, QVariant &data, ServerPlayer *) const {
        if (guojia->askForSkillInvoke(objectName(), data)) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *guojia, QVariant &data, ServerPlayer *) const {
        JudgeStruct *judge = data.value<JudgeStruct *>();
        guojia->obtainCard(judge->card);

        return false;
    }
};

IkYumeng::IkYumeng(): MasochismSkill("ikyumeng") {
    frequency = Frequent;
    n = 2;
}

void IkYumeng::onDamaged(ServerPlayer *guojia, const DamageStruct &damage) const{
    Room *room = guojia->getRoom();
    int x = damage.damage;
    for (int i = 0; i < x; i++) {
        if (!guojia->isAlive() || !room->askForSkillInvoke(guojia, objectName()))
            return;
        room->broadcastSkillInvoke("ikyumeng");

        QList<ServerPlayer *> _guojia;
        _guojia.append(guojia);
        QList<int> yiji_cards = room->getNCards(n, false);

        CardsMoveStruct move(yiji_cards, NULL, guojia, Player::PlaceTable, Player::PlaceHand,
                             CardMoveReason(CardMoveReason::S_REASON_PREVIEW, guojia->objectName(), objectName(), QString()));
        QList<CardsMoveStruct> moves;
        moves.append(move);
        room->notifyMoveCards(true, moves, false, _guojia);
        room->notifyMoveCards(false, moves, false, _guojia);

        QList<int> origin_yiji = yiji_cards;
        while (room->askForYiji(guojia, yiji_cards, objectName(), true, false, true, -1, room->getAlivePlayers())) {
            CardsMoveStruct move(QList<int>(), guojia, NULL, Player::PlaceHand, Player::PlaceTable,
                                 CardMoveReason(CardMoveReason::S_REASON_PREVIEW, guojia->objectName(), objectName(), QString()));
            foreach (int id, origin_yiji) {
                if (room->getCardPlace(id) != Player::DrawPile) {
                    move.card_ids << id;
                    yiji_cards.removeOne(id);
                }
            }
            origin_yiji = yiji_cards;
            QList<CardsMoveStruct> moves;
            moves.append(move);
            room->notifyMoveCards(true, moves, false, _guojia);
            room->notifyMoveCards(false, moves, false, _guojia);
            if (!guojia->isAlive())
                return;
        }

        if (!yiji_cards.isEmpty()) {
            CardsMoveStruct move(yiji_cards, guojia, NULL, Player::PlaceHand, Player::PlaceTable,
                                 CardMoveReason(CardMoveReason::S_REASON_PREVIEW, guojia->objectName(), objectName(), QString()));
            QList<CardsMoveStruct> moves;
            moves.append(move);
            room->notifyMoveCards(true, moves, false, _guojia);
            room->notifyMoveCards(false, moves, false, _guojia);

            DummyCard *dummy = new DummyCard(yiji_cards);
            guojia->obtainCard(dummy, false);
            delete dummy;
        }
    }
}

class IkMengyang: public TriggerSkill {
public:
    IkMengyang(): TriggerSkill("ikmengyang") {
        events << EventPhaseStart;
        frequency = Frequent;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer* &) const{
        if (player->getPhase() == Player::Start){
            if (TriggerSkill::triggerable(player))
                return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent , Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        if (player->askForSkillInvoke(objectName())){
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent , Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        JudgeStruct judge;
        forever {
            judge.pattern = ".|black";
            judge.good = true;
            judge.reason = objectName();
            judge.play_animation = false;
            judge.who = player;
            judge.time_consuming = true;

            room->judge(judge);
            if ((judge.isGood() && !player->askForSkillInvoke(objectName())) || judge.isBad())
                break;
        }

        return false;
    }
};

class IkMengyangMove: public TriggerSkill {
public:
    IkMengyangMove(): TriggerSkill("#ikmengyang-move") {
        events << FinishJudge;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (player != NULL) {
            JudgeStruct *judge = data.value<JudgeStruct *>();
            if (judge->reason == "ikmengyang") {
                if (judge->isGood()) {
                    if (room->getCardPlace(judge->card->getEffectiveId()) == Player::PlaceJudge) {
                        return QStringList(objectName());
                    }
                }
            }
        }
        return QStringList();
    }

    virtual bool effect(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        JudgeStruct *judge = data.value<JudgeStruct *>();
        room->moveCardTo(judge->card, judge->who, Player::PlaceHand, true);

        return false;
    }
};

class IkZhongyan: public OneCardViewAsSkill {
public:
    IkZhongyan(): OneCardViewAsSkill("ikzhongyan") {
        filter_pattern = ".|black|.|hand";
        response_pattern = "jink";
        response_or_use = true;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Jink *jink = new Jink(originalCard->getSuit(), originalCard->getNumber());
        jink->setSkillName(objectName());
        jink->addSubcard(originalCard->getId());
        return jink;
    }
};

IkZhihengCard::IkZhihengCard() {
    target_fixed = true;
}

void IkZhihengCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    if (source->isAlive())
        room->drawCards(source, subcards.length(), "ikzhiheng");
}

class IkZhiheng: public ViewAsSkill {
public:
    IkZhiheng(): ViewAsSkill("ikzhiheng") {
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        if (selected.length() >= Self->getMaxHp()) return false;
        return !Self->isJilei(to_select);
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.isEmpty())
            return NULL;

        IkZhihengCard *ikzhiheng_card = new IkZhihengCard;
        ikzhiheng_card->addSubcards(cards);
        ikzhiheng_card->setSkillName(objectName());
        return ikzhiheng_card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->canDiscard(player, "he") && !player->hasUsed("IkZhihengCard");
    }
};

class IkJiyuan: public TriggerSkill {
public:
    IkJiyuan(): TriggerSkill("ikjiyuan$") {
        events << TargetSpecified << PreHpRecover;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (triggerEvent == TargetSpecified) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("Peach") && player->getKingdom() == "yuki") {
                foreach (ServerPlayer *p, use.to)
                    if (p->hasLordSkill("ikjiyuan"))
                        room->setCardFlag(use.card, "ikjiyuan");
            }
        } else if (triggerEvent == PreHpRecover) {
            RecoverStruct rec = data.value<RecoverStruct>();
            if (rec.card && rec.card->hasFlag("ikjiyuan"))
                return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *sunquan, QVariant &data, ServerPlayer *) const{
        RecoverStruct rec = data.value<RecoverStruct>();

        room->notifySkillInvoked(sunquan, "ikjiyuan");
        room->broadcastSkillInvoke("ikjiyuan");

        LogMessage log;
        log.type = "#IkJiyuanExtraRecover";
        log.from = sunquan;
        log.to << rec.who;
        log.arg = objectName();
        room->sendLog(log);

        rec.recover++;
        data = QVariant::fromValue(rec);

        return false;
    }
};

class IkKuipo: public OneCardViewAsSkill {
public:
    IkKuipo(): OneCardViewAsSkill("ikkuipo") {
        filter_pattern = ".|black";
        response_or_use = true;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Dismantlement *dismantlement = new Dismantlement(originalCard->getSuit(), originalCard->getNumber());
        dismantlement->addSubcard(originalCard->getId());
        dismantlement->setSkillName(objectName());
        return dismantlement;
    }
};

IkGuisiCard::IkGuisiCard() {
}

bool IkGuisiCard::targetFilter(const QList<const Player *> &, const Player *to_select, const Player *Self) const{
    QStringList targetslist = Self->property("ikguisi_targets").toString().split("+");
    return targetslist.contains(to_select->objectName());
}

void IkGuisiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    room->removePlayerMark(source, "@guisi");
    room->addPlayerMark(source, "@guisiused");

    CardUseStruct use = source->tag["ikguisi"].value<CardUseStruct>();
    foreach (ServerPlayer *p, targets)
        use.nullified_list << p->objectName();
    source->tag["ikguisi"] = QVariant::fromValue(use);
}

class IkGuisiViewAsSkill: public ZeroCardViewAsSkill {
public:
    IkGuisiViewAsSkill():ZeroCardViewAsSkill("ikguisi") {
        response_pattern = "@@ikguisi";
    }

    virtual const Card *viewAs() const{
        return new IkGuisiCard;
    }
};

class IkGuisi: public TriggerSkill {
public:
    IkGuisi(): TriggerSkill("ikguisi") {
        events << TargetSpecifying;
        view_as_skill = new IkGuisiViewAsSkill;
        frequency = Limited;
        limit_mark = "@guisi";
    }

    virtual QMap<ServerPlayer *,QStringList> triggerable(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const{
        QMap<ServerPlayer *,QStringList> skill_list;
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.to.length() <= 1 || !use.card->isNDTrick())
            return skill_list;
        foreach (ServerPlayer *p, room->findPlayersBySkillName(objectName()))
            if (p->getMark("@guisi") > 0)
                skill_list.insert(p, QStringList(objectName()));
        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *ganning) const{
        CardUseStruct use = data.value<CardUseStruct>();
        QStringList target_list;
        foreach (ServerPlayer *p, use.to)
            target_list << p->objectName();
        room->setPlayerProperty(ganning, "ikguisi_targets", target_list.join("+"));
        ganning->tag["ikguisi"] = data;
        room->askForUseCard(ganning, "@@ikguisi", "@ikguisi-card");
        data = ganning->tag["ikguisi"];

        return false;
    }
};

class IkBiju: public TriggerSkill {
public:
    IkBiju(): TriggerSkill("ikbiju") {
        events << EventPhaseChanging;
        frequency = Frequent;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *lvmeng, QVariant &data, ServerPlayer* &) const{
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (TriggerSkill::triggerable(lvmeng) && change.to == Player::Discard && !lvmeng->hasFlag("IkBijuSlashInPlayPhase"))
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *lvmeng, QVariant &, ServerPlayer *) const{
        if (lvmeng->askForSkillInvoke(objectName())) {
            if (lvmeng->getHandcardNum() > lvmeng->getMaxCards())
                room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *lvmeng, QVariant &, ServerPlayer *) const{
        lvmeng->skip(Player::Discard);
        return false;
    }
};

class IkBijuRecord: public TriggerSkill {
public:
    IkBijuRecord(): TriggerSkill("#ikbiju-record") {
        events << PreCardUsed << CardResponded;
        frequency = Compulsory;
        global = true;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *lvmeng, QVariant &data, ServerPlayer* &) const{
        if (lvmeng->getPhase() == Player::Play) {
            const Card *card = NULL;
            if (triggerEvent == PreCardUsed)
                card = data.value<CardUseStruct>().card;
            else
                card = data.value<CardResponseStruct>().m_card;
            if (card->isKindOf("Slash"))
                lvmeng->setFlags("IkBijuSlashInPlayPhase");
        }
        return QStringList();
    }
};

class IkPojian: public TriggerSkill {
public:
    IkPojian(): TriggerSkill("ikpojian") {
        events << EventPhaseStart;
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target)
            && target->getPhase() == Player::Finish
            && target->getMark("ikpojian") >= 5
            && target->getMark("@pojian") == 0;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        room->broadcastSkillInvoke(objectName());
        room->notifySkillInvoked(player, objectName());

        LogMessage log;
        log.type = "#IkPojianWake";
        log.from = player;
        log.arg = QString::number(player->getMark("ikpojian"));
        log.arg2 = objectName();
        room->sendLog(log);
        room->addPlayerMark(player, "@pojian");

        room->recover(player, RecoverStruct(player));
        room->changeMaxHpForAwakenSkill(player);

        room->acquireSkill(player, "ikqinghua");
        return false;
    }
};

class IkPojianRecord: public TriggerSkill {
public:
    IkPojianRecord(): TriggerSkill("#ikpojian-record") {
        events << PreCardUsed << CardResponded << EventPhaseStart;
        frequency = Compulsory;
        global = true;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *lvmeng, QVariant &data, ServerPlayer* &) const{
        if (triggerEvent == EventPhaseStart) {
            if (lvmeng->getPhase() == Player::RoundStart)
                lvmeng->setMark("ikpojian", 0);
        } else {
            const Card *card = NULL;
            if (triggerEvent == PreCardUsed)
                card = data.value<CardUseStruct>().card;
            else {
                CardResponseStruct resp = data.value<CardResponseStruct>();
                if (resp.m_isUse)
                    card = resp.m_card;
            }
            if (card && !card->isKindOf("EquipCard"))
                return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *, ServerPlayer *lvmeng, QVariant &data, ServerPlayer *) const{
        lvmeng->addMark("ikpojian");
        return false;
    }
};

IkQinghuaCard::IkQinghuaCard() {
}

bool IkQinghuaCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void IkQinghuaCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    const Card *card = room->askForCardShow(effect.to, effect.from, "ikqinghua");
    room->showCard(effect.to, card->getEffectiveId());
    QString suit = card->getSuitString();
    if (room->askForCard(effect.from, ".|" + suit, "@ikqinghua-discard:::" + suit)) {
        room->throwCard(card, effect.to);
        QList<ServerPlayer *> targets;
        targets << effect.from << effect.to;
        room->sortByActionOrder(targets);
        foreach (ServerPlayer *p, targets)
            room->recover(p, RecoverStruct(effect.from));
    }
}

class IkQinghua: public ZeroCardViewAsSkill {
public:
    IkQinghua(): ZeroCardViewAsSkill("ikqinghua") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("IkQinghuaCard");
    }

    virtual const Card *viewAs() const{
        return new IkQinghuaCard;
    }
};

IkKurouCard::IkKurouCard() {
    target_fixed = true;
}

void IkKurouCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    room->loseHp(source);
    if (source->isAlive())
        room->drawCards(source, 2, "ikkurou");
}

class IkKurou: public ZeroCardViewAsSkill {
public:
    IkKurou(): ZeroCardViewAsSkill("ikkurou") {
    }

    virtual const Card *viewAs() const{
        return new IkKurouCard;
    }
};

class IkZaiqi: public TriggerSkill {
public:
    IkZaiqi(): TriggerSkill("ikzaiqi") {
        events << HpRecover;
        frequency = Frequent;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(player) || player->getPhase() != Player::Play || !player->hasFlag("Global_Dying")) return QStringList();
        QStringList skill;
        RecoverStruct recover;
        for (int i = 0; i < recover.recover; i++)
            skill << objectName();
        return skill;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        if (player->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        player->drawCards(1, objectName());
        return true;
    }
};

IkGuidengCard::IkGuidengCard() {
    will_throw = false;
    handling_method = Card::MethodNone;
}

void IkGuidengCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *zhouyu = effect.from;
    ServerPlayer *target = effect.to;
    Room *room = zhouyu->getRoom();
    Card::Suit suit = getSuit();

    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, zhouyu->objectName(), target->objectName(), "ikguideng", QString());
    room->obtainCard(target, this, reason);

    if (target->isAlive()) {
        if (target->isNude()) {
            room->loseHp(target);
        } else {
            if (room->askForSkillInvoke(target, "ikguideng_discard", "prompt:::" + Card::Suit2String(suit))) {
                room->showAllCards(target);
                DummyCard *dummy = new DummyCard;
                foreach (const Card *card, target->getCards("he")) {
                    if (card->getSuit() == suit)
                        dummy->addSubcard(card);
                }
                if (dummy->subcardsLength() > 0)
                    room->throwCard(dummy, target);
                delete dummy;
            } else {
                room->loseHp(target);
            }
        }
    }
}

class IkGuideng: public OneCardViewAsSkill {
public:
    IkGuideng(): OneCardViewAsSkill("ikguideng") {
        filter_pattern = ".|.|.|hand";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng() && !player->hasUsed("IkGuidengCard");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        IkGuidengCard *card = new IkGuidengCard;
        card->addSubcard(originalCard);
        card->setSkillName(objectName());
        return card;
    }
};

class IkChenhong: public DrawCardsSkill {
public:
    IkChenhong(): DrawCardsSkill("ikchenhong") {
        frequency = Compulsory;
    }

    virtual int getDrawNum(ServerPlayer *zhouyu, int n) const{
        Room *room = zhouyu->getRoom();
        room->broadcastSkillInvoke(objectName());

        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = zhouyu;
        log.arg = objectName();
        room->sendLog(log);

        return n + 1;
    }
};

class IkChenhongMaxCards: public MaxCardsSkill {
public:
    IkChenhongMaxCards(): MaxCardsSkill("#ikchenhong") {
    }

    virtual int getFixed(const Player *target) const{
        if (target->hasSkill("ikchenhong"))
            return target->getMaxHp();
        else
            return -1;
    }
};

IkWanmeiCard::IkWanmeiCard() {
    handling_method = Card::MethodNone;
}

bool IkWanmeiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty()) return false;
    int id = getEffectiveId();

    Indulgence *indulgence = new Indulgence(getSuit(), getNumber());
    indulgence->addSubcard(id);
    indulgence->setSkillName("ikwanmei");
    indulgence->deleteLater();

    bool canUse = !Self->isLocked(indulgence);
    if (canUse && to_select != Self && !to_select->containsTrick("indulgence") && !Self->isProhibited(to_select, indulgence))
        return true;
    bool canDiscard = false;
    foreach (const Card *card, Self->getHandcards()) {
        if (card->getEffectiveId() == id && !Self->isJilei(Sanguosha->getCard(id))) {
            canDiscard = true;
            break;
        }
    }
    if (!canDiscard || !to_select->containsTrick("indulgence"))
        return false;
    foreach (const Card *card, to_select->getJudgingArea()) {
        if (card->isKindOf("Indulgence") && Self->canDiscard(to_select, card->getEffectiveId()))
            return true;
    }
    return false;
}

const Card *IkWanmeiCard::validate(CardUseStruct &cardUse) const{
    ServerPlayer *to = cardUse.to.first();
    if (!to->containsTrick("indulgence")) {
        Indulgence *indulgence = new Indulgence(getSuit(), getNumber());
        indulgence->addSubcard(getEffectiveId());
        indulgence->setSkillName("ikwanmei");
        return indulgence;
    }
    return this;
}

void IkWanmeiCard::onUse(Room *room, const CardUseStruct &use) const{
    CardUseStruct card_use = use;

    QVariant data = QVariant::fromValue(card_use);
    RoomThread *thread = room->getThread();
    thread->trigger(PreCardUsed, room, card_use.from, data);
    card_use = data.value<CardUseStruct>();

    LogMessage log;
    log.from = card_use.from;
    log.to = card_use.to;
    log.type = "#UseCard";
    log.card_str = card_use.card->toString();
    room->sendLog(log);

    CardMoveReason reason(CardMoveReason::S_REASON_THROW, card_use.from->objectName(), QString(), "ikwanmei", QString());
    room->moveCardTo(this, card_use.from, NULL, Player::DiscardPile, reason, true);

    thread->trigger(CardUsed, room, card_use.from, data);
    card_use = data.value<CardUseStruct>();
    thread->trigger(CardFinished, room, card_use.from, data);
}

void IkWanmeiCard::onEffect(const CardEffectStruct &effect) const{
    foreach (const Card *judge, effect.to->getJudgingArea()) {
        if (judge->isKindOf("Indulgence") && effect.from->canDiscard(effect.to, judge->getEffectiveId())) {
            effect.from->getRoom()->throwCard(judge, NULL, effect.from);
            effect.from->drawCards(1, "ikawanmei");
            return;
        }
    }
}

class IkWanmeiViewAsSkill: public OneCardViewAsSkill {
public:
    IkWanmeiViewAsSkill(): OneCardViewAsSkill("ikwanmei") {
        filter_pattern = ".|diamond";
        response_or_use = true;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("IkWanmeiCard");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        IkWanmeiCard *card = new IkWanmeiCard;
        card->addSubcard(originalCard);
        card->setSkillName(objectName());
        return card;
    }
};

class IkWanmei: public TriggerSkill {
public:
    IkWanmei(): TriggerSkill("ikwanmei") {
        events << CardFinished;
        view_as_skill = new IkWanmeiViewAsSkill;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->isKindOf("Indulgence") && use.card->getSkillName() == objectName())
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        player->drawCards(1, objectName());
        return false;
    }
};

IkXuanhuoCard::IkXuanhuoCard() {
}

bool IkXuanhuoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty())
        return false;

    if (to_select->hasFlag("IkXuanhuoSlashSource") || to_select == Self)
        return false;

    const Player *from = NULL;
    foreach (const Player *p, Self->getAliveSiblings()) {
        if (p->hasFlag("IkXuanhuoSlashSource")) {
            from = p;
            break;
        }
    }

    const Card *slash = Card::Parse(Self->property("ikxuanhuo").toString());
    if (from && !from->canSlash(to_select, slash, false))
        return false;

    int card_id = subcards.first();
    int range_fix = 0;
    if (Self->getWeapon() && Self->getWeapon()->getId() == card_id) {
        const Weapon *weapon = qobject_cast<const Weapon *>(Self->getWeapon()->getRealCard());
        range_fix += weapon->getRange() - Self->getAttackRange(false);
    } else if (Self->getOffensiveHorse() && Self->getOffensiveHorse()->getId() == card_id) {
        range_fix += 1;
    }

    return Self->inMyAttackRange(to_select, range_fix);
}

void IkXuanhuoCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->setFlags("IkXuanhuoTarget");
}

class IkXuanhuoViewAsSkill: public OneCardViewAsSkill {
public:
    IkXuanhuoViewAsSkill(): OneCardViewAsSkill("ikxuanhuo") {
        filter_pattern = ".!";
        response_pattern = "@@ikxuanhuo";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        IkXuanhuoCard *ikxuanhuo_card = new IkXuanhuoCard;
        ikxuanhuo_card->addSubcard(originalCard);
        return ikxuanhuo_card;
    }
};

class IkXuanhuo: public TriggerSkill {
public:
    IkXuanhuo(): TriggerSkill("ikxuanhuo") {
        events << TargetConfirming;
        view_as_skill = new IkXuanhuoViewAsSkill;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *daqiao, QVariant &data, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(daqiao)) return QStringList();
        CardUseStruct use = data.value<CardUseStruct>();

        if (use.card->isKindOf("Slash") && use.to.contains(daqiao) && daqiao->canDiscard(daqiao, "he")) {
            QList<ServerPlayer *> players = room->getOtherPlayers(daqiao);
            players.removeOne(use.from);

            foreach (ServerPlayer *p, players) {
                if (use.from->canSlash(p, use.card, false) && daqiao->inMyAttackRange(p))
                    return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *daqiao, QVariant &data, ServerPlayer *) const{
        CardUseStruct use = data.value<CardUseStruct>();
        QString prompt = "@ikxuanhuo:" + use.from->objectName();
        room->setPlayerFlag(use.from, "IkXuanhuoSlashSource");
        // a temp nasty trick
        daqiao->tag["ikxuanhuo-card"] = QVariant::fromValue(use.card); // for the server (AI)
        room->setPlayerProperty(daqiao, "ikxuanhuo", use.card->toString()); // for the client (UI)
        if (room->askForUseCard(daqiao, "@@ikxuanhuo", prompt, -1, Card::MethodDiscard))
            return true;
        else {
            daqiao->tag.remove("ikxuanhuo-card");
            room->setPlayerProperty(daqiao, "ikxuanhuo", QString());
            room->setPlayerFlag(use.from, "-IkXuanhuoSlashSource");
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *daqiao, QVariant &data, ServerPlayer *) const{
        CardUseStruct use = data.value<CardUseStruct>();
        QList<ServerPlayer *> players = room->getOtherPlayers(daqiao);
        players.removeOne(use.from);

        daqiao->tag.remove("ikxuanhuo-card");
        room->setPlayerProperty(daqiao, "ikxuanhuo", QString());
        room->setPlayerFlag(use.from, "-IkXuanhuoSlashSource");
        foreach (ServerPlayer *p, players) {
            if (p->hasFlag("IkXuanhuoTarget")) {
                p->setFlags("-IkXuanhuoTarget");
                if (!use.from->canSlash(p, false))
                    return false;
                use.to.removeOne(daqiao);
                use.to.append(p);
                room->sortByActionOrder(use.to);
                data = QVariant::fromValue(use);
                room->getThread()->trigger(TargetConfirming, room, p, data);
                return false;
            }
        }

        return false;
    }
};

class IkWujie: public TriggerSkill {
public:
    IkWujie(): TriggerSkill("ikwujie") {
        events << CardsMoveOneTime;
        frequency = Frequent;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *luxun, QVariant &data, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(luxun)) return QStringList();
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from == luxun && move.from_places.contains(Player::PlaceHand) && move.is_last_handcard)
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *luxun, QVariant &, ServerPlayer *) const{
        if (luxun->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *luxun, QVariant &, ServerPlayer *) const{
        luxun->drawCards(1, objectName());
        return false;
    }
};

IkYuanheCard::IkYuanheCard() {
}

void IkYuanheCard::onUse(Room *room, const CardUseStruct &card_use) const{
    CardUseStruct use = card_use;
    use.to << use.from;
    SkillCard::onUse(room, use);
}

void IkYuanheCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    foreach (ServerPlayer *p, targets)
        p->drawCards(2, "ikyuanhe");
    foreach (ServerPlayer *p, targets)
        room->askForDiscard(p, "ikyuanhe", 2, 2, false, true);
}

class IkYuanhe: public OneCardViewAsSkill {
public:
    IkYuanhe(): OneCardViewAsSkill("ikyuanhe") {
        filter_pattern = ".|red|.|hand!";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("IkYuanheCard");
    }

    virtual const Card *viewAs(const Card *originalcard) const{
        IkYuanheCard *await = new IkYuanheCard;
        await->addSubcard(originalcard->getId());
        return await;
    }
};

IkYuluCard::IkYuluCard() {
}

bool IkYuluCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty())
        return false;

    return to_select->isMale() && to_select->isWounded() && to_select != Self;
}

void IkYuluCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    RecoverStruct recover(effect.from);
    room->recover(effect.from, recover, true);
    room->recover(effect.to, recover, true);
}

class IkYulu: public ViewAsSkill {
public:
    IkYulu(): ViewAsSkill("ikyulu") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getHandcardNum() >= 2 && !player->hasUsed("IkYuluCard");
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        if (selected.length() > 1 || Self->isJilei(to_select))
            return false;

        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() != 2)
            return NULL;

        IkYuluCard *ikyulu_card = new IkYuluCard();
        ikyulu_card->addSubcards(cards);
        return ikyulu_card;
    }
};

class IkCuimeng: public TriggerSkill {
public:
    IkCuimeng(): TriggerSkill("ikcuimeng") {
        events << CardsMoveOneTime;
        frequency = Frequent;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(player)) return QStringList();
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from == player && move.from_places.contains(Player::PlaceEquip))
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *sunshangxiang, QVariant &, ServerPlayer *) const{
        if (sunshangxiang->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *sunshangxiang, QVariant &, ServerPlayer *) const{
        sunshangxiang->drawCards(2, objectName());
        return false;
    }
};

class IkWushuang: public TriggerSkill {
public:
    IkWushuang(): TriggerSkill("ikwushuang") {
        events << TargetSpecified << CardFinished;
        frequency = Compulsory;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        QMap<ServerPlayer *, QStringList> skill_list;
        if (triggerEvent == CardFinished) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("Duel")) {
                foreach (ServerPlayer *p, room->getAllPlayers())
                    p->tag.remove("IkWushuang_" + use.card->toString());
            }
        } else if (triggerEvent == TargetSpecified) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("Slash") && TriggerSkill::triggerable(player))
                skill_list.insert(player, QStringList(objectName()));
            else if (use.card->isKindOf("Duel")) {
                if (TriggerSkill::triggerable(player))
                    skill_list.insert(player, QStringList(objectName()));
                foreach (ServerPlayer *p, use.to.toSet())
                    if (TriggerSkill::triggerable(p))
                        skill_list.insert(p, QStringList(objectName()));
            }
        }
        return skill_list;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *ask_who) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->isKindOf("Slash")) {
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = ask_who;
            log.arg = objectName();
            room->sendLog(log);
            room->notifySkillInvoked(ask_who, objectName());
            room->broadcastSkillInvoke(objectName());

            QVariantList jink_list = ask_who->tag["Jink_" + use.card->toString()].toList();
            for (int i = 0; i < use.to.length(); i++) {
                if (jink_list.at(i).toInt() == 1)
                    jink_list.replace(i, QVariant(2));
            }
            ask_who->tag["Jink_" + use.card->toString()] = QVariant::fromValue(jink_list);
        } else if (use.card->isKindOf("Duel")) {
            if (use.from == ask_who) {
                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = ask_who;
                log.arg = objectName();
                room->sendLog(log);
                room->notifySkillInvoked(ask_who, objectName());
                room->broadcastSkillInvoke(objectName());

                QStringList ikwushuang_tag;
                foreach (ServerPlayer *to, use.to)
                    ikwushuang_tag << to->objectName();
                ask_who->tag["IkWushuang_" + use.card->toString()] = ikwushuang_tag;
            } else {
                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = use.from;
                log.arg = objectName();
                room->sendLog(log);
                room->notifySkillInvoked(use.from, objectName());
                room->broadcastSkillInvoke(objectName());

                ask_who->tag["IkWushuang_" + use.card->toString()] = QStringList(use.from->objectName());
            }
        }

        return false;
    }
};

IkWudiCard::IkWudiCard() {
    handling_method = MethodUse;
}

bool IkWudiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    Duel *duel = new Duel(SuitToBeDecided, 0);
    duel->addSubcards(subcards);
    duel->deleteLater();
    return duel->targetFilter(targets, to_select, Self);
}

const Card *IkWudiCard::validate(CardUseStruct &cardUse) const{
    Duel *duel = new Duel(SuitToBeDecided, 0);
    duel->addSubcards(subcards);
    duel->setSkillName("ikwudi");
    return duel;
}

class IkWudi: public ViewAsSkill {
public:
    IkWudi(): ViewAsSkill("ikwudi") {
        response_or_use = true;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("IkWudiCard");
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        if (selected.isEmpty())
            return !to_select->isEquipped();
        else if (selected.length() == 1) {
            const Card *card = selected.first();
            return !to_select->isEquipped() && to_select->getSuit() == card->getSuit();
        } else
            return false;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() == 2) {
            Duel *duel = new Duel(Card::SuitToBeDecided, 0);
            duel->addSubcards(cards);
            duel->setSkillName(objectName());
            duel->deleteLater();
            if (duel->isAvailable(Self)) {
                IkWudiCard *card = new IkWudiCard;
                card->addSubcards(cards);
                return card;
            }
        }
        return NULL;
    }
};

IkMoyuCard::IkMoyuCard(){
    mute = true;
}

bool IkMoyuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!to_select->isMale())
        return false;

    Duel *duel = new Duel(Card::NoSuit, 0);
    duel->deleteLater();
    if (targets.isEmpty() && Self->isProhibited(to_select, duel))
        return false;

    if (targets.length() == 1 && to_select->isCardLimited(duel, Card::MethodUse))
        return false;

    return targets.length() < 2 && to_select != Self;
}

bool IkMoyuCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void IkMoyuCard::onUse(Room *room, const CardUseStruct &card_use) const{
    CardUseStruct use = card_use;
    QVariant data = QVariant::fromValue(use);
    RoomThread *thread = room->getThread();

    thread->trigger(PreCardUsed, room, card_use.from, data);
    use = data.value<CardUseStruct>();

    room->broadcastSkillInvoke("ikmoyu");

    LogMessage log;
    log.from = card_use.from;
    log.to << card_use.to;
    log.type = "#UseCard";
    log.card_str = toString();
    room->sendLog(log);

    CardMoveReason reason(CardMoveReason::S_REASON_THROW, card_use.from->objectName(), QString(), "ikmoyu", QString());
    room->moveCardTo(this, card_use.from, NULL, Player::DiscardPile, reason, true);

    thread->trigger(CardUsed, room, card_use.from, data);
    use = data.value<CardUseStruct>();
    thread->trigger(CardFinished, room, card_use.from, data);
}

void IkMoyuCard::use(Room *room, ServerPlayer *, QList<ServerPlayer *> &targets) const{
    ServerPlayer *to = targets.at(0);
    ServerPlayer *from = targets.at(1);

    Duel *duel = new Duel(Card::NoSuit, 0);
    duel->setCancelable(false);
    duel->setSkillName("_ikmoyu");
    if (!from->isCardLimited(duel, Card::MethodUse) && !from->isProhibited(to, duel))
        room->useCard(CardUseStruct(duel, from, to));
    else
        delete duel;
}

class IkMoyu: public OneCardViewAsSkill {
public:
    IkMoyu(): OneCardViewAsSkill("ikmoyu") {
        filter_pattern = ".!";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getAliveSiblings().length() > 1
               && player->canDiscard(player, "he") && !player->hasUsed("IkMoyuCard");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        IkMoyuCard *lijian_card = new IkMoyuCard;
        lijian_card->addSubcard(originalCard->getId());
        return lijian_card;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *card) const{
        return card->isKindOf("Duel") ? 0 : -1;
    }
};

class IkZhuoyue: public PhaseChangeSkill {
public:
    IkZhuoyue(): PhaseChangeSkill("ikzhuoyue") {
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *player) const {
        return TriggerSkill::triggerable(player)
            && player->getPhase() == Player::Finish;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        if (player->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *diaochan) const{
        diaochan->drawCards(1, objectName());
        return false;
    }
};

class IkHuichun: public OneCardViewAsSkill {
public:
    IkHuichun(): OneCardViewAsSkill("ikhuichun") {
        filter_pattern = ".|red";
        response_or_use = true;
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern.contains("peach") && !player->hasFlag("Global_PreventPeach")
                && player->getPhase() == Player::NotActive && player->canDiscard(player, "he");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Peach *peach = new Peach(originalCard->getSuit(), originalCard->getNumber());
        peach->addSubcard(originalCard->getId());
        peach->setSkillName(objectName());
        return peach;
    }

    virtual int getEffectIndex(const ServerPlayer *player, const Card *) const{
        int index = qrand() % 2 + 1;
        if (Player::isNostalGeneral(player, "huatuo"))
            index += 2;
        return index;
    }
};

IkQingnangCard::IkQingnangCard() {
}

bool IkQingnangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->isWounded();
}

bool IkQingnangCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.value(0, Self)->isWounded();
}

void IkQingnangCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.value(0, source);
    room->cardEffect(this, source, target);
}

void IkQingnangCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->getRoom()->recover(effect.to, RecoverStruct(effect.from));
}

class IkQingnang: public OneCardViewAsSkill {
public:
    IkQingnang(): OneCardViewAsSkill("ikqingnang") {
        filter_pattern = ".|.|.|hand!";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->canDiscard(player, "h") && !player->hasUsed("IkQingnangCard");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        IkQingnangCard *qingnang_card = new IkQingnangCard;
        qingnang_card->addSubcard(originalCard->getId());
        return qingnang_card;
    }
};

IkaiTsuchiPackage::IkaiTsuchiPackage()
    :Package("ikai-tsuchi")
{
    General *wind001 = new General(this, "wind001$", "kaze");
    wind001->addSkill(new IkShenai);
    wind001->addSkill(new IkXinqi);

    General *wind002 = new General(this, "wind002", "kaze");
    wind002->addSkill(new IkChilian);
    wind002->addSkill(new IkZhenhong);
    wind002->addSkill(new IkZhenhongTargetMod);
    related_skills.insertMulti("ikzhenhong", "#ikzhenhong-target");

    General *wind003 = new General(this, "wind003", "kaze");
    wind003->addSkill(new IkLipao);
    wind003->addSkill(new IkJiukuang);

    General *wind004 = new General(this, "wind004", "kaze", 3);
    wind004->addSkill(new IkYuxi);
    wind004->addSkill(new IkJingyou);

    General *wind006 = new General(this, "wind006", "kaze");
    wind006->addSkill("thjibu");
    wind006->addSkill(new IkYufeng);
    wind006->addSkill(new IkYufengClear);
    related_skills.insertMulti("ikyufeng", "#ikyufeng-clear");

    General *wind007 = new General(this, "wind007", "kaze", 3, false);
    wind007->addSkill(new IkHuiquan);
    wind007->addSkill("thjizhi");

    General *bloom001 = new General(this, "bloom001$", "hana");
    bloom001->addSkill(new IkJiaoman);
    bloom001->addSkill(new IkHuanwei);

    General *bloom002 = new General(this, "bloom002", "hana", 3);
    bloom002->addSkill(new IkTiansuo);
    bloom002->addSkill(new IkHuanji);

    General *bloom003 = new General(this, "bloom003", "hana");
    bloom003->addSkill(new IkAoli);
    bloom003->addSkill(new IkQingjian);

    General *bloom004 = new General(this, "bloom004", "hana");
    bloom004->addSkill(new IkLianbao);
    bloom004->addSkill(new IkLianbaoAct);
    related_skills.insertMulti("iklianbao", "#iklianbao");

    General *bloom005 = new General(this, "bloom005", "hana");
    bloom005->addSkill(new IkLuoyi);
    bloom005->addSkill(new IkLuoyiBuff);
    bloom005->addSkill(new IkLuoyiClear);
    related_skills.insertMulti("ikluoyi", "#ikluoyi");
    related_skills.insertMulti("ikluoyi", "#ikluoyi-clear");

    General *bloom006 = new General(this, "bloom006", "hana", 3);
    bloom006->addSkill(new IkTiandu);
    bloom006->addSkill(new IkYumeng);

    General *bloom007 = new General(this, "bloom007", "hana", 3, false);
    bloom007->addSkill(new IkMengyang);
    bloom007->addSkill(new IkMengyangMove);
    related_skills.insertMulti("ikmengyang", "#ikmengyang-move");
    bloom007->addSkill(new IkZhongyan);

    General *snow001 = new General(this, "snow001$", "yuki");
    snow001->addSkill(new IkZhiheng);
    snow001->addSkill(new IkJiyuan);

    General *snow002 = new General(this, "snow002", "yuki");
    snow002->addSkill(new IkKuipo);
    snow002->addSkill(new IkGuisi);

    General *snow003 = new General(this, "snow003", "yuki");
    snow003->addSkill(new IkBiju);
    snow003->addSkill(new IkBijuRecord);
    related_skills.insertMulti("ikbiju", "#ikbiju-record");
    snow003->addSkill(new IkPojian);
    snow003->addSkill(new IkPojianRecord);
    related_skills.insertMulti("ikpojian", "#ikpojian-record");
    snow003->addRelateSkill("ikqinghua");

    General *snow004 = new General(this, "snow004", "yuki");
    snow004->addSkill(new IkKurou);
    snow004->addSkill(new IkZaiqi);

    General *snow005 = new General(this, "snow005", "yuki", 3);
    snow005->addSkill(new IkGuideng);
    snow005->addSkill(new IkChenhong);
    snow005->addSkill(new IkChenhongMaxCards);
    related_skills.insertMulti("ikchenhong", "#ikchenhong");

    General *snow006 = new General(this, "snow006", "yuki", 3, false);
    snow006->addSkill(new IkWanmei);
    snow006->addSkill(new IkXuanhuo);

    General *snow007 = new General(this, "snow007", "yuki", 3);
    snow007->addSkill(new IkWujie);
    snow007->addSkill(new IkYuanhe);

    General *snow008 = new General(this, "snow008", "yuki", 3, false);
    snow008->addSkill(new IkYulu);
    snow008->addSkill(new IkCuimeng);

    General *luna002 = new General(this, "luna002", "tsuki");
    luna002->addSkill(new IkWushuang);
    luna002->addSkill(new IkWudi);

    General *luna003 = new General(this, "luna003", "tsuki", 3, false);
    luna003->addSkill(new IkMoyu);
    luna003->addSkill(new IkZhuoyue);

    General *luna006 = new General(this, "luna006", "tsuki", 3);
    luna006->addSkill(new IkHuichun);
    luna006->addSkill(new IkQingnang);

    addMetaObject<IkShenaiCard>();
    addMetaObject<IkXinqiCard>();
    addMetaObject<IkLianbaoCard>();
    addMetaObject<IkZhihengCard>();
    addMetaObject<IkGuisiCard>();
    addMetaObject<IkQinghuaCard>();
    addMetaObject<IkKurouCard>();
    addMetaObject<IkGuidengCard>();
    addMetaObject<IkWanmeiCard>();
    addMetaObject<IkXuanhuoCard>();
    addMetaObject<IkYuanheCard>();
    addMetaObject<IkYuluCard>();
    addMetaObject<IkWudiCard>();
    addMetaObject<IkMoyuCard>();
    addMetaObject<IkQingnangCard>();

    skills << new NonCompulsoryInvalidity << new IkQinghua;
}

ADD_PACKAGE(IkaiTsuchi)
