#include "src/ui/ui.h"

void UI::setGame(Game* game) {
    this->game = game;
    qDebug() << "";
    connect(game, &Game::announce, this, &UI::announcements);
    qDebug() << "UI connected";
}

void UI::announcements(QString message) {
    if (!mHide) std::cout << message.toStdString() + "\n";
}



