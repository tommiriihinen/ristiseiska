#ifndef PLAYERFACTORY_H
#define PLAYERFACTORY_H

#include <QDir>
#include <QCoreApplication>
#include <QObject>
#include <iostream>
#include "game_core/game.h"
#include "networking/server.h"
#include "networking/connection.h"
#include "machineplayer.h"
#include "randomplayer.h"
#include "socketplayer.h"


class PlayerFactory : public QObject
{
    Q_OBJECT
public:
    explicit PlayerFactory(QObject *parent = nullptr);

    void createPlayers(std::map<PlayerType, int> order, Game &game, bool benchmarking=false);
    void setBenchmarkingModel(QString model) {mBenchmarkingModel = model;}

signals:
    void allPlayersReady();

public slots:

private:
    Server* server;
    QString mBenchmarkingModel = "";
};

#endif // PLAYERFACTORY_H


