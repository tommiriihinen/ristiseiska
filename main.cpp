#include <QCoreApplication>
#include <iostream>

#include "headers/games/game.h"
#include "headers/games/humanplayer.h"
#include "headers/games/machineplayer.h"


int HUMAN_PLAYERS = 1;
int MACHINE_PLAYERS = 2;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // CREATE GAME
    Game* game = new Game();

    // CREATE PLAYERS
    std::vector<Player*> players;
    for (int playerN = 1; playerN <= HUMAN_PLAYERS; playerN++) {
        HumanPlayer* player = new HumanPlayer();
        std::string prefix = "human-";
        player->setName(QString::fromStdString(prefix.append(std::to_string(playerN))));
        game->addPlayer(player);
        players.push_back(player);
    }
    for (int playerN = 1; playerN <= MACHINE_PLAYERS; playerN++) {
        MachinePlayer* player = new MachinePlayer();
        std::string prefix = "robo-";
        player->setName(QString::fromStdString(prefix.append(std::to_string(playerN))));
        game->addPlayer(player);
        players.push_back(player);
    }

    game->setup();
    std::cout << "The Seven of Clubs\n";
    game->start();

    return a.exec();
}
