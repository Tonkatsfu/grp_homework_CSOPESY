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
#include <variant>
#include <cstdint>
#include <algorithm>
using Arg = std::variant<std::string, int>;

#ifdef _WIN32
#include <direct.h>  // for _mkdir
#else
#include <sys/stat.h>  // for mkdir
#endif

enum class OpCode { ADD, SUBTRACT, SLEEP, PRINT, FOR };

struct Instruction {
    OpCode opcode;
    std::vector<Arg> args;
    std::vector<Instruction> nestedInstructions;

    Instruction(OpCode op, std::initializer_list<Arg> arguments)
        : opcode(op), args(arguments) {}
    
    // Constructor for FOR loops
    Instruction(OpCode op, std::initializer_list<Arg> arguments, std::vector<Instruction> nested)
        : opcode(op), args(arguments), nestedInstructions(std::move(nested)) {}
};


struct Process
{
    std:: string name;
    int pid = 0;
    int totalInstructions = 100;
    int currentInstruction = 0;
    int assignedCoreID = -1;
    std:: time_t startTime;
    bool finished = false;
    std::vector<std::string> logs;
    std::ofstream logFile;  // <-- Log for "print" commands
    std::map<std::string, int> variables; //map for storing dzeclared variables of key value pair var name and value
    int sleepTicksRemaining = 0; // Number of ticks left to sleep
    bool isSleeping() const { return sleepTicksRemaining > 0; }
    std::vector<Instruction> instructionList;
    bool memoryAllocated = false; // Flag to check if memory is allocated for the process
    size_t lowerLimit = 0;
    size_t upperLimit = 0;


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

    void logPrintCommand(int coreID, std::string s) {
        if (logFile.is_open()) {
            auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            char buffer[80];
            strftime(buffer, sizeof(buffer), "%m/%d/%Y %I:%M:%S %p", std::localtime(&now));
            std::ostringstream oss;
            if(s == ""){
                oss << "[" << buffer << "] Core " << coreID << ": Hello world from " << name << "! \n";
                logs.push_back(oss.str());
                logFile << "[" << buffer << "] Core " << coreID << ": Hello world from " << name << "! \n";
            }else{
                oss << "[" << buffer << "] Core " << coreID << ": " << s << "\n";
                logs.push_back(oss.str());
                logFile << "[" << buffer << "] Core " << coreID << ": " << s << "\n";;
            }
            logFile.flush();
        }
    }

    void DECLARE(std::string var, int value, int coreID){
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%m/%d/%Y %I:%M:%S %p", std::localtime(&now));
        std::ostringstream oss;

        if(variables.find(var) == variables.end()){
            variables[var] = value;

            oss << "[" << buffer << "] Core " << coreID << ": Variable " << var << " with value " << value << " successfully declared \n";
            logs.push_back(oss.str());
            logFile << "[" << buffer << "] Core " << coreID << ": Variable " << var << " with value " << value << " successfully declared \n";
        }else{
            oss << "[" << buffer << "] Core " << coreID << ": Variable " << var << " with value " << value << " was unsuccesfull in declaration \n";
            logs.push_back(oss.str());
        }

        logFile.flush();
    }

    void ADD(Arg var1, Arg var2, Arg var3, int coreID) {
        if (!std::holds_alternative<std::string>(var1)) {
            std::cerr << "ADD: var1 must be a string (destination variable name)" << std::endl;
            return;
        }

        std::string destVar = std::get<std::string>(var1);

        // Lambda to resolve variable or integer
        auto resolve = [this, coreID](const Arg& arg) -> uint16_t {
        if (std::holds_alternative<int>(arg)) {
            return static_cast<uint16_t>(std::get<int>(arg));
        } else {
            std::string varName = std::get<std::string>(arg);
            if (variables.find(varName) == variables.end()) {
                // Auto-declare with 0 if not found
                DECLARE(varName, 0, coreID);
            }
            return variables[varName];
            }
        };


        // Auto-declare var1 if not exists
        if (variables.find(destVar) == variables.end()) {
            DECLARE(destVar, 0, coreID);  // Create destVar with initial value 0
        }

        uint16_t value2 = resolve(var2);
        uint16_t value3 = resolve(var3);

        int32_t result = value2 + value3;
        result = std::clamp(result, 0, static_cast<int32_t>(UINT16_MAX));

        variables[destVar] = result;

        // Logging
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%m/%d/%Y %I:%M:%S %p", std::localtime(&now));
        std::ostringstream oss;

        oss << "[" << buffer << "] Core " << coreID << ": ADD " << destVar
            << " = " << value2 << " + " << value3 << " = " << result << "\n";

        logs.push_back(oss.str());
        logFile << oss.str();
        logFile.flush();
    }

