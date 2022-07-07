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
    void removeDeck(Deck* deck);
    void clearDecks();
    void addCards(Deck &deck);
    void addCards(std::vector<Card> cards);
    Deck getDeck() {return dealers_deck;}

    void deal();
    void gather();


private:
    Deck dealers_deck;
    std::vector<Deck*> target_decks;

    int source(int i) { return std::rand()%i;}
};


#endif // DEALER_H
