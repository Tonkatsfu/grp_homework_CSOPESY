#include "scheduler.h"
#include "initialize.h"
#include "menu_processor.h"
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
#include <random>

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

            if (!initialized && readyQueue.empty()) {
                return;
            }

            if (!readyQueue.empty())
            {
                p = readyQueue.front();
                readyQueue.pop();
                p->assignedCoreID = coreID;
                runningProcesses[p->name] = p;
            }
        }

        if (p)
        {
            if (scheduler == "\"rr\"") {
                int slice = 0;
                bool wasRequeued = false;

                while (p->currentInstruction < p->totalInstructions && slice < quantumCycles) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(delayPerExec));
                    if (delayPerExec == 0) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    }
                    const Instruction& instr = p->instructionList[p->currentInstruction];

                    if (p->sleepTicksRemaining > 0) {
                        p->sleepTicksRemaining--;

                        std::lock_guard<std::mutex> lock(mtx);
                        readyQueue.push(p);
                        runningProcesses.erase(p->name);
                        wasRequeued = true;
                        break;
                    }

                     switch (instr.opcode) {
                        case OpCode::ADD:
                            p->ADD(instr.args[0], instr.args[1], instr.args[2], coreID);
                            break;
                        case OpCode::SUBTRACT:
                            p->SUBTRACT(instr.args[0], instr.args[1], instr.args[2], coreID);
                            break;
                        case OpCode::SLEEP:
                            p->SLEEP(std::get<int>(instr.args[0]), coreID);
                            break;
                        case OpCode::PRINT:
                            p->logPrintCommand(coreID, "");
                            break;
                        case OpCode::FOR:
                            p->FOR_LOOP(std::get<int>(instr.args[0]), instr.nestedInstructions, coreID);
                            break;
                    }

                    p->currentInstruction++;
                    slice++;

                    if (p->sleepTicksRemaining > 0) {
                        std::lock_guard<std::mutex> lock(mtx);
                        readyQueue.push(p);
                        runningProcesses.erase(p->name);
                        wasRequeued = true;
                        break;
                    }
            }

            if (!wasRequeued) {
                std::lock_guard<std::mutex> lock(mtx);
                if (p->currentInstruction >= p->totalInstructions) {
                    p->finished = true;
                    finishedProcesses.push_back(p);
                } else {
                    readyQueue.push(p);
                }
                runningProcesses.erase(p->name);
            }
        }
            else {
                // FCFS
                //std::cout << "scheduler is FCFS\n\n";

                while (p->currentInstruction < p->totalInstructions) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(delayPerExec));
                    if (delayPerExec == 0)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1)); 
                    }
                    const Instruction& instr = p->instructionList[p->currentInstruction];

                    if(p->sleepTicksRemaining > 0){
                        p->sleepTicksRemaining--;

                        std::lock_guard<std::mutex> lock(mtx);
                        readyQueue.push(p);
                        runningProcesses.erase(p->name);
                        break;
                    }

                    switch (instr.opcode) {
                        case OpCode::ADD:
                            p->ADD(instr.args[0], instr.args[1], instr.args[2], coreID);
                            break;
                        case OpCode::SUBTRACT:
                            p->SUBTRACT(instr.args[0], instr.args[1], instr.args[2], coreID);
                            break;
                        case OpCode::SLEEP:
                            p->SLEEP(std::get<int>(instr.args[0]), coreID);
                            break;
                        case OpCode::PRINT:
                            if (!instr.args.empty() && std::holds_alternative<std::string>(instr.args[0])) {
                                std::string message = std::get<std::string>(instr.args[0]);
                                p->logPrintCommand(coreID, message);
                                } else {
                                p->logPrintCommand(coreID, ""); // Fallback
                            }
                            break;
                        case OpCode::FOR:
                            p->FOR_LOOP(std::get<int>(instr.args[0]), instr.nestedInstructions, coreID);
                            break;
                    }
                    p->currentInstruction++;
                }

                std::lock_guard<std::mutex> lock(mtx);
                if (p->sleepTicksRemaining == 0 && p->currentInstruction >= p->totalInstructions) {
                    p->finished = true;
                    finishedProcesses.push_back(p);
                    runningProcesses.erase(p->name);
                }
            

            }
        }
    }
}

