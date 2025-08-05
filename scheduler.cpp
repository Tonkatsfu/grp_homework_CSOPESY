#include "scheduler.h"
#include "initialize.h"
#include "menu_processor.h"
#include "cpu_tick_global.h"
#include "memory_manager.h"
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

std::atomic<int> globalSliceCounter = 0;
std::mutex sliceLogMutex;

static std::string trim(const std::string& s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace(*start)) {
        start++;
    }
    auto end = s.end();
    do {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(*end));
    return std::string(start, end + 1);
}

void cpuWorker(int coreID)
{
    while (true)
    {
        Process* p = nullptr;
        {
        std::unique_lock<std::mutex> lock(mtx);

        if (readyQueue.empty() && initialized) {
            if (globalCpuTicker) globalCpuTicker->incIdleTicks(); // increment each loop when idle
            cv.wait(lock, [] {
                return !readyQueue.empty() || !initialized;
            });
        } else {
            cv.wait(lock, [] {
                return !readyQueue.empty() || !initialized;
            });
        }

        if (!initialized && readyQueue.empty()) {
            return;
        }

        if (!readyQueue.empty()) {
            p = readyQueue.front();
            readyQueue.pop();
            p->assignedCoreID = coreID;
            runningProcesses[p->name] = p;
        }
    }

        if (p)
        {
            if (!p->memoryAllocated)
            {
                if (!allocateMemory(p->pid, minMemPerProc))
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    readyQueue.push(p);
                    runningProcesses.erase(p->name);
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }
                p->memoryAllocated = true;
            }
            
            if (scheduler == "\"rr\"") {
                int slice = 0;
                bool wasRequeued = false;

                while (p->currentInstruction < p->totalInstructions && 
                       slice < quantumCycles && 
                       !p->finished) 
                {
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

                    // Execute instruction
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
                        case OpCode::WRITE:
                            p->WRITE(getRandomValidAddress(p->pid), std::get<int>(instr.args[0]));
                            break;
                        case OpCode::READ:
                            p->READ(std::get<std::string>(instr.args[0]), getRandomValidAddress(p->pid));
                    }

                    if (globalCpuTicker) globalCpuTicker->incActiveTicks(); // ACTIVE TICKS
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

                if (coreID == 0) {  
                    std::lock_guard<std::mutex> logLock(sliceLogMutex);
                    printMemoryStatus(globalSliceCounter++);
                }

                if (!wasRequeued) {
                    std::lock_guard<std::mutex> lock(mtx);
                    if (p->currentInstruction >= p->totalInstructions || p->accessViolation) {
                        p->finished = true;
                        finishedProcesses.push_back(p);
                        deallocateMemory(p->pid);
                        p->memoryAllocated = false;
                    } else {
                        readyQueue.push(p);
                    }
                    runningProcesses.erase(p->name);
                }
            }
            else {
                // FCFS
                while (p->currentInstruction < p->totalInstructions && !p->finished) {
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
                        case OpCode::WRITE:
                            p->WRITE(getRandomValidAddress(p->pid), std::get<int>(instr.args[0]));
                            break;
                        case OpCode::READ:
                            p->READ(std::get<std::string>(instr.args[0]), getRandomValidAddress(p->pid));
                    }

                    if (globalCpuTicker) globalCpuTicker->incActiveTicks(); // ACTIVE TICKS
                    p->currentInstruction++;
                }

                std::lock_guard<std::mutex> lock(mtx);
                if (p->sleepTicksRemaining == 0 && 
                    (p->currentInstruction >= p->totalInstructions || p->accessViolation)) {
                    p->finished = true;
                    finishedProcesses.push_back(p);
                    runningProcesses.erase(p->name);
                    deallocateMemory(p->pid);
                    p->memoryAllocated = false;
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

    
void addNewProcess(const std::string& processName, int memorySize)
{
    std::lock_guard<std::mutex> lock(mtx);

    Process* p = new Process(processName);
    p->pid = pidCounter++;

    // Set up random number generators
    std::random_device rd;
    std::mt19937 gen(rd());

    // number of instructions
    std::uniform_int_distribution<> instructionCountDist(minIns, maxIns);
    p->totalInstructions = instructionCountDist(gen);

    // Available opcodes
    std::vector<OpCode> opcodes = {OpCode::ADD, OpCode::SUBTRACT, OpCode::SLEEP, OpCode::PRINT, OpCode::FOR, OpCode::WRITE, OpCode::READ};
    std::uniform_int_distribution<> opcodeDist(0, opcodes.size() - 1);
    std::uniform_int_distribution<> valueDist(1, 100);
    std::uniform_int_distribution<> sleepDist(3, 10);
    std::uniform_int_distribution<> loopCountDist(2, 5);
    std::uniform_int_distribution<> loopBodySizeDist(1, 3);

    // Create random instructions
    for (int i = 0; i < p->totalInstructions; ) {
        OpCode opcode = opcodes[opcodeDist(gen)];
    
        switch (opcode) {
            case OpCode::WRITE:
                p->instructionList.push_back(Instruction(OpCode::WRITE, {valueDist(gen)}));
                i++;
                break;
            case OpCode::READ:
                p->instructionList.push_back(Instruction(OpCode::READ, {"x"}));
                i++;
                break;
            case OpCode::ADD:
                p->instructionList.push_back(Instruction(OpCode::ADD, {"x", "x", valueDist(gen)}));
                i++;
                break;
            case OpCode::SUBTRACT:
                p->instructionList.push_back(Instruction(OpCode::SUBTRACT, {"x", "x", valueDist(gen)}));
                i++;
                break;
            case OpCode::SLEEP:
                p->instructionList.push_back(Instruction(OpCode::SLEEP, {sleepDist(gen)}));
                i++;
                break;
            case OpCode::PRINT:
                p->instructionList.push_back(Instruction(OpCode::PRINT, {}));
                i++;
                break;
            case OpCode::FOR: {
                // Random number of iterations (2-5)
                int loopCount = loopCountDist(gen);
                
                // Create random instructions for loop body (1-3 instructions)
                int loopBodySize = loopBodySizeDist(gen);
                std::vector<Instruction> loopBody;
                
                for (int j = 0; j < loopBodySize; j++) {
                    OpCode bodyOpcode = opcodes[opcodeDist(gen) % 4]; // Exclude FOR from body for simplicity
                    switch (bodyOpcode) {
                        case OpCode::ADD:
                            loopBody.push_back(Instruction(OpCode::ADD, {"x", "x", valueDist(gen)}));
                            break;
                        case OpCode::SUBTRACT:
                            loopBody.push_back(Instruction(OpCode::SUBTRACT, {"x", "x", valueDist(gen)}));
                            break;
                            /*
                        case OpCode::SLEEP:
                            loopBody.push_back(Instruction(OpCode::SLEEP, {sleepDist(gen)}));
                            break;
                            */
                        case OpCode::PRINT:
                            loopBody.push_back(Instruction(OpCode::PRINT, {}));
                            break;
                    }
                }
                
                p->instructionList.push_back(Instruction(OpCode::FOR, {loopCount}, loopBody));
                i++; // Only count the FOR instruction itself, not the body
                break;
            }
        }
    }

    p->variables["x"] = 0;

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
                      << ")\tFinished\t" << p->currentInstruction << "/" << p->totalInstructions << "\n";
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
            if (hasEnoughFreeMemory(minMemPerProc))
            {
                addNewProcess("p" + std::to_string(counter++), minMemPerProc);
            }
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

void stopDummyProcesses() {
    if (generateProcess.load()) {
        generateProcess.store(false);

        // Wait for dummy process thread to finish
        if (dummyProcessThread && dummyProcessThread->joinable()) {
            dummyProcessThread->join();
            dummyProcessThread.reset();
        }

        // Clean up all processes
        std::lock_guard<std::mutex> lock(mtx);
        for (auto& pair : allProcesses) {
            if (pair.second->logFile.is_open()) {
                pair.second->logFile.flush();
                pair.second->logFile.close();
            }
        }
    }
}

Process* getProcessByPid(std::string targetPid) {
    for (const auto& pair : finishedProcesses) {
        if (pair && pair->name == targetPid) {
            return pair;
        }
    }
    return nullptr;
}

void printVMStat() {
    int totalMemory = getTotalMemory();
    int usedMemory  = getConsumedMemory();
    int freeMemory  = totalMemory - usedMemory;

    unsigned long idleTicks   = getIdleCpuTicks();
    unsigned long activeTicks = getActiveCpuTicks();
    unsigned long totalTicks  = idleTicks + activeTicks;

    unsigned long pagedIn  = getNumPagesPagedIn();
    unsigned long pagedOut = getNumPagesPagedOut();

    std::cout << std::endl; 
    std::cout << "Virtual Memory Statistics\n";
    std::cout << "+------------------------+------------------------+\n";
    std::cout << "| \033[34mVirtual Memory Key\033[0m     | \033[34mValue\033[0m                  |\n";
    std::cout << "+------------------------+------------------------+\n";

    // Memory Info
    std::cout << "| Total Memory           | " << std::left << std::setw(23) << (std::to_string(totalMemory) + " B") << "|\n";
    std::cout << "| Used Memory            | " << std::left << std::setw(23) << (std::to_string(usedMemory) + " B")  << "|\n";
    std::cout << "| Free Memory            | " << std::left << std::setw(23) << (std::to_string(freeMemory) + " B")  << "|\n";

    // CPU Info
    std::cout << "| Idle CPU Ticks         | " << std::left << std::setw(23) << idleTicks   << "|\n";
    std::cout << "| Active CPU Ticks       | " << std::left << std::setw(23) << activeTicks << "|\n";
    std::cout << "| Total CPU Ticks        | " << std::left << std::setw(23) << totalTicks  << "|\n";

    // Paging Info
    std::cout << "| Pages Paged In         | " << std::left << std::setw(23) << pagedIn     << "|\n";
    std::cout << "| Pages Paged Out        | " << std::left << std::setw(23) << pagedOut    << "|\n";

    std::cout << "+------------------------+------------------------+\n";
    std::cout << std::endl; 
}

void parseInstructions(const std::string& input, std::vector<Instruction>& output) {
    std::stringstream ss(input);
    std::string token;

    while (std::getline(ss, token, ';')) {
        token = trim(token);
        token.erase(std::remove(token.begin(), token.end(), '\\'), token.end());

        if (token.empty()) continue;

        std::stringstream parts(token);
        std::string cmd;
        parts >> cmd;

        // DECLARE
        if (cmd == "DECLARE") {
            std::string var; 
            int value;
            if (parts >> var >> value) {
                output.emplace_back(OpCode::DECLARE, std::vector<Arg>{ var, value });
            } else {
                std::cerr << "Invalid DECLARE: " << token << "\n";
            }
        } 

        // ADD
        else if (cmd == "ADD") {
            std::string v1, v2, v3;
            if (parts >> v1 >> v2 >> v3) {
                output.emplace_back(OpCode::ADD, std::vector<Arg>{ v1, v2, v3 });
            } else {
                std::cerr << "Invalid ADD: " << token << "\n";
            }
        }

        // WRITE 
        else if (cmd == "WRITE") {
            int addr; 
            std::string var;
            if (parts >> std::hex >> addr >> var) {
                output.emplace_back(OpCode::WRITE, std::vector<Arg>{ addr, var });
            } else {
                std::cerr << "Invalid WRITE: " << token << "\n";
            }
        }

        // READ 
        else if (cmd == "READ") {
            std::string var; 
            int addr;
            if (parts >> var >> std::hex >> addr) {
                output.emplace_back(OpCode::READ, std::vector<Arg>{ var, addr });
            } else {
                std::cerr << "Invalid READ: " << token << "\n";
            }
        }

        // PRINT
        else if (cmd == "PRINT" || cmd.rfind("PRINT(", 0) == 0) {
            auto start = token.find('(');
            auto end = token.rfind(')');
            if (start != std::string::npos && end != std::string::npos && end > start) {
                std::string inner = token.substr(start + 1, end - start - 1);

                std::vector<Arg> printArgs;
                std::stringstream expr(inner);
                std::string part;
                
                while (std::getline(expr, part, '+')) {
                    part = trim(part);
                    if (!part.empty() && part.front() == '"' && part.back() == '"') {
                        // String literal (strip quotes)
                        printArgs.emplace_back(part.substr(1, part.size() - 2));
                    } else {
                        // Variable name (no quotes, trimmed)
                        printArgs.emplace_back(trim(part));
                    }
                }

                output.emplace_back(OpCode::PRINT, printArgs);
            } else {
                std::cerr << "Invalid PRINT syntax: " << token << "\n";
            }
        }

        // UNKNOWN 
        else {
            std::cerr << "Unknown instruction: " << token << "\n";
        }
    }
}


void executeInstructions(Process* process) {
    for (auto& instr : process->instructionList) {
        switch (instr.opcode) {
            
            // DECLARE
            case OpCode::DECLARE:
                if (instr.args.size() >= 2 &&
                    std::holds_alternative<std::string>(instr.args[0]) &&
                    std::holds_alternative<int>(instr.args[1])) 
                {
                    process->DECLARE(std::get<std::string>(instr.args[0]), std::get<int>(instr.args[1]), process->pid);
                } else {
                    std::cerr << "Error: DECLARE missing arguments\n";
                }
                break;

            // ADD
            case OpCode::ADD:
                if (instr.args.size() >= 3) {
                    process->ADD(instr.args[0], instr.args[1], instr.args[2], process->pid);
                } else {
                    std::cerr << "Error: ADD missing arguments\n";
                }
                break;

            // WRITE
            case OpCode::WRITE:
                if (instr.args.size() >= 2 &&
                    std::holds_alternative<int>(instr.args[0]) &&
                    std::holds_alternative<std::string>(instr.args[1])) 
                {
                    std::string varName = trim(std::get<std::string>(instr.args[1]));
                    process->WRITE(std::get<int>(instr.args[0]), process->variables[varName]);
                } else {
                    std::cerr << "Error: WRITE missing arguments\n";
                }
                break;

            // READ
            case OpCode::READ:
                if (instr.args.size() >= 2 &&
                    std::holds_alternative<std::string>(instr.args[0]) &&
                    std::holds_alternative<int>(instr.args[1])) 
                {
                    std::string varName = trim(std::get<std::string>(instr.args[0]));
                    process->READ(varName, std::get<int>(instr.args[1]));
                } else {
                    std::cerr << "Error: READ missing arguments\n";
                }
                break;

            // PRINT
            case OpCode::PRINT: {
    std::string result;
    for (auto& arg : instr.args) {
        if (std::holds_alternative<std::string>(arg)) {
            std::string val = trim(std::get<std::string>(arg));

            // Remove quotes if present
            if (!val.empty() && val.front() == '"' && val.back() == '"') {
                val = val.substr(1, val.size() - 2);
            }

            // Check if variable exists
            if (process->variables.find(val) != process->variables.end()) {
                result += std::to_string(process->variables[val]);
            } else {
                result += val; // Literal
            }
        }
        else if (std::holds_alternative<int>(arg)) {
            result += std::to_string(std::get<int>(arg));
        }
    }
    process->logPrintCommand(process->pid, result);
    break;
}


        }
    }
}





