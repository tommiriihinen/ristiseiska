#ifndef UI_H
#define UI_H

#include <string>
#include <iostream>
#include "game_core/game.h"
#include "qstring.h"
#include "util/util.h"



class UI : public QObject
{
    Q_OBJECT
public:
    UI(QObject* parent = nullptr) {}

    void setGame(Game* game);
    void setShow(bool show) {mShow = show;}

public slots:
    void announcements(QString message, QString cmd);

private:
    Game* game;
    bool mShow { false };
};

#endif // UI_H
