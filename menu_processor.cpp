#include "menu_processor.h"
#include <cstdlib>
#include "scheduler.h"
#include "initialize.h"
#include "memory_manager.h"
#include <iomanip>

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
    std::cout << std::endl; 
    std::cout << "\033[33mType 'exit' to quit, 'clear' to clear the screen\033[0m\n";
}

void printProcessSMI()
{
    std::cout << "\nMemory Summary\n";
    std::cout << "-------------------------------------------------------------------------------------------------\n";
    std::cout << getMemoryUsageReport() << "\n";
    std::cout << "-------------------------------------------------------------------------------------------------\n";
    std::cout << "\033[34m"  
          << std::left << std::setw(20) << "PID"
          << std::right << std::setw(80) << "Memory Usage (MiB)"
          << "\033[0m\n";


    for (const auto& pair : allProcesses)
    {
        std::cout << std::left << std::setw(20) << pair.first
                  << std::right << std::setw(70) << getMemoryUsedByProcess(pair.second->pid) << " MiB" << std::endl;
    }

    std::cout << "\n\n";
}

void processCommand(const std::string& command)
{
    if (isInitialized)
    {
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

            if (spacePos == std::string::npos)
            {
                std::cout << "Missing memory size. Usage: screen -s <name> <memory>" << std::endl;
                return;
            }

            std::string processName = args.substr(0, spacePos);
            std::string memStr = args.substr(spacePos + 1);

            if (processName.empty())
            {
                std::cout << "Usage: screen -s <name> <memory>" << std::endl;
                return;
            }

            int memorySize;
            try
            {
                memorySize = std::stoi(memStr);
            }
            catch (...)
            {
                std::cout << "Invalid memory size. Usage: screen -s <name> <memory>" << std::endl;
                return;
            }

            if (memorySize < 64 || memorySize > 65536)
            {
                std::cout << "Memory must be between 64 and 65536." << std::endl;
                return;
            }

            if (allProcesses[processName] != nullptr && !allProcesses[processName]->finished)
            {
                std::cout << "Screen " << processName << " already exists. You may want to use screen -r <process name>.\n";
            }
            else
            {
                currentScreenName = processName;
                addNewProcess(processName, memorySize);

                if (allProcesses.count(processName) && allProcesses[processName] != nullptr)
                {
                    ScreenConsoles(*allProcesses[processName]);
                }
                else
                {
                    std::cout << "Failed to create screen for process: " << processName << std::endl;
                }
            }
        }

        else if (command.rfind("screen -r ", 0) == 0)
        {
            std::string screenName = command.substr(10);
            std::lock_guard<std::mutex> lock(mtx);
            auto it = allProcesses.find(screenName);
            if (it != allProcesses.end() && it->second->finished == false)
            {
                currentScreenName = screenName;
                ScreenConsoles(*it->second);
            }
            else
            {
                Process* p = getProcessByPid(screenName);
                if (p != nullptr)
                {
                    if (p->accessViolation == true)
                    {
                        std::cout << p->accessViolationMessage << std::endl;
                    }
                    else
                    {
                        std::cout << "No screen found with name: " << screenName << std::endl;
                    }
                }
                else
                {
                    std::cout << "No screen found with name: " << screenName << std::endl;
                }
            }
        }

        else if (command.rfind("screen -c ", 0) == 0)
{
    std::string args = command.substr(10);
    size_t firstSpace = args.find(' ');
    if (firstSpace == std::string::npos)
    {
        std::cout << "Missing memory size. Usage: screen -c <name> <memory> \"<instructions>\"" << std::endl;
        return;
    }

    std::string processName = args.substr(0, firstSpace);
    args = args.substr(firstSpace + 1);

    size_t secondSpace = args.find(' ');
    if (secondSpace == std::string::npos)
    {
        std::cout << "Missing instructions. Usage: screen -c <name> <memory> \"<instructions>\"" << std::endl;
        return;
    }

    std::string memStr = args.substr(0, secondSpace);
    std::string instructions = args.substr(secondSpace + 1);

    int memorySize;
    try
    {
        memorySize = std::stoi(memStr);
    }
    catch (...)
    {
        std::cout << "Invalid memory size. Usage: screen -c <name> <memory> \"<instructions>\"" << std::endl;
        return;
    }

    if (memorySize < 64 || memorySize > 65536)
    {
        std::cout << "Memory must be between 64 and 65536." << std::endl;
        return;
    }

    // Trim spaces around instructions
    auto trim = [](std::string &s) {
        s.erase(0, s.find_first_not_of(" \t\n\r"));
        s.erase(s.find_last_not_of(" \t\n\r") + 1);
    };
    trim(instructions);

    // Safely strip surrounding quotes if present
    if (instructions.size() >= 2 &&
        instructions.front() == '"' &&
        instructions.back() == '"')
    {
        instructions = instructions.substr(1, instructions.size() - 2);
    }

    size_t instrCount = std::count(instructions.begin(), instructions.end(), ';') + 1;
    if (instrCount < 1 || instrCount > 50)
    {
        std::cout << "Invalid command: Instruction count must be between 1 and 50." << std::endl;
        return;
    }

    if (allProcesses[processName] != nullptr && !allProcesses[processName]->finished)
    {
        std::cout << "Screen " << processName << " already exists. Use screen -r instead.\n";
    }
    else
    {
        currentScreenName = processName;
        addNewProcess(processName, memorySize);

        if (allProcesses.count(processName) && allProcesses[processName] != nullptr)
        {
            // Store instructions
            allProcesses[processName]->userInstructions = instructions;

            parseInstructions(
                allProcesses[processName]->userInstructions,
                allProcesses[processName]->instructionList
            );

            executeInstructions(allProcesses[processName]);

            // Launch process console
            ScreenConsoles(*allProcesses[processName]);
        }
        else
        {
            std::cout << "Failed to create process: " << processName << std::endl;
        }
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

        else if (command == "vmstat")
        {
            printVMStat();
        }

        else if (command == "process-smi")
        {
            if (currentScreenName != "")
            {
                ProcessSMI(currentScreenName);
            }
            else
            {
                printProcessSMI();
            }
        }

        else if (command == "report -util")
        {
            std::lock_guard<std::mutex> logLock(logFileMutex);
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
            std::cout << "Please enter a valid command." << std::endl;
        }
    }
    else
    {
        if (command == "initialize")
        {
            initialize();
            isInitialized = true;
        }
        else if (command == "exit")
        {
            terminateProgram = true;
        }
        else
        {
            std::cout << "To input other commands please initialize the program by typing initialize." << std::endl;
        }
    }
}
