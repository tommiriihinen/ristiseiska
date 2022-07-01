#include "headers/games/game.h"

Game::Game(QObject *parent)
{
    Deck stariting_deck;
    stariting_deck.fill();
    stariting_deck.print();

    mDealer.addCards(stariting_deck);
}

void Game::addPlayer(Player* player) {
    players.push_back(player);
    connect(this, &Game::take_action, player, &Player::take_action);
    connect(player, &Player::play_card, this, &Game::play_card);
    connect(player, &Player::give_card, this, &Game::give_card);
    connect(player, &Player::pass_turn, this, &Game::pass_turn);
    player->setBoard(&mBoard);
    mDealer.addDeck(player->getDeck());
    mSize++;
}

void Game::setup() {
    mDealer.deal();
}

void Game::start() {
    std::cout << "\n-----------------------The Seven of Clubs-----------------------\n";
    next_turn();
}

void Game::clean() {
    std::cout << "-----------------------the seven of clubs-----------------------\n\n";
    Deck cleaned_cards = mBoard.clean();
    mDealer.addCards(cleaned_cards);
}


void Game::play_card(Card card, bool continues) {

    std::cout << current_player->getName().toStdString();
    std::cout << " played ";
    std::cout << card.id().toStdString();
    std::cout << ".\n";

    if (continues) {
        std::cout << current_player->getName().toStdString();
        std::cout << " will continue.\n";
    }

    // player wants to play a card
    mBoard.playCard(card, *current_player->getDeck());

    if ((card.getRank() == ace or card.getRank() == king) and continues) {
        emit take_action(current_player, play);
    }
    next_turn();
}

void Game::give_card(Card card) {

    std::cout << current_player->getName().toStdString();
    std::cout << " took a card form ";
    std::cout << last_player->getName().toStdString();
    std::cout << ".\n";
    // player will be given a card by the last player
    last_player->getDeck()->put(card, *current_player->getDeck());
    next_turn();
}

void Game::pass_turn() {

    std::cout << current_player->getName().toStdString();
    std::cout << " passed.\n";

    if (mBoard.isEmpty()) {
        next_turn();
    } else {
        emit take_action(last_player, give);
    }

}

void Game::next_turn() {

    mBoard.print();

    Player* winner = check_win();
    if (winner != nullptr) {
        winner->incrementWins();
        for (Player* player : players) player->incrementGame();
        std::cout << current_player->getName().toStdString();
        std::cout << " has won the game.\n";
        emit victory(winner);

    } else {
        mTurn++;
        this->last_player = current_player;
        this->current_player = players[(mTurn-1) % mSize];
        emit take_action(current_player, play);
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

bool questionPrompt(std::string prompt) {
    bool answered = false;
    while(!answered) {
        std::cout << prompt << " (y/n): ";
        std::string input;
        std::cin >> input;
        if (input == "Y" || input == "y") return true;
        if (input == "N" || input == "n") return false;
    }
}

int numberPrompt(std::string prompt) {
    bool answered = false;
    while(!answered) {
        std::cout << prompt << " ";
        std::string input;
        std::cin >> input;
        return stoi(input);
    }
}
