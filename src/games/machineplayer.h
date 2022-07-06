#ifndef MACHINEPLAYER_H
#define MACHINEPLAYER_H

#include <chrono> // std::chrono::microseconds
#include <thread> // std::this_thread::sleep_for

#include "src/games/player.h"

const int HAND_HOLE_WEIGHT = 1;
const int CARD_LOCK_WEIGHT = 1;
const int BOARD_HOLE_WEIGHT = 2;
const int END_CARD_AFFINITY = 1;

struct Settings {
    bool slow_play { false };
    bool finishing { true };
    bool modelopponents { true };
};

class MachinePlayer: public Player
{
    Q_OBJECT
public:
    
    explicit MachinePlayer(bool slow, QObject *parent = nullptr);

public slots:
    void take_action(Player* player, GameAction action) override;

private:
    Card choosePlay();
    Card chooseGive();
    bool chooseContinue();

    std::map<Card, int> playing_scores, giving_scores;
    Settings mSettings;

    void update_playing_scores();
    void update_giving_scores(const Board &board);

};

int scoreCardForPlay(const Card &card, const Deck &deck);
int scoreCardForGive(const Card &card, const Deck &deck, const Board &board);
int holeDistance(Card begin, Card end, int rank_order[], const Deck &pure_suit_deck);
int highestRank(std::vector<Card> const &cards);
int lowestRank(std::vector<Card> const &cards);
bool isBetween(int n, int a, int b);
bool canFinish(const Deck &deck, const Board &board);



#endif // MACHINEPLAYER_H
