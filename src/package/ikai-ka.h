#ifndef IKAIKA_H
#define IKAIKA_H

#include "package.h"
#include "card.h"

class IkaiKaPackage : public Package{
    Q_OBJECT

public:
    IkaiKaPackage();
};

class IkZhijuCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE IkZhijuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class IkJilunCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE IkJilunCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class IkKangjinCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE IkKangjinCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual const Card *validate(CardUseStruct &cardUse) const;
};

#include <QDialog>
#include <QVBoxLayout>
#include <QCommandLinkButton>
#include <QButtonGroup>
class SelectSuitDialog: public QDialog {
    Q_OBJECT

public:
    static SelectSuitDialog *getInstance();

public slots:
    void popup();
    void selectSuit(QAbstractButton *button);

private:
    explicit SelectSuitDialog();

    QButtonGroup *group;
    QVBoxLayout *button_layout;

signals:
    void onButtonClick();
};

class IkHunkaoCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE IkHunkaoCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class IkHualanCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE IkHualanCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class IkHuangshiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE IkHuangshiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class IkDongzhaoCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE IkDongzhaoCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class IkJimuCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE IkJimuCard();
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class IkDengpoCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE IkDengpoCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class IkQihunCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE IkQihunCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual const Card *validate(CardUseStruct &cardUse) const;
};

#include <QDialog>
#include <QVBoxLayout>
#include <QCommandLinkButton>
#include <QButtonGroup>
class IkLingchaDialog: public QDialog {
    Q_OBJECT

public:
    static IkLingchaDialog *getInstance();

public slots:
    void popup();
    void selectCard(QAbstractButton *button);

private:
    explicit IkLingchaDialog();

    QButtonGroup *group;
    QVBoxLayout *button_layout;

    QHash<QString, const Card *> map;

signals:
    void onButtonClick();
};

class IkLingchaCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE IkLingchaCard();

    virtual bool targetFixed() const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual const Card *validate(CardUseStruct &card_use) const;
    virtual const Card *validateInResponse(ServerPlayer *user) const;
};

class IkDuanniCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE IkDuanniCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class IkShidaoCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE IkShidaoCard();

    virtual bool targetFixed() const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual const Card *validate(CardUseStruct &card_use) const;
    virtual const Card *validateInResponse(ServerPlayer *user) const;
};

class IkMingwangCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE IkMingwangCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual const Card *validate(CardUseStruct &cardUse) const;
};

class IkLinghuiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE IkLinghuiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class IkXiaowuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE IkXiaowuCard();

    virtual const Card *validate(CardUseStruct &card_use) const;
};

class IkHuanlveCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE IkHuanlveCard();

    virtual bool targetFixed() const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual const Card *validate(CardUseStruct &card_use) const;
    virtual const Card *validateInResponse(ServerPlayer *user) const;
};

class IkLihunCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE IkLihunCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class IkQisiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE IkQisiCard();

    virtual const Card *validate(CardUseStruct &cardUse) const;
    virtual const Card *validateInResponse(ServerPlayer *user) const;
};

class IkManwuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE IkManwuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class IkXianlvCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE IkXianlvCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class IkLianwuCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE IkLianwuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class IkLianwuDrawCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE IkLianwuDrawCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class IkXiekeCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE IkXiekeCard();

    virtual bool targetFixed() const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual const Card *validate(CardUseStruct &card_use) const;
    virtual const Card *validateInResponse(ServerPlayer *user) const;
};

class IkSuyiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE IkSuyiCard();
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class IkQiansheCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE IkQiansheCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class IkDaoleiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE IkDaoleiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif // IKAIKA_H
