#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include "src/game_core/game.h"
#include "src/players/player.h"
#include "src/players/playerfactory.h"
#include "src/players/machineplayer.h"
#include "src/ui/ui.h"
#include "src/util.h"


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

signals:

private:
    void changeSettings();

    Game mGame;
    UI mUI;
    PlayerFactory mPlayerFactory;
    int mGamesLeft = 0;
};

#endif // APPLICATION_H
