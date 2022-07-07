#ifndef NEURALPLAYER_H
#define NEURALPLAYER_H

#include "socketplayer.h"

class NeuralPlayer : public SocketPlayer
{
    Q_OBJECT
public:
    explicit NeuralPlayer(QObject *parent = nullptr);

signals:
    void send(QString data);
    void creationComplete(SocketPlayer* self);
    void announce(QString message);

public slots:
    void take_action(Player* player, GameAction action) override; // from game
    void recieve(QString data); // from connection
    void announcements(QString message);
    void whispers(Player* target, QString message);

private:
    GameAction mState;
    bool mActionPending;
};

#endif // NEURALPLAYER_H
