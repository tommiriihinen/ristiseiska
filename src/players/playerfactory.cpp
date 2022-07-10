#include "playerfactory.h"

PlayerFactory::PlayerFactory(QObject *parent)
    : QObject{parent}
{
    server = new Server();
    connect(server, &Server::allSocketPlayersReady, this, &PlayerFactory::allPlayersReady);
}

void PlayerFactory::createPlayers(std::map<PlayerType, int> order, Game &game) {
    int clientplrs = 0, comptrplrs = 0, randomplrs = 0, neuralplrs = 0;
    if (order.count(PlayerType::client)) clientplrs = order[PlayerType::client];
    if (order.count(PlayerType::comptr)) comptrplrs = order[PlayerType::comptr];
    if (order.count(PlayerType::random)) randomplrs = order[PlayerType::random];
    if (order.count(PlayerType::neural)) neuralplrs = order[PlayerType::neural];

    bool humans_playing = (clientplrs > 0);

    std::vector<IPlayer*> players;

    // CLIENT PLAYERS:
    for (int c = 1; c <= clientplrs; c++) {
        std::shared_ptr<SocketPlayer> player (new SocketPlayer);
        game.addPlayer(player);

        server->queueSocketPlayerProduction(player);
        // When all socket players are ready tell game that players are ready.
        std::string location = QCoreApplication::applicationDirPath().toStdString() + "\\candyclient.py";
        std::string command = "start python " + location;
        system(command.c_str());
    }
    for (int c = 1; c <= neuralplrs; c++) {
        std::shared_ptr<SocketPlayer> player (new SocketPlayer);
        game.addPlayer(player);

        server->queueSocketPlayerProduction(player);
        // When all socket players are ready tell game that players are ready.
        std::string command = "start python \"C:/Users/tommi/Documents/ristiseiska/scripts/pyclient/candyclient.py\""; //"run ";
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
    } else {
        server->StartServer();
    }
}
