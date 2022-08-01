#include "ui/ui.h"

void UI::setGame(Game* game) {
    this->game = game;
    connect(game, &Game::announce, this, &UI::announcements);
}

void UI::announcements(QString message, QString cmd) {
    if (mShow) {
        if (cmd == "MSG") {
            std::cout << message.toStdString() + "\n";
        }
    }
}



