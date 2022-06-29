
#include "headers/ui/ui.h"
#include "headers/games/humanplayer.h"
#include "headers/games/machineplayer.h"

HumanPlayer::HumanPlayer()
{

}

Card HumanPlayer::play_card(Board &board) {
    std::cout << "Player: "<< this->name.toStdString() << "\n";
    std::cout << "\nYour hand is: ";
    hand.print();
    std::cout << "\nYour options are: \n";
    std::vector<Card> options = findOptions(this->hand, board);
    for (Card card : options) {
        std::cout << card.id().toStdString() << "\n"; // << " score: " << scoreCardForPlay(card, hand)
    }

    std::string input;
    bool reading_input = true;
    while (reading_input) {
        std::cout << "Play card or [P]ass: ";
        std::cin >> input;

        if (input == "P") {
            return Card(joker, -1);
        }

        Card card = QString::fromStdString(input);
        if (!hand.contains(card)) {
            std::cout << input << " is not a card in your hand! ";
            continue;
        }

        if (!board.canPlay(card)) {
            std::cout << input << " doesn't fit on the board ";
            continue;
        }

        Deck* target = board.getOptions(card)[0];
        hand.put(card, *target);
        return card;
    }
}

Card HumanPlayer::give_card(Player &other_player, const Board &board) {
    Card card;
    std::cout << "\nYour hand is: ";
    hand.print();
    std::cout << this->name.toStdString() << " which card to give to " << other_player.getName().toStdString() << "?: ";
    for (Card card : hand.toVector()) {
        std::cout << card.id().toStdString()  << "\n"; //<< " score: " << scoreCardForGive(card, hand, board)
    }
    std::string input;
    bool reading_input = true;
    while (reading_input) {
        std::cin >> input;

        card = QString::fromStdString(input);
        if (!hand.contains(card)) {
            std::cout << input << " is not a card in your hand! \n";
            continue;
        }
        this->hand.put(card, *other_player.getDeck());
        reading_input = false;
    }
    return card;
}

bool HumanPlayer::will_continue(const Board &board) {
    std::string input;
    bool invalid_input = true;
    while(invalid_input) {
        return questionPrompt("Will you continue?");
    }
}
