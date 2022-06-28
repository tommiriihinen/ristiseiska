#ifndef PLAYER_H
#define PLAYER_H

#include "card.h"
#include "deck.h"
#include "board.h"
#include "qstring.h"

enum Card_event {ordinary_card, no_card, end_card};

class Player
{
public:
    Player();

    // plays the card on board if possible. Returns true if a card was played and false otherwise
    virtual Card_event play_card(Board &board) = 0;
    virtual void give_card(Player &player) = 0;
    virtual bool will_continue() = 0;

    Deck* getDeck() {return &this->hand;}
    void setName(QString name) {this->name = name;}
    QString getName() const {return this->name;}

protected:
    Deck hand;
    QString name;

};

#endif // PLAYER_H
