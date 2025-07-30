#ifndef PAGED_MEMORY_MANAGER_H
#define PAGED_MEMORY_MANAGER_H

#include "initialize.h" 
#include "scheduler.h"
#include <vector>
#include <map>

struct Process;

struct PageTableEntry {
    int frameNumber;
    bool isValid;
};

extern std::vector<bool> physicalMemoryFrames;
extern std::map<int, std::vector<PageTableEntry>> pageTables;

void initializePagedMemoryManager();
bool allocatePagedMemory(Process* process);
void deallocatePagedMemory(int processID);
bool hasEnoughPagedMemory(int requiredMem);

void printPagedMemoryStatus(std::ostream& os);

#endif