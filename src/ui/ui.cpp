#include "headers/ui/ui.h"

UI::UI()
{

}

bool questionPrompt(std::string prompt) {
    bool answered = false;
    while(!answered) {
        std::cout << prompt << " (y/n): ";
        std::string input;
        std::cin >> input;
        if (input == "Y" || input == "y") return true;
        if (input == "N" || input == "n") return false;
    }
}

bool questionPrompt(QString prompt) {
    return questionPrompt(prompt.toStdString());
}
