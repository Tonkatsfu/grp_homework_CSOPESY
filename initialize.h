#pragma once
#include <string>

extern int numCPU, quantumCycles, batchProcessFreq, minIns, maxIns, delayPerExec;
extern std::string scheduler;

void initialize();
