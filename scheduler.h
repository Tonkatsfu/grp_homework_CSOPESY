#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <iostream>
#include <memory> 
#include <string> 
#include <thread> 
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <thread>
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

struct Process
{
    std:: string name;
    int pid = 0;
    int totalInstructions = 100;
    int currentInstruction = 0;
    std:: time_t startTime;
    bool finished = false;
    std::vector<std::string> logs;
    std::ofstream logFile;  // <-- Log for "print" commands

    Process(const std::string& n) : name(n), startTime(std::time(nullptr))
    {
        // Create directory "process_logs" if it doesn't exist
#ifdef _WIN32
        _mkdir("process_logs");
#else
        mkdir("process_logs", 0777);
#endif

        std::string filePath = "process_logs/" + name + "_log.txt";
        logFile.open(filePath, std::ios::out);
    }

    ~Process() {
        if (logFile.is_open())
            logFile.close();
    }

    void logPrintCommand(int coreID) {
        if (logFile.is_open()) {
            auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            char buffer[80];
            strftime(buffer, sizeof(buffer), "%m/%d/%Y %I:%M:%S %p", std::localtime(&now));
            std::ostringstream oss;
            oss << "[" << buffer << "] Core " << coreID << ": print command executed\n";
            logs.push_back(oss.str());
            logFile << "[" << buffer << "] Core " << coreID << ": print command executed\n";
            logFile.flush();
        }
    }
};

extern std::unique_ptr<std::thread> mainSchedulerThread;
extern std::queue<Process*> readyQueue;
extern std::map<std::string, Process*> runningProcesses;
extern std::map<std::string, Process*> allProcesses;
extern std::atomic <bool> generateProcess;
extern int processGenerationIntervalTicks;
extern std::mutex mtx;


void startScheduler();
void stopScheduler();
void addNewProcess(const std::string& processName);
void printSchedulerStatus();
void dummyProcessGenerator();
void startDummyProcesses();
void stopDummyProcesses();

#endif
