#include "playerfactory.h"

PlayerFactory::PlayerFactory(QObject *parent)
    : QObject{parent}
{
    mServer = new QTcpServer(this);
    if (!mServer->listen(QHostAddress::Any, 55555)) {
        qCritical() << tr("Unable to start the server: %1.").arg(mServer->errorString());
        return;
    } else {
        qDebug() << "Server listening for connections";
    }

    connect(mServer, &QTcpServer::newConnection, this, &PlayerFactory::connectSocketPlayer);
}

void PlayerFactory::createPlayers(std::map<PlayerType, int> order, Game &game, bool benchmarking) {
    int clientplrs = 0, comptrplrs = 0, randomplrs = 0, neuralplrs = 0;
    if (order.count(PlayerType::client)) clientplrs = order[PlayerType::client];
    if (order.count(PlayerType::comptr)) comptrplrs = order[PlayerType::comptr];
    if (order.count(PlayerType::random)) randomplrs = order[PlayerType::random];
    if (order.count(PlayerType::neural)) neuralplrs = order[PlayerType::neural];

    bool humans_playing = (clientplrs > 0);

    std::vector<IPlayer*> players;

    // CLIENT PLAYERS:
    std::string debugargs = "";
    #ifdef QT_DEBUG
        debugargs += "-m pdb -c continue ";
    #endif

    for (int c = 1; c <= clientplrs; c++) {
        std::shared_ptr<SocketPlayer> player (new SocketPlayer);
        game.addPlayer(player);

        mSocketPlayerStatusMap.insert(player.get(), SocketPlayerStatus::not_connected);

        // Start client
        std::string title = "\"CandyClient\"";
        std::string location = QCoreApplication::applicationDirPath().toStdString();
        std::string command = "start " + title + " python " + debugargs + location + "/candyclient.py";
        system(command.c_str());
    }
    for (int c = 1; c <= neuralplrs; c++) {
        std::shared_ptr<SocketPlayer> player (new SocketPlayer);
        game.addPlayer(player);

        mSocketPlayerStatusMap.insert(player.get(), SocketPlayerStatus::not_connected);

        // Start client
        std::string title = "\"NeuralClient\"";
        std::string navigate = "cd " + QCoreApplication::applicationDirPath().toStdString();
        std::string activate_env  = "C:/ProgramData/Miniconda3/Scripts/activate.bat & conda activate tf";
        std::string python = "C:/ProgramData/Miniconda3/python.exe "; //
        std::string args = "";
        if (benchmarking) args += " -output 0 -model " + mBenchmarkingModel.toStdString() + " ";

        std::string script = "neuralclient.py";
        std::string command = "start " + title + " cmd /c \"" + navigate + " & "
                                                              + activate_env + " & "
                                                              + "python " + debugargs
                                                              + script + args
                                                              + "\"";
        system(command.c_str());        
    }

    // SERVER HOSTED PLAYERS:
    MISettings default_settings;
    if (humans_playing) default_settings.slow_play = true;

    for (int c = 1; c <= comptrplrs; c++) {
        std::shared_ptr<MachinePlayer> player (new MachinePlayer);
        player->setSettings(default_settings);
        std::string prefix = "goblin-";
        player->setName(QString::fromStdString(prefix.append(std::to_string(c))));
        game.addPlayer(player);
    }
    for (int c = 1; c <= randomplrs; c++) {
        std::shared_ptr<RandomPlayer> player (new RandomPlayer);
        std::string prefix = "random-";
        player->setName(QString::fromStdString(prefix.append(std::to_string(c))));
        game.addPlayer(player);
    }

    if (clientplrs + neuralplrs == 0) {
        emit allPlayersReady();
    }
}

void PlayerFactory::connectSocketPlayer() {

    qDebug() << "Connecting SocketPlayer";
    QTcpSocket *clientConnection = mServer->nextPendingConnection();
    connect(clientConnection, &QAbstractSocket::disconnected, clientConnection, &QObject::deleteLater);

    SocketPlayer* player = mSocketPlayerStatusMap.key(SocketPlayerStatus::not_connected); // get an unconnected SocketPlayer object
    player->setSocket(clientConnection);

    connect(player, &SocketPlayer::playerReady, this, &PlayerFactory::socketPlayerReady);
    mSocketPlayerStatusMap[player] = SocketPlayerStatus::naming;
    player->send("NICK;");
}

void PlayerFactory::socketPlayerReady(SocketPlayer* readyPlayer) {

    qDebug() << readyPlayer->getName() << " named and ready";
    disconnect(readyPlayer, &SocketPlayer::playerReady, this, &PlayerFactory::socketPlayerReady);

    Q_ASSERT(mSocketPlayerStatusMap[readyPlayer] == SocketPlayerStatus::naming);
    mSocketPlayerStatusMap.remove(readyPlayer);

    if (mSocketPlayerStatusMap.isEmpty()) {
        emit allPlayersReady();
    }
}
