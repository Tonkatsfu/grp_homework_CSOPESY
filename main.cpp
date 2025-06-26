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
#include "scheduler.h"
#include "cpu_tick.h"
#include "cpu_tick_global.h"

CpuTicker ticker;

int main()
{
    globalCpuTicker = &ticker;

#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    printHeader();
    ticker.start();
    startScheduler();

    std::string command;

    while (!terminateProgram)
    {
        if (currentScreenName.empty())
        {
            std::cout << "Enter a command: ";
        }

        std::getline(std::cin, command);

        processCommand(command);
    }

    ticker.stop();
    stopScheduler();
    return 0;
}
