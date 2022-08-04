#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <string>
#include <QString>
#include "players/player.h"

class Util
{
public:
    Util();

public:
    static bool questionPrompt(QString prompt);
    static int numberPrompt(QString prompt);
    static char charPrompt(QString prompt);
    static QString stringPrompt(QString prompt);
    static PlayerType playerTypePromt(QString prompt);
};

#endif // UTIL_H
