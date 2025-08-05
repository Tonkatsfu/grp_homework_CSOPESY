#include "cpu_tick.h"
#include <chrono>

void CpuTicker::start() {
    running = true;
    tickThread = std::thread([this]() {
        while (running) {
            tickCount++;
            if (tickCallback) tickCallback();
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Simulated tick interval
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

void CpuTicker::incActiveTicks() {
    activeTicks++;
}

void CpuTicker::incIdleTicks() {
    idleTicks++;
}

uint64_t CpuTicker::getTickCount() const {
    return tickCount.load();
}

uint64_t CpuTicker::getActiveTicks() const {
    return activeTicks.load();
}

uint64_t CpuTicker::getIdleTicks() const {
    return idleTicks.load();
}

void CpuTicker::resetTicks() {
    activeTicks.store(0);
    idleTicks.store(0);
}
