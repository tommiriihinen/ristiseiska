#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include "src/game_core/game.h"
#include "src/players/player.h"
#include "src/players/playerfactory.h"


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
    Game mGame;
    UI mUI;
    PlayerFactory mPlayerFactory;

};

#endif // APPLICATION_H
