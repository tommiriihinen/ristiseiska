#include "machineplayer.h"

void MachinePlayer::setSettings(MISettings s) {
    this->mSettings = s;
}

void MachinePlayer::take_action(IPlayer &player, GameAction action) {

    if (this != &player) return;

    if (mSettings.slow_play) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1500ms);
    }

    Card choice;
    bool continues = false;
    switch (action) {
    case play:
        choice = choosePlay();

        if (choice.getSuit() == none) {
            emit pass_turn();
            return;
        }
        if (choice.getRank() == ace or choice.getRank() == king) {
            continues = chooseContinue();
        }
        emit play_card(choice, continues);
        break;

    case give:
        choice = chooseGive();
        emit give_card(choice);
        break;
    }
}


Card MachinePlayer::choosePlay() {

    // Figure our what cards can be played
    std::vector<Card> options = findOptions(mHand, *board);
    // If nothing fits, pass
    if (options.empty()) return Card(none, -1);

    // Choose best option
    Card choice;
    update_playing_scores();
    sort(options.begin(), options.end(),
         [this](const Card &a, const Card &b) -> bool
     {
         return playing_scores[a] < playing_scores[b];
     });
    choice = options.front();

    // If can finish this turn first choose an ace or king
    if (canFinish(mHand, *board)) {
        std::vector<Card> aces = mHand.filter(ace);
        std::vector<Card> kings = mHand.filter(king);
        if (!aces.empty()) choice = aces.front();
        if (!kings.empty()) choice = kings.front();
    }
    return choice;
}

Card MachinePlayer::chooseGive() {

    Card choice;
    update_giving_scores(*board);
    std::vector<Card> options = mHand.toVector();
    sort(options.begin(), options.end(),
        [this](const Card &a, const Card &b) -> bool
    {
        return giving_scores[a] < giving_scores[b];
    });
    choice = options.front();
    return choice;
}

bool MachinePlayer::chooseContinue() {
    return canFinish(mHand, *board);
}

/* PRINCIPLES:
 *
 * PLACING CARDS: Done
 * 1. tries to keep the largest holes in it's hand most open on the table.
 * 1.1. everything else is kept closed.
 * 1.2. the more cards it can keep closed with a single card, the better.
 *
 * GIVING CARDS: Done
 * 2. gives the cards that have the lowest potential for playing on the current board
 *
 * CONTINUING: Done
 * 3. if able to win
 *
 * AWARENESS OF OTHER PLAYERS:
 * -none
 *
 * STRATEGY
 * 4.1. you have only one bad card in suit. Keep the suit closed and try to give the card.
 *
 * PROBLEMS:
 * - Doesn't know the value of holding 7 and maybe 6 & 8 in some cases.
 * - Will think to give cards that are part of a series.
 *
 */

void MachinePlayer::update_playing_scores() {
    // Best card - Highest score -> tries to keep in hand
    std::map<Card, int> new_card_scores;

    for (Card card : mHand.toVector()) {
        int score = scoreCardForPlay(card, mHand, mSettings.weights);
        new_card_scores[card] = score;
    }
    this->playing_scores = new_card_scores;
}

void MachinePlayer::update_giving_scores(const Board &board) {
    std::map<Card, int> new_card_scores;
    for (Card card : mHand.toVector()) {
        int score = scoreCardForGive(card, mHand, board, mSettings.weights);
        new_card_scores[card] = score;
    }
    this->giving_scores = new_card_scores;
}

int MachinePlayer::scoreCardForPlay(const Card &card, const Deck &deck, const MIWeights &w) {
    int score = 0;
    int rank = card.getRank();
    Suit suit = card.getSuit();

    std::vector<Card> suit_cards = deck.filter(suit);

    // The other cards in this suit will be compared for holes against these.
    int highest_rank = highestRank(suit_cards);
    int lowest_rank = lowestRank(suit_cards);

    // HOLE SCORING: (More holes before card -> lower score -> quicker play)
    // [A, 2, 3, 4, 5, 8, 6, 7]
    if (rank <= 8) {
        int low_ranks[9] {1, 2, 3, 4, 5, 8, 6, 7, -1}; // -1 is to tell loop to stop
        score -= holeDistance(card, Card(suit, lowest_rank), low_ranks, suit_cards) * w.hand_hole_weight;
    }
    // [K, Q, J, X, 9, 8, 6, 7]
    if (rank >= 6) {
        int high_ranks[9] {13, 12, 11, 10, 9, 8, 6, 7, -1}; // -1 is to tell loop to stop
        score -= holeDistance(card, Card(suit, highest_rank), high_ranks, suit_cards) * w.hand_hole_weight;
    }

    // LOCK SCORING: (More cards kept locked by a card -> higher score -> only played when forced)
    if (rank == highest_rank && rank > 7) {
        score += (13 - rank) * w.card_lock_weigth;
    }
    if (rank == lowest_rank && rank < 7) {
        score += (rank - 1) * w.card_lock_weigth;
    }
    return score;
}

