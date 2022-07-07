#include "src/ui/ui.h"

void UI::setGame(Game* game) {
    this->game = game;
    connect(game, &Game::announce, this, &UI::announcements);
}

void UI::announcements(QString message) {
    if (mShow) std::cout << message.toStdString() + "\n";
}



