#ifndef BENCHMARKER_H
#define BENCHMARKER_H

#include "src/game_core/game.h"
#include "src/players/player.h"
#include "src/players/playerfactory.h"
#include <QObject>

struct Benchmark {
    double comptrs[7] = { 0 };
    double randoms[7] = { 0 };

    QString toString();
};

class Benchmarker : public QObject
{
    Q_OBJECT
public:
    explicit Benchmarker(Game &game, PlayerFactory &p, QObject *parent = nullptr);

    void startBenchmark(pIPlayer player, int benchmarkTarget);

signals:
    void startGame();
    void benchmarkComplete(Benchmark bm);

public slots:
    void gameEnded(IPlayer &winner);


private:
    void startSpecificBenchmark(PlayerType oppt, int opponents);
    void revertGameBack();

    int mBenchmarkStep;
    int mBenchmarkTarget;
    int mGamesPlayed;

    pIPlayer mBenchplayer { nullptr };
    Benchmark mLatestBenchmark;
    PlayerType mCurrentOpponent;

    Game* game;
    PlayerFactory* pf;

    std::vector<pIPlayer> mOriginalPlayers;
    GameSettings mOriginalSettings;
};

#endif // BENCHMARKER_H
