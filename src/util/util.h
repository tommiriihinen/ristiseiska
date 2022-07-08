#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <string>
#include <QString>

class Util
{
public:
    Util();

public:
    static bool questionPrompt(QString prompt);
    static int numberPrompt(QString prompt);
    static char multipleChoicePrompt(QString prompt);
};

#endif // UTIL_H
