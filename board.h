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

    void print();
    bool canPlay(const Card &card);
    bool isEmpty() const;
    std::vector<Deck*> getOptions(const Card &card);

private:
    SuitComponent club, diamond, heart, spade;
    std::vector<SuitComponent*> suitComponents { &club, &diamond, &heart, &spade };
};

#endif // BOARD_H
