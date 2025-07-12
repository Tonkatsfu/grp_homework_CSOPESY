#ifndef memory_manager_h
#define memory_manager_h

#include "initialize.h"
#include <vector>
#include <string>

struct MemoryBlock 
{
    int startAddress; // Starting address of the memory block
    int size;         // Size of the memory block in bytes
    bool isFree;      // Indicates if the block is free or allocated
    int processId;  // ID of the process that owns the block, -1 if free

    MemoryBlock(int start, int sz, bool free, int pid = -1)
        : startAddress(start), size(sz), isFree(free), processId(pid) {}
};

extern std::vector<MemoryBlock> memoryBlocks;

void initializeMemoryManager();
bool allocateMemory(int processID, int memoryRequired);
void deallocateMemory(int processID);
bool hasEnoughFreeMemory(int requiredMem);
void printMemoryStatus(int qq);

#endif