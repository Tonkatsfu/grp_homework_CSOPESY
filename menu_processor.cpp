#include "menu_processor.h"
#include "initialize.h"
#include "scheduler.h"
#include "cpu_tick_global.h"
#include "memory_manager.h"

#include <cstdlib>

bool isInitialized = false;

void printHeader() {
    std::cout << "\033[36m ,-----. ,---.   ,-----. ,------. ,------. ,---.,--.   ,--. \n"
              << "'  .--./'   .-' '  .-.  '|  .--. '|  .---''   .- '\\  `.'  /  \n"
              << "|  |    `.  `-. |  | |  ||  '--' ||  `--, `.  `-. '.    /   \n"
              << "'  '--'\\.-'    |'  '-'  '|  | --' |  `---..-'    |  |  |    \n"
              << " `-----'`-----'  `-----' `--'     `------'`-----'   `--'    \033[0m\n\n";
    std::cout << "-------------------------------------------------------------------------------------------------\n";
    std::cout << "\033[32mHello, welcome to CSOPESY commandline!\033[0m\n";
    std::cout << "Developers:\n";
    std::cout << "Matthew Chua\n";
    std::cout << "Ian Gabriel De Jesus\n";
    std::cout << "Joemar Lapasaran\n";
    std::cout << "Neo Monserrat\n";
    std::cout << std::endl; 
    std::cout << "\033[33mType 'exit' to quit, 'clear' to clear the screen\033[0m\n";
    std::cout << "-------------------------------------------------------------------------------------------------\n";
}

void processCommand(const std::string& command) {
    if(isInitialized) {
        // Screen -ls (Print the Scheduler Status)
        if (command == "screen -ls") {
            printSchedulerStatus(std::cout);
        }
        
        // Clear (Clear Screen)
        else if (command == "clear") {
            #ifdef _WIN32
            system("cls");
            
            #else
            system("clear");
            
            #endif
            if (currentScreenName.empty()) {
                printHeader();
            }

            else {
                ScreenConsoles(*allProcesses[currentScreenName]);
            }
        }
        
        // Exit (Exit Emulator)
        else if (command == "exit") {
            if (currentScreenName.empty()) {
                std::cout << std::endl; 
                std::cout << "Terminating command line emulator." << std::endl;
                std::cout << std::endl; 
                terminateProgram = true;
            }

            else {
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
        
        // Screen -s (Create Screen)
        else if (command.rfind("screen -s ", 0) == 0) {
            std::string args = command.substr(10);
            size_t spacePos = args.find(' ');

            std::string processName;
            int memorySize;
            
            if (spacePos != std::string::npos) {
                processName = args.substr(0, spacePos);
                std::string memStr = args.substr(spacePos + 1);
                
                try {
                    memorySize = std::stoi(memStr);
                } catch (...) {
                    std::cout << "Invalid memory size. Usage: screen -s <name> <memory>" << std::endl;
                    return;
                }
            } 
            
            else {
                std::cout << "Missing memory size. Usage: screen -s <name> <memory>" << std::endl;
                return;
            }

            if (processName.empty()) {
                std::cout << "Usage: screen -s <name> <memory>" << std::endl;
                return;
            }

            if (memorySize < 64 || memorySize > 65536) {
                std::cout << "Memory must be between 64 and 65536." << std::endl;
                return;
            }

            if (allProcesses[processName] != nullptr && !allProcesses[processName]->finished) {
                std::cout << "Screen " << processName << " already exists. Use screen -r <process name>.\n";
            } else {
                currentScreenName = processName;
                addNewProcess(processName, memorySize);
                ScreenConsoles(*allProcesses[processName]);
            }
        }

        // Screen -c (Create Screen with Instructions)
        else if (command.rfind("screen -c ", 0) == 0) {
            std::istringstream iss(command);
            std::string cmd, flag, processName, memoryStr, instructionString;

            iss >> cmd >> flag >> processName >> memoryStr;
            std::getline(iss, instructionString);

            // Clean instruction string (remove leading/trailing quotes/spaces)
            instructionString.erase(0, instructionString.find_first_not_of(" \""));
            instructionString.erase(instructionString.find_last_not_of("\" ") + 1);

            if (processName.empty() || memoryStr.empty() || instructionString.empty()) {
                std::cout << "Invalid command. Usage: screen -c <name> <memory> \"<instructions>\"\n";
                return;
            }
            
            int memorySize;
            try {
                memorySize = std::stoi(memoryStr);
            } catch (...) {
                std::cout << "Invalid memory size. Usage: screen -c <name> <memory> \"<instructions>\"\n";
                return;
            }

            // Validate instruction count
            std::vector<std::string> instructions;
            std::stringstream ss(instructionString);
            std::string token;
            while (std::getline(ss, token, ';')) {
                std::string trimmed = token;
                trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
                trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);
                if (!trimmed.empty()) instructions.push_back(trimmed);
            }

            if (instructions.size() < 1 || instructions.size() > 50) {
                std::cout << "Invalid command. Instruction count must be between 1 and 50.\n";
                return;
            }

            if (allProcesses[processName] != nullptr && !allProcesses[processName]->finished) {
                std::cout << "Screen " << processName << " already exists. Use screen -r <process name>.\n";
            } else {
                currentScreenName = processName;
                addNewProcessWithInstructions(processName, memorySize, instructions);
                //ScreenConsoles(*allProcesses[processName]);
            }
        }

        // Screen -r (View Screen)
        else if (command.rfind("screen -r ", 0) == 0) {
            std:: string screenName = command.substr(10);
            std:: lock_guard<std::mutex> lock(mtx); 
            auto it = allProcesses.find(screenName);
            
            if (it != allProcesses.end() && it->second->finished == false) {
                currentScreenName = screenName;
                ScreenConsoles(*it->second);
            } else {
                std::cout << "No screen found with name: " << screenName << std::endl;
            }
        }

        // Scheduler -start (Start the Scheduler)
        else if (command == "scheduler -start") {
            startScheduler();
            startDummyProcesses();
        }

        // Scheduler -stop (Stop the Scheduler)
        else if (command == "scheduler -stop") {
            stopDummyProcesses();
            stopScheduler();
        }

        // Process -smi (View process details)
        else if (command == "process -smi") {
            if (!currentScreenName.empty()) {
                ProcessSMI(currentScreenName);
            } else {
                std::cout << "You are not in a process screen. Use screen -s or screen -r first.\n";
            }
        }

        // Report -util (Generates a report)
        else if (command == "report -util") {
            generateSchedulerReport();
        }

        // Process-smi
        else if (command == "process-smi") {
            displayProcessSMI();
        }

        // Vmstat
        else if (command == "vmstat") {
            displayVMStat();
        }

        
        else {
            std::cout <<"Please enter a valid command." << std::endl;
        }
    } else {
        // Initialize
        if(isInitialized == false && command == "initialize") {
            initialize();
            initializeMemoryManager();
            isInitialized = true;
        }
        
        else if (command == "exit") {
            terminateProgram = true;
        }
        
        else {
            std::cout <<"To input other commands please initialize the program by typing initialize." << std::endl;
        }
    }
}

