#include "headers/games/machineplayer.h"

MachinePlayer::MachinePlayer()
{

}

Card_event MachinePlayer::play_card(Board &board) {

    // Figure our what cards can be played
    std::vector<Card> options = findOptions(hand, board);
    if (options.empty()) return no_card;

    // Choose the best card out of these
    update_score_map();
    sort(options.begin(), options.end(),
         [this](const Card &a, const Card &b) -> bool
     {
         return card_scores[a] > card_scores[b];
     });

    Card card = options.front();
    Deck* target = board.getOptions(card)[0];
    hand.put(card, *target);
    std::cout << name.toStdString() << " plays "
              << card.id().toStdString() << ". Cards left: "
              << hand.size() << "\n";

    if (card.getRank() == 1 || card.getRank() == 13) return end_card;
    return ordinary_card;

}

void MachinePlayer::give_card(Player &player) {
    std::vector<Card> cards = hand.toVector();

    update_score_map();
    sort(cards.begin(), cards.end(),
         [this](const Card &a, const Card &b) -> bool
     {
         return card_scores[a] > card_scores[b];
     });

    Card card = cards.back(); // very much WIP
    hand.put(card, *player.getDeck());
}

bool MachinePlayer::will_continue() {
    return false;
}


/* PRINCIPLES:
 *
 * PLACING CARDS: Done
 * 1. tries to keep the largest holes in it's hand most open on the table.
 * 1.1. everything else is kept closed.
 * 1.2. the more cards it can keep closed with a single card, the better.
 *
 * GIVING CARDS: Done
 * 2. gives the cards it most needs to be played.
 *
 * CONTINUING: Done
 * 3. never
 *
 * AWARENESS OF OTHER PLAYERS:
 * -none
 *
 */

void MachinePlayer::update_score_map() {
    // Best card - Highest score -> tries to keep in hand
    std::map<Card, int> new_card_scores;

    // Suits are scored independently
    for (int s = 0; s < 4; s++) {
        // Filter only one suit
        Suit suit = (Suit) s;
        std::vector<Card> suit_cards = hand.filter(suit);

        // The other cards in this suit will be compared for holes against these.
        int highest_rank = highestRank(suit_cards);
        int lowest_rank = lowestRank(suit_cards);

        for (Card card : suit_cards) {
            int rank = card.getRank();
            int score = 0;

            // HOLE SCORING: (More holes before card -> lower score -> quicker play)
            // [A, 2, 3, 4, 5, 8, 6, 7]
            if (rank <= 8) {
                int low_ranks[9] {1, 2, 3, 4, 5, 8, 6, 7, -1}; // -1 is to tell loop to stop
                score -= holeDistance(card, Card(suit, lowest_rank), low_ranks, suit_cards);
            }
            // [K, Q, J, X, 9, 8, 6, 7]
            if (rank >= 6) {
                int high_ranks[9] {13, 12, 11, 10, 9, 8, 6, 7, -1}; // -1 is to tell loop to stop
                score -= holeDistance(card, Card(suit, highest_rank), high_ranks, suit_cards);
            }

            // LOCK SCORING: (Cards kept locked by a card -> higher score -> only played when forced)
            if (rank == highest_rank && rank > 7) {
                score += 13 - rank;
            }
            if (rank == lowest_rank && rank < 7) {
                score += rank - 1;
            }

            //qDebug() << "Scored: " << card.id() << ": " << score;
            new_card_scores[card] = score;
        }
    }
    this->card_scores = new_card_scores;
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


