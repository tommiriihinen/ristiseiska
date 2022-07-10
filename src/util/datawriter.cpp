#include "datawriter.h"

DataWriter::DataWriter(QObject *parent)
    : QObject{parent}
{

}

/* File building
 *
 *
 * Game:
 *
 * / = Start
 * ; = Data separator
 *      [1] comma-separated decks
 *          P1: CA C3 ... S8 ,
 *          P2: C2 C8 ... SK ,
 *      [2] comma-separeted turns
 *          P:C7,
 *          G:HA
 *      [3] winner
 *          3
 */

void DataWriter::connectGame(Game* game) {
    this->game = game;
}

void DataWriter::newFile() {
    QDateTime date = QDateTime::currentDateTime();
    QString formattedTime = date.toString("ddMMyyyy hhmmss");

    QString filePath = QCoreApplication::applicationDirPath();

    QDir dir(filePath);
    if (dir.mkdir("data")) std::cout << "Created a new data folder\n";

    QString filename = QCoreApplication::applicationDirPath() + "/data/" + formattedTime + ".txt";

    mFile = new QFile(filename);
    if (mFile->open(QIODevice::ReadWrite)) {
        mStream = new QTextStream(mFile);
        fileOpen = true;
        std::cout << "Created a save file\n";
        qDebug() << mFile->fileName();
    } else {
        qDebug() << mFile->fileName();
        qDebug() << mFile->errorString();
        std::cout << "Could not create a new save\n";
    }
    Benchmark placeholder;
    write(placeholder.toString());
}

void DataWriter::saveFile() {
    mFile->flush();
    std::cout << "Saved.\n";
    delete mFile; mFile = nullptr;
    delete mStream; mStream = nullptr;
    fileOpen = false;
}

void DataWriter::addMetadata(Benchmark &bm) {
    // Here we overwrite the space we left at the beginning of the file.
    // Now if you are reading this, this is fucking stupid. Normally
    // you would write new stuff to the beginning of a fresh file and
    // then copy the rest over, but, I wanted to see if this works.
    assert(fileOpen);
    qDebug() << "Wrting metadata";
    QString metadata = bm.toString();
    int end = metadata.length();

    mFile->close();
    if (mFile->open(QIODevice::ReadWrite)) {
        uchar *buffer = mFile->map(0, end);
        if (buffer == nullptr) {
            qDebug() << mFile->error();
            qDebug() << mFile->errorString();

        }
        // The following line will edit (both read from and write to)
        // the file without clearing it first:
        for (int i=0; i<end; ++i) buffer[i] = metadata[i].toLatin1();
        mFile->unmap(buffer);
    } else {
        std::cout << "Could not add metadata";
    }
}

void DataWriter::gameStarted() {

    assert(game->isRunning());

    for (auto p : game->getPlayers()) {
        connect(p.get(), &IPlayer::play_card, this, &DataWriter::listenPlay);
        connect(p.get(), &IPlayer::give_card, this, &DataWriter::listenGive);
        connect(p.get(), &IPlayer::pass_turn, this, &DataWriter::listenPass);
    }

    mPlayer_count = 0;
    mPlayerIDs.clear();

    // Write number of players to file
    mPlayer_count = game->getPlayers().size();
    write("/" + QString::number(mPlayer_count) + ";");

    // ID players and write starting hands on file
    for (auto p : game->getPlayers()) {
        mPlayerIDs[p.get()] = QString::number(mPlayerIDs.size());
        write("P" + mPlayerIDs[p.get()] + " " + p->getDeck()->toString(true) + ",");
    }
    write(";");
}

void DataWriter::gameEnded(IPlayer &winner) {
    write(";" + mPlayerIDs[&winner]);

    for (auto p : game->getPlayers()) {
        disconnect(p.get(), &IPlayer::play_card, this, &DataWriter::listenPlay);
        disconnect(p.get(), &IPlayer::give_card, this, &DataWriter::listenGive);
        disconnect(p.get(), &IPlayer::pass_turn, this, &DataWriter::listenPass);
    }
}

void DataWriter::listenPlay(const Card card, bool continues) {
    IPlayer* p = game->getCurrentPlayer().get();
    write("P " + mPlayerIDs[p] + " " + card.id(true) + ",");
}

void DataWriter::listenGive(const Card card) {
    IPlayer* p = game->getLastPlayer().get();
    write("G " + mPlayerIDs[p] + " " + card.id(true) + ",");
}

void DataWriter::listenPass() {
//    Player* p = game->getCurrentPlayer();
//    write("S " + mPlayerIDs[p] + ",");
}

void DataWriter::write(QString data) {
    if (fileOpen) *mStream << data;
}

// Settings -> Data collection on
// new file
// write metadata
// 1000000 games
    //Save
// Settings -> Save file
// save
// file close
