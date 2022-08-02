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

        server->queueSocketPlayerProduction(player.get());
        // When all socket players are ready tell game that players are ready.
        std::string title = "CandyClient";
        std::string location = QCoreApplication::applicationDirPath().toStdString();
        std::string command = "start \"" + title + "\" python " + location + "/candyclient.py";
        system(command.c_str());
    }
    for (int c = 1; c <= neuralplrs; c++) {
        std::shared_ptr<SocketPlayer> player (new SocketPlayer);
        game.addPlayer(player);

        server->queueSocketPlayerProduction(player.get());
        // When all socket players are ready tell game that players are ready.
        std::string title = "NeuralClient";
        std::string location = "cd " + QCoreApplication::applicationDirPath().toStdString() + " & ";
        std::string conda  = "C:/ProgramData/Miniconda3/Scripts/activate.bat & conda activate tf & ";
        std::string python = "C:/ProgramData/Miniconda3/python.exe  "; //-m pdb -c continue
        std::string script = "neuralclient.py";
        std::string command = "start \"" + title + "\" cmd /c \"" + location + conda + python + script + "\"";
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
