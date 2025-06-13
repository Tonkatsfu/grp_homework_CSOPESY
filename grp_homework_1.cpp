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