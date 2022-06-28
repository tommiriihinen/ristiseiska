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



    bool running = true;
    while(running) {
        // CREATE GAME
        Game* game = new Game();

        int humans, robots;
        std::string input;
        std::cout << "How many human players?: (total max: 7, min: 3): ";
        std::cin >> input;
        humans = stoi(input);
        std::cout << "How many robot players?: (total max: 7, min: 3): ";
        std::cin >> input;
        robots = stoi(input);

        // CREATE PLAYERS
        std::vector<Player*> players;
        for (int playerN = 1; playerN <= humans; playerN++) {
            HumanPlayer* player = new HumanPlayer();
            std::string prefix = "human-";
            player->setName(QString::fromStdString(prefix.append(std::to_string(playerN))));
            game->addPlayer(player);
            players.push_back(player);
        }
        for (int playerN = 1; playerN <= robots; playerN++) {
            MachinePlayer* player = new MachinePlayer();
            std::string prefix = "robo-";
            player->setName(QString::fromStdString(prefix.append(std::to_string(playerN))));
            game->addPlayer(player);
            players.push_back(player);
        }


        game->setup();
        std::cout << "The Seven of clubss\n";
        game->start();
        delete game;

        running = questionPrompt("Want to play again?");
    }



    return a.exec();
}
