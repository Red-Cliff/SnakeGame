#ifndef GAMEMODE_H
#define GAMEMODE_H

#include <cstdint>

// 经典：有墙体显示，但撞到墙会从对面穿出
// 有限：撞墙直接结束
enum class GameMode : std::int32_t {
    Classic = 0,
    Limited = 1,
};

inline const char* gameModeToString(GameMode mode) {
    switch (mode) {
        case GameMode::Classic:
            return "Classic";
        case GameMode::Limited:
            return "Limited";
        default:
            return "Unknown";
    }
}

inline GameMode sanitizeGameMode(std::int32_t value) {
    switch (static_cast<GameMode>(value)) {
        case GameMode::Classic:
        case GameMode::Limited:
            return static_cast<GameMode>(value);
        default:
            return GameMode::Limited;
    }
}

#endif
