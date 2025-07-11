#include "memory_manager.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>

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

std::vector<MemoryBlock> getAllocatedBlocks() {
    std::vector<MemoryBlock> allocated;
    for (const auto& block : memoryBlocks) {
        if (!block.isFree) {
            allocated.push_back(block);
        }
    }
    return allocated;
}

int calculateExternalFragmentation() {
    int fragmentation = 0;
    for (const auto& block : memoryBlocks) {
        if (block.isFree) {
            fragmentation += block.size;
        }
    }
    return fragmentation / 1024; // convert to KB
}
