#ifndef PLAYERFACTORY_H
#define PLAYERFACTORY_H

#include <QObject>
#include <iostream>
#include "src/game_core/game.h"
#include "humanplayer.h"
#include "machineplayer.h"
#include "socketplayer.h"
#include "src/networking/server.h"
#include "src/networking/connection.h"
#include <QDir>
#include <QCoreApplication>

class PlayerFactory : public QObject
{
    Q_OBJECT
public:
    explicit PlayerFactory(QObject *parent = nullptr);

    void createPlayers(Game &game);

signals:
    void allPlayersReady();

public slots:

private:
    Server* server;
};

#endif // PLAYERFACTORY_H
