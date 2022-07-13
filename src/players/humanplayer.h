#ifndef HUMANPLAYER_H
#define HUMANPLAYER_H

#include "player.h"
#include "players/socketplayer.h"
#include "util/util.h"


class HumanPlayer: public SocketPlayer
{
    Q_OBJECT
public:
    explicit HumanPlayer(QObject *parent = nullptr)
        : SocketPlayer(parent) {}

signals:
    void send(QString data);
    void creationComplete(SocketPlayer* self);
    void announce(QString message);

public slots:
    void take_action(Player* player, GameAction action) override; // from game
    void recieve(QString data); // from connection
    void announcements(QString message);
    void whispers(Player* target, QString message);

private:
    GameAction mState;
    bool mActionPending;
};

#endif // HUMANPLAYER_H


