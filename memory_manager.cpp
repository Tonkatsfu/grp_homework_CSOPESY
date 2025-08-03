#include "memory_manager.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <map>
#include <vector>
#include <chrono>
#include <ctime>
#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

std::vector<MemoryBlock> memoryBlocks;
std::map<int, PageTable> pageTables;
std::vector<int> freeFrameList;
std::map<int, std::vector<uint8_t>> physicalMemory;

std::fstream backingStore;
int frameCount;
const std::string backingStoreFile = "csopesy-backing-store.txt";

void initializeMemoryManager() {
    memoryBlocks.clear();
    memoryBlocks.emplace_back(0, maxOverallMem, true);

    frameCount = maxOverallMem / memPerFrame;
    freeFrameList.clear();
    for (int i = 0; i < frameCount; ++i) {
        freeFrameList.push_back(i);
    }

    backingStore.open(backingStoreFile, std::ios::in | std::ios::out | std::ios::binary);
    if (!backingStore.is_open()) {
        backingStore.open(backingStoreFile, std::ios::out | std::ios::binary);
        backingStore.close();
        backingStore.open(backingStoreFile, std::ios::in | std::ios::out | std::ios::binary);
    }
    backingStore.seekp(0, std::ios::end);
    int size = backingStore.tellp();
    if (size < maxOverallMem) {
        backingStore.seekp(maxOverallMem - 1);
        backingStore.write("", 1);
    }

    std::cout << "Memory Manager Initialized with " << memoryBlocks.size()
              << " block(s), " << frameCount << " frames, and backing store initialized.\n";
}

bool allocateMemory(int processID, int memoryRequired) {
    pageTables[processID] = PageTable();
    int requiredPages = memoryRequired / memPerFrame;

    for (int i = 0; i < requiredPages; ++i) {
        if (freeFrameList.empty()) return false;

        int frame = freeFrameList.back();
        freeFrameList.pop_back();

        pageTables[processID].pages[i] = {frame, true, false, false};
        physicalMemory[frame] = std::vector<uint8_t>(memPerFrame, 0);
    }

    return true;
}

void deallocateMemory(int processID) {
    if (pageTables.find(processID) != pageTables.end()) {
        for (auto& [vpn, entry] : pageTables[processID].pages) {
            if (entry.valid) {
                freeFrameList.push_back(entry.frameNumber);
                physicalMemory.erase(entry.frameNumber);
            }
        }
        pageTables.erase(processID);
    }
}

bool hasEnoughFreeMemory(int requiredMem) {
    int requiredPages = requiredMem / memPerFrame;
    return freeFrameList.size() >= requiredPages;
}

int handlePageFault(int processID, int virtualPageNum) {
    if (freeFrameList.empty()) {
        for (auto& [pid, table] : pageTables) {
            for (auto& [vpn, entry] : table.pages) {
                if (entry.valid) {
                    int frame = entry.frameNumber;

                    backingStore.seekp((pid * 1000 + vpn) * memPerFrame);
                    backingStore.write(reinterpret_cast<char*>(physicalMemory[frame].data()), memPerFrame);

                    physicalMemory.erase(frame);
                    entry.valid = false;
                    entry.frameNumber = -1;
                    freeFrameList.push_back(frame);
                    break;
                }
            }
        }
    }

    int frame = freeFrameList.back();
    freeFrameList.pop_back();
    std::vector<uint8_t> data(memPerFrame, 0);

    backingStore.seekg((processID * 1000 + virtualPageNum) * memPerFrame);
    backingStore.read(reinterpret_cast<char*>(data.data()), memPerFrame);

    physicalMemory[frame] = data;
    pageTables[processID].pages[virtualPageNum] = {frame, true, false, false};
    return frame;
}

int getPhysicalAddress(int processID, int virtualAddress) {
    int virtualPageNum = virtualAddress / memPerFrame;
    int offset = virtualAddress % memPerFrame;

    if (!pageTables[processID].pages[virtualPageNum].valid) {
        handlePageFault(processID, virtualPageNum);
    }

    int frame = pageTables[processID].pages[virtualPageNum].frameNumber;
    return frame * memPerFrame + offset;
}

uint16_t READ_MEMORY(int processID, int virtualAddress) {
    int physicalAddr = getPhysicalAddress(processID, virtualAddress);
    int frame = physicalAddr / memPerFrame;
    int offset = physicalAddr % memPerFrame;

    return (physicalMemory[frame][offset] << 8) | physicalMemory[frame][offset + 1];
}

void WRITE_MEMORY(int processID, int virtualAddress, uint16_t value) {
    int physicalAddr = getPhysicalAddress(processID, virtualAddress);
    int frame = physicalAddr / memPerFrame;
    int offset = physicalAddr % memPerFrame;

    physicalMemory[frame][offset] = (value >> 8) & 0xFF;
    physicalMemory[frame][offset + 1] = value & 0xFF;

    pageTables[processID].pages[virtualAddress / memPerFrame].dirty = true;
}

void printMemoryStatus(int qq) {
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%m/%d/%Y %I:%M:%S %p", std::localtime(&now));

    system("mkdir memory_logs >nul 2>&1");
    std::string filePath = "memory_logs/memory_stamp_" + std::to_string(qq) + ".txt";
    std::ofstream logFile(filePath, std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "Error: Could not open " << filePath << "\n";
        return;
    }

    int procInMem = pageTables.size();
    int totalFrames = maxOverallMem / memPerFrame;
    int usedFrames = totalFrames - freeFrameList.size();

    logFile << "Timestamp: " << buffer << "\n";
    logFile << "Processes in memory: " << procInMem << "\n";
    logFile << "Used Frames: " << usedFrames << "/" << totalFrames << "\n";
    logFile << "Free Frames: " << freeFrameList.size() << "\n";

    for (const auto& [pid, table] : pageTables) {
        logFile << "\nProcess " << pid << ":\n";
        for (const auto& [vpn, entry] : table.pages) {
            if (entry.valid) {
                logFile << "  VPN: " << vpn << ", Frame: " << entry.frameNumber << "\n";
            }
        }
    }

    logFile << "----end----\n";
    logFile.close();
}
