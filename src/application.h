#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include "src/game_core/game.h"
#include "src/players/player.h"
#include "src/players/playerfactory.h"
#include "src/players/machineplayer.h"
#include "src/ui/ui.h"
#include "src/util/benchmarker.h"
#include "src/util/datawriter.h"
#include "src/util/util.h"

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
    void gameEnded(Player* winner);
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
    Player* askPlayer();

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
