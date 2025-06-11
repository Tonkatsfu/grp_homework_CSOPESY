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
        initializeScheduler();
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
        std::cout << "Terminating command line emulator." << std::endl;
        exit(0);
    }
    else
    {
        std::cout << "Invalid command.\n";
    }
}
