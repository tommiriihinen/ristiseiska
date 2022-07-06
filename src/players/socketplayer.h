#ifndef SOCKETPLAYER_H
#define SOCKETPLAYER_H

#include "player.h"
#include "src/networking/connection.h"
#include <QString>


class SocketPlayer : public Player
{
    Q_OBJECT
public:
    explicit SocketPlayer(QObject* parent = nullptr);

signals:
    void send(QString data);
    void creationComplete(SocketPlayer* self);
    void announce(QString message);

public slots:
    void take_action(Player* player, GameAction action) override; // from game
    void recieve(QString data); // from connection
    void announcements(QString message);

private:
    GameAction mState;
    bool mActionPending;
};

#endif // SOCKETPLAYER_H
