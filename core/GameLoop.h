#ifndef GAMELOOP_H
#define GAMELOOP_H

#include "core/StateMachine.h"
#include "gameplay/Food.h"
#include "gameplay/Snake.h"
#include "systems/InputSystem.h"
#include "systems/RenderSystem.h"
#include "systems/TimeSystem.h"
#include "GameMode.h"
#include <chrono>
#include <deque>

class GameLoop {
public:
    void run(bool showStartScreen = true);

private:
    void reset();

    // Game Loop 三段式
    void processInput();
    void update();
    void render();

    // UI（只负责绘制一次，不阻塞 Game Loop）
    void drawMenu();
    void drawModeSelect();
    void drawPaused();
    void drawGameOver();

    // gameplay
    void spawnSpecialFood();
    int getSpecialFoodDisplayValue(); // 0 表示不存在/已过期

    // persistence
    void saveGame();
    bool loadGame();

    // helpers
    static int inferDirectionFromBody(const std::deque<Point>& body);
    std::chrono::milliseconds baseTickInterval() const;

    Snake snake;
    Food food;
    RenderSystem renderSystem;
    InputSystem inputSystem;
    TimeSystem timeSystem;

    GameMode gameMode = GameMode::Limited;

    struct SpecialFood {
        bool active = false;
        Point position{0, 0};
        std::chrono::steady_clock::time_point spawnTime{};
    } specialFood;

    int lastScoreBand = 0;
    int score = 0;

    // 方向（0 上 / 1 下 / 2 左 / 3 右）
    int direction = 3;
    bool isSnakeMoving = false;

    // FSM
    StateMachine fsm;
    bool uiDirty = true;

    // “下一次更新”加速（按与当前方向相同键触发）
    bool boostNextTick = false;
};

#endif

