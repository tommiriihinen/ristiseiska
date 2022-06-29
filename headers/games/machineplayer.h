#ifndef MACHINEPLAYER_H
#define MACHINEPLAYER_H

#include "headers/games/player.h"

const int HAND_HOLE_WEIGHT = 1;
const int CARD_LOCK_WEIGHT = 1;
const int BOARD_HOLE_WEIGHT = 2;
const int END_CARD_AFFINITY = 1;

class MachinePlayer: public Player
{
public:
    MachinePlayer();

    Card play_card(Board &board);
    Card give_card(Player &player, const Board &board);
    bool will_continue(const Board &board);

private:
    std::map<Card, int> playing_scores, giving_scores;
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
