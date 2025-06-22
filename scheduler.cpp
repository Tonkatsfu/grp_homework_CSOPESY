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
#include <atomic>

std::queue<Process*> readyQueue;
std::vector<Process*> finishedProcesses;
std::mutex mtx;
std::condition_variable cv;
bool initialized = false;
std::atomic <bool> generateProcess = false;
std::atomic<int> pidCounter = 0;

std::unique_ptr<std::thread> dummyProcessThread;
std::unique_ptr<std::thread> mainSchedulerThread;
std::vector<std::thread> cpuCores;
std::map<std::string, Process*> allProcesses;
std::map<std::string, Process*> runningProcesses;
int processGenerationIntervalTicks = 5000;

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
                runningProcesses[p->name] = p;
            }
        }

        if (p)
        {
            while (p->currentInstruction < p->totalInstructions)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50)); 
                p->logPrintCommand(coreID);  
                p->currentInstruction++;
            }

            std::lock_guard<std::mutex> lock(mtx);
            p->finished = true;
            finishedProcesses.push_back(p);
            runningProcesses.erase(p->name);
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
    Process* p = new Process(processName);
    p->pid = pidCounter++;
    readyQueue.push(p);
    allProcesses[p->name] = p;
    cv.notify_all(); 
}


void printSchedulerStatus()
{
    // Clear the screen before printing status
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif

    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "\n\033[36mRunning processes:\033[0m\n";
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
    else
    {
        std::cout << "  None\n";
    }

    std::cout << "\n\033[33mQueued processes:\033[0m\n";
    std::queue<Process*> tmpQueue = readyQueue;
    if (tmpQueue.empty())
    {
        std::cout << "  None\n";
    }
    else
    {
        while (!tmpQueue.empty())
        {
            Process* p = tmpQueue.front();
            tmpQueue.pop();
            std::cout << "  " << p->name
                      << "\t(" << std::put_time(std::localtime(&p->startTime), "%m/%d/%Y %I:%M:%S%p")
                      << ")\tWaiting\n";
        }
    }

    std::cout << "\n\033[32mFinished processes:\033[0m\n";
    if (finishedProcesses.empty())
    {
        std::cout << "  None\n";
    }
    else
    {
        for (auto p : finishedProcesses)
        {
            std::cout << p->name << "\t(" << std::put_time(std::localtime(&p->startTime), "%m/%d/%Y %I:%M:%S%p")
                      << ")\tFinished\t" << p->totalInstructions << "/" << p->totalInstructions << "\n";
        }
    }
}

void dummyProcessGenerator()
{
    int counter = 0;
    while (generateProcess)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(processGenerationIntervalTicks));
        addNewProcess("p" + std::to_string(counter));
        counter++;
    }
}

void startDummyProcesses()
{
    if (!generateProcess.load())
    {
        generateProcess.store(true);
        dummyProcessThread = std::make_unique<std::thread>(dummyProcessGenerator);
    }
}

void stopDummyProcesses()
{
    if (generateProcess.load())
    {
        generateProcess.store(false);
        if (dummyProcessThread && dummyProcessThread->joinable())
        {
            dummyProcessThread->join();
            dummyProcessThread.reset();
        }
    }
}





