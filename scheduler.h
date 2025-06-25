#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <map>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <atomic>

#ifdef _WIN32
#include <direct.h>  // for _mkdir
#else
#include <sys/stat.h>  // for mkdir
#endif

#include "process_instruction.h"  
struct Process
{
    std::string name;
    int pid = 0;
    int totalInstructions = 100;
    int currentInstruction = 0;
    int assignedCoreID = -1;
    std::time_t startTime;
    bool finished = false;

    std::vector<Instruction> instructions;
    std::vector<std::string> logs;
    std::ofstream logFile;
    
    std::string lastOutput;  // <-- This is what you were missing

    Process(const std::string& n) : name(n), startTime(std::time(nullptr))
    {
#ifdef _WIN32
        _mkdir("process_logs");
#else
        mkdir("process_logs", 0777);
#endif
        std::string filePath = "process_logs/" + name + "_log.txt";
        logFile.open(filePath, std::ios::out);
    }

    ~Process()
    {
        if (logFile.is_open())
            logFile.close();
    }

    void logPrintCommand(int coreID)
    {
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%m/%d/%Y %I:%M:%S %p", std::localtime(&now));
        std::ostringstream oss;
        oss << "[" << buffer << "] Core " << coreID << ": print command executed\n";
        logs.push_back(oss.str());
        if (logFile.is_open()) {
            logFile << oss.str();
            logFile.flush();
        }
    }

    void addOutput(const std::string& output)
    {
        lastOutput = output;  // <-- Store the latest output here too
        std::string formatted = "    > " + output + "\n";
        logs.push_back(formatted);
        if (logFile.is_open()) {
            logFile << formatted;
            logFile.flush();
        }
    }
};


// Global declarations
extern std::unique_ptr<std::thread> mainSchedulerThread;
extern std::queue<Process*> readyQueue;
extern std::map<std::string, Process*> runningProcesses;
extern std::map<std::string, Process*> allProcesses;
extern std::atomic<bool> generateProcess;
extern int processGenerationIntervalTicks;
extern std::mutex mtx;

// Scheduler function declarations
void startScheduler();
void stopScheduler();
void addNewProcess(const std::string& processName);
void printSchedulerStatus();
void dummyProcessGenerator();
void startDummyProcesses();
void stopDummyProcesses();

#endif
