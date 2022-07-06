#ifndef MACHINEPLAYER_H
#define MACHINEPLAYER_H

#include <chrono> // std::chrono::microseconds
#include <thread> // std::this_thread::sleep_for

#include "player.h"
#include "src/util.h"

struct MIWeights {
    int hand_hole_weight { 1 };
    int card_lock_weigth { 1 };
    int board_hole_weight { 2 };
    int end_card_affinity { 1 };
};

struct MISettings {
    bool slow_play { false };
    bool finishing { true };
    bool modelopponents { true };
    MIWeights weights;
};

class MachinePlayer: public Player
{
    Q_OBJECT
public:
    
    explicit MachinePlayer(QObject *parent = nullptr);
    void setSettings(MISettings s);

public slots:
    void take_action(Player* player, GameAction action) override;

private:
    Card choosePlay();
    Card chooseGive();
    bool chooseContinue();

    std::map<Card, int> playing_scores, giving_scores;
    void update_playing_scores();
    void update_giving_scores(const Board &board);

    MISettings mSettings;

public:
    //Free functions just organized under one name
    static MISettings askSettings();
    static int scoreCardForPlay(const Card &card, const Deck &deck, const MIWeights &w = MIWeights());
    static int scoreCardForGive(const Card &card, const Deck &deck, const Board &board, const MIWeights &w = MIWeights());

};

int holeDistance(Card begin, Card end, int rank_order[], const Deck &pure_suit_deck);
int highestRank(std::vector<Card> const &cards);
int lowestRank(std::vector<Card> const &cards);
bool isBetween(int n, int a, int b);
bool canFinish(const Deck &deck, const Board &board);




#endif // MACHINEPLAYER_H
