#include "randomplayer.h"


void RandomPlayer::take_action(Player* player, GameAction action) {

    if (this != player) return;

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

Card RandomPlayer::choosePlay() {

    // Figure our what cards can be played
    std::vector<Card> options = findOptions(mHand, *board);
    // If nothing fits, pass
    if (options.empty()) return Card(none, -1);

    // Choose random option
    Card choice;
    int index = genRand(options.size()-1);
    return options[index];
}

Card RandomPlayer::chooseGive() {

    Card choice;
    std::vector<Card> options = mHand.toVector();
    int index = genRand(options.size()-1);
    return options[index];
}

bool RandomPlayer::chooseContinue() {
    return genRand(1);
}

int RandomPlayer::genRand(int max) {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<int> gen(0, max); // uniform, unbiased
    return gen(rng);
}
