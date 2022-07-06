#include "application.h"

Application::Application(QObject *parent)
    : QObject{parent}
{
    connect(&mPlayerFactory, &PlayerFactory::allPlayersReady, this, &Application::playersReady);
    connect(&mGame, &Game::victory, this, &Application::gameEnded);
    mUI.setGame(&mGame);
}

void Application::start() {

    mPlayerFactory.createPlayers(mGame);
}

void Application::playersReady() {
    if (mGamesLeft <= 0)  {
        if (Util::questionPrompt("Want to change settings?")) changeSettings();
        mGamesLeft = Util::numberPrompt("How many games to play?");
    }
    mGame.setup();
    mGame.start();
}

void Application::gameEnded(Player* winner) {
    mGame.clean();

    for (Player* player : mGame.getPlayers()) {
        std::cout << player->getName().toStdString() << "'s winrate is: " << player->getWinrate()*100 << "%\n";
    }

    mGamesLeft--;
    emit playersReady();
}

void Application::changeSettings() {
    //App settings
    if (Util::questionPrompt("Change general settings?")) {

    }

    //Ui settings
    if (Util::questionPrompt("Change UI settings?")) {
        mUI.setHide(Util::questionPrompt("Hide UI?"));
    }

    //Game settings
    if (Util::questionPrompt("Change game settings?")) {

        if (Util::questionPrompt("Change machine behaviour?")) {
            MISettings s = askSettings();
            std::vector<Player*> players = mGame.getPlayers();
            for (Player* p : players) {
                if (MachinePlayer* mp = dynamic_cast<MachinePlayer*>(p)) {
                    qDebug() << "Machine modified";
                    mp->setSettings(s);
                }
            }
        }
    }
}

