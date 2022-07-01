#ifndef SOCKETPLAYER_H
#define SOCKETPLAYER_H

#include "headers/games/player.h"

class SocketPlayer : public Player
{
    Q_OBJECT
public:
    explicit SocketPlayer(QObject *parent = nullptr) : Player(parent) {}

public slots:
    void take_action(Player* player, GameAction action) override;
};

#endif // SOCKETPLAYER_H
