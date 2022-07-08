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
    std::cout << "Saved\n";
    delete mFile;
    delete mStream;
    std::cout << "Closed\n";
    mFile = nullptr;
    mStream = nullptr;
    fileOpen = false;
}

void DataWriter::addMetadata(Benchmark &bm) {
    // Here we overwrite the space we left at the beginning of the file.
    // Now if you are reading this, this is fucking stupid. Normally
    // you would write new stuff to the beginning of a fresh file and
    // then copy the rest over, but, I wanted to see if this works.

    QString metadata = bm.toString();
    int end = metadata.length();
    uchar *buffer = mFile->map(0, end);

    // The following line will edit (both read from and write to)
    // the file without clearing it first:
    for (int i=0; i<end; ++i) buffer[i] = metadata[i].toLatin1();

    mFile->unmap(buffer);
}

void DataWriter::gameStart() {

    for (Player* p : game->getPlayers()) {
        connect(p, &Player::play_card, this, &DataWriter::listenPlay);
        connect(p, &Player::give_card, this, &DataWriter::listenGive);
        connect(p, &Player::pass_turn, this, &DataWriter::listenPass);
    }

    mPlayer_count = 0;
    mPlayerIDs.clear();

    mPlayer_count = game->getPlayers().size();
    // Write number of players to file
    write("/" + QString::number(mPlayer_count) + ";");

    // ID players from 0 to n
    for (Player* p : game->getPlayers()) {
        mPlayerIDs[p] = QString::number(mPlayerIDs.size());
        // write players hands to file
        write("P" + mPlayerIDs[p] + " " + p->getDeck()->toString(true) + ",");
    }
    write(";");
}

void DataWriter::gameEnd(Player* winner) {
    write(";" + mPlayerIDs[winner]);

    for (Player* p : game->getPlayers()) {
        disconnect(p, &Player::play_card, this, &DataWriter::listenPlay);
        disconnect(p, &Player::give_card, this, &DataWriter::listenGive);
        disconnect(p, &Player::pass_turn, this, &DataWriter::listenPass);
    }
}

void DataWriter::listenPlay(Card card, bool continues) {
    Player* p = game->getCurrentPlayer();
    write(mPlayerIDs[p] + " " + card.id(true) + ",");
}

void DataWriter::listenGive(Card card) {
    Player* p = game->getLastPlayer();
    write("G " + mPlayerIDs[p] + " " + card.id(true) + ",");

}

void DataWriter::listenPass() {
    Player* p = game->getCurrentPlayer();
    write("P " + mPlayerIDs[p] + ",");
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
