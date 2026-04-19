#include "systems/TimeSystem.h"

void TimeSystem::reset() {
    lastFrameTime = Clock::now();
    accumulator = std::chrono::milliseconds(0);
}

std::chrono::milliseconds TimeSystem::beginFrame() {
    const auto now = Clock::now();
    auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFrameTime);
    lastFrameTime = now;

    if (dt > std::chrono::milliseconds(250)) {
        dt = std::chrono::milliseconds(250);
    }
    return dt;
}

void TimeSystem::add(std::chrono::milliseconds dt) {
    accumulator += dt;
}

bool TimeSystem::canUpdate(std::chrono::milliseconds tick) const {
    return accumulator >= tick;
}

void TimeSystem::consume(std::chrono::milliseconds tick) {
    accumulator -= tick;
}
