#ifndef UI_H
#define UI_H

#include <string>
#include <iostream>
#include "src/game_core/game.h"
#include "qstring.h"
#include "src/util.h"



class UI : public QObject
{
    Q_OBJECT
public:
    UI(QObject* parent = nullptr) {qDebug() << "create UI";}

    void setGame(Game* game);
    void setHide(bool hide) {mHide = hide;}

public slots:
    void announcements(QString message);

private:
    Game* game;
    bool mHide { false };
};

#endif // UI_H
