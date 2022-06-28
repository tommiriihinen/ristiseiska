#ifndef GAME_H
#define GAME_H

#include "headers/logic_base/dealer.h"
#include "headers/games/board.h"
#include "headers/games/player.h"

class Game
{
public:
    Game();

    int getTurn() const;
    void addPlayer(Player* player) {players.push_back(player);}

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
    Dealer dealer;
};

#endif // GAME_H
