#include "src/ui/ui.h"

void UI::setGame(Game* game) {
    this->game = game;
    qDebug() << "";
    connect(game, &Game::announce, this, &UI::announcements);
//    connect(game, &Game::play_card, this, &UI::card_played);
//    connect(game, &Game::give_card, this, &UI::card_given);
//    connect(game, &Game::pass_turn, this, &UI::passed);
    qDebug() << "UI connected";
}

void UI::announcements(QString message) {
    std::cout << message.toStdString();
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


