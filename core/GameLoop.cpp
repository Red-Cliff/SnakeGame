#include "core/GameLoop.h"

#include "persistence/SaveSystem.h"
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace {
    constexpr auto kRenderInterval = std::chrono::milliseconds(33); // ~30 FPS
    constexpr auto kBoostTick = std::chrono::milliseconds(50);
    constexpr auto kMaxSleep = std::chrono::milliseconds(5);
}

int GameLoop::inferDirectionFromBody(const std::deque<Point>& body) {
    if (body.size() < 2) {
        return 3; // 默认向右
    }

    const auto& head = body[0];
    const auto& neck = body[1];

    if (head.x == neck.x) {
        return (head.y < neck.y) ? 0 : 1;
    }
    if (head.y == neck.y) {
        return (head.x > neck.x) ? 3 : 2;
    }

    return 3;
}

std::chrono::milliseconds GameLoop::baseTickInterval() const {
    // 速度分档（随分数动态加速）
    if (score < 50) return std::chrono::milliseconds(160);   // 低
    if (score < 150) return std::chrono::milliseconds(120);  // 中
    if (score < 300) return std::chrono::milliseconds(90);   // 高
    return std::chrono::milliseconds(70);                    // 更快
}

void GameLoop::reset() {
    snake = Snake();
    food = Food();

    specialFood.active = false;
    lastScoreBand = 0;

    score = 0;
    direction = 3;
    isSnakeMoving = false;

    boostNextTick = false;
    uiDirty = true;
}

void GameLoop::run(bool showStartScreen) {
    reset();

    fsm.state = showStartScreen ? GameState::MENU : GameState::RUNNING;
    fsm.menuPhase = MenuPhase::MAIN;
    uiDirty = true;

    timeSystem.reset();
    auto lastRenderTime = TimeSystem::Clock::now();

    bool running = true;
    while (running) {
        processInput();

        const auto dt = timeSystem.beginFrame();
        const auto now = TimeSystem::Clock::now();

        if (fsm.state == GameState::RUNNING) {
            timeSystem.add(dt);

            while (true) {
                const auto baseTick = baseTickInterval();
                const auto tick = boostNextTick ? kBoostTick : baseTick;
                if (!timeSystem.canUpdate(tick)) {
                    break;
                }

                if (boostNextTick) {
                    boostNextTick = false;
                }

                update();
                timeSystem.consume(tick);

                if (fsm.state != GameState::RUNNING) {
                    uiDirty = true;
                    break;
                }
            }
        }

        const bool shouldRender = (fsm.state == GameState::RUNNING) ? (now - lastRenderTime >= kRenderInterval)
                                                                   : uiDirty;
        if (shouldRender) {
            render();
            lastRenderTime = now;
            uiDirty = false;
        }

        if (fsm.state == GameState::EXIT) {
            running = false;
            continue;
        }

        // 让出 CPU，但不让输入采集“睡太久”
        auto sleepFor = std::chrono::milliseconds(1);
        if (fsm.state == GameState::RUNNING) {
            const auto untilNextRender = std::chrono::duration_cast<std::chrono::milliseconds>(
                (lastRenderTime + kRenderInterval) - TimeSystem::Clock::now());
            if (untilNextRender > std::chrono::milliseconds(0)) {
                sleepFor = std::min(untilNextRender, kMaxSleep);
            }
        } else {
            sleepFor = kMaxSleep;
        }

        if (sleepFor > std::chrono::milliseconds(0)) {
            std::this_thread::sleep_for(sleepFor);
        }
    }
}

