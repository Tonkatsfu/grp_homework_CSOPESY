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
        }
    }
}

void initializeScheduler()
{
    std::lock_guard<std::mutex> lock(mtx);
    for (int i = 1; i <= 10; ++i)
    {
        readyQueue.push(new Process(
            std::string("process") + (i < 10 ? "0" : "") + std::to_string(i) 
        ));
    }
    initialized = true;
    cv.notify_all();
}

void printSchedulerStatus()
{
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "\n\033[36mRunning processes:\033[0m\n";
    int core = 0;
    std::queue<Process*> tmp = readyQueue;
    while (!tmp.empty())
    {
        auto p = tmp.front(); tmp.pop();
        std::cout << p->name << "\t(" << std::put_time(std::localtime(&p->startTime), "%m/%d/%Y %I:%M:%S%p")
                  << ")\tCore: " << core++ % 4 << "\t" << p->currentInstruction << "/" << p->totalInstructions << "\n";
    }

    std::cout << "\n\033[32mFinished processes:\033[0m\n";
    for (auto p : finishedProcesses)
    {
        std::cout << p->name << "\t(" << std::put_time(std::localtime(&p->startTime), "%m/%d/%Y %I:%M:%S%p")
                  << ")\tFinished\t" << p->totalInstructions << "/" << p->totalInstructions << "\n";
    }
}

void runScheduler()
{
    const int NUM_CORES = 4;
    std::vector<std::thread> cpuCores;

    for (int i = 0; i < NUM_CORES; ++i)
    {
        cpuCores.emplace_back(cpuWorker, i);
    }

    std::thread schedulerThread([]() {
        initializeScheduler(); 
    });

    while (true)
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (readyQueue.empty() && finishedProcesses.size() == 10)
            {
                initialized = false; 
                cv.notify_all();
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (schedulerThread.joinable()) schedulerThread.join();
    for (auto& t : cpuCores)
        if (t.joinable()) t.join();

    std::cout << "--------------------------------------------\n";
    printSchedulerStatus();
    std::cout << "--------------------------------------------\n";
}
