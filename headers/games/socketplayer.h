#ifndef SOCKETPLAYER_H
#define SOCKETPLAYER_H

#include "headers/games/player.h"

class SocketPlayer : public Player
{
public:
    SocketPlayer();

    Card play_card(Board &board);
    Card give_card(Player &player, const Board &board);
    virtual bool will_continue(const Board &board);

};

#endif // SOCKETPLAYER_H
