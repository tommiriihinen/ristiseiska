#include "src/logic_base/dealer.h"

Dealer::Dealer()
{

}

void Dealer::addDeck(Deck* deck) {
    this->target_decks.push_back(deck);
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

    for (Deck *deck : this->target_decks) {
        if (!deck->isEmpty()) {
            deck->transfer(this->dealers_deck);
        }
    }
    std::cout << "Dealer shuffles.\n";
    this->dealers_deck.shuffle();
    std::cout << "Dealer deals.\n";

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





