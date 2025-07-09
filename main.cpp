/*
#include <iostream>
#include <string>
#include <thread>
#include "menu_processor.h"
#include "scheduler.h"

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

    stopScheduler();
    return 0;
}
*/

#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include "menu_processor.h"
#include "initialize.h"
#include "scheduler.h"
#include "cpu_tick.h"
#include "cpu_tick_global.h"
#include "memory_manager.h"

CpuTicker ticker;

std::map<int, std::string> processesInMemoryMap;
int nextProcessId = 1;

int main()
{
    globalCpuTicker = &ticker;

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
            ticker.start();
            startScheduler();
            isInitialized = true;
        }
        processCommand(command);
    }

    ticker.stop();
    stopScheduler();
    return 0;
}
