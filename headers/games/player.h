#ifndef PLAYER_H
#define PLAYER_H

#include "headers/logic_base/card.h"
#include "headers/logic_base/deck.h"
#include "headers/games/board.h"
#include "qstring.h"

enum GameAction {play, give};

// Responsible for making play-decisions
class Player : public QObject
{
    Q_OBJECT
public:
    Player(QObject *parent = nullptr) {}

    void incrementWins() {mWins++;}
    void incrementGame() {mGames++;}

    void setName(QString name) {mName = name;}
    void setBoard(Board* board) {this->board = board;}

    Deck* getDeck() {return &mHand;}
    QString getName() const {return mName;}
    double getWinrate() const {return (double) mWins/mGames;}

signals:
    void play_card(Card card, bool continues);
    void give_card(Card card);
    void pass_turn();

public slots:
    virtual void take_action(Player* player, GameAction action) = 0;

protected:
    Board* board;
    Deck mHand;
    QString mName { "null" };
    int mWins { 0 };
    int mGames { 0 };

};

std::vector<Card> findOptions(Deck &deck, Board &board);

#endif // PLAYER_H
