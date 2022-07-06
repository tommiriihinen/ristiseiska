#include "util.h"

Util::Util()
{

}

bool Util::questionPrompt(std::string prompt) {
    bool answered = false;
    while(!answered) {
        std::cout << prompt << " (y/n): ";
        std::string input;
        std::cin >> input;
        if (input == "Y" || input == "y") return true;
        if (input == "N" || input == "n") return false;
    }
}

int Util::numberPrompt(std::string prompt) {
    bool answered = false;
    while(!answered) {
        std::cout << prompt << " ";
        std::string input;
        std::cin >> input;
        return stoi(input);
    }
}
