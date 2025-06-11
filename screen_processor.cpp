#include "screen_processor.h"

std::map<std::string, ScreenDisplay> activeScreens;
std::string currentScreenName = "";

void ScreenConsoles(const ScreenDisplay& screen)
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif

    std::cout << "\n\033[36m--- Screen: " << screen.name << " ---\033[0m\n";
    std::cout << "Process Name: " << screen.processName << "\n";
    std::cout << "Current Line: " << screen.currentLine << " / " << screen.totalLines << "\n";

    char buffer[80];
    strftime(buffer, sizeof(buffer), "%m/%d/%Y, %I:%M:%S %p", localtime(&screen.creationTime));
    std::cout << "Creation Time: " << buffer << "\n";

    std::cout << "\033[36m------------------------\033[0m\n";
    std::cout << "Type 'exit' to return to the main menu.\n";
}
