#include <QCoreApplication>
#include <iostream>

#include "application.h"
#include "game_core/game.h"
#include "players/humanplayer.h"
#include "players/machineplayer.h"
#include "players/playerfactory.h"


int HUMAN_PLAYERS = 1;
int MACHINE_PLAYERS = 2;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    std::cout << QCoreApplication::applicationDirPath().toStdString();

    Application application;
    application.start();

    return a.exec();
}
