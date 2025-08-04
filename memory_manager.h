#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "initialize.h"
#include <vector>
#include <map>
#include <cstdint>

struct MemoryBlock {
    int startAddress;
    int size;
    bool isFree;
    int processId;

    MemoryBlock(int start, int sz, bool free, int pid = -1)
        : startAddress(start), size(sz), isFree(free), processId(pid) {}
};

struct PageTableEntry {
    int frameNumber = -1;
    bool valid = false;
    bool dirty = false;
    bool referenced = false;
};

struct PageTable {
    std::map<int, PageTableEntry> pages; // virtual page -> entry
};

extern std::vector<MemoryBlock> memoryBlocks;
extern std::map<int, PageTable> pageTables;
extern std::vector<int> freeFrameList;
extern std::map<int, std::vector<uint8_t>> physicalMemory;
extern int frameCount;

void initializeMemoryManager();
bool allocateMemory(int processID, int memoryRequired);
void deallocateMemory(int processID);
bool hasEnoughFreeMemory(int requiredMem);
int getPhysicalAddress(int processID, int virtualAddress);
uint16_t READ_MEMORY(int processID, int virtualAddress);
void WRITE_MEMORY(int processID, int virtualAddress, uint16_t value);
void printMemoryStatus(int qq);
int handlePageFault(int processID, int virtualPageNum);
bool isAddressValid(int processID, int virtualAddress);
int getRandomValidAddress(int processID);
std::string getMemoryUsageReport();
int getMemoryUsedByProcess(int processID);

int getTotalMemory();
int getConsumedMemory();
unsigned long getIdleCpuTicks();
unsigned long getActiveCpuTicks();
unsigned long getNumPagesPagedIn();
unsigned long getNumPagesPagedOut();
#endif
