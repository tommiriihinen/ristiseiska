#include "deck.h"


void Deck::fill() {
    qDebug() << "A new deck\n";
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

bool Deck::contains(Card card) {
    for (Card deck_card : this->cards) {
        if (deck_card == card) return true;
    }
    return false;
}

bool Deck::contains(QString id) {
    for (Card deck_card : this->cards) {
        if (deck_card.id() == id) {
            return true;
        }
    }
    return false;
}

int Deck::size() {
    return this->cards.size();
}

Deck Deck::filter(Suit suit) const {
    Deck hand_copy = Deck(*this);
    Deck filtrate;
    for (Card card : this->cards) {
        if (card.getSuit() == suit) {
            hand_copy.put(card, filtrate);
        }
    }
    return filtrate;
}

Deck Deck::filter(int rank) const {
    Deck hand_copy = Deck(*this);
    Deck filtrate;
    for (Card card : this->cards) {
        if (card.getRank() == rank) {
            hand_copy.put(card, filtrate);
        }
    }
    return filtrate;
}

Card Deck::topCard() const {
    return this->cards.back();
}

void Deck::put(Card card, Deck &target_deck) {
    if (!this->contains(card)) throw std::invalid_argument( "starting deck doesn't have the card!" );
    if (target_deck.contains(card)) throw std::invalid_argument( "target deck already has the card!" );

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
    target_deck.cards = this->cards;
    this->cards.clear();
}

void Deck::swap(Deck &target_dec) {

}

void Deck::shuffle() {
    auto rng = std::default_random_engine {};
    std::shuffle(std::begin(this->cards), std::end(this->cards), rng);
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


void Deck::print() {
    for (Card card : this->cards) {
        card.print();
    }
    std::cout << "cards: " << this->size() << "\n";
}

