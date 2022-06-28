#ifndef BOARD_H
#define BOARD_H

#include "deck.h"


struct SuitComponent {
    Deck higher, seven, lower;
};

class Board
{
public:
    Board();

    void print() const;
    bool canPlay(const Card &card);
    bool isEmpty() const;
    // Finds the decks in which the player is allowed to place a specific card
    std::vector<Deck*> getOptions(const Card &card);

private:
    SuitComponent club, diamond, heart, spade;
    std::vector<SuitComponent*> suitComponents { &club, &diamond, &heart, &spade };
};

#endif // BOARD_H
