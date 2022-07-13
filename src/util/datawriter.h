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
    void gameEnded(Player* winner);

    void listenPlay(Card card, bool continues);
    void listenGive(Card card);
    void listenPass();

private:
    void extractStartingDecks();
    void write(QString line);

    QFile* mFile;
    QTextStream* mStream;
    QString mMetadata;
    int mMetadataLength;
    QString mData;
    bool fileOpen { false };

    Game* game;
    int mPlayer_count { 0 };
    std::map<Player*, QString> mPlayerIDs;
    std::map<Player*, Deck> mStartingHands;
};

#endif // DATAWRITER_H