void startCpuWorkers()
{
    for (int i = 0; i<numCPU; i++)
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

/*
void addNewProcess(const std::string& processName)
{
    std::lock_guard<std::mutex> lock(mtx);
    Process* p = new Process(processName);
    p->pid = pidCounter++;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(minIns, maxIns);
    p->totalInstructions = dist(gen);

    readyQueue.push(p);
    allProcesses[p->name] = p;
    cv.notify_all(); 
}
    */

    
void addNewProcess(const std::string& processName) {
    std::lock_guard<std::mutex> lock(mtx);
    Process* p = new Process(processName);
    p->pid = pidCounter++;

    // Set up random number generators
    std::random_device rd;
    std::mt19937 gen(rd());

    // Number of instructions (must be even for PRINT/ADD pairs)
    std::uniform_int_distribution<> instructionCountDist(minIns, maxIns);
    p->totalInstructions = instructionCountDist(gen);
    if (p->totalInstructions % 2 != 0) {
        p->totalInstructions++; // Make even
    }

    // Random ADD values between 1-10
    std::uniform_int_distribution<> addValueDist(1, 10);

    // Initialize variable
    p->variables["x"] = 0;

    // Create alternating PRINT/ADD instructions
    for (int i = 0; i < p->totalInstructions; i++) {
        if (i % 2 == 0) { // PRINT instruction
            // Create PRINT instruction with formatted string
        } 
        else { // ADD instruction
            int addValue = addValueDist(gen);
            p->instructionList.push_back(Instruction(OpCode::ADD, {"x", "x", addValue}));
        }
    }

    // Add to system
    readyQueue.push(p);
    allProcesses[p->name] = p;
    cv.notify_all();
}




void printSchedulerStatus(std::ostream& os)
{
    // Clear the screen before printing status
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif

    int runningCores = runningProcesses.size();
    int availCores = numCPU - runningCores;

    double cpuPercentage = (static_cast<double>(runningCores) / numCPU) * 100;

    os << "CPU Utilization: " << cpuPercentage << "%\n" ;
    os << "Cores used: " << runningCores << " \n";
    os << "Cores available: " << availCores << " \n";

    std::lock_guard<std::mutex> lock(mtx);
    os << "\nRunning processes:\n";
    if (!runningProcesses.empty())
    {
        for (const auto& pair : runningProcesses)
        {
            auto p = pair.second;
            os << "  " << p->name
                      << "\t(" << std::put_time(std::localtime(&p->startTime), "%m/%d/%Y %I:%M:%S%p")
                      << ")\tCore: " << p->assignedCoreID 
                      << "\t" << p->currentInstruction << "/" << p->totalInstructions << "\n";
        }
    }
    else
    {
        os << "  None\n";
    }

    os << "\nQueued processes:\n";
    std::queue<Process*> tmpQueue = readyQueue;
    if (tmpQueue.empty())
    {
        os << "  None\n";
    }
    else
    {
        while (!tmpQueue.empty())
        {
            Process* p = tmpQueue.front();
            tmpQueue.pop();
            os << "  " << p->name
                      << "\t(" << std::put_time(std::localtime(&p->startTime), "%m/%d/%Y %I:%M:%S%p")
                      << ")\tWaiting\n";
        }
    }

    os << "\nFinished processes:\n";
    if (finishedProcesses.empty())
    {
        os << "  None\n";
    }
    else
    {
        for (auto p : finishedProcesses)
        {
            os << p->name << "\t(" << std::put_time(std::localtime(&p->startTime), "%m/%d/%Y %I:%M:%S%p")
                      << ")\tFinished\t" << p->totalInstructions << "/" << p->totalInstructions << "\n";
        }
    }
}

void dummyProcessGenerator()
{
    int ticks = 0;
    int counter = 0;
    while (generateProcess)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(delayPerExec));
        if (delayPerExec == 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        ticks++;
        if (ticks >= batchProcessFreq)
        {
            addNewProcess("p" + std::to_string(counter++));
            ticks = 0;
        }
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





