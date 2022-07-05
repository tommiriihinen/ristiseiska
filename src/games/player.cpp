#include "src/games/player.h"


void Player::game_ended(Player* winner) {
    if (winner->mName == this->mName) mWins++;
    mGames++;
}

std::vector<Card> findOptions(Deck &deck, Board &board) {
    std::vector<Card> cards = deck.toVector();
    std::vector<Card> options;
    //std::copy_if (cards.begin(), cards.end(), std::back_inserter(options), [&](Card card){return board.canPlay(card);} );
    for (Card card : cards) {
        if (board.canPlay(card)) {
            options.push_back(card);
        }
    }
    return options;
}

bool canPass(Deck &deck, Board &board) {
    return findOptions(deck, board).empty();
}
