/*
    This header file is for the main menu functions from displaying the header to accepting commands.
*/

#include "screen_processor.h"

/*
    Displays the header, this includes title, instructions and current active screens.
*/
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

/*
    Function that handles commands possible commands are;
    - initialize
    - screen
    - scheduler-test
    - scheduler-stop
    - report util
    - clear
    - exit
*/
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