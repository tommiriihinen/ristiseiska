#include "src/games/board.h"

Board::Board()
{
    qDebug() << "A board was created.";
}

bool Board::canPlay(const Card &card) const {
    return !this->getOptions(card).empty();
//    Suit suit = card.getSuit();
//    int rank = card.getRank();
//    SuitComponent* sc = mSuitComponents[suit];

//    if (this->isEmpty() and !(suit == clubs and rank == 7)) return false;

//    if (sc->lower.contains(Card(suit, rank+1))) return true;
//    if (sc->higher.contains(Card(suit, rank-1))) return true;

//    if (rank == 7 and sc->seven.isEmpty()) return true;
//    if (rank == 6 and sc->lower.isEmpty()) return true;
//    if (rank == 8 and sc->higher.isEmpty()) return true;
}


std::vector<Deck*> Board::getOptions(const Card &card) const {
    std::vector<Deck*> options;

    Suit suit = card.getSuit();
    int rank = card.getRank();
    SuitComponent* sc = mSuitComponents[suit];

    if (this->isEmpty()) {
        if (!(suit == clubs && rank == 7)) return options;
    }

    if (rank == 7) options.push_back(&sc->seven);
    if (sc->seven.isEmpty()) return options;
    if (rank == 6) options.push_back(&sc->lower);
    if (sc->lower.isEmpty()) return options;
    if (rank == 8) options.push_back(&sc->higher);
    if (sc->higher.isEmpty()) return options;

    if (sc->lower.contains(Card(suit, rank+1))) options.push_back(&sc->lower);
    if (sc->higher.contains(Card(suit, rank-1))) options.push_back(&sc->higher);

    return options;
}

std::vector<Card> Board::getSuit(Suit suit) const {
    std::vector<Card> cards;
    SuitComponent* sc = mSuitComponents[suit];
    for (Card card : sc->higher.toVector()) cards.push_back(card);
    for (Card card : sc->seven.toVector()) cards.push_back(card);
    for (Card card : sc->lower.toVector()) cards.push_back(card);
    return cards;
}

bool Board::isEmpty() const {
    bool b = true;
    for (SuitComponent* sc : mSuitComponents) {
        if (!sc->higher.isEmpty()) b = false;
        if (!sc->seven.isEmpty()) b = false;
        if (!sc->lower.isEmpty()) b = false;
    }
    return b;
}

void Board::print() const {
    std::cout << this->toString().toStdString();
}


Deck Board::clean() {
    Deck cleaning_deck;
    for (SuitComponent* sc : mSuitComponents) {
        sc->higher.transfer(cleaning_deck);
        sc->seven.transfer(cleaning_deck);
        sc->lower.transfer(cleaning_deck);
    }
    return cleaning_deck;
}

QString Board::toString() const {
    std::string string;
    std::string top_row, mid_row, bot_row;
    for (SuitComponent* sc : mSuitComponents) {
        if (sc->higher.isEmpty()) top_row.append("  |");
        else top_row.append(sc->higher.topCard().id().toStdString().append("|"));

        if (sc->seven.isEmpty()) mid_row.append("  |");
        else mid_row.append(sc->seven.topCard().id().toStdString().append("|"));

        if (sc->lower.isEmpty()) bot_row.append("  |");
        else bot_row.append(sc->lower.topCard().id().toStdString().append("|"));
    }
    string.append(" +--+--+--+--+\n");
    string.append(" |"+top_row+ "\n");
    string.append(" |"+mid_row+ "\n");
    string.append(" |"+bot_row+ "\n");
    string.append(" +--+--+--+--+\n");
    return QString::fromStdString(string);
}

void Board::playCard(Card &card, Deck &deck) {
    int rank = card.getRank();
    Suit suit = card.getSuit();
    SuitComponent* sc = mSuitComponents[suit];

    Deck* target = nullptr;
    if (rank >= 8) target = &sc->higher;
    else if (rank <= 6) target = &sc->lower;
    else if (rank == 7) target = &sc->seven;
    deck.put(card, *target);

}
