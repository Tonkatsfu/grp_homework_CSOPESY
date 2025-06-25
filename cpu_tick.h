#ifndef CPU_TICK_H
#define CPU_TICK_H

#include <atomic>
#include <thread>
#include <functional>

class CpuTicker {
public:
    void start();  // starts ticking
    void stop();
    void registerCallback(std::function<void()> callback);  // hook called every tick
    uint64_t getTickCount() const;

private:
    std::atomic<bool> running = false;
    std::thread tickThread;
    std::function<void()> tickCallback;
    std::atomic<uint64_t> tickCount = 0;
};

#endif
