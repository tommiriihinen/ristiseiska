#ifndef DECK_H
#define DECK_H

#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <random>
#include <vector>
#include "QDebug"
#include "headers/logic_base/card.h"


class Deck
{
public:
    Deck() {}
    Deck(Deck const &d) {this->cards = d.cards; }
    Deck(std::vector<Card> cards) {for (Card card : cards) this->cards.push_back(card);}

    bool isEmpty();
    bool contains(Card card) const;
    bool contains(QString id) const;
    int size() const;
    std::vector<Card> filter(Suit suit) const;
    std::vector<Card> filter(int rank) const;
    std::vector<Card> toVector() const;
    Card topCard() const;

    void fill();
    void put(Card &card, Deck &target_deck);
    void put(Deck &target_deck);
    void transfer(Deck &target_deck);
    void swap(Deck &target_dec);

    void shuffle();
    void suitSort();
    void rankSort();

    void print() const;

private:
    std::vector<Card> cards;

};

#endif // DECK_H
