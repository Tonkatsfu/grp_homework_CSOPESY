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

        std::cout << "Enter a command: ";

        std::getline(std::cin, command);

        processCommand(command);

    }

    return 0;
}
