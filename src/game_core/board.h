#ifndef BOARD_H
#define BOARD_H

#include "game_core/card.h"
#include "game_core/deck.h"
#include <vector>


struct SuitComponent {
    Deck higher, seven, lower;
};

class Board
{
public:
    Board();

    void print() const;
    bool canPlay(const Card &card) const;
    void playCard(const Card &card, Deck &deck);
    bool isEmpty() const;

    // Finds the decks in which the player is allowed to place a specific card
    std::vector<Deck*> getOptions(const Card &card) const;
    std::vector<Card> getSuit(Suit suit) const;

    QString toString() const;

    Deck clean();

private:
    SuitComponent mClubs, mDiamond, mHearts, mSpades;
    std::vector<SuitComponent*> mSuitComponents { &mClubs, &mDiamond, &mHearts, &mSpades };
};

#endif // BOARD_H
