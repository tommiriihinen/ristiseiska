#include "application.h"

Application::Application(QObject *parent)
    : QObject{parent}
{
    connect(&mPlayerFactory, &PlayerFactory::allPlayersReady, this, &Application::playersReady);
    mUI.setGame(&mGame);
}

void Application::start() {

    mPlayerFactory.createPlayers(mGame);

}

void Application::playersReady() {
    mGame.setup();
    mGame.start();
}

void Application::gameEnded(Player* winner) {
    qDebug() << "GAME ENDED: WINNER " << winner->getName();
    mGame.clean();

    for (Player* player : mGame.getPlayers()) {
        std::cout << player->getName().toStdString() << "'s winrate is: " << player->getWinrate()*100 << "%\n";
    }

    if (questionPrompt("Want to play more?")) {
        emit playersReady();
    }
}
