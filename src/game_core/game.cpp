#include "src/game_core/game.h"

Game::Game(QObject *parent)
    : mBoard(), mDealer(), mSettings() {
    Deck stariting_deck;
    stariting_deck.fill();
    mDealer.addCards(stariting_deck);
}

void Game::addPlayer(Player* player) {
    players.push_back(player);

    connect(this, &Game::take_action, player, &Player::take_action);
    connect(player, &Player::play_card, this, &Game::play_card, Qt::QueuedConnection);
    connect(player, &Player::give_card, this, &Game::give_card, Qt::QueuedConnection);
    connect(player, &Player::pass_turn, this, &Game::pass_turn, Qt::QueuedConnection);
    connect(this, &Game::victory, player, &Player::game_ended);

    if (SocketPlayer* sp = dynamic_cast<SocketPlayer*>(player)) {
        connect(this, &Game::announce, sp, &SocketPlayer::announcements);
        connect(this, &Game::whisper, sp, &SocketPlayer::whispers);
    }
    player->setBoard(&mBoard);
    mDealer.addDeck(player->getDeck());
}
bool Game::removePlayer(Player* p) {
    // doesn't contain
    if (std::find(players.begin(), players.end(), p) == players.end()) {
        qDebug() << "SOS";
        return false;
    }
    // remove
    players.erase(std::remove(players.begin(), players.end(), p), players.end());

    disconnect(this, &Game::take_action, p, &Player::take_action);
    disconnect(p, &Player::play_card, this, &Game::play_card);
    disconnect(p, &Player::give_card, this, &Game::give_card);
    disconnect(p, &Player::pass_turn, this, &Game::pass_turn);
    disconnect(this, &Game::victory, p, &Player::game_ended);

    if (SocketPlayer* sp = dynamic_cast<SocketPlayer*>(p)) {
        disconnect(this, &Game::announce, sp, &SocketPlayer::announcements);
        disconnect(this, &Game::whisper, sp, &SocketPlayer::whispers);
    }
    mDealer.removeDeck(p->getDeck());
    return true;
}

void Game::clearPlayers() {
    mDealer.clearDecks();
    for (Player* p : players) {
        delete p;
    }
    players.clear();

}


void Game::setup() {
    mDealer.deal();
}

void Game::start() {

    emit announce("------------------------Instructions:-------------------------\n"
                  "On your turn you may play a card in accordance to the rules of \n"
                  "ristiseiska  or  pass  if and only if you are unable to do so.\n"
                  "\n"
                  "Cards  are  played  by  typing  the first  letter of the suit:\n"
                  "\n"
                  "            [C]lubs/[D]iamonds/[H]earts/[S]pades\n"
                  "\n"
                  "followed by the rank of the card. (10 is replaced by X)\n"
                  "Example: C7\n"
                  "The turn is passed by typing [P]");
    emit announce("----------------------The Seven of Clubs----------------------");
    next_turn();
}

void Game::clean() {
    emit announce("----------------------the seven of clubs----------------------");
    Deck cleaned_cards = mBoard.clean();
    mDealer.gather();
    mDealer.addCards(cleaned_cards);
    //qDebug() << mDealer.dealers_deck.toString();

    switch (mSettings.seat_change) {
    case SeatChange::static_seats:
        // Do nothing
        break;

    case SeatChange::roundrobin:
        std::rotate(players.begin(), players.begin() + 1, players.end());
        break;

    case SeatChange::random:
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::shuffle(players.begin(), players.end(), std::default_random_engine(seed));
        break;
    }
}

void Game::play_card(Card card, bool continues) {

    emit announce(current_player->getName()
                  + " played "
                  + card.id()
                  + " and has "
                  + QString::number(current_player->getDeck()->size())
                  + " cards");

    if (continues) {
        emit announce(current_player->getName() + " will continue");
    }

    // player wants to play a card
    mBoard.playCard(card, *current_player->getDeck());

    if ((card.getRank() == ace or card.getRank() == king) and continues) {
        emit take_action(current_player, play);
    } else {
        next_turn();
    }
}

void Game::give_card(Card card) {
    emit announce(current_player->getName() + " took a card form " + last_player->getName());
    emit whisper(current_player, last_player->getName() + " gave you " + card.id());

    last_player->getDeck()->put(card, *current_player->getDeck());
    next_turn();
}

void Game::pass_turn() {
    emit announce(current_player->getName() + " passed");

    // Passing before C7 is ok.
    if (mBoard.isEmpty()) {
        next_turn();
    } else {
        emit take_action(last_player, give);
    }

}

void Game::next_turn() {

    Player* winner = check_win();
    if (winner != nullptr) {
        //emit announce(current_player->getName() + " has won the game");
        emit victory(winner);

    } else {
        mTurn++;
        this->last_player = current_player;
        this->current_player = players[(mTurn-1) % players.size()];

        emit announce(current_player->getName() + "'s turn:");
        emit announce(mBoard.toString());

        emit take_action(current_player, play);
    }
}

Player* Game::check_win() {
    Player* winner = nullptr;
    for (Player* player : players) {
        if (player->getDeck()->isEmpty()) {
            winner = player;
        }
    }
    return winner;
}



