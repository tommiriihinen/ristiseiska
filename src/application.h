#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include <QCoreApplication>
#include "game_core/game.h"
#include "players/player.h"
#include "players/playerfactory.h"
#include "players/machineplayer.h"
#include "ui/ui.h"
#include "util/benchmarker.h"
#include "util/datawriter.h"
#include "util/util.h"

enum class AppState {single_game, auto_game, benchmark, menu, player_creation};

struct AppSettings {

};

class Application : public QObject
{
    Q_OBJECT
public:
    explicit Application(QObject *parent = nullptr);

    void start();

public slots:
    void playersReady();
    void gameEnded(IPlayer &winner);
    void benchmarkEnded(Benchmark bm);

signals:
    void startGame();

private:
    void menu();
    void saving_routine();
    void benchmark_routine();
    void settings_routine();


    void printScores();

    std::map<PlayerType, int> askPlayers();
    pIPlayer askPlayer();

    Game mGame;
    UI mUI;
    PlayerFactory mPlayerFactory;
    DataWriter mDataWriter;
    Benchmarker mBenchmarker;

    AppState mState { AppState::player_creation };
    int mGamesLeft { 0 };
    int mGamesTotal { 0 };


};

#endif // APPLICATION_H
