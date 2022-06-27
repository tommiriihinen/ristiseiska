#include <QCoreApplication>

#include "card.h"
#include "deck.h"
#include "dealer.h"
#include "player.h"
#include "humanplayer.h"
#include <iostream>

int HUMAN_PLAYERS = 3;
int MACHINE_PLAYERS = 0;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // CREATE GAME
    Game* game = new Game();

    // CREATE CARDS
    Deck* main_deck = new Deck();
    main_deck->fill();
    main_deck->print();

    // CREATE PLAYERS
    std::vector<Player*> players;
    for (int playerN = 1; playerN <= HUMAN_PLAYERS; playerN++) {
        HumanPlayer* player = new HumanPlayer();
        player->setName(QString::fromStdString(std::to_string(playerN)));
        game->addPlayer(player);
        players.push_back(player);
    }

    // DEAL CARDS
    qDebug() << "Players hands: ";
    Dealer dealer;
    dealer.addCards(*main_deck);
    for (auto player : players) {
        player->getDeck()->print();
        dealer.addDeck(player->getDeck());
    }
    dealer.deal();

    std::cout << "The Seven of Clubs\n";
    game->start();

    qDebug("end");
    return a.exec();
}
