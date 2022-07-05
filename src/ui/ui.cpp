#include "src/ui/ui.h"

void UI::setGame(Game* game) {
    this->game = game;
    qDebug() << "";
    //connect(game, &Game::take_action, this, &UI::action_expected);
//    connect(game, &Game::play_card, this, &UI::card_played);
//    connect(game, &Game::give_card, this, &UI::card_given);
//    connect(game, &Game::pass_turn, this, &UI::passed);
    qDebug() << "UI connected";
}

void UI::action_expected(Player* player, GameAction action) {
    qDebug() << "print";
    if (action == play) {
        std::cout << player->getName().toStdString();
        std::cout << " play a card.n";
    }
    if (action == give) {
        std::cout << game->getLastPlayer()->getName().toStdString();
        std::cout << " gives ";
        std::cout << game->getCurrentPlayer()->getName().toStdString();
        std::cout << " a card.\n";
    }
    game->getBoard()->print();
}

//void UI::card_played(Card card, bool continues) {
//    std::cout << game->getCurrentPlayer()->getName().toStdString();
//    std::cout << " played ";
//    std::cout << card.id().toStdString();
//    std::cout << ".\n";

//    if (continues) {
//        std::cout << game->getCurrentPlayer()->getName().toStdString();
//        std::cout << " will continue.\n";
//    }

//    game->getBoard()->print();
//}

//void UI::card_given(Card card) {
//    std::cout << game->getCurrentPlayer()->getName().toStdString();
//    std::cout << " took a card form ";
//    std::cout << game->getLastPlayer()->getName().toStdString();
//    std::cout << ".\n";
//}

//void UI::passed() {
//    std::cout << game->getCurrentPlayer()->getName().toStdString();
//    std::cout << " passed.\n";
//}


