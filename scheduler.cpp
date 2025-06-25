#include "scheduler.h"
#include "process_instruction.h"

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

// Globals
std::queue<Process*> readyQueue;
std::vector<Process*> finishedProcesses;
std::mutex mtx;
std::condition_variable cv;
std::mutex coutMutex; // NEW: for thread-safe output

bool initialized = false;
std::atomic<bool> generateProcess = false;
std::atomic<int> pidCounter = 0;

std::unique_ptr<std::thread> dummyProcessThread;
std::unique_ptr<std::thread> mainSchedulerThread;
std::vector<std::thread> cpuCores;

std::map<std::string, Process*> allProcesses;
std::map<std::string, Process*> runningProcesses;
std::vector<std::string> outputs;

const int NUM_CORES = 4;
int processGenerationIntervalTicks = 5000;

// CPU worker thread
void cpuWorker(int coreID) {
    while (true) {
        Process* p = nullptr;

        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [] { return !readyQueue.empty() || !initialized; });

            if (!initialized && readyQueue.empty()) return;

            if (!readyQueue.empty()) {
                p = readyQueue.front();
                readyQueue.pop();

                if (p && !p->finished) {
                    p->assignedCoreID = coreID;
                    runningProcesses[p->name] = p;
                } else {
                    p = nullptr;
                }
            }
        }

        if (p) {
            ProcessContext ctx;
            ctx.name = p->name;
            ctx.instructionPointer = 0;

            while (ctx.instructionPointer < p->instructions.size()) {
                const Instruction& inst = p->instructions[ctx.instructionPointer];

                if (inst.type == Instruction::Type::PRINT) {
                    p->logPrintCommand(coreID);
                    std::string output = "Output from process " + p->name + " on core " + std::to_string(coreID);
                    p->addOutput(output);
                }

                executeInstruction(ctx, inst);
                ctx.instructionPointer++;
                p->currentInstruction++;

                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }

            {
                std::lock_guard<std::mutex> lock(mtx);
                p->finished = true;
                finishedProcesses.push_back(p);
                runningProcesses.erase(p->name);
            }
        }
    }
}

// Scheduler lifecycle
void startCpuWorkers() {
    for (int i = 0; i < NUM_CORES; i++) {
        cpuCores.emplace_back(cpuWorker, i);
    }
}

void joinCpuWorkers() {
    for (std::thread& t : cpuCores) {
        if (t.joinable()) {
            std::thread localThread = std::move(t);
            localThread.join();
        }
    }
    cpuCores.clear();
}

void runScheduler() {
    startCpuWorkers();
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return !initialized; });
    }
    joinCpuWorkers();
}

void startScheduler() {
    std::lock_guard<std::mutex> lock(mtx);
    if (!initialized) {
        initialized = true;
        mainSchedulerThread = std::make_unique<std::thread>(runScheduler);
        cv.notify_all();
    }
}

void stopScheduler() {
    std::unique_lock<std::mutex> lock(mtx);
    if (initialized) {
        initialized = false;
        cv.notify_all();
        lock.unlock();

        if (mainSchedulerThread && mainSchedulerThread->joinable()) {
            mainSchedulerThread->join();
            mainSchedulerThread.reset();
        }
    }
}

// Process control
void addNewProcess(const std::string& processName) {
    std::lock_guard<std::mutex> lock(mtx);

    Process* p = new Process(processName);
    p->pid = pidCounter++;

    p->instructions.push_back({Instruction::Type::PRINT});
    p->instructions.push_back({Instruction::Type::DECLARE, {"x", "5"}});
    p->instructions.push_back({Instruction::Type::ADD, {"x", "3", "2"}});
    p->instructions.push_back({Instruction::Type::PRINT});

    p->totalInstructions = p->instructions.size();
    readyQueue.push(p);
    allProcesses[p->name] = p;

    cv.notify_all();
}

// Dummy process generator
void dummyProcessGenerator() {
    int counter = 0;
    while (generateProcess) {
        std::this_thread::sleep_for(std::chrono::milliseconds(processGenerationIntervalTicks));
        addNewProcess("p" + std::to_string(counter++));
    }
}

void startDummyProcesses() {
    if (!generateProcess.load()) {
        generateProcess.store(true);
        dummyProcessThread = std::make_unique<std::thread>(dummyProcessGenerator);
    }
}

void stopDummyProcesses() {
    if (generateProcess.load()) {
        generateProcess.store(false);
        if (dummyProcessThread && dummyProcessThread->joinable()) {
            dummyProcessThread->join();
            dummyProcessThread.reset();
        }
    }
}

void printSchedulerStatus() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif

    // Title Header
    std::cout << "\033[1;34m======= SCHEDULER STATUS =======\033[0m\n\n";

    int runningCores = runningProcesses.size();
    int availCores = NUM_CORES - runningCores;
    double cpuPercentage = (static_cast<double>(runningCores) / NUM_CORES) * 100;

    std::cout << "CPU Utilization: " << cpuPercentage << "%\n"
              << "Cores used: " << runningCores << "\n"
              << "Cores available: " << availCores << "\n";

    std::lock_guard<std::mutex> lock(mtx);

    std::cout << "\n\033[36mRunning processes:\033[0m\n";
    if (!runningProcesses.empty()) {
        for (const auto& pair : runningProcesses) {
            auto p = pair.second;
            std::cout << "  " << p->name
                      << "\t(" << std::put_time(std::localtime(&p->startTime), "%m/%d/%Y %I:%M:%S%p")
                      << ")\tCore: " << p->assignedCoreID
                      << "\t" << p->currentInstruction << "/" << p->totalInstructions << "\n";
        }
    } else {
        std::cout << "  None\n";
    }

    std::cout << "\n\033[33mQueued processes:\033[0m\n";
    std::queue<Process*> tmpQueue = readyQueue;
    if (tmpQueue.empty()) {
        std::cout << "  None\n";
    } else {
        while (!tmpQueue.empty()) {
            Process* p = tmpQueue.front();
            tmpQueue.pop();
            std::cout << "  " << p->name
                      << "\t(" << std::put_time(std::localtime(&p->startTime), "%m/%d/%Y %I:%M:%S%p")
                      << ")\tWaiting\n";
        }
    }

    std::cout << "\n\033[32mFinished processes:\033[0m\n";
    if (finishedProcesses.empty()) {
        std::cout << "  None\n";
    } else {
        for (auto p : finishedProcesses) {
            std::cout << p->name
                      << "\t(" << std::put_time(std::localtime(&p->startTime), "%m/%d/%Y %I:%M:%S%p")
                      << ")\tFinished\t" << p->totalInstructions << "/" << p->totalInstructions << "\n";
        }
    }

    std::cout << "\n\033[35mPress ENTER to return to the main menu...\033[0m";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();

#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}
