#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <iostream>
#include <memory> 
#include <string> 
#include <thread> 

extern std::unique_ptr<std::thread> mainSchedulerThread;


void startScheduler();
void stopScheduler();
void addNewProcess(const std::string& processName);
void printSchedulerStatus();

#endif
