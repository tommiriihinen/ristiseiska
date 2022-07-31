#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QDebug>
#include "connection.h"
#include "players/socketplayer.h"

/* How this shit works
 * 1. PlayerFactory constructs a SocketPlayer
 * 2. PlayerFactory adds the player to Servers production queue and connection queue
 * 3. PlayerFactory starts the client
 * 4. The client automatically tries to connect the server
 *      5. When client connects a Connection object is created and given to the SocketPlayer
 *      6. SocketPlayer is removed from production queue
 * 7. Client chooses a name
 * 8. SocketPlayer recieves the name and tells the server that it is ready to play
 * 9. When all SocketPlayers are ready, the server tells the game this.
 *
 */

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
    void socketPlayerReady(SocketPlayer* self);

protected:
    void incomingConnection(long long socketDescriptor);

private:
    std::vector<SocketPlayer*> mProductionQueue;
    std::vector<SocketPlayer*> mConnectionQueue;

};

#endif // SERVER_H
