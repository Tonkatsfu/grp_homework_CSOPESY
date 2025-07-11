#include "cpu_tick.h"
#include <chrono>

void CpuTicker::start() {
    running = true;
    tickThread = std::thread([this]() {
        while (running) {
            tickCount++;
            if (tickCallback) tickCallback();
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Simulate 1 tick
        }
    });
}

void CpuTicker::stop() {
    running = false;
    if (tickThread.joinable()) tickThread.join();
}

void CpuTicker::registerCallback(std::function<void()> callback) {
    tickCallback = callback;
}

uint64_t CpuTicker::getTickCount() const {
    return tickCount.load();
}
