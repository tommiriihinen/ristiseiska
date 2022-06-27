#ifndef HUMANPLAYER_H
#define HUMANPLAYER_H

#include "player.h"
#include "game.h"
#include "board.h"

class HumanPlayer: public Player
{
public:
    HumanPlayer();

    Card_event play_card(Board* board);
    void give_card(Player* player);
    bool will_continue();

};

#endif // HUMANPLAYER_H
