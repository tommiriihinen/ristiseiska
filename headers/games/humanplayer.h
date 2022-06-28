#ifndef HUMANPLAYER_H
#define HUMANPLAYER_H

#include "headers/games/player.h"

class HumanPlayer: public Player
{
public:
    HumanPlayer();

    Card play_card(Board &board);
    void give_card(Player &player);
    bool will_continue();

private:
};

#endif // HUMANPLAYER_H
