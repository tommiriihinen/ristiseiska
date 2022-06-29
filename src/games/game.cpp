#include "headers/games/game.h"

Game::Game()
{
    Deck stariting_deck;
    stariting_deck.fill();
    stariting_deck.print();

    this->dealer = new Dealer();
    this->dealer->addCards(stariting_deck);
}

void Game::addPlayer(Player* player) {
    players.push_back(player);
    dealer->addDeck(player->getDeck());
}


int Game::getTurn() const {
    return this->turn;
}

Board* Game::getBoard() {
    return &this->board;
}

void Game::setup() {
    this->dealer->print();
    this->dealer->deal();

}

void Game::start() {
    std::cout << "\n-----------------------The Seven of Clubs-----------------------\n";

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
            winner->incrementWins();
            game_is_on = false;
        }
    }
    std::cout << "\n-----------------------the seven of clubs-----------------------\n";

    // count games
    for (Player* player : players) player->incrementGame();
}

void Game::clean() {
    Deck cleaned_cards = board.clean();
    dealer->addCards(cleaned_cards);
}

void Game::play_turn(Player* current_player, Player* last_player) {

    Card played_card = current_player->play_card(this->board);

    // No cards played
    if (this->board.isEmpty()) {
        std::cout << current_player->getName().toStdString() << " doesn't have the Seven of Clubs\n";

    // Couldn't play a card
    } else if (played_card.getSuit() == joker) {

        std::cout << current_player->getName().toStdString() << " passes.\n";
        Card given_card = last_player->give_card(*current_player, board);
        std::cout << last_player->getName().toStdString() << " gave "
                  << current_player->getName().toStdString() << " "
                  << given_card.id().toStdString() << ".\n";
        current_player->getDeck()->suitSort();

    // A card was played
    } else {
        // Announce the played card
        std::cout << current_player->getName().toStdString() << " plays "
                  << played_card.id().toStdString() << ". Cards left: "
                  << current_player->getDeck()->size() << "\n";

        // Played card was an Ace or a King
        if (played_card.getRank() == ace || played_card.getRank() == king) {

            if (current_player->will_continue(board)) {
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
