#include "menu_processor.h"
#include <cstdlib>

void printHeader()
{
    std::cout << "\033[36m ,-----. ,---.   ,-----. ,------. ,------. ,---.,--.   ,--. \n"
              << "'  .--./'   .-' '  .-.  '|  .--. '|  .---''   .- '\\  `.'  /  \n"
              << "|  |    `.  `-. |  | |  ||  '--' ||  `--, `.  `-. '.    /   \n"
              << "'  '--'\\.-'    |'  '-'  '|  | --' |  `---..-'    |  |  |    \n"
              << " `-----'`-----'  `-----' `--'     `------'`-----'   `--'    \033[0m\n\n";
    std::cout << "\033[32mHello, welcome to CSOPESY commandline!\033[0m\n";
    std::cout << "\033[33mType 'exit' to quit, 'clear' to clear the screen\033[0m\n";

    if (!activeScreens.empty())
    {
        std::cout << "\n--- Active Screens ---\n";
        for (const auto& pair : activeScreens)
        {
            std::cout << "  - " << pair.first << "\n";
        }
        std::cout << "----------------------\n";
    }
}

void processCommand(const std::string& command)
{
    if (command == "initialize")
    {
        std::cout << command << " command recognized. Doing something." << std::endl;
    }
    else if (command == "screen -ls")
    {
        printSchedulerStatus();
    }
    else if (command == "clear")
    {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
        if (currentScreenName.empty())
            printHeader();
        else
            ScreenConsoles(activeScreens[currentScreenName]);
    }


    else if (command == "exit")
    {
        if (currentScreenName.empty())
        {
            std::cout << "Terminating command line emulator." << std::endl;
            stopScheduler();
            exit(0); 
        }
        else 
        {
            std::cout << "Returning to main menu." << std::endl;
            currentScreenName = ""; 
           
#ifdef _WIN32
            system("cls");
#else
            system("clear");
#endif
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
            newScreen.processName = screenName;
            newScreen.currentLine = 0;
            newScreen.totalLines = 100;
            newScreen.creationTime = time(0);
            activeScreens[screenName] = newScreen;
            std:: cout << "Created " << screenName << " successfully.\n";
            currentScreenName = screenName;
            ScreenConsoles(activeScreens[currentScreenName]);
            addNewProcess(screenName);
            startScheduler();
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
