#ifndef GAME_H
#define GAME_H

#include "src/players/socketplayer.h"
#include "src/logic_base/dealer.h"
#include "src/game_core/board.h"
#include "src/players/player.h"

enum class SeatChange {static_seats, roundrobin, random};

struct GameSettings {
    SeatChange seat_change { SeatChange::random };
};

// Responsible for moving cards
class Game : public QObject
{
    Q_OBJECT
public:
    explicit Game(QObject *parent = nullptr);

    void addPlayer(Player* player);

    // before game
    void setup();
    // start game
    void start();
    // after game
    void clean();

    std::vector<Player*> getPlayers() {return this->players;}
    Player* getCurrentPlayer() {return this->current_player;}
    Player* getLastPlayer() {return this->last_player;}
    Board* getBoard() {return &this->mBoard;}
    int getTurn() {return mTurn;}

signals:
    void take_action(Player* player, GameAction action); // players connected ,server ui connected
    void victory(Player* winner); // players connected, server ui connected
    void announce(QString message); // narrates the game on this channel)
    void whisper(Player* target, QString message);

public slots:
    void play_card(Card card, bool continues);
    void give_card(Card card);
    void pass_turn();

private:
    int mTurn { 0 };
    int mSize { 0 };
    Board mBoard;
    Dealer mDealer;
    GameSettings mSettings;

    std::vector<Player*> players;
    Player* current_player, * last_player;


    void next_turn();
    // returns the player who has won. If no one has, returns nullptr
    Player* check_win();
};

#endif // GAME_H
