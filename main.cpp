#include <QCoreApplication>
#include <iostream>

#include "src/application.h"
#include "src/game_core/game.h"
#include "src/players/humanplayer.h"
#include "src/players/machineplayer.h"
#include "src/players/playerfactory.h"


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
