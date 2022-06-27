#ifndef DECK_H
#define DECK_H

#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <random>
#include <vector>
#include "card.h"
#include "QDebug"


class Deck
{
public:
    Deck() {}

    bool isEmpty();
    bool contains(Card card);
    bool contains(QString id);
    int size();
    std::vector<Card> filter(Suit suit);
    std::vector<Card> filter(int rank);
    Card topCard() const;

    void fill();
    void put(Card card, Deck &target_deck);
    void put(Deck &target_deck);
    void transfer(Deck &target_deck);
    void swap(Deck &target_dec);

    void shuffle();
    void suitSort();
    void rankSort();

    void print();

private:
    std::vector<Card> cards;

};

#endif // DECK_H
