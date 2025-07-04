#include "initialize.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

int numCPU, quantumCycles, batchProcessFreq, minIns, maxIns, delayPerExec;
std::string scheduler;

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
        } else {
            std::string skip;
            std::getline(config, skip);
        }
    }

    std::cout << "Configuration Loaded:\n"
              << "  numCPU: " << numCPU << "\n"
              << "  scheduler: " << scheduler << "\n"
              << "  quantumCycles: " << quantumCycles << "\n"
              << "  batchProcessFreq: " << batchProcessFreq << "\n"
              << "  minIns: " << minIns << "\n"
              << "  maxIns: " << maxIns << "\n"
              << "  delayPerExec: " << delayPerExec << "\n\n";
}
