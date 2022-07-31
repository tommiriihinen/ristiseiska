#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QDebug>
#include "connection.h"
#include "players/socketplayer.h"

class Server : public QTcpServer
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    void StartServer();

    void queueSocketPlayerProduction(std::shared_ptr<SocketPlayer> player);

signals:
    void allSocketPlayersReady();
    void initSocketPlayers();

public slots:
    void socketPlayerReady(std::shared_ptr<SocketPlayer> id);

protected:
    void incomingConnection(long long socketDescriptor);

private:
    std::vector<std::shared_ptr<SocketPlayer>> mProductionQueue;
    std::vector<std::shared_ptr<SocketPlayer>> mConnectionQueue;

};

#endif // SERVER_H
