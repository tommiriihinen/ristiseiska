#include "benchmarker.h"

QString Benchmark::toString() {
    QString string;
    string += "Benchmark:            \n"
              "P:  Comptr:   Random: \n";
    for (int i = 0; i < 5; i++) {
        string += QString::number(i+3)                + ":  "
                + QString::number(comptrs[i], 'f', 6) + "  "
                + QString::number(randoms[i], 'f', 6)         + "\n";
    }
    string += "                      \n"
              "Games data:           \n";
    return string;
}


Benchmarker::Benchmarker(Game* g, PlayerFactory* p, QObject *parent)
    : QObject{parent}, game{g}, pf{p}
{

}

void Benchmarker::gameEnded(Player* winner) {

    mGamesPlayed++;
    // Play until target reached
    if (mGamesPlayed < mBenchmarkTarget) {
        emit startGame();
        return;
    }

    // Calculate relative score
    int game_index = mBenchmarkStep % 5;
    int game_players = 3 + game_index;
    double relative_score = mBenchplayer->getWinrate() * game_players;

    std::cout << game_players << ": " << mBenchplayer->getWinrate()*100 << "% score: " << relative_score << "\n";

    // Save data
    if (mCurrentOpponent == PlayerType::comptr) mLatestBenchmark.comptrs[game_index] = relative_score;
    if (mCurrentOpponent == PlayerType::random) mLatestBenchmark.randoms[game_index] = relative_score;

    // Start next benchmark
    mBenchmarkStep++;
    if (mBenchmarkStep / 5 == 0) mCurrentOpponent = PlayerType::comptr;
    if (mBenchmarkStep / 5 == 1) mCurrentOpponent = PlayerType::random;

    if (mBenchmarkStep / 5 == 2) {
        // Quit benchmark
        //revertGameBack();
        //disconnect(this, &Benchmarker::startGame, game, &Game::start);
        //disconnect(game, &Game::victory, this, &Benchmarker::gameEnded);
        emit benchmarkComplete(mLatestBenchmark);
        return;
    }
    int opponents = (2 + mBenchmarkStep % 5);
    startSpecificBenchmark(mCurrentOpponent, opponents);
}


void Benchmarker::startBenchmark(Player* player, int benchmarkTarget) {
    //connect(this, &Benchmarker::startGame, game, &Game::start);
    //connect(game, &Game::victory, this, &Benchmarker::gameEnded);

    mBenchplayer = player;
    mBenchmarkTarget = benchmarkTarget;
    mBenchmarkStep = 0;

//    mOriginalSettings = game->getSettings();
//    for (Player* p : game->getPlayers()) {
//        mOriginalPlayers.push_back(p);
//        game->removePlayer(p);
//    }

    GameSettings gs;
    gs.seat_change = SeatChange::random;
    game->setSettings(gs);

    mCurrentOpponent = PlayerType::comptr;
    startSpecificBenchmark(mCurrentOpponent, 2);
}


void Benchmarker::startSpecificBenchmark(PlayerType oppt, int opponents) {
    game->removePlayer(mBenchplayer);
    game->clearPlayers();

    mBenchplayer->resetStats();
    game->addPlayer(mBenchplayer);

    mGamesPlayed = 0;

    std::map<PlayerType, int> order;
    order[oppt] = opponents;
    pf->createPlayers(order, *game); // starts when ready
}


void Benchmarker::revertGameBack() {
    game->removePlayer(mBenchplayer);
    game->clearPlayers();

    mBenchplayer->resetStats();

    game->addPlayers(mOriginalPlayers);
    game->setSettings(mOriginalSettings);
}
