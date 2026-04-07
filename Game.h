#ifndef GAME_H
#define GAME_H

#include "Snake.h"
#include "Food.h"
#include "Renderer.h"
#include "Input.h"
#include "SaveSystem.h"
#include "GameMode.h"
#include <chrono>

class Game {
public:
    void run(bool showStartScreen = true); // 添加布尔参数，默认值为 true
    void startScreen(); // 显示开始界面
    void endScreen();   // 显示结算界面
    void reset();       // 重置游戏状态

private:
    GameMode selectModeScreen(); // 模式选择界面
    void spawnSpecialFood();
    int getSpecialFoodDisplayValue(); // 0 表示不存在/已过期
    void update();
    void render();
    void handleInput();
    void saveGame();
    bool loadGame();
    void handlePause(); // 处理暂停逻辑

    Snake snake;
    Food food;
    Renderer renderer;
    Input input;
    GameMode gameMode = GameMode::Limited;

    struct SpecialFood {
        bool active = false;
        Point position{0, 0};
        std::chrono::steady_clock::time_point spawnTime{};
    } specialFood;

    int lastScoreBand = 0; // 用于检测“每 50 分变色”节点

    int score;
    bool isRunning;
    bool isSnakeMoving = false; // 标志变量，表示蛇是否开始移动
    bool boostNextTick = false;  // 当按下与当前方向相同的键时，加速下一帧
    bool boostWarmupThisTick = false; // 缩短当前帧，作为加速前摇
};

#endif