int MachinePlayer::scoreCardForGive(const Card &card, const Deck &deck, const Board &board, const MIWeights &w) {
    int score = 0;
    int rank = card.getRank();
    Suit suit = card.getSuit();

    std::vector<Card> suit_cards = deck.filter(suit);
    // The other cards in this suit will be compared for holes against these.
    int highest_rank = highestRank(suit_cards);
    int lowest_rank = lowestRank(suit_cards);

    // PLAYABILITY SCORING:
    if (rank != highest_rank && rank != lowest_rank) return 99; // dont give anything but your end cards.

    if (rank == ace || rank == king) score += w.end_card_affinity; // giving aces or kings is disincentivezed

    std::vector<Card> suit_board = board.getSuit(suit);
    if (suit_board.empty()) {
        highest_rank = 7;
        lowest_rank = 7;
        score += -1;
    } else {
        highest_rank = highestRank(suit_board);
        lowest_rank = lowestRank(suit_board);
    }

    // [A, 2, 3, 4, 5, 8, 6, 7]
    if (rank <= 8) {
        int low_ranks[9] {1, 2, 3, 4, 5, 8, 6, 7, -1}; // -1 is to tell loop to stop
        score -= holeDistance(card, Card(suit, lowest_rank), low_ranks, suit_board) * w.board_hole_weight;
    }
    // [K, Q, J, X, 9, 8, 6, 7]
    if (rank >= 6) {
        int high_ranks[9] {13, 12, 11, 10, 9, 8, 6, 7, -1}; // -1 is to tell loop to stop
        score -= holeDistance(card, Card(suit, highest_rank), high_ranks, suit_board) * w.board_hole_weight;
    }
    return score;
}

int holeDistance(Card begin, Card end, int sequence[], const Deck &pure_suit_deck) {
    Suit suit = begin.getSuit();
    int rank = begin.getRank();
    int end_rank = end.getRank();
    int distance = 0;
    int i = -1;
    while (sequence[i+1] != -1) { // go trough the sequence. stop when next element is -1.
        i++;
        if (!isBetween(sequence[i], rank, end_rank)) continue; // skip when counter is not between the cards
        if (!pure_suit_deck.contains(Card(suit, sequence[i]))) distance++; // count when there is a hole
    }
    return distance;
}

int highestRank(std::vector<Card> const &cards) {
    int highest = 0;
    for (Card card : cards) {
        int rank = card.getRank();
        if (rank > highest) highest = rank;
    }
    return highest;
}

int lowestRank(std::vector<Card> const &cards) {
    int lowest = 13;
    for (Card card : cards) {
        int rank = card.getRank();
        if (rank < lowest) lowest = rank;
    }
    return lowest;
}

bool isBetween(int n, int a, int b) {
    if (a > b) std::swap(a, b);
    return n > a && n < b;
}

bool canFinish(const Deck &hand, const Board &board) {

    int fitting_end_cards = 0, fitting_cards = 0;
    for (Card card : hand.toVector()) {
        if (board.canPlay(card)) {
            if (card.getRank() == ace || card.getRank() == king) {
                fitting_end_cards++;
            }
            fitting_cards++;
        }
    }
    if (fitting_end_cards >= hand.size() - 1 && fitting_cards == hand.size()) return true;
    return false;
}

MISettings MachinePlayer::askSettings() {
    MISettings s;
    s.slow_play = Util::questionPrompt("Slow play");
    s.finishing = Util::questionPrompt("Finishing");
    if (Util::questionPrompt("Change weights")) {
        MIWeights &w = s.weights;
        w.board_hole_weight = Util::numberPrompt("board_hole_weight (default 1)");
        w.hand_hole_weight  = Util::numberPrompt("hand_hole_weight  (default 1)");
        w.card_lock_weigth  = Util::numberPrompt("card_lock_weigth  (default 2)");
        w.end_card_affinity = Util::numberPrompt("end_card_affinity (default 1)");
    }
    return s;
}
