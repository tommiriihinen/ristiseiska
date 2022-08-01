#include "util.h"

Util::Util()
{

}

bool Util::questionPrompt(QString prompt) {
    bool answered = false;
    while(!answered) {
        std::cout << prompt.toStdString() << " (y/n): ";
        std::string input;
        std::cin >> input;
        if (input == "Y" || input == "y") return true;
        if (input == "N" || input == "n") return false;
    }
    return false;
}


int Util::numberPrompt(QString prompt) {
    while(true) {
        std::cout << prompt.toStdString() << ": ";
        std::string input;
        std::cin >> input;
        try {
            return stoi(input);
        } catch (std::exception e) {
            std::cout << "not a valid number";
        }
    }
}

char Util::multipleChoicePrompt(QString prompt) {
    std::cout << prompt.toStdString();

    std::string response;
    while (response.size() != 1) {
        std::cin >> response;
    }
    char first_letter = response[0];
    first_letter = toupper(first_letter);
    return first_letter;
}

PlayerType Util::playerTypePromt(QString prompt) {
    std::string response;
    while (true) {
        std::cout << prompt.toStdString();
        std::cin >> response;
        if (response == "goblin") return PlayerType::comptr;
        if (response == "client") return PlayerType::client;
        if (response == "neural") return PlayerType::neural;
        if (response == "random") return PlayerType::random;
    }


}
