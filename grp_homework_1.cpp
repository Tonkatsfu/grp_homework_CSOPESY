#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <iomanip>

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

void printHeader()
{
    std::cout << " ,-----. ,---.   ,-----. ,------. ,------. ,---.,--.   ,--. \n"
            << "'  .--./'   .-' '  .-.  '|  .--. '|  .---''   .- '\\  `.'  /  \n"
            << "|  |    `.  `-. |  | |  ||  '--' ||  `--, `.  `-. '.    /   \n"
            << "'  '--'\\.-'    |'  '-'  '|  | --' |  `---..-'    |  |  |    \n"
            << " `-----'`-----'  `-----' `--'     `------'`-----'   `--'    \n"
            << std::endl;
    std::cout << "\033[32mHello, welcome to CSOPESY commandline!\033[0m" << std::endl;
    std::cout << "\033[33mType 'exit' to quit, 'clear' to clear the screen\033[0m" << std::endl;

    if (!activeScreens.empty())
    {
        std::cout <<"\n--- Active Screens ---" <<std::endl;
        for (const auto& pair : activeScreens)
        {
            std:: cout << "  - " << pair.first << std::endl;
        }
        std::cout << "----------------------" << std::endl;
    }
}

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

void processCommand(const std::string& command)
{
    if (command == "initialize" ||
        command == "screen" ||
        command == "scheduler-test" ||
        command == "scheduler-stop" ||
        command == "report-util")
        {
            std::cout << command << " command recognized. Doing something." << std::endl;
        }
    
    else if (command == "clear")
    {
#ifdef _WIN32 /*If on windows*/
        system("cls");
#else /*If on Linux/macOS*/
        system("clear");
#endif
        if (currentScreenName.empty())
        {
            printHeader();
        }
        else
        {
            ScreenConsoles(activeScreens[currentScreenName]);
        }
    }

    else if(command == "exit")
    {
        if (currentScreenName.empty())
        {
            std::cout <<"Terminating command line emulator." << std::endl;
            exit(0);
        }
        else
        {
            std:: cout << "Returning to main menu." << std::endl;
            currentScreenName = "";
            system("cls");
            printHeader();
        }
    }

    else if (command.rfind("screen -s ", 0) == 0)
    {
        std::string screenName = command.substr(10);
        if (screenName.empty())
        {
            std::cout << "Usage: screen -s <name>" << std::endl;
            return;
        }

        if (activeScreens.count(screenName))
        {
            std::cout << "Screen name " << screenName <<" already exists." << std::endl;
        }

        else
        {
            ScreenDisplay newScreen;
            newScreen.name = screenName;
            newScreen.processName = "Placeholder Process";
            newScreen.currentLine = 0;
            newScreen.totalLines = 100;
            newScreen.creationTime = time(0);
            activeScreens[screenName] = newScreen;
            std:: cout << "Created " << screenName << " successfully.\n";
            currentScreenName = screenName;
            ScreenConsoles(activeScreens[currentScreenName]);
        }
    }

    else if (command.rfind("screen -r ", 0) == 0)
    {
        std:: string screenName = command.substr(10);
        if (screenName.empty())
        {
            std::cout << "Usage: screen -r <name>" << std::endl;
            return;
        }

        if (activeScreens.count(screenName))
        {
            currentScreenName = screenName;
            ScreenConsoles(activeScreens[currentScreenName]);
        }

        else
        {
            std:: cout << "Screen " << screenName << " not found. Please use screen -s " << screenName << " to create it." << std::endl;
        }
    }

    else 
    {
        std::cout <<"Please enter a valid command." << std::endl;
    }
}

int main()
{
    printHeader();
    std:: string command;

    while(true)
    {
        std::cout <<"Enter a command: ";
        std::getline(std::cin, command);
        processCommand(command);
    }

    return 0;
}