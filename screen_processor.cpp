#include "screen_processor.h"
#include "scheduler.h"

//std::map<std::string, Process*> activeScreens;
std::string currentScreenName = "";

void ScreenConsoles(const Process& process)
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif

    std::cout << "\n\033[36m--- Process name: " << process.name << " ---\033[0m\n";
    std::cout << "ID: " << process.pid << "\n";
}

void ProcessSMI(const std::string& processName) {
    {
        std::lock_guard<std::mutex> lock(mtx); // pause the mutex process so that it can be safely accessed

        auto it = allProcesses.find(processName);
        if (it == allProcesses.end()) {
            std::cout << "Process not found.\n";
            return;
        }

        Process* process = it->second;
        std::cout << "Logs:\n";
        for (const std::string& log : process->logs) {
            std::cout << log;
        }

        if(process->finished == false){
            std::cout << "Current Instruction Line: " << process->currentInstruction << "\n";
            std::cout << "Lines of Code: " << process->totalInstructions << "\n\n";
        }else{
            std::cout << "\nFinished!\n\n";
        }
    } 
}

