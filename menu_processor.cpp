#include "menu_processor.h"
#include <cstdlib>
#include "scheduler.h"
#include "initialize.h"

bool terminateProgram = false;
bool isInitialized = false;

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
    std::cout << "Neo Monserratt\n";
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
    if(isInitialized){
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
        std::string screenName = command.substr(10);
        if (screenName.empty())
        {
            std::cout << "Usage: screen -s <name>" << std::endl;
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
            if(allProcesses[screenName] != NULL && allProcesses[screenName]->finished == false){
                std:: cout << "Screen " << screenName << " already exists you may want to use screen -r <process name>.\n";
            }else{
                currentScreenName = screenName;
                addNewProcess(screenName);
                //startScheduler();
                ScreenConsoles(*allProcesses[screenName]);
            }
        }
    }

    else if (command.rfind("screen -r ", 0) == 0)
    {
        std:: string screenName = command.substr(10);
        if (screenName.empty())
        {
            std::cout << "Usage: screen -r <name>" << std::endl;
            return;
        }

        if (allProcesses.count(screenName) && allProcesses[screenName]->finished == false)
        {
            currentScreenName = screenName;
            ScreenConsoles(*allProcesses[screenName]);
        }

        else
        {
            std:: cout << "Process " << screenName << " not found. Please use screen -s " << screenName << " to create it." << std::endl;
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
        std::ofstream logFile("csopesy-log.txt", std::ios::app);
        if (logFile.is_open())
        {
            auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            char buffer[80];
            strftime(buffer, sizeof(buffer), "%m/%d/%Y %I:%M:%S%p", std::localtime(&now));

            printSchedulerStatus(logFile);
            logFile.close();
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
    }
    else
    {
        if (command == "initialize"){
            initialize();
            isInitialized = true;
        }else if (command == "exit"){
            terminateProgram = true;
        }
        else{
            std::cout <<"To input other commands please initialize the program by typing initialize." << std::endl;
        }
    }
    
}

