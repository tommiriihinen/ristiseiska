#include "game.h"

Game::Game()
{

}

int Game::getTurn() const {
    return this->turn;
}

Board* Game::getBoard() {
    return &this->board;
}

void Game::start() {
    this->turn = 0;
    int n = players.size();
    bool game_is_on = true;
    while(game_is_on) {
        this->turn++;
        std::cout << "\nTurn: " << turn << " ";
        Player* current_player = players[(turn-1) % n];
        end_card_replay:
        Card_event event = current_player->play_card(&this->board);

        switch (event) {
        case ordinary_card:
            break;

        case end_card:
            goto end_card_replay;
            break;

        case no_card:
            if (this->board.isEmpty()) break;
            Player* last_player = players[(turn-2) % n];
            last_player->give_card(current_player);
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