void GameLoop::processInput() {
    const auto keys = inputSystem.drainKeys();

    switch (fsm.state) {
        case GameState::MENU: {
            for (const char key : keys) {
                if (fsm.menuPhase == MenuPhase::MAIN) {
                    if (key == 'T' || key == 't') {
                        fsm.menuPhase = MenuPhase::MODE_SELECT;
                        uiDirty = true;
                    } else if (key == 'S' || key == 's') {
                        if (loadGame()) {
                            direction = inferDirectionFromBody(snake.getBody());
                            isSnakeMoving = false;

                            system("cls");
                            fsm.state = GameState::RUNNING;
                            uiDirty = true;
                        } else {
                            uiDirty = true;
                        }
                    } else if (key == 27) {
                        fsm.state = GameState::EXIT;
                        uiDirty = true;
                    }
                } else { // MODE_SELECT
                    if (key == '1') {
                        gameMode = GameMode::Classic;
                        reset();
                        system("cls");
                        fsm.state = GameState::RUNNING;
                        uiDirty = true;
                    } else if (key == '2') {
                        gameMode = GameMode::Limited;
                        reset();
                        system("cls");
                        fsm.state = GameState::RUNNING;
                        uiDirty = true;
                    } else if (key == 27) {
                        fsm.menuPhase = MenuPhase::MAIN;
                        uiDirty = true;
                    }
                }
            }
            break;
        }
        case GameState::RUNNING: {
            const int currentDirection = direction;
            int nextDirection = currentDirection;

            for (const char key : keys) {
                if (key == 'P' || key == 'p') {
                    fsm.state = GameState::PAUSED;
                    uiDirty = true;
                    return;
                }
                if (key == 27) {
                    fsm.state = GameState::EXIT;
                    uiDirty = true;
                    return;
                }

                int dir = 0;
                if (!InputSystem::tryMapKeyToDirection(key, dir)) {
                    continue; // 非法键：忽略
                }

                // 最新合法输入覆盖策略：同一帧可读多个输入，只保留最后一个合法方向
                // 注意：反向判定必须基于“当前方向”（currentDirection），而不是在同一帧里被不断更新的 nextDirection。
                if (!isSnakeMoving) {
                    // 第一帧启动：允许任意方向作为起步方向（此时不会出现 180° 自撞问题）
                    nextDirection = dir;
                    isSnakeMoving = true;
                } else {
                    if (!InputSystem::isOpposite(dir, currentDirection)) {
                        nextDirection = dir;
                    }

                    // 按与当前方向相同的键：加速下一次 update
                    if (dir == currentDirection) {
                        boostNextTick = true;
                    }
                }
            }

            direction = nextDirection;
            break;
        }
        case GameState::PAUSED: {
            for (const char key : keys) {
                if (key == 'P' || key == 'p') {
                    system("cls");
                    fsm.state = GameState::RUNNING;
                    uiDirty = true;
                } else if (key == 'S' || key == 's') {
                    saveGame();
                    uiDirty = true;
                } else if (key == 'Q' || key == 'q') {
                    reset();
                    fsm.state = GameState::MENU;
                    fsm.menuPhase = MenuPhase::MAIN;
                    uiDirty = true;
                } else if (key == 27) {
                    fsm.state = GameState::EXIT;
                    uiDirty = true;
                }
            }
            break;
        }
        case GameState::GAME_OVER: {
            for (const char key : keys) {
                if (key == 'R' || key == 'r') {
                    reset();
                    system("cls");
                    fsm.state = GameState::RUNNING;
                    uiDirty = true;
                } else if (key == 'Q' || key == 'q') {
                    reset();
                    fsm.state = GameState::MENU;
                    fsm.menuPhase = MenuPhase::MAIN;
                    uiDirty = true;
                } else if (key == 27) {
                    fsm.state = GameState::EXIT;
                    uiDirty = true;
                }
            }
            break;
        }
        case GameState::EXIT:
            break;
    }
}

