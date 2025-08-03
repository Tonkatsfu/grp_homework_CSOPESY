#ifndef INITIALIZE_H
#define INITIALIZE_H

#pragma once
#include <string>

extern int numCPU, quantumCycles, batchProcessFreq, minIns, maxIns, delayPerExec, 
           maxOverallMem, memPerFrame, minMemPerProc, maxMemPerProc;
extern std::string scheduler;

void initialize();

#endif