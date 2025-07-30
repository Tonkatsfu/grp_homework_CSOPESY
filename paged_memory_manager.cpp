#include "paged_memory_manager.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <numeric>

std::vector<bool> physicalMemoryFrames;
std::map<int, std::vector<PageTableEntry>> pageTables;

void initializePagedMemoryManager() {
    int totalFrames = maxOverallMem / memPerFrame;
    physicalMemoryFrames.assign(totalFrames, true); // true means free
    pageTables.clear();
    std::cout << "+-------------------------------------------------+\n";
    std::cout << "| \033[32mPaged Memory Manager initialized with \033[0m";
    std::cout << totalFrames << " \033[32mframe(s)\033[0m      |\n";
    std::cout << "+-------------------------------------------------+\n";
}

bool allocatePagedMemory(Process* process) {
    int pagesRequired = (process->memorySize + memPerFrame - 1) / memPerFrame;
    
    if (!hasEnoughPagedMemory(pagesRequired)) {
        return false;
    }

    std::vector<PageTableEntry> newPageTable(pagesRequired);
    int allocatedFrames = 0;
    
    for (size_t i = 0; i < physicalMemoryFrames.size() && allocatedFrames < pagesRequired; ++i) {
        if (physicalMemoryFrames[i]) {
            physicalMemoryFrames[i] = false; // Mark as used
            newPageTable[allocatedFrames].frameNumber = i;
            newPageTable[allocatedFrames].isValid = true;
            allocatedFrames++;
        }
    }

    pageTables[process->pid] = newPageTable; // Use process->pid
    return true;
}

void deallocatePagedMemory(int processID) {
    if (pageTables.count(processID)) {
        for (const auto& entry : pageTables[processID]) {
            if (entry.isValid) {
                physicalMemoryFrames[entry.frameNumber] = true; // Mark frame as free
            }
        }
        pageTables.erase(processID);
    }
}

bool hasEnoughPagedMemory(int requiredMem) {
    int pagesRequired = (requiredMem + memPerFrame - 1) / memPerFrame;
    int freeFrames = std::count(physicalMemoryFrames.begin(), physicalMemoryFrames.end(), true);
    return freeFrames >= pagesRequired;
}

void printPagedMemoryStatus(std::ostream& os) {
    os << "Paged Memory Status:\n";
    os << "Total Frames: " << physicalMemoryFrames.size() << "\n";
    os << "Frame Size: " << memPerFrame << " bytes\n";
    os << "Free Frames: " << std::count(physicalMemoryFrames.begin(), physicalMemoryFrames.end(), true) << "\n";
    os << "Used Frames: " << physicalMemoryFrames.size() - std::count(physicalMemoryFrames.begin(), physicalMemoryFrames.end(), true) << "\n";
    
    os << "Page Tables:\n";
    for (const auto& pair : pageTables) {
        os << "  Process ID: " << pair.first << ", Pages: " << pair.second.size() << "\n";
        for (size_t i = 0; i < pair.second.size(); ++i) {
            os << "    Logical Page " << i << " -> Frame " << pair.second[i].frameNumber << (pair.second[i].isValid ? "" : " (Invalid)") << "\n";
        }
    }
}