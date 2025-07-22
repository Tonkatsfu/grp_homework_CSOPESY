#include "initialize.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>

int numCPU, quantumCycles, batchProcessFreq, minIns, maxIns, delayPerExec;
int maxOverallMem, memPerFrame, memPerProc, minMemPerProc, maxMemPerProc;
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
        } else if (key == "mem-per-proc") {
            config >> memPerProc;
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
        !isValidMemorySize(memPerProc) ||
        !isValidMemorySize(minMemPerProc) ||
        !isValidMemorySize(maxMemPerProc)) 
    {
        std::cerr << "\n\033[31mError: One or more memory parameters are not a power of 2 or out of valid range (64–65536 bytes).\033[0m\n";
        std::exit(EXIT_FAILURE);
    }

    std::cout << "\nConfiguration Loaded!\n";
    std::cout << "-----------------------------------------------\n";
    std::cout << "\033[34mConfiguration:\033[0m\n";
    std::cout << std::endl;
    std::cout << "  numCPU: " << numCPU << "\n"
              << "  scheduler: " << scheduler << "\n"
              << "  quantumCycles: " << quantumCycles << "\n"
              << "  batchProcessFreq: " << batchProcessFreq << "\n"
              << "  minIns: " << minIns << "\n"
              << "  maxIns: " << maxIns << "\n"
              << "  delayPerExec: " << delayPerExec << "\n"
              << "  maxOverallMem: " << maxOverallMem << "\n"
              << "  memPerFrame: " << memPerFrame << "\n"
              << "  memPerProc: " << memPerProc << "\n"
              << "  minMemPerProc: " << minMemPerProc << "\n"
              << "  maxMemPerProc: " << maxMemPerProc << "\n";
}
