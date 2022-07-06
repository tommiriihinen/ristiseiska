#include <QCoreApplication>
#include <iostream>

#include "src/application.h"
#include "src/games/game.h"
#include "src/games/humanplayer.h"
#include "src/games/machineplayer.h"
#include "src/games/playerfactory.h"


int HUMAN_PLAYERS = 1;
int MACHINE_PLAYERS = 2;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Application application;
    application.start();

    return a.exec();
}
