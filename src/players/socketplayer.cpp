
#include "socketplayer.h"

void SocketPlayer::take_action(IPlayer &player, GameAction action) {

    if (this != &player) return;

    mHand.suitSort();

    mActionPending = true;

    if (action == play) {
        mState = play;
        std::vector<Card> options = findOptions(mHand, *board);
        for (Card option : options) {
            emit send("OPTION;" + option.id());
        }
        emit send("MSG;Your cards: " + mHand.toString());
        emit send("MSG;Play card or [P]ass:");
        emit send("PLAY;");
    }
    if (action == give) {
        mState = give;
        emit send("MSG;Give a card");
        for (Card option : mHand.toVector()) {
            emit send("OPTION;" + option.id());
        }
        emit send("MSG;Your cards: " + mHand.toString());
        emit send("GIVE;");

    }
}


void SocketPlayer::recieve(QString data) {

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

        if (parts[0] == "P" or parts[0] == "p") {

            if (!canPass(mHand, *board)) {
                emit send("ERROR;Can't pass when cards fit");
                return;
            }
            emit pass_turn();
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
        assert(parts.size() == 2);

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

void SocketPlayer::announcements(QString message, QString command) {
    emit send(command + ";" + message);
}

void SocketPlayer::whispers(IPlayer &target, QString message, QString command) {
    if (this == &target) {
        emit send(command + ";" + message);
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

