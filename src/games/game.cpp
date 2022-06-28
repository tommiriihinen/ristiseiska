#include "headers/games/game.h"

Game::Game()
{

}

int Game::getTurn() const {
    return this->turn;
}

Board* Game::getBoard() {
    return &this->board;
}

void Game::setup() {
    // CREATE CARDS
    Deck* main_deck = new Deck();
    main_deck->fill();
    main_deck->print();

    // DEAL CARDS
    qDebug() << "Players hands: ";
    Dealer dealer;
    dealer.addCards(*main_deck);
    for (auto player : players) {
        player->getDeck()->print();
        dealer.addDeck(player->getDeck());
    }
    dealer.deal();
}

void Game::start() {
    this->turn = 0;
    int n = players.size();
    bool game_is_on = true;
    while(game_is_on) {
        this->turn++;
        std::cout << "\nTurn: " << turn << "\n";
        this->board.print();
        Player* current_player = players[(turn-1) % n];
        end_card_replay:
        Card_event event = current_player->play_card(this->board);

        switch (event) {
        case ordinary_card:
            break;

        case end_card:
            if (current_player->will_continue()) {
            std::cout << current_player->getName().toStdString() << " continues.\n";
            goto end_card_replay;
            }
            std::cout << current_player->getName().toStdString() << " doesn't continue.\n";
            break;

        case no_card:
            if (this->board.isEmpty()) break;
            Player* last_player = players[(turn-2) % n];
            std::cout << current_player->getName().toStdString() << " passes.\n";
            last_player->give_card(*current_player);
            std::cout << last_player->getName().toStdString() << " gave " << current_player->getName().toStdString() << " a card.\n";
            current_player->getDeck()->suitSort();
            break;

        }

        // Check for winners
        for (Player* player : players) {
            if (player->getDeck()->isEmpty()) {
                std::cout <<"\n\n" << player->getName().toStdString() << " has won the game!";
                game_is_on = false;
            }
        }
    }
}
