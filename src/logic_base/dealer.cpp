#include "dealer.h"

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


void Dealer::deal() {
    for (Deck *deck : this->target_decks) {
        if (!deck->isEmpty()) {
            deck->print();
            deck->transfer(this->dealers_deck);
        }
    }
    std::cout << "Dealer shuffles.\n";
    this->dealers_deck.shuffle();
    this->dealers_deck.print();
    std::cout << "Dealer deals.\n";

    int turn = 0;
    int n_targets = target_decks.size();
    while (!this->dealers_deck.isEmpty()) {
        this->dealers_deck.put(*target_decks[turn % n_targets]);
        turn++;
        for (Deck* deck : target_decks) {
            deck->print();
        }
    }
    for (Deck* deck : target_decks) {
        deck->suitSort();
        deck->print();
    }




}




