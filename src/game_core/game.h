#ifndef GAME_H
#define GAME_H

#include "players/socketplayer.h"
#include "game_core/dealer.h"
#include "game_core/board.h"
#include "players/player.h"
//#include "util/datawriter.h"

enum class SeatChange {static_seats, roundrobin, random};
enum class GameQuality {pretty, fast};

struct GameSettings {
    SeatChange seat_change { SeatChange::random };
    GameQuality game_quality { GameQuality::pretty };
};

// Responsible for moving cards
class Game : public QObject
{
    Q_OBJECT
public:
    explicit Game(QObject *parent = nullptr);

    void addPlayer(pIPlayer player);
    bool removePlayer(pIPlayer p); // Would use const ref insted, but requires dynamic_casting
    void addPlayers(std::vector<pIPlayer> players);
    void clearPlayers();

    void setSettings(GameSettings gs);

    GameSettings getSettings() const {return mSettings;}
    std::vector<pIPlayer> getPlayers() {return this->players;}
    pIPlayer getCurrentPlayer() {return this->current_player;}
    pIPlayer getLastPlayingPlayer() {return this->last_playing_player;}
    Board* getBoard() {return &this->mBoard;}
    int getTurn() {return mTurn;}
    Deck getDealersDeck() {return mDealer.getDeck();}
    bool isRunning() {return mRunning;}

signals:
    void take_action(IPlayer &player, GameAction action); // players connected ,server ui connected
    void victory(IPlayer &winner); // players connected, server ui connected

    void announce(QString message, QString command = "MSG"); // allows for commands
    void whisper(IPlayer &target, QString message, QString command = "MSG"); // messages to only one

    void started();

public slots:
    void play_card(const Card &card, const bool continues);
    void give_card(const Card &card);
    void pass_turn();
    void start();
    void stop();

private:
    bool mRunning { false };
    int mTurn { 0 };

    Board mBoard;
    Dealer mDealer;
    GameSettings mSettings;

    std::vector<pIPlayer> players;
    pIPlayer current_player, last_playing_player;
    int mConsecutivePassCounter { 0 };

    void next_turn();
    void clean();
    // returns the player who has won. If no one has, returns nullptr
    pIPlayer check_win();

    int playerIndex(pIPlayer p);
};

#endif // GAME_H
