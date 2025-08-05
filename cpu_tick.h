#ifndef CPU_TICK_H
#define CPU_TICK_H

#include <atomic>
#include <thread>
#include <functional>

class CpuTicker {
public:
    void start();  
    void stop();
    void registerCallback(std::function<void()> callback);  
    
    void incActiveTicks();
    void incIdleTicks();

    uint64_t getTickCount() const;
    uint64_t getActiveTicks() const;
    uint64_t getIdleTicks() const;

    void resetTicks();

private:
    std::atomic<bool> running = false;
    std::thread tickThread;
    std::function<void()> tickCallback;

    std::atomic<uint64_t> tickCount = 0;
    std::atomic<uint64_t> activeTicks = 0;
    std::atomic<uint64_t> idleTicks = 0;
};

#endif
