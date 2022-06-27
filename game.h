#ifndef GAME_H
#define GAME_H

#include "board.h"
#include "player.h"

class Game
{
public:
    Game();

    int getTurn() const;
    void addPlayer(Player* player) {players.push_back(player);}
    void clear();
    void start();
    Board* getBoard();

private:
    int turn { 0 };
    Board board;
    std::vector<Player*> players;
};

#endif // GAME_H
