#include <iostream>
#include <string>
#include <thread>
#include "menu_processor.h"
#include "scheduler.h"

int main()
{
    std::thread schedulerThread(runScheduler);
    printHeader();
    std:: string command;

    while(true)
    {
        std::cout <<"Enter a command: ";
        std::getline(std::cin, command);
        processCommand(command);
    }

    schedulerThread.join();
    return 0;
}