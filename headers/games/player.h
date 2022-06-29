#ifndef PLAYER_H
#define PLAYER_H

#include "headers/logic_base/card.h"
#include "headers/logic_base/deck.h"
#include "headers/games/board.h"
#include "qstring.h"

class Player
{
public:
    Player();

    // plays the card on board if possible. Returns a joker if couldn't play a card
    virtual Card play_card(Board &board) = 0;
    virtual Card give_card(Player &player, const Board &board) = 0;
    virtual bool will_continue(const Board &board) = 0;

    void incrementWins() {wins++;}
    void incrementGame() {games++;}
    double getWinrate() const {return (double) wins/games;}

    Deck* getDeck() {return &this->hand;}
    void setName(QString name) {this->name = name;}
    QString getName() const {return this->name;}

protected:
    Deck hand;
    QString name;
    int wins { 0 };
    int games { 0 };

};

std::vector<Card> findOptions(Deck &deck, Board &board);

#endif // PLAYER_H
