#include "initialize.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iomanip>

int numCPU, quantumCycles, batchProcessFreq, minIns, maxIns, delayPerExec;
int maxOverallMem, memPerFrame, minMemPerProc, maxMemPerProc;
std::string scheduler;

bool isValidMemorySize(int mem) {
    return mem >= 64 && mem <= 65536 && (mem & (mem - 1)) == 0;
}

void initialize() {
    std::ifstream config("config.txt");

    if (!config.is_open()) {
        std::cerr << "Failed to open config.txt\n";
        return;
    } 
    
    std::string key;
    while (config >> key) {
        if (key == "num-cpu") {
            config >> numCPU;
        } else if (key == "scheduler") {
            config >> scheduler;
        } else if (key == "quantum-cycles") {
            config >> quantumCycles;
        } else if (key == "batch-process-freq") {
            config >> batchProcessFreq;
        } else if (key == "min-ins") {
            config >> minIns;
        } else if (key == "max-ins") {
            config >> maxIns;
        } else if (key == "delay-per-exec") {
            config >> delayPerExec;
        } else if (key == "max-overall-mem") {
            config >> maxOverallMem;
        } else if (key == "mem-per-frame") {
            config >> memPerFrame;
        } else if (key == "min-mem-per-proc") {
            config >> minMemPerProc;
        } else if (key == "max-mem-per-proc") {
            config >> maxMemPerProc;
        } else {
            std::string skip;
            std::getline(config, skip); 
        }
    }

    // Validate memory sizes
    if (!isValidMemorySize(maxOverallMem) ||
        !isValidMemorySize(memPerFrame) ||
        !isValidMemorySize(minMemPerProc) ||
        !isValidMemorySize(maxMemPerProc)) 
    {
        std::cerr << "\n\033[31mError: One or more memory parameters are not a power of 2 or out of valid range (64–65536 bytes).\033[0m\n";
        std::exit(EXIT_FAILURE);
    }

    std::cout << "\033[2J\033[1;1H";
    std::cout << "\033[33m[System] Configuration file loaded!\033[0m\n\n";

    std::cout << "+------------------------+------------------------+\n";
    std::cout << "| \033[34mConfiguration Key\033[0m      | \033[34mValue\033[0m                  |\n";
    std::cout << "+------------------------+------------------------+\n";

    std::cout << "| numCPU                 | " << std::left << std::setw(23) << numCPU << "|\n";
    std::cout << "| scheduler              | " << std::left << std::setw(23) << scheduler << "|\n";
    std::cout << "| quantumCycles          | " << std::left << std::setw(23) << quantumCycles << "|\n";
    std::cout << "| batchProcessFreq       | " << std::left << std::setw(23) << batchProcessFreq << "|\n";
    std::cout << "| minIns                 | " << std::left << std::setw(23) << minIns << "|\n";
    std::cout << "| maxIns                 | " << std::left << std::setw(23) << maxIns << "|\n";
    std::cout << "| delayPerExec           | " << std::left << std::setw(23) << delayPerExec << "|\n";
    std::cout << "| maxOverallMem          | " << std::left << std::setw(23) << maxOverallMem << "|\n";
    std::cout << "| memPerFrame            | " << std::left << std::setw(23) << memPerFrame << "|\n";
    std::cout << "| minMemPerProc          | " << std::left << std::setw(23) << minMemPerProc << "|\n";
    std::cout << "| maxMemPerProc          | " << std::left << std::setw(23) << maxMemPerProc << "|\n";

    std::cout << "+------------------------+------------------------+\n";
}
