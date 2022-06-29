#ifndef BOARD_H
#define BOARD_H

#include "headers/logic_base/card.h"
#include "headers/logic_base/deck.h"
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
    bool isEmpty() const;

    // Finds the decks in which the player is allowed to place a specific card
    std::vector<Deck*> getOptions(const Card &card) const;
    std::vector<Card> getSuit(Suit suit) const;

    Deck clean();

private:
    SuitComponent club, diamond, heart, spade;
    std::vector<SuitComponent*> suitComponents { &club, &diamond, &heart, &spade };
};

#endif // BOARD_H
