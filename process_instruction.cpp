#include "process_instruction.h"
#include <iostream>
#include <algorithm>
#include <mutex>

// Optional: mutex for synchronized console output
extern std::mutex coutMutex;

void executeInstruction(ProcessContext& ctx, const Instruction& inst) {
    using Type = Instruction::Type;

    if (ctx.sleepTicks > 0) {
        ctx.sleepTicks--;
        return;
    }

    switch (inst.type) {
        case Type::PRINT: {
            std::string msg = "Hello world from " + ctx.name + "!";
            {
                std::lock_guard<std::mutex> lock(coutMutex);  
                std::cout << msg << std::endl;
            }
            break;
        }

        case Type::DECLARE: {
            ctx.variables[inst.args[0]] = static_cast<uint16_t>(std::stoi(inst.args[1]));
            break;
        }

        case Type::ADD:
        case Type::SUBTRACT: {
            auto getVal = [&](const std::string& s) -> uint16_t {
                if (isdigit(s[0])) return static_cast<uint16_t>(std::stoi(s));
                return ctx.variables[s];
            };
            uint16_t v2 = getVal(inst.args[1]);
            uint16_t v3 = getVal(inst.args[2]);
            uint16_t result = inst.type == Type::ADD
                ? std::clamp<uint32_t>(v2 + v3, 0, UINT16_MAX)
                : std::clamp<int32_t>(v2 - v3, 0, UINT16_MAX);
            ctx.variables[inst.args[0]] = result;
            break;
        }

        case Type::SLEEP: {
            ctx.sleepTicks = static_cast<uint8_t>(std::stoi(inst.args[0]));
            break;
        }

        case Type::FOR: {
            if (ctx.forStack.empty() || ctx.forStack.back().first != ctx.instructionPointer) {
                ctx.forStack.push_back({ctx.instructionPointer, inst.repeats});
            }
            auto& [start, count] = ctx.forStack.back();
            if (count > 0) {
                for (const auto& subInst : inst.subInstructions)
                    executeInstruction(ctx, subInst);
                count--;
            } else {
                ctx.forStack.pop_back(); // done
            }
            break;
        }
    }
}
