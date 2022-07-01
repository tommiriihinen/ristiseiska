
#include "headers/games/socketplayer.h"


void SocketPlayer::take_action(Player* player, GameAction action) {
    return;
}

/* << PLAY
 * >> C8, C9, DX ... (activation order)
 * << PLAYED C9
 *
 * << GIVE
 * >> DA, HA, HK ... (activation order)
 *
 * << PLAYED D9
 * << PLAY
 * >> ...
 *
 * [n players] min 3 max 7
 * [1, 2, 3, 4, 5, 6, 7] number of cards in hand (max 18, min 0)
 * [play, give] wanted action
 * [1, ..., 52] hand
 * [1, ..., 52] board
 * [1, ..., 52] others hands
 *
 * Input Vector size = 166
 *
 * [1, ..., 52] cards
 * [continue]
 *
 * Output Vector size = 53
 *
 */
