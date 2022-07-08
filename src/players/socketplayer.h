#ifndef SOCKETPLAYER_H
#define SOCKETPLAYER_H

#include "player.h"
#include "src/networking/connection.h"
#include <QString>


class SocketPlayer : public Player
{
    Q_OBJECT
public:
    explicit SocketPlayer(QObject *parent = nullptr)
        : Player(parent) {
        qDebug() << "socket created";
    }
    ~SocketPlayer() {
        emit destroySocket();
    }

signals:
    void send(QString data);
    void creationComplete(SocketPlayer* self);
    void destroySocket();
    void announce(QString message);

public slots:
    void take_action(Player* player, GameAction action); // from game
    void recieve(QString data); // from connection
    void announcements(QString message, QString command);
    void whispers(Player* target, QString message, QString command);

private:
    GameAction mState;
    bool mActionPending;
};

#endif // SOCKETPLAYER_H
