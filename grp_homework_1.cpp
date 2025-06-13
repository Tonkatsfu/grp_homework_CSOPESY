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

int main()
{
    printHeader();
    startScheduler();

    for (int i = 1; i <= 10; ++i)
    {
        addNewProcess("Process_" + std::to_string(i));
    }

    std::string command;

    while (true)
    {
        std::cout << "Enter a command: ";
        std::getline(std::cin, command);

        processCommand(command);

        if (command == "exit")
        {
            break;
        }

    }

    stopScheduler();
    return 0;
}
