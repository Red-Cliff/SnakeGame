#ifndef TIMESYSTEM_H
#define TIMESYSTEM_H

#include <chrono>

class TimeSystem {
public:
    using Clock = std::chrono::steady_clock;

    void reset();

    // 每帧调用：返回本帧经过的毫秒数（已做上限截断，避免长暂停导致“螺旋死亡”）
    std::chrono::milliseconds beginFrame();

    void add(std::chrono::milliseconds dt);
    bool canUpdate(std::chrono::milliseconds tick) const;
    void consume(std::chrono::milliseconds tick);

private:
    Clock::time_point lastFrameTime{};
    std::chrono::milliseconds accumulator{0};
};

#endif
