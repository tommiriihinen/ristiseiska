#ifndef MACHINEPLAYER_H
#define MACHINEPLAYER_H

#include "headers/games/player.h"

class MachinePlayer: public Player
{
public:
    MachinePlayer();

    Card_event play_card(Board &board);
    void give_card(Player &player);
    bool will_continue();

private:
    std::map<Card, int> card_scores;
    void update_score_map();


};

int holeDistance(Card begin, Card end, int rank_order[], const Deck &pure_suit_deck);
int highestRank(std::vector<Card> const &cards);
int lowestRank(std::vector<Card> const &cards);
bool isBetween(int n, int a, int b);




#endif // MACHINEPLAYER_H