void GameLoop::update() {
    // 特殊食物即使蛇没动也会过期
    (void)getSpecialFoodDisplayValue();

    if (!isSnakeMoving) {
        return;
    }

    const bool wrapThroughWalls = (gameMode == GameMode::Classic);
    if (!snake.move(direction, wrapThroughWalls)) {
        isSnakeMoving = false;
        fsm.state = GameState::GAME_OVER;
        uiDirty = true;
        return;
    }

    const bool checkWalls = (gameMode == GameMode::Limited);
    if (snake.checkCollision(checkWalls)) {
        isSnakeMoving = false;
        fsm.state = GameState::GAME_OVER;
        uiDirty = true;
        return;
    }

    // 吃到特殊食物：加分但不增长蛇身
    if (specialFood.active && snake.getHead() == specialFood.position) {
        const int oldScore = score;
        const int displayValue = getSpecialFoodDisplayValue();
        if (displayValue > 0) {
            score += displayValue * 10;
        }
        specialFood.active = false;

        const int oldBand = oldScore / 50;
        const int newBand = score / 50;
        if (newBand != oldBand) {
            spawnSpecialFood();
        }
        lastScoreBand = newBand;
    }

    if (snake.getHead() == food.getPosition()) {
        const int oldScore = score;
        snake.grow();
        food.generate(snake);
        score += 10;

        // 避免普通食物生成到特殊食物位置
        if (specialFood.active) {
            for (int retry = 0; retry < 20 && food.getPosition() == specialFood.position; ++retry) {
                food.generate(snake);
            }
        }

        const int oldBand = oldScore / 50;
        const int newBand = score / 50;
        if (newBand != oldBand) {
            spawnSpecialFood();
        }
        lastScoreBand = newBand;
    }
}

void GameLoop::render() {
    switch (fsm.state) {
        case GameState::MENU:
            if (fsm.menuPhase == MenuPhase::MAIN) {
                drawMenu();
            } else {
                drawModeSelect();
            }
            return;
        case GameState::PAUSED:
            drawPaused();
            return;
        case GameState::GAME_OVER:
            drawGameOver();
            return;
        case GameState::RUNNING:
        case GameState::EXIT:
            break;
    }

    const int specialValue = getSpecialFoodDisplayValue();
    const bool specialActive = specialFood.active && (specialValue > 0);
    const Point specialPos = specialFood.position;

    // 颜色随速度分档变化：白 / 黄 / 橙 / 红
    int snakeColor = 15; // 白
    if (score < 50) snakeColor = 15;
    else if (score < 150) snakeColor = 14; // 黄
    else if (score < 300) snakeColor = 6;  // 橙
    else snakeColor = 12;                 // 红

    renderSystem.render(snake, food, score, gameMode, specialActive, specialPos, specialValue, snakeColor);
}

void GameLoop::drawMenu() {
    system("cls");
    std::cout << "====================\n";
    std::cout << "   Snake Game!\n";
    std::cout << "====================\n";
    std::cout << "Press T to Start\n";
    std::cout << "Press S to Load Game\n";
    std::cout << "Press ESC to Exit\n";
    std::cout << "====================\n";
}

void GameLoop::drawModeSelect() {
    system("cls");
    std::cout << "====================\n";
    std::cout << "  Select Game Mode\n";
    std::cout << "====================\n";
    std::cout << "1) Classic  (walls shown, wrap through)\n";
    std::cout << "2) Limited  (hit wall = game over)\n";
    std::cout << "====================\n";
    std::cout << "Press 1 or 2 (ESC to Back)\n";
}

void GameLoop::drawPaused() {
    system("cls");

    const int specialValue = getSpecialFoodDisplayValue();
    const bool specialActive = specialFood.active && (specialValue > 0);
    const Point specialPos = specialFood.position;

    // 颜色随速度分档变化：白 / 黄 / 橙 / 红
    int snakeColor = 15; // 白
    if (score < 50) snakeColor = 15;
    else if (score < 150) snakeColor = 14; // 黄
    else if (score < 300) snakeColor = 6;  // 橙
    else snakeColor = 12;                 // 红

    renderSystem.render(snake, food, score, gameMode, specialActive, specialPos, specialValue, snakeColor);

    std::cout << "\n====================\n";
    std::cout << "   Game Paused\n";
    std::cout << "====================\n";
    std::cout << "Press S to Save Game\n";
    std::cout << "Press P to Resume\n";
    std::cout << "Press Q to Return to Menu\n";
    std::cout << "Press ESC to Exit\n";
    std::cout << "====================\n";
}

