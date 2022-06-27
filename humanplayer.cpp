#include "humanplayer.h"

HumanPlayer::HumanPlayer()
{

}

Card_event HumanPlayer::play_card(Board* board) {
    std::cout << "Player: "<< this->name.toStdString() << "\n";
    board->print();
    std::cout << "\nYour hand is: ";
    hand.print();

    std::string input;
    bool reading_input = true;
    while (reading_input) {
        std::cout << "Play card or (P)ass: ";
        std::cin >> input;

        if (input == "P") {
            std::cout << this->name.toStdString() << " passed.\n";
            return no_card;
        }

        Card card = QString::fromStdString(input);
        if (!hand.contains(card)) {
            std::cout << input << " is not a card in your hand! ";
            continue;
        }

        if (!board->canPlay(card)) {
            std::cout << input << " doesn't fit on the board ";
            continue;
        }

        Deck* target = board->getOptions(card)[0];
        hand.put(card, *target);
        if (card.getRank() == 1 || card.getRank() == 13) {
            return end_card;
        }
        return ordinary_card;
    }
}

void HumanPlayer::give_card(Player* other_player) {
    std::cout << "\nYour hand is: ";
    hand.print();
    std::cout << this->name.toStdString() << " which card to give to " << other_player->getName().toStdString() << "?: ";
    std::string input;
    bool reading_input = true;
    while (reading_input) {
        std::cin >> input;

        Card card = QString::fromStdString(input);
        if (!hand.contains(card)) {
            std::cout << input << " is not a card in your hand! \n";
            continue;
        }
        this->hand.put(card, *other_player->getDeck());
        reading_input = false;
    }
}

bool HumanPlayer::will_continue() {
    std::string input;
    bool invalid_input = true;
    while(invalid_input) {
        std::cout << "Will you continue? (y/n)";
        std::cin >> input;
        if (input == "y") return true;
        if (input == "n") return false;

    }
}
