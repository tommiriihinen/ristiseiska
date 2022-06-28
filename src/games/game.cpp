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
        Player* last_player = players[(turn-2) % n];
        play_turn(current_player, last_player);

        Player* winner = check_win();
        if (winner != nullptr) {
            std::cout <<"\n" << winner->getName().toStdString() << " has won the game!\n";
            game_is_on = false;
        }
    }
}

void Game::clean() {

}

void Game::play_turn(Player* current_player, Player* last_player) {

    Card played_card = current_player->play_card(this->board);

    // No cards played
    if (this->board.isEmpty()) {
        std::cout << current_player->getName().toStdString() << " dosen't have the Seven of clubss\n";

    // Couldn't play a card
    } else if (played_card.getSuit() == joker) {


        std::cout << current_player->getName().toStdString() << " passes.\n";
        last_player->give_card(*current_player);
        std::cout << last_player->getName().toStdString() << " gave " << current_player->getName().toStdString() << " a card.\n";
        current_player->getDeck()->suitSort();

    // A card was played
    } else {
        // Announce the played card
        std::cout << current_player->getName().toStdString() << " plays "
                  << played_card.id().toStdString() << ". Cards left: "
                  << current_player->getDeck()->size() << "\n";

        // Played card was an Ace or a King
        if (played_card.getRank() == ace || played_card.getRank() == king) {

            if (current_player->will_continue()) {
                std::cout << current_player->getName().toStdString() << " continues.\n";
                play_turn(current_player, last_player);
            } else {
                std::cout << current_player->getName().toStdString() << " doesn't continue.\n";
            }
        }
    }
}

Player* Game::check_win() {
    Player* winner = nullptr;
    for (Player* player : players) {
        if (player->getDeck()->isEmpty()) {
            winner = player;
        }
    }
    return winner;
}
