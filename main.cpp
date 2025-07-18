#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include "menu_processor.h"
#include "initialize.h"
#include "scheduler.h"
#include "cpu_tick_global.h"
#include "memory_manager.h"

std::map<int, std::string> processesInMemoryMap;
int nextProcessId = 1;

int main()
{

#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    printHeader();

    std::string command;

    while (!terminateProgram)
    {
        /*
        if (currentScreenName.empty())
        {
            std::cout << "Enter a command: ";
        }
            */

        std::cout << "Enter a command: ";

        std::getline(std::cin, command);

        if(isInitialized == false && command == "initialize"){
            initialize();
            initializeMemoryManager();
            startScheduler();
            isInitialized = true;
        }
        processCommand(command);
    }

    stopScheduler();
    return 0;
}