void GameLoop::drawGameOver() {
    system("cls");
    std::cout << "====================\n";
    std::cout << "   Game Over!\n";
    std::cout << "====================\n";
    std::cout << "Final Score: " << score << "\n";
    std::cout << "Mode: " << gameModeToString(gameMode) << "\n";
    std::cout << "Press R to Restart\n";
    std::cout << "Press Q to Return to Menu\n";
    std::cout << "Press ESC to Exit\n";
    std::cout << "====================\n";
}

void GameLoop::spawnSpecialFood() {
    static bool initialized = false;
    if (!initialized) {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        initialized = true;
    }

    // 从所有可用空格中随机选一个，保证可生成（除非蛇占满）
    std::vector<Point> candidates;
    candidates.reserve(18 * 18);
    for (int y = 1; y <= 18; ++y) {
        for (int x = 1; x <= 18; ++x) {
            Point p{x, y};
            if (p == food.getPosition()) {
                continue;
            }
            bool onSnake = false;
            for (const auto& seg : snake.getBody()) {
                if (seg == p) {
                    onSnake = true;
                    break;
                }
            }
            if (onSnake) {
                continue;
            }
            candidates.push_back(p);
        }
    }

    if (candidates.empty()) {
        return;
    }

    const auto idx = static_cast<size_t>(std::rand() % static_cast<int>(candidates.size()));
    specialFood.active = true;
    specialFood.position = candidates[idx];
    specialFood.spawnTime = std::chrono::steady_clock::now();
}

int GameLoop::getSpecialFoodDisplayValue() {
    if (!specialFood.active) {
        return 0;
    }

    constexpr auto kLifetime = std::chrono::seconds(5);
    const auto now = std::chrono::steady_clock::now();
    const auto elapsed = now - specialFood.spawnTime;
    if (elapsed >= kLifetime) {
        specialFood.active = false;
        return 0;
    }

    const auto remainingMs = std::chrono::duration_cast<std::chrono::milliseconds>(kLifetime - elapsed).count();
    int secondsLeft = static_cast<int>((remainingMs + 999) / 1000); // 5..1
    secondsLeft = std::clamp(secondsLeft, 1, 9);
    return secondsLeft;
}

void GameLoop::saveGame() {
    SaveSystem::displaySaveSlots();
    std::cout << "Select a save slot (1-3): ";
    int slot;
    std::cin >> slot;

    if (slot < 1 || slot > 3) {
        std::cout << "Invalid slot!" << std::endl;
        return;
    }

    const std::string filename = "save_slot_" + std::to_string(slot) + ".dat";
    SaveSystem::save(snake, food, score, gameMode, filename);

    std::cout << "Save successful!\n";
}

bool GameLoop::loadGame() {
    SaveSystem::displaySaveSlots();
    std::cout << "Select a load slot (1-3): ";
    int slot;
    std::cin >> slot;

    if (slot < 1 || slot > 3) {
        std::cout << "Invalid slot!" << std::endl;
        return false;
    }

    const std::string filename = "save_slot_" + std::to_string(slot) + ".dat";

    // 若文件不存在，则该槽位为空
    {
        std::ifstream inFile(filename, std::ios::binary);
        if (!inFile) {
            std::cout << "Selected slot is empty." << std::endl;
            return false;
        }
    }

    if (!SaveSystem::load(snake, food, score, gameMode, filename)) {
        std::cout << "Load failed (file may be corrupted)." << std::endl;
        return false;
    }

    // 读档不恢复特殊食物；按当前分数设置变色段位
    specialFood.active = false;
    lastScoreBand = score / 50;

    return true;
}
