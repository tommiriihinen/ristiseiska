#ifndef UI_H
#define UI_H

#include <string>
#include <iostream>
#include "qstring.h"

class UI
{
public:
    UI();
};

bool questionPrompt(std::string prompt);
int numberPrompt(std::string prompt);
//bool questionPrompt(QString prompt);

#endif // UI_H
