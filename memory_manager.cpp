#include "memory_manager.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>

#include <chrono>
#include <ctime>
#include <cstdlib>

namespace fs = std::filesystem;
std::vector<MemoryBlock> memoryBlocks;

void initializeMemoryManager()
{
    memoryBlocks.clear();
    memoryBlocks.emplace_back(0, maxOverallMem, true);
    std::cout << "Memory Manager Initialized with " 
              << memoryBlocks.size() << " block(s).\n";
}

// First-Fit
bool allocateMemory(int processID, int memoryRequired)
{
    for (auto it = memoryBlocks.begin(); it != memoryBlocks.end(); ++it)
    {
        if (it->isFree && it-> size >= memoryRequired)
        {
            int originalSize = it->size;
            int originalStart = it->startAddress;

            it->isFree = false;
            it->processId = processID;
            it->size = memoryRequired;

            if (originalSize > memoryRequired)
            {
                memoryBlocks.emplace_back(originalStart + memoryRequired, 
                                        originalSize - memoryRequired, true);
            }

            std::sort(memoryBlocks.begin(), memoryBlocks.end(), 
                      [](const MemoryBlock& a, const MemoryBlock& b) {
                          return a.startAddress < b.startAddress;
                      });
            return true;
        }
    }
    return false;
}

void deallocateMemory(int processID)
{
    for (auto& block : memoryBlocks)
    {
        if (!block.isFree && block.processId == processID)
        {
            block.isFree = true;
            block.processId = 0;

            std::vector<MemoryBlock> newMemoryState;
            std::sort(memoryBlocks.begin(), memoryBlocks.end(), 
                      [](const MemoryBlock& a, const MemoryBlock& b) {
                          return a.startAddress < b.startAddress;
                      });
            
            if (!memoryBlocks.empty())
            {
                newMemoryState.push_back(memoryBlocks[0]);
                for (size_t i = 1; i < memoryBlocks.size(); ++i)
                {
                    MemoryBlock& last = newMemoryState.back();
                    MemoryBlock& current = memoryBlocks[i];

                    if (last.isFree && current.isFree)
                    {
                        last.size += current.size;
                    }
                    else
                    {
                        newMemoryState.push_back(current);
                    }
                }
            }
            memoryBlocks = newMemoryState;
            return;
        }
    }
}

bool hasEnoughFreeMemory(int requiredMem) 
{
    for (const auto& block : memoryBlocks) {
        if (block.isFree && block.size >= requiredMem) {
            return true;
        }
    }
    return false;
}

void printMemoryStatus(int qq) {
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%m/%d/%Y %I:%M:%S %p", std::localtime(&now));

    // Create the "memory_logs" directory using system call (Windows)
    system("mkdir memory_logs >nul 2>&1");

    std::string filePath = "memory_logs/memory_stamp_" + std::to_string(qq) + ".txt";
    std::ofstream logFile(filePath, std::ios::app);

    if (!logFile.is_open()) {
        std::cerr << "Error: Could not open " << filePath << "\n";
        return;
    }

    int procInMem = 0;
    int startMemUsage = 99999;
    int endMemUsage = 0;
    int totalExtFrag = 0;
    bool firstFreeFound = false;
    int lastFreeEnd = -1;
    int firstMemSize = 0; //REMOVE THIS IF WHAT I SAID IN GC IS INCORRECT
    int totalFreeMemBlocks = 0; //REMOVE THIS IF WHAT I SAID IN GC IS INCORRECT

    for (MemoryBlock mem : memoryBlocks) {
        if (!mem.isFree) {
            procInMem++;
            if (startMemUsage >= mem.startAddress) {
                startMemUsage = mem.startAddress;
            }
            if (endMemUsage < (mem.startAddress + mem.size)) {
                endMemUsage = mem.startAddress + mem.size;
            }
        } else {
            if (!firstFreeFound) {
                // First free block, just track its end
                lastFreeEnd = mem.startAddress + mem.size;
                firstFreeFound = true;
                totalExtFrag += mem.size;
                firstMemSize += mem.size; //REMOVE THIS IF WHAT I SAID IN GC IS INCORRECT
                totalFreeMemBlocks++; //REMOVE THIS IF WHAT I SAID IN GC IS INCORRECT
            } else {
                // Not contiguous with the previous free block
                if (mem.startAddress != lastFreeEnd) {
                    totalExtFrag += mem.size;
                    totalFreeMemBlocks++; //REMOVE THIS IF WHAT I SAID IN GC IS INCORRECT
                } else {
                    // Update the end of contiguous block
                    lastFreeEnd = mem.startAddress + mem.size;
                }
            }
        }
    }

    if(totalFreeMemBlocks < 2){ //REMOVE THIS IF WHAT I SAID IN GC IS INCORRECT
        totalExtFrag = totalExtFrag - firstMemSize;
    }

    logFile << "Timestamp: " << buffer << "\n";
    logFile << "Number of processes in memory: " << procInMem << "\n";
    logFile << "Total External Fragmentation: " << totalExtFrag << "\n\n";
    logFile << "----end---- = " << endMemUsage << "\n";

    for (MemoryBlock mem : memoryBlocks) {
        if (!mem.isFree) {
            logFile << mem.startAddress + mem.size << "\n";
            logFile << "P" << mem.processId << "\n";
            logFile << mem.startAddress << "\n\n";
        }
    }

    logFile << "----start---- = " << startMemUsage << "\n\n";
    logFile.close();
}
