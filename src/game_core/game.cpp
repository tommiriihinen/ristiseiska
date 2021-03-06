#include "src/game_core/game.h"

Game::Game(QObject *parent)
    : mBoard(), mDealer(), mSettings() {
    Deck stariting_deck;
    stariting_deck.fill();
    mDealer.addCards(stariting_deck);
}

void Game::addPlayer(pIPlayer p) {
    players.push_back(p);

    connect(this, &Game::take_action, p.get(), &IPlayer::take_action); //Qt::QueuedConnection
    connect(p.get(), &IPlayer::play_card, this, &Game::play_card, Qt::QueuedConnection);
    connect(p.get(), &IPlayer::give_card, this, &Game::give_card, Qt::QueuedConnection);
    connect(p.get(), &IPlayer::pass_turn, this, &Game::pass_turn, Qt::QueuedConnection);
    connect(this, &Game::victory, p.get(), &IPlayer::game_ended);

    // If the player is SocketPlayer do some exstra connections
    if (auto sp = std::dynamic_pointer_cast<SocketPlayer>(p)) {
        connect(this, &Game::announce, sp.get(), &SocketPlayer::announcements);
        connect(this, &Game::whisper, sp.get(), &SocketPlayer::whispers);
    }
    p->setBoard(mBoard);
    mDealer.addDeck(p->getDeck());
}

bool Game::removePlayer(pIPlayer p) {
    // doesn't contain
    assert(std::find(players.begin(), players.end(), p) != players.end());
    // remove
    players.erase(std::remove(players.begin(), players.end(), p), players.end());

    disconnect(this, &Game::take_action, p.get(), &IPlayer::take_action);
    disconnect(p.get(), &IPlayer::play_card, this, &Game::play_card);
    disconnect(p.get(), &IPlayer::give_card, this, &Game::give_card);
    disconnect(p.get(), &IPlayer::pass_turn, this, &Game::pass_turn);
    disconnect(this, &Game::victory, p.get(), &IPlayer::game_ended);

    if (auto sp = std::dynamic_pointer_cast<SocketPlayer>(p)) {
        disconnect(this, &Game::announce, sp.get(), &SocketPlayer::announcements);
        disconnect(this, &Game::whisper, sp.get(), &SocketPlayer::whispers);
    }
    mDealer.removeDeck(p->getDeck());
    return true;
}

void Game::addPlayers(std::vector<pIPlayer> new_players) {
    for (const auto &p : new_players) this->addPlayer(p);
}

void Game::clearPlayers() {
    std::vector<pIPlayer> original = players;
    for (const auto &p : original) {
        removePlayer(p);
    }
}

void Game::start() {

    assert(!mRunning);
    assert(!players.empty());
    assert(!mDealer.getDeck().isEmpty());

    mDealer.deal();

    emit announce("------------------------Instructions:-------------------------\n"
                  "On your turn you may play a card in accordance to the rules of\n"
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

    mRunning = true;
    emit started();
    next_turn();
}

void Game::clean() {
    emit announce("----------------------the seven of clubs----------------------");

    mTurn = 0;

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

void Game::stop() {
    emit announce("Ending game");
    mRunning = false;

}


void Game::play_card(const Card &card, const bool continues) {
    assert(mBoard.canPlay(card));

    mBoard.playCard(card, *current_player->getDeck());

    emit announce(card.id(), "CARD");
    emit announce(mBoard.toString());
    emit announce(current_player->getName()
                  + " played "
                  + card.id()
                  + " and has "
                  + QString::number(current_player->getDeck()->size())
                  + " cards");

    if (!continues) next_turn();

    if (continues) {
        if ((card.getRank() == ace or card.getRank() == king)                    // is possible to continue
                and current_player->getDeck()->size() > 0                        // hand is not empty
                and !findOptions(*current_player->getDeck(), mBoard).empty()) {  // can play afterwards
            emit announce(current_player->getName() + " will continue");
            emit take_action(*current_player, play);

        } else {
            emit whisper(*current_player.get(), "You cannot continue", "ERROR");
            next_turn();
        }
    }

}

void Game::give_card(const Card &card) {
    emit announce(current_player->getName() + " took a card from " + last_player->getName());
    emit announce(card.id(), "CARD");
    emit whisper(*current_player.get(), last_player->getName() + " gave you " + card.id());

    last_player->getDeck()->put(card, *current_player->getDeck());
    next_turn();
}

void Game::pass_turn() {
    emit announce(current_player->getName() + " passed");

    // Passing before C7 is ok.
    if (mBoard.isEmpty()) {
        next_turn();
    } else {
        // Otherwise take a card from the last player
        emit take_action(*last_player.get(), give);
    }

}

void Game::next_turn() {

    if (!mRunning) {
        qDebug() << "Game was stopped from outside";
        return; // if stop() is called game ends here
    }

    auto winner = check_win();

    if (winner == nullptr) {
        mTurn++;
        this->last_player = current_player;
        this->current_player = players[(mTurn-1) % players.size()];

        emit announce(current_player->getName() + "'s turn:");
        emit take_action(*current_player.get(), play);
    } else {
        clean();
        mRunning = false;
        emit announce(current_player->getName() + " has won the game");
        emit victory(*winner.get());
    }
}

pIPlayer Game::check_win() {
    pIPlayer winner = nullptr;
    for (auto player : players) {
        if (player->getDeck()->isEmpty()) {
            winner = player;
        }
    }
    return winner;
}



