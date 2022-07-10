#include "server.h"

Server::Server(QObject *parent)
    : QTcpServer{parent}
{

}

void Server::StartServer()
{
    if(!this->listen(QHostAddress::Any, 55555))
    {
        qDebug() << "Could not start server";
    }
    else
    {
        qDebug() << "Listening...";
    }
}

void Server::queueSocketPlayerProduction(std::shared_ptr<SocketPlayer> player) {
    mProductionQueue.push_back(player);
    mConnectionQueue.push_back(player);
}

void Server::incomingConnection(long long socketDescriptor)
{
    assert(!mProductionQueue.empty());

    auto player = mProductionQueue.back();
    mProductionQueue.pop_back();

    qDebug() << socketDescriptor << " Connecting...";
    Connection *thread = new Connection(socketDescriptor, this);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    // Connect socketplayer communication to it's corresponding QTcpSocket wrapper (Connection)
    connect(player.get(), &SocketPlayer::send, thread, &Connection::send);
    connect(thread, &Connection::recieved, player.get(), &SocketPlayer::recieve);

    // Socket player tells when it is ready to play
    connect(player.get(), &SocketPlayer::creationComplete, this, &Server::socketPlayerReady);
    // Socket deletion
    connect(player.get(), &SocketPlayer::destroySocket, thread, &Connection::destroySocket);
    // Socket is created by signals and slots so that the socket object lives in the outside thread
    connect(this, &Server::initSocketPlayers, thread, &Connection::createSocket);

    thread->start();

    if (mProductionQueue.empty()) {
        qDebug() << "All socketplayers are produced ";
        emit initSocketPlayers();
    }

}

void Server::socketPlayerReady(std::shared_ptr<SocketPlayer> id) {
    assert(!mConnectionQueue.empty());

    mConnectionQueue.erase(std::remove(mConnectionQueue.begin(), mConnectionQueue.end(), id), mConnectionQueue.end());
    qDebug() << "Socketplayer ready";
    if (mConnectionQueue.empty()) {
        qDebug() << "All socketplayers are ready ";
        emit allSocketPlayersReady();
    }
}
