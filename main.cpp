#include "menu_processor.h"
#include "initialize.h"
#include "scheduler.h"
#include "cpu_tick_global.h"
#include "memory_manager.h"

#include <iostream>
#include <string>
#include <thread>
#include <chrono>

std::map<int, std::string> processesInMemoryMap;
int nextProcessId = 1;

int main() {

#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    printHeader();

    std::string command;

    while (!terminateProgram) {
        std::cout << std::endl; 
        std::cout << "Enter a command: ";
        std::getline(std::cin, command);
        processCommand(command);
    } 

    return 0;
}
