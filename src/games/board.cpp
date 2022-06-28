#include "headers/games/board.h"

Board::Board()
{

}

bool Board::canPlay(const Card &card) const {
    return !this->getOptions(card).empty();
}

std::vector<Deck*> Board::getOptions(const Card &card) const {
    std::vector<Deck*> options;

    Suit suit = card.getSuit();
    int rank = card.getRank();
    SuitComponent* sc = this->suitComponents[suit];

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

bool Board::isEmpty() const {
    bool b = true;
    for (SuitComponent* sc : this->suitComponents) {
        if (!sc->higher.isEmpty()) b = false;
        if (!sc->seven.isEmpty()) b = false;
        if (!sc->lower.isEmpty()) b = false;
    }
    return b;
}

void Board::print() const {

    std::string top_row, mid_row, bot_row;
    for (SuitComponent* sc : this->suitComponents) {
        if (sc->higher.isEmpty()) top_row.append("  |");
        else top_row.append(sc->higher.topCard().id().toStdString().append("|"));

        if (sc->seven.isEmpty()) mid_row.append("  |");
        else mid_row.append(sc->seven.topCard().id().toStdString().append("|"));

        if (sc->lower.isEmpty()) bot_row.append("  |");
        else bot_row.append(sc->lower.topCard().id().toStdString().append("|"));
    }
    std::cout << " +--+--+--+--+\n" <<
                 " |" << top_row << "\n" <<
                 " |" << mid_row << "\n" <<
                 " |" << bot_row << "\n" <<
                 " +--+--+--+--+\n";


}
