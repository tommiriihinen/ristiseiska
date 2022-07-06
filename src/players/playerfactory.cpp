#include "playerfactory.h"

PlayerFactory::PlayerFactory(QObject *parent)
    : QObject{parent}
{
    server = new Server();
    connect(server, &Server::allSocketPlayersReady, this, &PlayerFactory::allPlayersReady);
}

void PlayerFactory::createPlayers(Game &game) {
    int clientplrs, serverplrs, robotplrs, neuralplrs, randomplrs;
    std::string input;
    std::cout << "How many client players (total max: 7, min: 3): ";
    std::cin >> input;
    clientplrs = stoi(input);
    std::cout << "How many robot players  (total max: 7, min: 3): ";
    std::cin >> input;
    robotplrs = stoi(input);
    std::cout << "How many AI players     (total max: 7, min: 3): ";
    std::cin >> input;
    neuralplrs = stoi(input);
    std::cout << "How many server players (total max: 7, min: 3): ";
    std::cin >> input;
    serverplrs = stoi(input);
    std::cout << "How many random players (total max: 7, min: 3): ";
    std::cin >> input;
    randomplrs = stoi(input);

    bool humans_playing = (clientplrs + serverplrs > 0);

    std::vector<Player*> players;
    QString pythonfolder = QDir::currentPath();

    // CLIENT HOSTED PLAYERS:
    for (int c = 1; c <= clientplrs; c++) {
        SocketPlayer* player = new SocketPlayer();
        game.addPlayer(player);
        players.push_back(player);

        server->queueSocketPlayerProduction(player);
        // When all socket players are ready tell game that players are ready.
        std::string location = QCoreApplication::applicationDirPath().toStdString() + "\\humanclient.py";
        std::string command = "start cmd /k python " + location;
        system(command.c_str());
        // :/scripts/scripts/pyclient/humanclient.py
    }
    for (int c = 1; c <= neuralplrs; c++) {
        SocketPlayer* player = new SocketPlayer();
        game.addPlayer(player);
        players.push_back(player);

        server->queueSocketPlayerProduction(player);
        // When all socket players are ready tell game that players are ready.
        std::string command = "start python \"C:/Users/tommi/Documents/ristiseiska/scripts/pyclient/humanclient.py\""; //"run ";
        system(command.c_str());
    }

    // SERVER HOSTED PLAYERS:
    for (int c = 1; c <= serverplrs; c++) {
        HumanPlayer* player = new HumanPlayer();
        std::string prefix = "human-";
        player->setName(QString::fromStdString(prefix.append(std::to_string(c))));
        game.addPlayer(player);
        players.push_back(player);
    }

    MISettings default_settings;
    if (humans_playing) default_settings.slow_play = true;

    for (int c = 1; c <= robotplrs; c++) {
        MachinePlayer* player = new MachinePlayer();
        player->setSettings(default_settings);
        std::string prefix = "robo-";
        player->setName(QString::fromStdString(prefix.append(std::to_string(c))));
        game.addPlayer(player);
        players.push_back(player);
    }
    for (int c = 1; c <= randomplrs; c++) {
        RandomPlayer* player = new RandomPlayer();
        std::string prefix = "random-";
        player->setName(QString::fromStdString(prefix.append(std::to_string(c))));
        game.addPlayer(player);
        players.push_back(player);
    }

    if (clientplrs + neuralplrs == 0) {
        emit allPlayersReady();
    } else {
        server->StartServer();
    }
}
