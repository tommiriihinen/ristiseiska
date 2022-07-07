
#include "humanplayer.h"


void HumanPlayer::take_action(Player* player, GameAction action) {

    if (this != player) return;

    mActionPending = true;

    if (action == play) {
        mState = play;
        emit send("PLAY;");
        emit send("MSG;Your cards: " + mHand.toString());
        emit send("MSG;Options:" );
        std::vector<Card> options = findOptions(mHand, *board);
        if (options.empty()) {
            emit send("MSG;None");
        }
        for (Card option : options) {
            emit send("MSG;" + option.id());
        }
        emit send("MSG;Play card or [P]ass:");
    }
    if (action == give) {
        mState = give;
        emit send("GIVE;");
        emit send("MSG;Give a card");
        emit send("MSG;Your cards: " + mHand.toString());
    }
}


void HumanPlayer::recieve(QString data) {

    if (mName == "null") {
        mName = data;
        emit creationComplete(this);
        return;
    }

    if (!mActionPending) {
        return;
    }

    if (mState == play) {

        QList<QString> parts = data.split(";");
        if (parts[0] == "P") {

            if (!canPass(mHand, *board)) {
                emit send("ERROR;Can't pass when cards fit");
                return;
            }
            emit pass_turn();
            qDebug() << "emit pass";
            mActionPending = false;
            emit send("WAIT;");
            return;
        }

        Card card = Card(parts[0]);
        if (card.getSuit() == none or card.getRank() == -1) {
            emit send("ERROR;Not a valid card");
            return;
        }
        if (!mHand.contains(card)) {
            emit send("ERROR;You don't have this card");
            return;
        }
        if (!board->canPlay(card)) {
            emit send("ERROR;This card doesn't fit the board");
            return;
        }
        bool continues = parts[1] == "1";
        emit play_card(card, continues);
        mActionPending = false;
        emit send("WAIT;");
        return;
    }

    if (mState == give) {

        Card card = Card(data);
        if (card.getSuit() == none or card.getRank() == -1) {
            emit send("ERROR;Not a valid card");
            return;
        }
        if (!mHand.contains(card)) {
            emit send("ERROR;Can't give what you don't have");
            return;
        }
        emit give_card(card);
        mActionPending = false;
        emit send("WAIT;");
        return;
    }
}

void HumanPlayer::announcements(QString message) {
    emit send("MSG;" + message);
}

void HumanPlayer::whispers(Player* target, QString message) {
    if (this == dynamic_cast<SocketPlayer*>(target)) {
        emit send("MSG;" + message);
    }
}
