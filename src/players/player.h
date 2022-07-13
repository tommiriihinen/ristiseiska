#ifndef PLAYER_H
#define PLAYER_H

#include "game_core/card.h"
#include "game_core/deck.h"
#include "game_core/board.h"
#include "qstring.h"

enum class PlayerType {client, comptr, random, neural};
enum GameAction {play, give};

// Responsible for making play-decisions
class Player : public QObject
{
    Q_OBJECT
public:
    Player(QObject *parent = nullptr) {}

    void setName(QString name) {mName = name;}
    void setBoard(Board* board) {this->board = board;}
    void resetStats() {mWins = 0; mGames = 0;}

    Deck* getDeck() {return &mHand;}
    QString getName() const {return mName;}
    double getWinrate() const {return (double) mWins/mGames;}

    template<class T>
    bool operator==(const T &p) const {
        return this == dynamic_cast<Player*>(&p);
    }

signals:
    void play_card(Card card, bool continues);
    void give_card(Card card);
    void pass_turn();

public slots:
    virtual void take_action(Player* player, GameAction action) = 0;
    //virtual void turn_switch(Player* current_player);
    virtual void game_ended(Player* winner);

protected:
    Board* board;
    Deck mHand;
    QString mName { "null" };
    int mWins { 0 };
    int mGames { 0 };

public:

};

std::vector<Card> findOptions(Deck &deck, Board &board);
bool canPass(Deck &deck, Board &board);

#endif // PLAYER_H
