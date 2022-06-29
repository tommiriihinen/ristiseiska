#ifndef GAME_H
#define GAME_H

#include "headers/logic_base/dealer.h"
#include "headers/games/board.h"
#include "headers/games/player.h"
#include "headers/ui/ui.h"

class Game
{
public:
    Game();

    int getTurn() const;
    void addPlayer(Player* player);

    // before game
    void setup();
    // start game
    void start();
    // after game
    void clean();

    Board* getBoard();

private:
    int turn { 0 };
    Board board;
    std::vector<Player*> players;
    Dealer* dealer;

    void play_turn(Player* player, Player* last_player);
    // returns the player who has won. If no one has, returns nullptr
    Player* check_win();
};

#endif // GAME_H
