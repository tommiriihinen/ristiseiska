#ifndef HUMANPLAYER_H
#define HUMANPLAYER_H

#include "player.h"
//#include "src/game_core/machineplayer.h"
#include "src/ui/ui.h"

class HumanPlayer: public Player
{
    Q_OBJECT
public:
    explicit HumanPlayer(QObject *parent = nullptr) : Player(parent) {qDebug() << "creating human";}

public slots:
    void take_action(Player* player, GameAction action) override;

private:
    Card choosePlay();
    Card chooseGive();
    bool chooseContinue();

};

#endif // HUMANPLAYER_H


