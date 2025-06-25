#include "screen_processor.h"
#include "scheduler.h"
#include <iomanip>  // for std::setw
#include <sstream>  // for std::ostringstream

std::string currentScreenName = "";

void ScreenConsoles(const Process& process) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif

    std::cout << "+----------------------------------------------+\n";
    std::cout << "|              Process Information             |\n";
    std::cout << "+----------------------------------------------+\n";
    std::cout << "| Process Name : " << std::setw(30) << std::left << process.name << "|\n";
    std::cout << "| Process ID   : " << std::setw(30) << std::left << process.pid << "|\n";
    std::cout << "| Core ID      : " << std::setw(30) << std::left << process.assignedCoreID << "|\n";
    std::cout << "| Current Line : " << process.currentInstruction << " / " << process.totalInstructions;
    std::cout << std::setw(18 - std::to_string(process.currentInstruction).length() - std::to_string(process.totalInstructions).length()) << " " << "|\n";
    std::cout << "+----------------------------------------------+\n\n";

    std::cout << "Logs (last 5):\n";

    int logStart = process.logs.size() > 5 ? process.logs.size() - 5 : 0;
    for (size_t i = logStart; i < process.logs.size(); ++i) {
        std::cout << process.logs[i];
    }

    std::cout << "\n--------------------------------------------------\n";
}



void ProcessSMI(const std::string& processName)
{
    std::lock_guard<std::mutex> lock(mtx);

    auto it = allProcesses.find(processName);
    if (it == allProcesses.end()) {
        std::cout << "Process not found.\n";
        return;
    }

    const Process* process = it->second;

    std::cout << "\n\033[36m--- Process: " << process->name << " (ID: " << process->pid << ") ---\033[0m\n";

    std::cout << "\nLogs (last 5):\n";
    int logStart = std::max(0, static_cast<int>(process->logs.size()) - 5);
    for (int i = logStart; i < process->logs.size(); ++i) {
        std::cout << "  " << process->logs[i];
    }

    if (!process->finished) {
        std::cout << "\nCurrent Instruction: " << process->currentInstruction
                  << " / " << process->totalInstructions << "\n";
    } else {
        std::cout << "\nStatus: \033[32mFinished\033[0m\n";
    }

    std::cout << "\n\033[33mPress ENTER to return.\033[0m\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}
