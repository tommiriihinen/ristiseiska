#ifndef CARD_H
#define CARD_H

#include <iostream>
#include <string>
#include "qstring.h"
#include "qdebug.h"


enum Suit { clubs, diamonds, hearts, spades, joker };
enum Rank { ace = 1, jack = 11, queen = 12, king = 13  };

class Card
{
public:
    Card(Suit suit, int rank) {
        this->suit = suit;
        this->rank = rank;
    }
    Card(QString id);

    Suit  getSuit() const;
    int getRank() const ;

    QString id() const;
    void print();

    bool operator ==(const Card &c) {
        if (this->suit == c.suit && this->rank == c.rank) return true;
        else return false;
    }
    bool operator <(const Card &c) const {
        if (rank == c.rank) {
            return suit < c.suit;
        }
        return rank < c.rank;
    }

private:
    Suit suit { clubs };
    int rank { 7 };



};

#endif // CARD_H

