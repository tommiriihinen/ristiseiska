#ifndef RANDOMPLAYER_H
#define RANDOMPLAYER_H

#include "player.h"

class RandomPlayer : public Player
{
    Q_OBJECT
public:
    explicit RandomPlayer(QObject *parent = nullptr);
};

#endif // RANDOMPLAYER_H
