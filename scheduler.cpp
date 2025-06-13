#include "scheduler.h"
#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <map>

struct Process
{
    std:: string name;
    int totalInstructions = 100;
    int currentInstruction = 0;
    std:: time_t startTime;
    bool finished = false;

    Process(const std::string& n) : name(n), startTime(std::time(nullptr)) {}
};

std::queue<Process*> readyQueue;
std::vector<Process*> finishedProcesses;
std::mutex mtx;
std::condition_variable cv;
bool initialized = false;

std::unique_ptr<std::thread> mainSchedulerThread;
std::vector<std::thread> cpuCores;
std::map<int, Process*> runningProcesses;

void cpuWorker(int coreID)
{
    while(true)
    {
        Process* p = nullptr;
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [] { return !readyQueue.empty() || !initialized; });

            if (!initialized && readyQueue.empty()) return;

            if (!readyQueue.empty())
            {
                p = readyQueue.front();
                readyQueue.pop();
                runningProcesses[coreID] = p;
            }
        }

        if (p)
        {
            while (p->currentInstruction < p->totalInstructions)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                p->currentInstruction++;
            }

            std::lock_guard<std::mutex> lock(mtx);
            p->finished = true;
            finishedProcesses.push_back(p);
            runningProcesses.erase(coreID);
        }
    }
}

void startCpuWorkers()
{
    const int NUM_CORES = 4;
    for (int i = 0; i<NUM_CORES; i++)
    {
        cpuCores.emplace_back(cpuWorker, i);
    }
}

void joinCpuWorkers()
{
    for (std::thread& t : cpuCores) 
    {
        if (t.joinable()) 
        {
            t.join();
        }
    }
    cpuCores.clear();
}

void runScheduler()
{
    startCpuWorkers();
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, []{ return !initialized; });
    }

    joinCpuWorkers();
}

void startScheduler()
{
    std::lock_guard<std::mutex> lock(mtx);
    if (!initialized)
    {
        initialized = true;
        mainSchedulerThread = std::make_unique<std::thread>(runScheduler);
    }
}

void stopScheduler()
{
    std::unique_lock<std::mutex> lock(mtx);
    if (initialized)
    {
        initialized = false;
        cv.notify_all();;
        lock.unlock();

        if (mainSchedulerThread && mainSchedulerThread->joinable()) 
        {
            mainSchedulerThread->join(); 
            mainSchedulerThread.reset();
        }
    }
}

void addNewProcess(const std::string& processName)
{
    std::lock_guard<std::mutex> lock(mtx);
    readyQueue.push(new Process(processName));
    cv.notify_all(); 
}


void printSchedulerStatus()
{
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "\n\033[36mRunning processes:\033[0m\n";
    int core = 0;
    std::queue<Process*> tmp = readyQueue;
    if (!runningProcesses.empty())
    {
        for (const auto& pair : runningProcesses)
        {
            auto p = pair.second;
            std::cout << "  " << p->name
                      << "\t(" << std::put_time(std::localtime(&p->startTime), "%m/%d/%Y %I:%M:%S%p")
                      << ")\tCore: " << pair.first 
                      << "\t" << p->currentInstruction << "/" << p->totalInstructions << "\n";
        }
    }

    std::cout << "\n\033[32mFinished processes:\033[0m\n";
    for (auto p : finishedProcesses)
    {
        std::cout << p->name << "\t(" << std::put_time(std::localtime(&p->startTime), "%m/%d/%Y %I:%M:%S%p")
                  << ")\tFinished\t" << p->totalInstructions << "/" << p->totalInstructions << "\n";
    }
}



