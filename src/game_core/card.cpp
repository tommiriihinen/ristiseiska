#include "game_core/card.h"


Card::Card(QString id) {
    std::string ids = id.toStdString();
    Suit suit = none;
    int rank = -1;
    switch (toupper(ids[0])) {
    case 'C':
        suit = clubs;
        break;
    case 'D':
        suit = diamonds;
        break;
    case 'H':
        suit = hearts;
        break;
    case 'S':
        suit = spades;
    }

    switch (toupper(ids[1])) {
    case 'A':
        rank = 1;
        break;
    case '2':
        rank = 2;
        break;
    case '3':
        rank = 3;
        break;
    case '4':
        rank = 4;
        break;
    case '5':
        rank = 5;
        break;
    case '6':
        rank = 6;
        break;
    case '7':
        rank = 7;
        break;
    case '8':
        rank = 8;
        break;
    case '9':
        rank = 9;
        break;
    case 'X':
        rank = 10;
        break;
    case 'J':
        rank = 11;
        break;
    case 'Q':
        rank = 12;
        break;
    case 'K':
        rank = 13;
        break;
    }
    this->rank = rank;
    this->suit = suit;
}

Suit Card::getSuit() const {
    return this->suit;
}

int Card::getRank() const {
    return this->rank;
}

QString Card::id(bool simple) const {
    QString id = "";

    switch (this->suit) {
    case clubs:
        if (simple) id.append("C");
        else id.append("♣"); // ♣ ♧
        break;
    case diamonds:
        if (simple) id.append("D");
        else id.append("♦"); // ♦ ♢
        break;
    case hearts:
        if (simple) id.append("H");
        else id.append("♥"); // ♥ ♡
        break;
    case spades:
        if (simple) id.append("S");
        else id.append("♠"); // ♠ ♤
        break;
    }

    switch (this->rank) {
    case 1:
        id.append("A");
        break;
    case 2:
        id.append("2");
        break;
    case 3:
        id.append("3");
        break;
    case 4:
        id.append("4");
        break;
    case 5:
        id.append("5");
        break;
    case 6:
        id.append("6");
        break;
    case 7:
        id.append("7");
        break;
    case 8:
        id.append("8");
        break;
    case 9:
        id.append("9");
        break;
    case 10:
        id.append("X");
        break;
    case 11:
        id.append("J");
        break;
    case 12:
        id.append("Q");
        break;
    case 13:
        id.append("K");
        break;
    }
    return id;
}

void Card::print() {
    std::cout << this->id().toStdString() << " ";
}
