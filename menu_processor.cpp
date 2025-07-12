#include "menu_processor.h"
#include <cstdlib>
#include "scheduler.h"
#include "initialize.h"

bool terminateProgram = false;
bool isInitialized = false;

std::mutex logFileMutex;

void printHeader()
{
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

    /*
    if (!activeScreens.empty())
    {
        std::cout << "\n--- Active Screens ---\n";
        for (const auto& pair : activeScreens)
        {
            std::cout << "  - " << pair.first << "\n";
        }
        std::cout << "----------------------\n";
    }
        */
}

void processCommand(const std::string& command)
{
    if(isInitialized) {
        if (command == "screen -ls")
        {
            printSchedulerStatus(std::cout);
        }
        
        else if (command == "clear")
        {
    #ifdef _WIN32
            system("cls");
    #else
            system("clear");
    #endif
            if (currentScreenName.empty())
                printHeader();
            else
                ScreenConsoles(*allProcesses[currentScreenName]);
        }
        
        else if (command == "exit")
        {
            if (currentScreenName.empty())
            {
                std::cout << "Terminating command line emulator." << std::endl;
                stopDummyProcesses();
                stopScheduler();
                //exit(0); 
                terminateProgram = true;
            }
            else 
            {
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
        
        else if (command.rfind("screen -s ", 0) == 0)
        {
            std::string args = command.substr(10);
            size_t spacePos = args.find(' ');

            std::string processName;
            int memorySize;

            if (processName.empty())
            {
                std::cout << "Usage: screen -s <name>" << std::endl;
                return;
            }

            if (spacePos != std::string::npos) {
                processName = args.substr(0, spacePos);
                std::string memStr = args.substr(spacePos + 1);

                try {
                    memorySize = std::stoi(memStr);
                } catch (...) {
                    std::cout << "Invalid memory size. Usage: screen -s <name> <memory>" << std::endl;
                    return;
                }
            } else {
                std::cout << "Missing memory size. Usage: screen -s <name> <memory>" << std::endl;
                return;
            }

            if (memorySize < 64 || memorySize > 65536) {
                std::cout << "Memory must be between 64 and 65536." << std::endl;
                return;
            }

            /*
            if (activeScreens.count(screenName))
            {
                std::cout << "Screen name " << screenName <<" already exists." << std::endl;
            }
                */

            else
            {
                if(allProcesses[processName] != NULL && allProcesses[processName]->finished == false){
                    std:: cout << "Screen " << processName << " already exists you may want to use screen -r <process name>.\n";
                } else {
                    currentScreenName = processName;
                    addNewProcess(processName, memorySize);
                    //startScheduler();
                    ScreenConsoles(*allProcesses[processName]);
                }
            }
        }

        else if (command.rfind("screen -r ", 0) == 0)
        {
            std:: string screenName = command.substr(10);
            std:: lock_guard<std::mutex> lock(mtx); 
            auto it = allProcesses.find(screenName);
            if (it != allProcesses.end() && it->second->finished == false)
            {
                currentScreenName = screenName;
                ScreenConsoles(*it->second);
            }

            else
            {
                std::cout << "No screen found with name: " << screenName << std::endl;
            }
        }

        else if (command == "scheduler -start")
        {
            startDummyProcesses();
        }

        else if (command == "scheduler -stop")
        {
            stopDummyProcesses();
        }

        else if(command == "process-smi"){
            if (currentScreenName != "") //if the current screen is not the main menu, thus a process screen. CHANGE THIS IF THERE WILL BE OTHER SCREENS THAN A PROCESS SCREEN!
            {
                ProcessSMI(currentScreenName);
            }else{
                std:: cout << "You are currently not in a process screen, use screen -s <process name> to create one or screen -r <process name> to resume a process screen" << std::endl;
            }
        }
        
        else if (command == "report -util")
        {
            std::lock_guard<std::mutex> logLock(logFileMutex); //lock the mutex to ensure thread safety
            std::ofstream logFile("csopesy-log.txt", std::ios::app);
            if (logFile.is_open())
            {
                auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                char buffer[80];
                strftime(buffer, sizeof(buffer), "%m/%d/%Y %I:%M:%S%p", std::localtime(&now));

                printSchedulerStatus(logFile);
                logFile.close();
                std::cout << "Succesfully generated report in file csopesy-log.txt" << std::endl;
                printSchedulerStatus(std::cout);
            }

            else
            {
                std::cout << "Failed to open csopesy-log.txt for writing." << std::endl;
            }
        }
        
        else 
        {
            std::cout <<"Please enter a valid command." << std::endl;
        }
    } else {
        if (command == "initialize"){
            //initialize();
            isInitialized = true;
        }else if (command == "exit"){
            terminateProgram = true;
        }
        else{
            std::cout <<"To input other commands please initialize the program by typing initialize." << std::endl;
        }
    }
    
}

