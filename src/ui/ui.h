#ifndef UI_H
#define UI_H

#include <string>
#include <iostream>
#include "src/games/game.h"
#include "qstring.h"

class UI : public QObject
{
    Q_OBJECT
public:
    UI(QObject* parent = nullptr) {qDebug() << "create UI";}

    void setGame(Game* game);

public slots:
    void announcements(QString message);

private:
    Game* game;
};

#endif // UI_H
