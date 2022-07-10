#ifndef RANDOMPLAYER_H
#define RANDOMPLAYER_H

#include "player.h"
#include <random>

class RandomPlayer : public IPlayer
{
    Q_OBJECT
public:
    explicit RandomPlayer(QObject *parent = nullptr)
        : IPlayer(parent) {}

public slots:
    void take_action(IPlayer &player, GameAction action) override;

private:
    Card choosePlay();
    Card chooseGive();
    bool chooseContinue();

    int genRand(int max);
};

#endif // RANDOMPLAYER_H

