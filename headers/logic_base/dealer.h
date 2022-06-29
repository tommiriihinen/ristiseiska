#ifndef DEALER_H
#define DEALER_H

#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include "deck.h"


class Dealer
{
public:
    Dealer();

    void addDeck(Deck* deck);
    void clearDecks();
    void addCards(Deck &deck);
    void addCards(std::vector<Card> cards);
    void deal();
    void print();

private:
    std::vector<Deck*> target_decks;
    Deck dealers_deck;
    int source(int i) { return std::rand()%i;}
};


#endif // DEALER_H
