/*
    This header file is for functions and declarations relating to the main screen function.
*/

#include <iostream>
#include <map>

/*
    Struct data for the screen process.
*/
struct ScreenDisplay
{
    std::string name;
    std::string processName;
    int currentLine;
    int totalLines;
    time_t creationTime;
};

std::map<std::string, ScreenDisplay> activeScreens;
std::string currentScreenName = "";


/*
    Function to display the requested screen data
    @params
        - screen : a ScreenDisplay struct to be displayed
*/
void ScreenConsoles(const ScreenDisplay& screen)
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    std::cout << "\n\033[36m--- Screen: " << screen.name << " ---\033[0m" << std::endl;
    std::cout << "Process Name: " << screen.processName << std::endl;
    std::cout << "Current Line: " << screen.currentLine << " / " << screen.totalLines << std::endl;

    char buffer[80];
    strftime(buffer, sizeof(buffer), "%m/%d/%Y, %I:%M:%S %p", localtime(&screen.creationTime));
    std::cout << "Creation Time: " << buffer << std::endl;

    std::cout << "\033[36m------------------------\033[0m" << std::endl;
    std::cout << "Type 'exit' to return to the main menu." << std::endl;
}