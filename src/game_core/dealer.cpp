#include "src/game_core/dealer.h"

Dealer::Dealer()
{

}

void Dealer::addDeck(Deck* deck) {
    this->target_decks.push_back(deck);
}

void Dealer::removeDeck(Deck* deck) {
    target_decks.erase(std::remove(target_decks.begin(), target_decks.end(), deck));
}


void Dealer::clearDecks() {
    this->target_decks.clear();
}

void Dealer::addCards(Deck &deck) {
    deck.transfer(this->dealers_deck);
}

void Dealer::addCards(std::vector<Card> cards) {
    Deck deck = Deck(cards);
    deck.transfer(this->dealers_deck);
}

void Dealer::deal() {
    this->dealers_deck.shuffle();

    // Change who gets the first card
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(target_decks.begin(), target_decks.end(), std::default_random_engine(seed));

    int turn = 0;
    int n_targets = target_decks.size();
    while (!this->dealers_deck.isEmpty() && n_targets != 0) {
        this->dealers_deck.put(*target_decks[turn % n_targets]);
        turn++;
    }
    for (Deck* deck : target_decks) {
        deck->suitSort();
    }
}

void Dealer::gather() {
    for (Deck *deck : this->target_decks) {
        if (!deck->isEmpty()) {
            deck->transfer(this->dealers_deck);
        }
    }
}





