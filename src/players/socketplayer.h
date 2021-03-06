#ifndef SOCKETPLAYER_H
#define SOCKETPLAYER_H

#include "player.h"
#include "src/networking/connection.h"
#include <QString>


class SocketPlayer : public IPlayer
{
    Q_OBJECT
public:
    explicit SocketPlayer(QObject *parent = nullptr)
        : IPlayer(parent) {
        qDebug() << "socket created";
    }
    ~SocketPlayer() {
        emit destroySocket();
    }

    bool isListener() const override {return true;}

signals:
    void send(QString data);
    void creationComplete(std::shared_ptr<SocketPlayer> self);
    void destroySocket();
    void announce(QString message);

public slots:
    void take_action(IPlayer &player, GameAction action) override; // from game
    void recieve(QString data); // from connection
    void announcements(QString message, QString command);
    void whispers(IPlayer &target, QString message, QString command);

private:
    GameAction mState;
    bool mActionPending;
};

#endif // SOCKETPLAYER_H
