#include "headers/games/machineplayer.h"

Card MachinePlayer::play_card(Board &board) {

    if (settings.slow_play) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1500ms);
    }

    // Figure our what cards can be played
    std::vector<Card> options = findOptions(hand, board);
    // Return joker as a sign of passing.
    if (options.empty()) return Card(joker, -1);

    Card choice;
    update_playing_scores();
    sort(options.begin(), options.end(),
         [this](const Card &a, const Card &b) -> bool
     {
         return playing_scores[a] < playing_scores[b];
     });

    choice = options.front();

    // If every card fits go into ending mode
    if (canFinish(hand, board)) {
        std::vector<Card> aces = hand.filter(ace);
        std::vector<Card> kings = hand.filter(king);
        if (!aces.empty()) choice = aces.front();
        if (!kings.empty()) choice = kings.front();
    }

    Deck* target = board.getOptions(choice).front();
    hand.put(choice, *target);

    return choice;

}

Card MachinePlayer::give_card(Player &player, const Board &board) {

    update_giving_scores(board);
    std::vector<Card> options = hand.toVector();
    sort(options.begin(), options.end(),
        [this](const Card &a, const Card &b) -> bool
    {
        return giving_scores[a] < giving_scores[b];
    });

    Card choice = options.front();
    hand.put(choice, *player.getDeck());
    return choice;
}

bool MachinePlayer::will_continue(const Board &board) {
    return canFinish(hand, board);
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
 *
 */

void MachinePlayer::update_playing_scores() {
    // Best card - Highest score -> tries to keep in hand
    std::map<Card, int> new_card_scores;

    for (Card card : this->hand.toVector()) {
        int score = scoreCardForPlay(card, this->hand);
        //qDebug() << "Scored: " << card.id() << ": " << score;
        new_card_scores[card] = score;
    }
    this->playing_scores = new_card_scores;
}

void MachinePlayer::update_giving_scores(const Board &board) {
    std::map<Card, int> new_card_scores;
    for (Card card : hand.toVector()) {
        int score = scoreCardForGive(card, hand, board);
        new_card_scores[card] = score;
    }
    this->giving_scores = new_card_scores;
}

int scoreCardForPlay(const Card &card, const Deck &deck) {
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
        score -= holeDistance(card, Card(suit, lowest_rank), low_ranks, suit_cards) * HAND_HOLE_WEIGHT;
    }
    // [K, Q, J, X, 9, 8, 6, 7]
    if (rank >= 6) {
        int high_ranks[9] {13, 12, 11, 10, 9, 8, 6, 7, -1}; // -1 is to tell loop to stop
        score -= holeDistance(card, Card(suit, highest_rank), high_ranks, suit_cards) * HAND_HOLE_WEIGHT;
    }

    // LOCK SCORING: (More cards kept locked by a card -> higher score -> only played when forced)
    if (rank == highest_rank && rank > 7) {
        score += (13 - rank) * CARD_LOCK_WEIGHT;
    }
    if (rank == lowest_rank && rank < 7) {
        score += (rank - 1) * CARD_LOCK_WEIGHT;
    }
    return score;
}

int scoreCardForGive(const Card &card, const Deck &deck, const Board &board) {
    int score = 0;
    int rank = card.getRank();
    Suit suit = card.getSuit();

    std::vector<Card> suit_cards = deck.filter(suit);
    // The other cards in this suit will be compared for holes against these.
    int highest_rank = highestRank(suit_cards);
    int lowest_rank = lowestRank(suit_cards);

    // PLAYABILITY SCORING:
    if (rank != highest_rank && rank != lowest_rank) return 99; // dont give anything but your end cards.

    if (rank == ace || rank == king) score += END_CARD_AFFINITY; // giving aces or kings is disincentivezed

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
        score -= holeDistance(card, Card(suit, lowest_rank), low_ranks, suit_board) * BOARD_HOLE_WEIGHT;
    }
    // [K, Q, J, X, 9, 8, 6, 7]
    if (rank >= 6) {
        int high_ranks[9] {13, 12, 11, 10, 9, 8, 6, 7, -1}; // -1 is to tell loop to stop
        score -= holeDistance(card, Card(suit, highest_rank), high_ranks, suit_board) * BOARD_HOLE_WEIGHT;
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


