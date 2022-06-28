#include "headers/games/player.h"



Player::Player()
{

}


std::vector<Card> findOptions(Deck &deck, Board &board) {
    std::vector<Card> cards = deck.toVector();
    std::vector<Card> options;
    std::copy_if (cards.begin(), cards.end(), std::back_inserter(options), [&](Card card){return board.canPlay(card);} );
    return options;
}
