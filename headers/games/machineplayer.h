#ifndef MACHINEPLAYER_H
#define MACHINEPLAYER_H

#include "headers/games/player.h"

class MachinePlayer: public Player
{
public:
    MachinePlayer();

    Card play_card(Board &board);
    void give_card(Player &player);
    bool will_continue();

private:
    std::map<Card, int> playing_scores, giving_scores;
    void update_playing_scores();
    void update_giving_scores(Board &board);


};

int score_card_for_play(Card &card, Deck &deck);
int score_card_for_give(Card &card, Deck &deck, Board &board);
int holeDistance(Card begin, Card end, int rank_order[], const Deck &pure_suit_deck);
int highestRank(std::vector<Card> const &cards);
int lowestRank(std::vector<Card> const &cards);
bool isBetween(int n, int a, int b);




#endif // MACHINEPLAYER_H
