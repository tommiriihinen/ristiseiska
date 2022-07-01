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

    int humans, robots;
    std::string input;
    std::cout << "How many human players (total max: 7, min: 3): ";
    std::cin >> input;
    humans = stoi(input);
    std::cout << "How many robot players (total max: 7, min: 3): ";
    std::cin >> input;
    robots = stoi(input);

    Game game;

    // CREATE PLAYERS
    std::vector<Player*> players;
    for (int playerN = 1; playerN <= humans; playerN++) {
        HumanPlayer* player = new HumanPlayer();
        std::string prefix = "human-";
        player->setName(QString::fromStdString(prefix.append(std::to_string(playerN))));
        game.addPlayer(player);
        players.push_back(player);
    }
    bool slow_machines = false;
    if (humans > 0) slow_machines = true;
    for (int playerN = 1; playerN <= robots; playerN++) {
        MachinePlayer* player = new MachinePlayer(slow_machines);
        std::string prefix = "robo-";
        player->setName(QString::fromStdString(prefix.append(std::to_string(playerN))));
        game.addPlayer(player);
        players.push_back(player);
    }

    bool running = true;
    while(running) {

        int games = numberPrompt("How many games to play: ");
        for (int g = 0; g < games; g++) {
            game.setup();
            game.start();
            game.clean();

            for (Player* player : players) {
                std::cout << player->getName().toStdString() << "'s winrate is: " << player->getWinrate()*100 << "%\n";
            }
        }
        running = questionPrompt("Want to play more?");
    }
    a.exit();
    return a.exec();
}
