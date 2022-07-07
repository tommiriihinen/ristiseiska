#include "application.h"

Application::Application(QObject *parent)
    : QObject{parent}
{
    connect(&mPlayerFactory, &PlayerFactory::allPlayersReady, this, &Application::playersReady);
    connect(&mGame, &Game::victory, this, &Application::gameEnded, Qt::QueuedConnection);
    mUI.setGame(&mGame);
}

void Application::start() {

    mPlayerFactory.createPlayers(askPlayers(), mGame);
}

void Application::playersReady() {

    switch (mState) {
    case AppState::player_creation:
        menu();
        break;

    case AppState::benchmark:
        mGame.setup();
        mGame.start();
        break;
    }
}

void Application::gameEnded(Player* winner) {
    mGame.clean();
    mGamesTotal++;

    switch (mState) {
    case AppState::single_game:
        printScores();
        menu();
        break;

    case AppState::auto_game:
        mGamesLeft--;
        if (mGamesLeft > 0) {
            std::cout << "Game: " << mGamesTotal << "\n";
            mGame.setup();
            mGame.start();
        } else {
            printScores();
            menu();
        }
        break;

    case AppState::benchmark:
        mGamesLeft--;
        if (mGamesLeft > 0) {
            mGame.setup();
            mGame.start();
        } else {
            benchmark();
        }
        break;
    }
}

void Application::menu() {
    mState = AppState::menu;

    while (mState == AppState::menu) {
        char m = Util::multipleChoicePrompt("---------Game Menu--------\n"
                                            "P: Play a single game\n"
                                            "M: Play multiple games\n"
                                            "C: Change players\n"
                                            "B: Benchmark\n"
                                            "S: Change settings\n"
                                            "Q: Quit\n"
                                            "--------------------------\n");
        if (m == 'P') {
            mState = AppState::single_game;
            mGame.setup();
            mGame.start();

        } else if (m == 'M') {
            mState = AppState::auto_game;

            mGamesLeft = Util::numberPrompt("How many games to play?");
            mGame.setup();
            mGame.start();

        } else if (m == 'C') {
            mState = AppState::player_creation;
            mGame.clearPlayers();
            mPlayerFactory.createPlayers(askPlayers(), mGame);

        } else if (m == 'B') {
            mState = AppState::benchmark;

            benchplayer = askPlayer();
            std::cout << "Benchmarking: " << benchplayer->getName().toStdString() << "\n";

            GameSettings gs;
            gs.seat_change = SeatChange::random;
            mGame.setSettings(gs);

            benchmarkStep = 0;
            benchmark();

        } else if (m == 'S') {
            changeSettings();

        } else if (m == 'Q') {
            break;
        }
    }
}

void Application::changeSettings() {
    //App settings
    bool browsing_settings = true;
    std::cout << "---------Settings---------\n";

    while (browsing_settings) {
        char r = Util::multipleChoicePrompt("S: Show UI\n"
                                            "M: Change machine behavour\n"
                                            "T: Change seating rotation\n"
                                            "D: Check dealers deck\n"
                                            "Q: Quit settings\n");
        if (r == 'S') {
            mUI.setShow(Util::questionPrompt("Show UI?"));
        }

        if (r == 'M') {
            MISettings s = MachinePlayer::askSettings();
            std::vector<Player*> players = mGame.getPlayers();
            for (Player* p : players) {
                if (MachinePlayer* mp = dynamic_cast<MachinePlayer*>(p)) {
                    mp->setSettings(s);
                }
            }
        }

        if (r == 'T') {
            char t = Util::multipleChoicePrompt("\n"
                                                "S: Random shuffle\n"
                                                "R: Round-robin\n"
                                                "O: Static order\n");
            SeatChange sc;
            if (t == 'S') sc = SeatChange::random;
            if (t == 'R') sc = SeatChange::roundrobin;
            if (t == 'O') sc = SeatChange::static_seats;
            GameSettings gs;
            gs.seat_change = sc;
            mGame.setSettings(gs);
        }
        if (r == 'D') {
            std::cout << mGame.getDealersDeck().toString().toStdString();
        }
        if (r == 'Q') {
            browsing_settings = false;
        }
    }
    std::cout << "--------------------------\n";
}

void Application::benchmark() {

    // Write data

    if (benchmarkStep == 0) {
        benchmarkData = "Comptr: ";

    } else if (benchmarkStep == 6) {
        benchmarkData += "Random: ";

    } else if (benchmarkStep == 11) {
        qDebug() << benchmarkData;
        menu();
        return;
    }

    if (benchmarkStep > 0) {
        int last_game_players = 3 + ((benchmarkStep - 1) % 5);
        double relative_score = benchplayer->getWinrate() * last_game_players;
        qDebug() << benchplayer->getWinrate()*100 << "% score: " << relative_score;
        QString newline = QString::number(last_game_players) + ": " + QString::number(relative_score, 'f', 6) + ", ";
        benchmarkData += newline;
    }

    // Prepare next round

    int opponents = 2 + (benchmarkStep % 5);
    PlayerType opponent;
    if (benchmarkStep / 5) opponent = PlayerType::random;
    else opponent = PlayerType::comptr;

    std::map<PlayerType, int> order;
    order[opponent] = opponents;

    mGame.removePlayer(benchplayer);
    mGame.clearPlayers();

    benchplayer->resetStats();

    mGamesLeft = 1000;
    benchmarkStep++;

    mGame.addPlayer(benchplayer);
    mPlayerFactory.createPlayers(order, mGame);
    // After players are ready -> start
}

void Application::printScores() {
    std::cout << "Game " << mGamesTotal << " scores:\n";
    for (Player* player : mGame.getPlayers()) {
        std::cout << player->getName().toStdString() << "'s winrate is: " << player->getWinrate()*100 << "%\n";
    }
}

std::map<PlayerType, int> Application::askPlayers() {
    std::map<PlayerType, int> order;
    order[PlayerType::client] = Util::numberPrompt("How many client players (total max: 7, min: 3)");
    order[PlayerType::comptr] = Util::numberPrompt("How many robot  players (total max: 7, min: 3)");
    order[PlayerType::random] = Util::numberPrompt("How many random players (total max: 7, min: 3)");
    order[PlayerType::neural] = Util::numberPrompt("How many neural players (total max: 7, min: 3)");
    return order;
}


Player* Application::askPlayer() {
    Player* choice;
    std::vector<Player*> players = mGame.getPlayers();

    std::cout << "Players:\n";
    for (Player* p : players) std::cout << p->getName().toStdString() << "\n";

    bool no_player = true;
    while (no_player) {
        std::string input;
        std::cin >> input;
        QString name = QString::fromStdString(input);
        for (Player* p : players) {
            if (p->getName() == name) {
                choice = p;
                no_player = false;
            }
        }
    }
    return choice;
}
