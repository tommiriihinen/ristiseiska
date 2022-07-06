#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <string>

class Util
{
public:
    Util();

public:
    static bool questionPrompt(std::string prompt);
    static int numberPrompt(std::string prompt);
};

#endif // UTIL_H
