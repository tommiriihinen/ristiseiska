
#include "humanplayer.h"


void HumanPlayer::take_action(Player* player, GameAction action) {

    if (player->getName() != this->mName) return;

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

Card HumanPlayer::choosePlay() {
    std::cout << "Your hand is: ";
    mHand.print();

    std::vector<Card> options = findOptions(mHand, *board);

    std::cout << "Your options are: ";
    for (Card card : options) std::cout << card.id().toStdString() << " "; //<< " score: " << scoreCardForPlay(card, hand)
    std::cout << "\n";

    std::string input;
    bool reading_input = true;
    while (reading_input) {
        std::cout << "Play card or [P]ass: ";
        std::cin >> input;

        if (input == "P") {
            return Card(none, -1);

        } else {
            Card card = QString::fromStdString(input);
            if (!mHand.contains(card)) {
                std::cout << input << " is not a card in your hand.\n";
                continue;
            }
            if (!board->canPlay(card)) {
                std::cout << input << " doesn't fit on the board.\n";
                continue;
            }
            return card;
        }
    }
    return Card(none, -1);
}

Card HumanPlayer::chooseGive() {
    Card card;
    std::cout << "\nYour hand is: ";
    mHand.print();
    std::cout << mName.toStdString() << " which card to give to last player: ";
//    for (Card card : hand.toVector()) {
//        std::cout << card.id().toStdString() << "\n"; // << " score: " << scoreCardForGive(card, hand, board)
//    }
    std::string input;
    bool reading_input = true;
    while (reading_input) {
        std::cin >> input;

        card = QString::fromStdString(input);
        if (!mHand.contains(card)) {
            std::cout << input << " is not a card in your hand.\n";
            continue;
        }
    }
    return card;
}

bool HumanPlayer::chooseContinue() {
    return questionPrompt("Will you continue?");
}
