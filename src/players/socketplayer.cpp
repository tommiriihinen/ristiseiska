
#include "socketplayer.h"

SocketPlayer::SocketPlayer(QObject *parent)
    : IPlayer(parent) {
    qDebug() << "Constructing socketplayer";
}

SocketPlayer::~SocketPlayer() {
    qDebug() << "Destructing socketplayer";
    send("KILL;");
    mSocket->close();
    mSocket->deleteLater();
}

void SocketPlayer::setSocket(QTcpSocket *socket){
    mSocket = socket;
    connect(socket, &QTcpSocket::readyRead, this, &SocketPlayer::recieve);
}

void SocketPlayer::take_action(IPlayer &player, GameAction action) {

    if (this != &player) return;

    mHand.suitSort();

    mActionPending = true;

    if (action == play) {
        mState = play;
        std::vector<Card> options = findOptions(mHand, *board);
        for (Card option : options) {
            send("OPTION;" + option.id());
        }
        send("MSG;Your cards: " + mHand.toString());
        send("PROMPT;Play card or [P]ass:");
        send("PLAY;");
    }
    if (action == give) {
        mState = give;
        for (Card option : mHand.toVector()) {
            send("OPTION;" + option.id());
        }
        send("MSG;Your cards: " + mHand.toString());
        send("PROMPT;Give a card");
        send("GIVE;");

    }
}


void SocketPlayer::recieve() {

    QByteArray utfData = mSocket->readAll();
    // qDebug() << mName << " RECIEVED: " << utfData;
    QString data = QString::fromUtf8(utfData);

    for (auto& line : data.split(MSG_DELIMITER)) {
        //qDebug() << "line: " + line
        if (line != "") {
            handleMessage(line);
        }
    }
}

void SocketPlayer::handleMessage(QString message) {

    if (mName == "null") {
        mName = message;
        emit playerReady(this);
        return;
    }

    if (!mActionPending) {
        return;
    }

    if (mState == play) {

        QList<QString> parts = message.split(";");

        if (parts[0] == "P" or parts[0] == "p") {

            if (!canPass(mHand, *board)) {
                send("ERROR;Can't pass when cards fit");
                return;
            }
            emit pass_turn();
            mActionPending = false;
            return;
        }

        Card card = Card(parts[0]);
        if (card.getSuit() == none or card.getRank() == -1) {
            qDebug() << card.id();
            send("ERROR;Not a valid card");
            return;
        }
        if (!mHand.contains(card)) {
            send("ERROR;You don't have this card");
            return;
        }
        if (!board->canPlay(card)) {
            send("ERROR;This card doesn't fit the board");
            return;
        }
        Q_ASSERT(parts.size() == 2);

        bool continues = parts[1] == "1";
        emit play_card(card, continues);
        mActionPending = false;
        return;
    }

    if (mState == give) {

        Card card = Card(message);
        if (card.getSuit() == none or card.getRank() == -1) {
            send("ERROR;Not a valid card");
            return;
        }
        if (!mHand.contains(card)) {
            send("ERROR;Can't give what you don't have");
            return;
        }
        emit give_card(card);
        mActionPending = false;
        return;
    }
}

void SocketPlayer::send(QString data) {
    QByteArray utfData = data.toUtf8();
    // qDebug() << mName << " SENDING: " << utfData;
    mSocket->write(utfData.append(MSG_DELIMITER_UTF8));
}

void SocketPlayer::recieveAnnouncement(QString message, QString command) {
    send(command + ";" + message);
}

void SocketPlayer::recieveWhisper(IPlayer &target, QString message, QString command) {
    if (this == &target) {
        send(command + ";" + message);
    }
}



/* << PLAY
 * >> C8:1, C9:0, DX:0 ... (activation order)
 * << PLAYED C9
 *
 * << GIVE
 * >> DA, HA, HK ... (activation order)
 *
 * << PLAYED D9
 * << PLAY
 * >> ...
 *
 * [n players] min 3 max 7
 * [1, 2, 3, 4, 5, 6, 7] number of cards in hand (max 18, min 0)
 * [play, give] wanted action
 * [1, ..., 52] hand
 * [1, ..., 52] board
 * [1, ..., 52] others hands
 *
 * Input Vector size = 166
 *
 * [1, ..., 52] cards
 * [continue]
 *
 * Output Vector size = 53
 *
 */

