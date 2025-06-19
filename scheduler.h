#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <iostream>
#include <memory> 
#include <string> 
#include <thread> 

extern std::unique_ptr<std::thread> mainSchedulerThread;
extern std::atomic <bool> generateProcess;
extern int processGenerationIntervalTicks;


void startScheduler();
void stopScheduler();
void addNewProcess(const std::string& processName);
void printSchedulerStatus();
void dummyProcessGenerator();
void startDummyProcesses();
void stopDummyProcesses();

#endif
