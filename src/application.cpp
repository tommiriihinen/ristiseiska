#include "application.h"

Application::Application(QObject *parent)
    : QObject{parent}, mGame(), mUI(), mPlayerFactory(), mDataWriter(), mBenchmarker(mGame, mPlayerFactory)
{
    // players ready
    connect(&mPlayerFactory, &PlayerFactory::allPlayersReady, this, &Application::playersReady);
    // game start
    connect(this, &Application::startGame, &mGame, &Game::start);
    // game started
    connect(&mGame, &Game::started, &mDataWriter, &DataWriter::gameStarted);
    // game end
    connect(&mGame, &Game::victory, &mDataWriter, &DataWriter::gameEnded);
    connect(&mGame, &Game::victory, this, &Application::gameEnded, Qt::QueuedConnection);
    connect(&mGame, &Game::victory, &mBenchmarker, &Benchmarker::gameEnded, Qt::QueuedConnection);
    // benchmarker
    connect(&mBenchmarker, &Benchmarker::benchmarkComplete, this, &Application::benchmarkEnded);
    connect(&mBenchmarker, &Benchmarker::startGame, this, &Application::startGame);

    mUI.setGame(&mGame);
    mDataWriter.connectGame(&mGame);
}

void Application::start() {

    std::cout << "Ristiseiska Server running on c++14\n";
    mPlayerFactory.createPlayers(askPlayers(), mGame);
}

void Application::playersReady() {

    if (mState == AppState::player_creation) {
        menu();
        return;
    }
    emit startGame();
}

void Application::gameEnded(IPlayer &winner) {
    mGamesTotal++;

    switch (mState) {

    case AppState::auto_game:
        mGamesLeft--;
        if (mGamesLeft > 0) {
            std::cout << "Games left: " << mGamesLeft << "\n";
            emit startGame();
            break;
        }
        // When games ready proceed same as in single game
    case AppState::single_game:
        printScores();
        menu();
        break;

    case AppState::benchmark:
        break;

    default:
        Q_ASSERT("Game ended while application is in bad state");
    }
}

void Application::benchmarkEnded(Benchmark bm) {
    if (mDataWriter.isOpen()) mDataWriter.addMetadata(bm);
    menu();
}

void Application::menu() {
    mState = AppState::menu;

    QString header          = "---------Game Menu--------\n";
    QString single_game     = "P: Play a single game\n";
    QString auto_game       = "M: Play multiple games\n";
    QString saving          = "S: New save\n";
    QString change_players  = "C: Change players\n";
    QString benchmarking    = "B: Benchmark\n";
    QString change_settings = "T: Change settings\n";
    QString quitting        = "Q: Quit\n";
    QString footer          = "--------------------------\n";

    while (mState == AppState::menu) {

        if (mDataWriter.isOpen()) saving = "S: Save file\n";

        char m = Util::charPrompt(header + single_game + auto_game + saving +
                                            change_players + benchmarking + change_settings +
                                            quitting + footer);
        if (m == 'P') {
            mState = AppState::single_game;
            emit startGame();

        } else if (m == 'M') {
            mState = AppState::auto_game;
            mGamesLeft = Util::numberPrompt("How many games to play?");
            emit startGame();

        } else if (m == 'S') {
            saving_routine(); // returns

        } else if (m == 'C') {
            mState = AppState::player_creation;
            mGame.clearPlayers();
            mPlayerFactory.createPlayers(askPlayers(), mGame); // -> playersReady()

        } else if (m == 'B') {
            mState = AppState::benchmark;
            benchmark_routine(); // -> start game

        } else if (m == 'T') {
            settings_routine(); // returns

        } else if (m == 'Q') {
            QCoreApplication::instance()->quit();
            break;
        }
    }
}

void Application::saving_routine() {

    if (!mDataWriter.isOpen()) {
        std::cout << "Creating new save file...\n";
        mDataWriter.newFile();

    } else {
        std::cout << "Saving...\n";
        mDataWriter.saveFile();
    }
}

void Application::benchmark_routine() {
    auto benchplayer = askPlayer();
    int benchmarkTarget = Util::numberPrompt("How many times to run benchmark?");

    PlayerType opponents[2];
    opponents[0] = Util::playerTypePromt("Choose opponent 1:");
    opponents[1] = Util::playerTypePromt("Choose opponent 2:");

    if (opponents[0] == PlayerType::neural or opponents[1] == PlayerType::neural) {
        QString model = Util::stringPrompt("Which model to use:");
        mPlayerFactory.setBenchmarkingModel(model);
    }

    int maxOpponents = Util::numberPrompt("Maximum number of players in game (3-7)");

    std::cout << "Benchmarking: " << benchplayer->getName().toStdString() << "\n";
    mBenchmarker.startBenchmark(benchplayer, benchmarkTarget, opponents, maxOpponents);
}

void Application::settings_routine() {
    //App settings
    bool browsing_settings = true;
    std::cout << "---------Settings---------\n";

    while (browsing_settings) {
        char r = Util::charPrompt("S: Show UI\n"
                                            "M: Change machine behavour\n"
                                            "T: Change seating rotation\n"
                                            "D: Check dealers deck\n"
                                            "Q: Quit settings\n");
        if (r == 'S') {
            mUI.setShow(Util::questionPrompt("Show UI?"));
        }

        if (r == 'M') {
            MISettings s = MachinePlayer::askSettings();
            for (auto p : mGame.getPlayers()) {
                if (auto mp = std::dynamic_pointer_cast<MachinePlayer>(p)) {
                    mp->setSettings(s);
                }
            }
        }

        if (r == 'T') {
            char t = Util::charPrompt("\n"
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

void Application::printScores() {
    std::cout << "Game " << mGamesTotal << " scores:\n";
    for (auto player : mGame.getPlayers()) {
        std::cout << player->getName().toStdString() << "'s winrate is: " << player->getWinrate()*100 << "%\n";
    }
}

std::map<PlayerType, int> Application::askPlayers() {
    std::map<PlayerType, int> order;

    bool invalid_input = true;
    while(invalid_input) {
        int total = 0;
        total += order[PlayerType::client] = Util::numberPrompt("How many client players (total max: 7, min: 3)");
        total += order[PlayerType::comptr] = Util::numberPrompt("How many robot  players (total max: 7, min: 3)");
        total += order[PlayerType::random] = Util::numberPrompt("How many random players (total max: 7, min: 3)");
        total += order[PlayerType::neural] = Util::numberPrompt("How many neural players (total max: 7, min: 3)");

        if (total > 7) std::cout << "Cannot have more than 7 players\n";
        else if (total < 3) std::cout << "Cannot have less than 3 players\n";
        else invalid_input = false;
    }
    return order;
}


pIPlayer Application::askPlayer() {
    pIPlayer choice;
    auto players = mGame.getPlayers();

    std::cout << "Players:\n";
    for (auto p : players) std::cout << "  " << p->getName().toStdString() << "\n";

    while (true) {
        std::cout << "Select a player by typing a name:\n";
        std::string input;
        std::cin >> input;
        QString name = QString::fromStdString(input);
        for (auto p : players) {
            if (p->getName() == name) {
                return p;
            }
        }
    }
}
