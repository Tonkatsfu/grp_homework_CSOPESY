#include "menu_processor.h"
#include "initialize.h"
#include "scheduler.h"
#include "cpu_tick_global.h"
#include "memory_manager.h"

#include <cstdlib>

std::mutex logFileMutex;

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
    std::cout << "-------------------------------------------------------------------------------------------------\n";
    std::cout << "\033[33mType 'exit' to quit, 'clear' to clear the screen\033[0m\n";
}

void processCommand(const std::string& command) {
    if(isInitialized) {
        // Screen -ls (Print the Scheduler Status)
        if (command == "screen -ls") {
            std::cout << std::endl; 
            std::cout << "-----------------------------------------------------------------------\n";
            std::cout << "\033[34mScheduler Statistics\033[0m\n";
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
                std::cout << "Terminating command line emulator." << std::endl;
                stopDummyProcesses();
                stopScheduler();
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
            std::cout << std::endl; 
            std::lock_guard<std::mutex> logLock(logFileMutex); 
            std::ofstream logFile("csopesy-log.txt", std::ios::app);
            if (logFile.is_open())
            {
                auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                char buffer[80];
                strftime(buffer, sizeof(buffer), "%m/%d/%Y %I:%M:%S%p", std::localtime(&now));

                printSchedulerStatus(logFile);
                logFile.close();
                std::cout << "\033[34mSuccesfully generated report in file csopesy-log.txt\033[0m" << std::endl;
                std::cout << "-----------------------------------------------------------------------\n";
            }

            else
            {
                std::cout << "Failed to open csopesy-log.txt for writing." << std::endl;
            }
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

