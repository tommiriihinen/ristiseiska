#ifndef DATAWRITER_H
#define DATAWRITER_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDateTime>
#include <QCoreApplication>
#include <QTextDocumentWriter>
#include <iostream>
#include <fstream>
#include "game_core/game.h"
#include "players/player.h"
#include "util/benchmarker.h"

struct MetaData {
    Benchmark mBenchmark;

    int mGames[5] = { 0 };

    QString toString();
};

class DataWriter : public QObject
{
    Q_OBJECT
public:
    explicit DataWriter(QObject *parent = nullptr);

    void connectGame(Game* game);
    void newFile();
    void saveFile();
    void addMetadata(Benchmark &benchmark);

    bool isOpen() const {return fileOpen;}

signals:

public slots:

    void gameStarted();
    void gameEnded(IPlayer &winner);

    void listenPlay(Card card, bool continues);
    void listenGive(Card card);
    void listenPass();

private:
    void extractStartingDecks();
    void write(QString line);

    QFile* mFile;
    QTextStream* mStream;
    MetaData mMetadata;
    QString mData;
    bool fileOpen { false };

    Game* game;
    int mPlayer_count { 0 };
    std::map<IPlayer*, QString> mPlayerIDs;
    std::map<IPlayer*, Deck> mStartingHands;
};

#endif // DATAWRITER_H