    void SUBTRACT(Arg var1, Arg var2, Arg var3, int coreID) {
        if (!std::holds_alternative<std::string>(var1)) {
            std::cerr << "SUBTRACT: var1 must be a string (destination variable name)" << std::endl;
            return;
        }

        std::string destVar = std::get<std::string>(var1);

        // Lambda to resolve variable or integer
        auto resolve = [this, coreID](const Arg& arg) -> int32_t {
        if (std::holds_alternative<int>(arg)) {
            return static_cast<uint16_t>(std::get<int>(arg));
        } else {
            std::string varName = std::get<std::string>(arg);
            if (variables.find(varName) == variables.end()) {
                // Auto-declare with 0 if not found
                DECLARE(varName, 0, coreID);
            }
            return variables[varName];
            }
        };


        // Auto-declare var1 if not exists
        if (variables.find(destVar) == variables.end()) {
            DECLARE(destVar, 0, coreID);  // Create destVar with initial value 0
        }

        uint16_t value2 = resolve(var2);
        uint16_t value3 = resolve(var3);

        int32_t result = std::clamp(result, 0, static_cast<int32_t>(UINT16_MAX));

        variables[destVar] = result;

        // Logging
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%m/%d/%Y %I:%M:%S %p", std::localtime(&now));
        std::ostringstream oss;

        oss << "[" << buffer << "] Core " << coreID << ": SUBTRACT " << destVar
            << " = " << value2 << " - " << value3 << " = " << result << "\n";

        logs.push_back(oss.str());
        logFile << oss.str();
        logFile.flush();
    }

    void SLEEP(uint16_t ticks, int coreID){
        sleepTicksRemaining = ticks;
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%m/%d/%Y %I:%M:%S %p", std::localtime(&now));
        std::ostringstream oss;

        oss << "[" << buffer << "] Core " << coreID << ": Sleeping for " << ticks << "CPU ticks \n";

        logs.push_back(oss.str());
        logFile << oss.str();
        logFile.flush();

    }

    void FOR_LOOP(int repeats, const std::vector<Instruction>& instructions, int coreID) {
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%m/%d/%Y %I:%M:%S %p", std::localtime(&now));
    std::ostringstream oss;
    
    oss << "[" << buffer << "] Core " << coreID << ": Starting FOR loop with " << repeats << " iterations\n";
    logs.push_back(oss.str());
    logFile << oss.str();
    logFile.flush();

    for (int i = 0; i < repeats; i++) {
        // Execute each instruction in the loop body
        for (const auto& instr : instructions) {
            switch (instr.opcode) {
                case OpCode::ADD:
                    ADD(instr.args[0], instr.args[1], instr.args[2], coreID);
                    break;
                case OpCode::SUBTRACT:
                    SUBTRACT(instr.args[0], instr.args[1], instr.args[2], coreID);
                    break;
                case OpCode::SLEEP:
                    SLEEP(std::get<int>(instr.args[0]), coreID);
                    break;
                case OpCode::PRINT:
                    logPrintCommand(coreID, "");
                    break;
                case OpCode::FOR:
                    FOR_LOOP(std::get<int>(instr.args[0]), instr.nestedInstructions, coreID);
                    break;
            }
        
            // Check for sleep ticks after each instruction
            if (sleepTicksRemaining > 0) {
                return; // Exit and let scheduler handle the sleep
            }
        }
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
extern bool initialized;


void startScheduler();
void stopScheduler();
void addNewProcess(const std::string& processName);
void printSchedulerStatus(std::ostream& os);
void dummyProcessGenerator();
void startDummyProcesses();
void stopDummyProcesses();

#endif
