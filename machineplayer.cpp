#include "machineplayer.h"

MachinePlayer::MachinePlayer()
{

}

Card_event MachinePlayer::play_card(Board &board) {

    // On an empty board only C7 can be played
    if (board.isEmpty()) {
        Card club7 = Card(clubs, 7);
        if (this->hand.contains(club7)) {
            Deck* target = board.getOptions(club7)[0];
            hand.put(club7, *target);
            std::cout << this->name.toStdString() << " plays "
                      << club7.id().toStdString() << ". Cards left: "
                      << this->hand.size() << "\n";
            return ordinary_card;
        } else {
            return no_card;
        }
    }

    // Figure our what cards can be played
    std::vector<Card> options;
    for (Card card : this->hand.toVector()) {
        if (board.canPlay(card)) {
            options.push_back(card);
        }
    }
    if (options.empty()) return no_card;

    // Choose the best card out of these
    this->update_score_map();
    sort(options.begin(), options.end(), [ this] ( const Card& lhs, const Card& rhs )
    {
       return this->card_scores[lhs] < this->card_scores[rhs];
    });

    for (Card card : options) {
        //qDebug() << "Machine wants to play " << card.id() << " with score " << card_scores[card];
    }

    Card card = options.front();
    Deck* target = board.getOptions(card)[0];
    hand.put(card, *target);
    std::cout << this->name.toStdString() << " plays "
              << card.id().toStdString() << ". Cards left: "
              << this->hand.size() << "\n";

    if (card.getRank() == 1 || card.getRank() == 13) {
        return end_card;
    }
    return ordinary_card;

}

void MachinePlayer::give_card(Player &player) {
    std::vector<Card> cards = this->hand.toVector();
    this->update_score_map();
    sort(cards.begin(), cards.end(), [ this] ( const Card& lhs, const Card& rhs )
    {
       return this->card_scores[lhs] < this->card_scores[rhs];
    });
    Card card = cards.back();
    this->hand.put(card, *player.getDeck());
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
    std::map<Card, double> new_card_scores;

    for (int s = 0; s < 4; s++) {
        // Filter only one suit
        Suit suit = (Suit) s;
        std::vector<Card> suit_cards = this->hand.filter(suit);

        int highest_rank = highestRank(suit_cards);
        int lowest_rank = lowestRank(suit_cards);


        for (Card card : suit_cards) {
            int rank = card.getRank();
            double score = 0.0;

            // Hole scoring:
            // [A, 2, 3, 4, 5, 6, 8, 7]
            if (rank <= 8) {
                int low_ranks[9] {1, 2, 3, 4, 5, 6, 8, 7, -1};
                score -= holeDistance(card, Card(suit, lowest_rank), low_ranks, suit_cards);
            }
            // [K, Q, J, X, 9, 8, 6, 7]
            if (rank >= 6) {
                int high_ranks[9] {13, 12, 11, 10, 9, 8, 6, 7, -1};
                score -= holeDistance(card, Card(suit, highest_rank), high_ranks, suit_cards);
            }

            // Lock scoring:
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

int holeDistance(Card begin, Card end, int rank_order[], const Deck &pure_suit_deck) {
    Suit suit = begin.getSuit();
    int rank = begin.getRank();
    int end_rank = end.getRank();
    int distance = 0;
    int i = -1;
    while (rank_order[i+1] != -1) {
        i++;
        if (!isBetween(rank_order[i], rank, end_rank)) {
            continue;
        }
        if (!pure_suit_deck.contains(Card(suit, rank_order[i]))) {
            distance++;
        }
    }
    return distance;
}

int highestRank(std::vector<Card> const &cards) {
    int highest = 0;
    for (Card card : cards) {
        int rank = card.getRank();
        if (rank > highest) {
            highest = rank;
        }
    }
    return highest;
}

int lowestRank(std::vector<Card> const &cards) {
    int lowest = 13;
    for (Card card : cards) {
        int rank = card.getRank();
        if (rank < lowest) {
            lowest = rank;
        }
    }
    return lowest;
}

bool isBetween(int n, int a, int b) {
    if (a > b) std::swap(a, b);
    return n > a && n < b;
}

std::vector<Card> findOptions(Deck deck, Board &board) {
    std::vector<Card> options;
    for (Card card : deck.toVector()) {
        if (board.canPlay(card)) {
            options.push_back(card);
        }
    }
}
