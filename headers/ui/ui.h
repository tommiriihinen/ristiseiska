#ifndef UI_H
#define UI_H

#include <string>
#include <iostream>
#include "headers/games/game.h"
#include "qstring.h"

class UI : public QObject
{
    Q_OBJECT
public:
    UI(QObject* parent = nullptr) {qDebug() << "create UI";}

    void setGame(Game* game);

public slots:
    void action_expected(Player* player, GameAction action);
//    void card_played(Card card, bool continues);
//    void card_given(Card card);
//    void passed();

private:
    Game* game;
};

#endif // UI_H
