#include "player.h"

void Player::game_ended(Player* winner) {
    if (this == winner) mWins++;
    mGames++;
}

std::vector<Card> findOptions(Deck &deck, Board &board) {
    std::vector<Card> cards = deck.toVector();
    std::vector<Card> options;
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
