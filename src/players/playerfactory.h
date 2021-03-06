#ifndef PLAYERFACTORY_H
#define PLAYERFACTORY_H

#include <QDir>
#include <QCoreApplication>
#include <QObject>
#include <iostream>
#include "src/game_core/game.h"
#include "src/networking/server.h"
#include "src/networking/connection.h"
#include "machineplayer.h"
#include "randomplayer.h"
#include "socketplayer.h"


class PlayerFactory : public QObject
{
    Q_OBJECT
public:
    explicit PlayerFactory(QObject *parent = nullptr);

    void createPlayers(std::map<PlayerType, int> order, Game &game);

signals:
    void allPlayersReady();

public slots:

private:
    Server* server;
};

#endif // PLAYERFACTORY_H


