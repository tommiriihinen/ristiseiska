#ifndef HUMANPLAYER_H
#define HUMANPLAYER_H

#include "headers/games/player.h"

class HumanPlayer: public Player
{
public:
    HumanPlayer();

    Card play_card(Board &board);
    Card give_card(Player &player, const Board &board);
    bool will_continue(const Board &board);

private:
};

#endif // HUMANPLAYER_H
