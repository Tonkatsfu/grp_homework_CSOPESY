#ifndef INITIALIZE_H
#define INITIALIZE_H

#pragma once
#include <string>

extern int numCPU, quantumCycles, batchProcessFreq, minIns, maxIns, delayPerExec, 
           maxOverallMem, memPerFrame, memPerProc;
extern std::string scheduler;

void initialize();

#endif