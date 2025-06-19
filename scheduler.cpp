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

struct Process
{
    std:: string name;
    int totalInstructions = 100;
    int currentInstruction = 0;
    std:: time_t startTime;
    bool finished = false;
    std::ofstream logFile;  // <-- Log for "print" commands

    Process(const std::string& n) : name(n), startTime(std::time(nullptr)) {
        logFile.open(name + "_log.txt", std::ios::out);
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
            logFile << "[" << buffer << "] Core " << coreID << ": print command executed\n";
            logFile.flush();
        }
    }
};

std::queue<Process*> readyQueue;
std::vector<Process*> finishedProcesses;
std::mutex mtx;
std::condition_variable cv;
bool initialized = false;
std::atomic <bool> generateProcess = false;

std::unique_ptr<std::thread> dummyProcessThread;
std::unique_ptr<std::thread> mainSchedulerThread;
std::vector<std::thread> cpuCores;
std::map<int, Process*> runningProcesses;
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
                runningProcesses[coreID] = p;
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
        addNewProcess("Dummy Process" + std::to_string(counter));
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





