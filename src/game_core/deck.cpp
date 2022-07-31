#include "src/game_core/deck.h"


void Deck::fill() {
    std::vector<Suit> suits{ clubs, diamonds, hearts, spades};
    std::vector<int> ranks{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    for (Suit suit : suits) {
        for (int rank : ranks) {
            this->cards.push_back(Card(suit, rank));
        }
    }
}

bool Deck::isEmpty() {
    return this->cards.empty();
}

bool Deck::contains(Card card) const {
    for (Card deck_card : this->cards) {
        if (deck_card == card) return true;
    }
    return false;
}

bool Deck::contains(QString id) const {
    for (Card deck_card : this->cards) {
        if (deck_card.id() == id) {
            return true;
        }
    }
    return false;
}

int Deck::size() const {
    return this->cards.size();
}

std::vector<Card> Deck::filter(Suit suit) const {
    std::vector<Card> filtrate;
    for (Card card : this->cards) {
        if (card.getSuit() == suit) {
            filtrate.push_back(card);
        }
    }
    return filtrate;
}

std::vector<Card> Deck::filter(int rank) const {
    std::vector<Card> filtrate;
    for (Card card : this->cards) {
        if (card.getRank() == rank) {
            filtrate.push_back(card);
        }
    }
    return filtrate;
}

std::vector<Card> Deck::toVector() const {
    return this->cards;
}

Card Deck::topCard() const {
    return this->cards.back();
}

void Deck::put(const Card &card, Deck &target_deck) {
    //qDebug() << "Put: " << card.id() << " from " << this->toString() << " to " << target_deck.toString();
    assert(this->contains(card));
    assert(!target_deck.contains(card));

    // remove from this deck
    for (std::vector<Card>::iterator it = this->cards.begin() ; it != this->cards.end(); ++it) {
        if (*it == card) {
            this->cards.erase(it);
            break;
        }
    }

    // add to target
    target_deck.cards.push_back(card);
}

void Deck::put(Deck &target_deck) {
    Card top_card = this->cards[0];
    this->put(top_card, target_deck);
}


void Deck::transfer(Deck &target_deck) {
    Deck copy = Deck(*this);
    for (Card card : copy.toVector()) {
        this->put(card, target_deck);
    }
}

void Deck::swap(Deck &target_dec) {

}

void Deck::shuffle() {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(std::begin(this->cards), std::end(this->cards), std::default_random_engine(seed));
}

void Deck::suitSort() {
    std::sort(this->cards.begin(), this->cards.end(), [ ]( const Card& lhs, const Card& rhs )
    {
        if (lhs.getSuit() == rhs.getSuit()) {
           return lhs.getRank() < rhs.getRank();
        }
        return lhs.getSuit() < rhs.getSuit();
    });

}

void Deck::rankSort() {
    std::sort(this->cards.begin(), this->cards.end());
}

QString Deck::toString(bool simple) const {
    QString s;
    for (Card card : this->cards) {
        s.append(card.id(simple).append(" "));
    }
    return s;
}


void Deck::print() const {
    for (Card card : this->cards) {
        card.print();
    }
    std::cout << "cards: " << this->size() << "\n";
}

