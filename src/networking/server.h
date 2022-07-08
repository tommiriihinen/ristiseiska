#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QDebug>
#include "connection.h"
#include "src/players/socketplayer.h"

class Server : public QTcpServer
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    void StartServer();

    void queueSocketPlayerProduction(SocketPlayer* player);

signals:
    void allSocketPlayersReady();
    void initSocketPlayers();

public slots:
    void socketPlayerReady(SocketPlayer *id);

protected:
    void incomingConnection(long long socketDescriptor);

private:
    std::vector<SocketPlayer*> mProductionQueue;
    std::vector<SocketPlayer*> mConnectionQueue;

};

#endif // SERVER_H
