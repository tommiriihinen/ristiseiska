#include "benchmarker.h"

QString Benchmark::toString() {

    QString string;
    string += QString("#  Benchmark:\n");
    string += QString("#    P:         Comptr:     Random:\n");
    string += QString("#    3 Players  %2    %3 \n").arg(QString::number(comptrs[0], 'f', 6)).arg(QString::number(randoms[0], 'f', 6));
    string += QString("#    4 Players  %2    %3 \n").arg(QString::number(comptrs[1], 'f', 6)).arg(QString::number(randoms[1], 'f', 6));
    string += QString("#    5 Players  %2    %3 \n").arg(QString::number(comptrs[2], 'f', 6)).arg(QString::number(randoms[2], 'f', 6));
    string += QString("#    6 Players  %2    %3 \n").arg(QString::number(comptrs[3], 'f', 6)).arg(QString::number(randoms[3], 'f', 6));
    string += QString("#    7 Players  %2    %3 \n").arg(QString::number(comptrs[4], 'f', 6)).arg(QString::number(randoms[4], 'f', 6));
    return string;
}


Benchmarker::Benchmarker(Game &g, PlayerFactory &p, QObject *parent)
    : QObject{parent}, game{&g}, pf{&p}
{

}

void Benchmarker::gameEnded(IPlayer &winner) {

    if (mBenchplayer == nullptr) return;

    mGamesPlayed++;
    // Play until target reached
    if (mGamesPlayed < mBenchmarkTarget) {

        if (mGamesPlayed % 50 == 0) {

            int barWidth = 70;
            double progress = mGamesPlayed/(double)mBenchmarkTarget;

                std::cout << "[";
                int pos = barWidth * progress;
                for (int i = 0; i < barWidth; ++i) {
                    if (i < pos) std::cout << "=";
                    else if (i == pos) std::cout << ">";
                    else std::cout << " ";
                }
                std::cout << "] " << int(progress * 100.0) << " %\r";
                std::cout.flush();
            }

        emit startGame();
        return;
    }

    std::cout << std::endl;

    // Calculate relative score
    int game_index = mBenchmarkStep % mRounds;
    int game_players = 3 + game_index;
    double relative_score = mBenchplayer->getWinrate() * game_players;

    std::cout << game->getPlayers().size() << ": " << mBenchplayer->getWinrate()*100 << "% score: " << relative_score << "\n";

    // Save data
    if (mCurrentOpponent == mOpponents[0]) mLatestBenchmark.comptrs[game_index] = relative_score;
    if (mCurrentOpponent == mOpponents[1]) mLatestBenchmark.randoms[game_index] = relative_score;

    // Start next benchmark
    mBenchmarkStep++;
    if (mBenchmarkStep / mRounds == 0) mCurrentOpponent = mOpponents[0];
    if (mBenchmarkStep / mRounds == 1) mCurrentOpponent = mOpponents[1];

    if (mBenchmarkStep / mRounds == 2) {
        // Quit benchmark
        revertGameBack();
        //disconnect(this, &Benchmarker::startGame, game, &Game::start);
        //disconnect(game, &Game::victory, this, &Benchmarker::gameEnded);
        mBenchplayer = nullptr;
        emit benchmarkComplete(mLatestBenchmark);
        return;
    }
    int opponents = (2 + mBenchmarkStep % mRounds);
    startSpecificBenchmark(mCurrentOpponent, opponents);
}


void Benchmarker::startBenchmark(pIPlayer player, int benchmarkTarget, PlayerType types[2], int maxOpponents) {
    //connect(this, &Benchmarker::startGame, game, &Game::start);
    //connect(game, &Game::victory, this, &Benchmarker::gameEnded);

    mBenchplayer = player;
    mBenchmarkTarget = benchmarkTarget;
    mOpponents[0] = types[0];
    mOpponents[1] = types[1];
    mRounds = maxOpponents - 2;
    mBenchmarkStep = 0;

    mOriginalSettings = game->getSettings();
    mOriginalPlayers = game->getPlayers();

    GameSettings gs;
    gs.seat_change = SeatChange::random;
    gs.game_quality = GameQuality::fast;
    game->setSettings(gs);

    mCurrentOpponent = mOpponents[0];
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
    pf->createPlayers(order, *game, true); // starts when ready
}


void Benchmarker::revertGameBack() {
    game->removePlayer(mBenchplayer);
    game->clearPlayers();

    mBenchplayer->resetStats();

    game->addPlayers(mOriginalPlayers);
    game->setSettings(mOriginalSettings);
}
