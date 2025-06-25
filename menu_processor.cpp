#include "menu_processor.h"
#include <cstdlib>
#include "scheduler.h"
#include "initialize.h"

bool terminateProgram = false;
bool isInitialized = false;

void printHeader()
{
    std::cout << "\033[36m ,-----. ,---.   ,-----. ,------. ,------. ,---.,--.   ,--. \n"
              << "'  .--./'   .-' '  .-.  '|  .--. '|  .---''   .- '\\  `.'  /  \n"
              << "|  |    `.  `-. |  | |  ||  '--' ||  `--, `.  `-. '.    /   \n"
              << "'  '--'\\.-'    |'  '-'  '|  | --' |  `---..-'    |  |  |    \n"
              << " `-----'`-----'  `-----' `--'     `------'`-----'   `--'    \033[0m\n\n";
    std::cout << "\033[32mHello, welcome to CSOPESY commandline!\033[0m\n";
    std::cout << "\033[33mType 'exit' to quit, 'clear' to clear the screen\033[0m\n";

    /*
    if (!activeScreens.empty())
    {
        std::cout << "\n--- Active Screens ---\n";
        for (const auto& pair : activeScreens)
        {
            std::cout << "  - " << pair.first << "\n";
        }
        std::cout << "----------------------\n";
    }
        */
}

void processCommand(const std::string& command)
{
    if (isInitialized)
    {
        if (command == "screen -ls")
        {
            printSchedulerStatus();
            printHeader();
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
                ScreenConsoles(*allProcesses[currentScreenName]);
        }
        else if (command == "exit")
        {
            if (currentScreenName.empty())
            {
                std::cout << "Terminating command line emulator." << std::endl;
                stopDummyProcesses();
                stopScheduler();
                terminateProgram = true;
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

    if (allProcesses[screenName] != nullptr && !allProcesses[screenName]->finished)
    {
        std::cout << "Screen " << screenName << " already exists. Use screen -r <process name>.\n";
    }
    else
    {
        currentScreenName = screenName;
        addNewProcess(screenName);
        startScheduler();
        ScreenConsoles(*allProcesses[screenName]);  
    }
}

        else if (command.rfind("screen -r ", 0) == 0)
        {
            std::string screenName = command.substr(10);
            if (screenName.empty())
            {
                std::cout << "Usage: screen -r <name>" << std::endl;
                return;
            }

            if (allProcesses.count(screenName) && !allProcesses[screenName]->finished)
            {
                currentScreenName = screenName;
                ScreenConsoles(*allProcesses[screenName]);

                std::string screenCommand;
                std::getline(std::cin, screenCommand);
                processCommand(screenCommand); // allow commands inside the screen
            }
            else
            {
                std::cout << "Process " << screenName << " not found. Use screen -s to create it.\n";
            }
        }
        else if (command == "scheduler -start")
        {
            startDummyProcesses();
            startScheduler();
        }
        else if (command == "scheduler -stop")
        {
            stopDummyProcesses();
            stopScheduler();
        }
        else if (command == "process-smi")
        {
            if (!currentScreenName.empty())
            {
                ProcessSMI(currentScreenName);
            }
            else
            {
                std::cout << "You are currently not in a process screen. Use screen -s or screen -r first.\n";
            }
        }
        else
        {
            std::cout << "Please enter a valid command." << std::endl;
        }
    }
    else
    {
        if (command == "initialize")
        {
            initialize();
            isInitialized = true;
        }
        else if (command == "exit")
        {
            terminateProgram = true;
        }
        else
        {
            std::cout << "To input other commands, please initialize the program by typing 'initialize'.\n";
        }
    }
}


