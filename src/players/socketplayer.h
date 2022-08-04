#ifndef SOCKETPLAYER_H
#define SOCKETPLAYER_H

#include "player.h"
#include "networking/connection.h"
#include <QString>


class SocketPlayer : public IPlayer
{
    Q_OBJECT
public:
    explicit SocketPlayer(QObject *parent = nullptr);

    ~SocketPlayer();

    bool isListener() const override {return true;}

    void setSocket(QTcpSocket *socket);

    void handleMessage(QString message);
    void send(QString message);

signals:
    void playerReady(SocketPlayer* self); // notify playerFactory
    void announce(QString message);

public slots:
    void take_action(IPlayer &player, GameAction action) override; // from game

    void recieveAnnouncement(QString message, QString command); // form game
    void recieveWhisper(IPlayer &target, QString message, QString command); // from game

    void recieve(); // from socket

private:
    GameAction mState;
    bool mActionPending = false;
    QTcpSocket* mSocket = nullptr;

    QString MSG_DELIMITER { "/" };
    QByteArray MSG_DELIMITER_UTF8 { MSG_DELIMITER.toUtf8() };
};

#endif // SOCKETPLAYER_H
