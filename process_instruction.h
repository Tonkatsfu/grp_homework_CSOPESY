#ifndef PROCESS_INSTRUCTION_H
#define PROCESS_INSTRUCTION_H

#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <cstdint>

struct Instruction {
    enum Type { PRINT, DECLARE, ADD, SUBTRACT, SLEEP, FOR } type;
    std::vector<std::string> args; // store args like var names or literals
    std::vector<Instruction> subInstructions; // for FOR loops
    int repeats = 0; // for FOR
};

struct ProcessContext {
    std::string name;
    std::unordered_map<std::string, uint16_t> variables;
    std::vector<Instruction> instructions;
    size_t instructionPointer = 0;
    uint8_t sleepTicks = 0;
    std::vector<std::pair<size_t, int>> forStack; // index + repeat count
};

void executeInstruction(ProcessContext& ctx, const Instruction& inst);

#endif
