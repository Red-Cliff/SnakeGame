#include "Game.h"
#include <iostream>
#include <conio.h>
#include <thread>
#include <chrono>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <vector>

void Game::startScreen() {
    system("cls");
    auto printStartMenu = []() {
        std::cout << "====================\n";
        std::cout << "   Snake Game!\n";
        std::cout << "====================\n";
        std::cout << "Press T to Start\n";
        std::cout << "Press S to Load Game\n";
        std::cout << "Press ESC to Exit\n";
        std::cout << "====================\n";
    };

    printStartMenu();

    while (true) {
        if (_kbhit()) {
            char key = _getch();
            if (key == 'T' || key == 't') {
                gameMode = selectModeScreen();
                break;
            } else if (key == 'S' || key == 's') {
                if (loadGame()) {
                    break;
                }

                system("cls");
                printStartMenu();
            } else if (key == 27) { // ESC 键退出
                exit(0); // 直接退出程序
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

GameMode Game::selectModeScreen() {
    system("cls");
    std::cout << "====================\n";
    std::cout << "  Select Game Mode\n";
    std::cout << "====================\n";
    std::cout << "1) Classic  (walls shown, wrap through)\n";
    std::cout << "2) Limited  (hit wall = game over)\n";
    std::cout << "====================\n";
    std::cout << "Press 1 or 2\n";

    while (true) {
        if (_kbhit()) {
            char key = _getch();
            if (key == '1') {
                system("cls");
                return GameMode::Classic;
            }
            if (key == '2') {
                system("cls");
                return GameMode::Limited;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void Game::endScreen() {
    system("cls");
    std::cout << "====================\n";
    std::cout << "   Game Over!\n";
    std::cout << "====================\n";
    std::cout << "Final Score: " << score << "\n";
    std::cout << "Press R to Restart, Q to Quit\n";
    std::cout << "====================\n";

    while (true) {
        if (_kbhit()) {
            char key = _getch();
            if (key == 'R' || key == 'r') {
                std::cout << "\033[2J\033[H"; // 清屏，确保结算界面被刷新
                reset();
                run(false); // 跳过开始界面直接开始下一局
                return;
            } else if (key == 'Q' || key == 'q') {
                reset(); // 确保游戏状态被正确重置
                startScreen(); // 返回开始界面
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void Game::reset() {
    snake = Snake();
    food = Food();
    specialFood.active = false;
    score = 0;
    lastScoreBand = 0;
    isRunning = true;
    isSnakeMoving = false; // 每次游戏开始时重置蛇的移动状态
}

void Game::spawnSpecialFood() {
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

int Game::getSpecialFoodDisplayValue() {
    if (!specialFood.active) {
        return 0;
    }

    constexpr auto kLifetime = std::chrono::seconds(5);
    auto now = std::chrono::steady_clock::now();
    auto elapsed = now - specialFood.spawnTime;
    if (elapsed >= kLifetime) {
        specialFood.active = false;
        return 0;
    }

    auto remainingMs = std::chrono::duration_cast<std::chrono::milliseconds>(kLifetime - elapsed).count();
    int secondsLeft = static_cast<int>((remainingMs + 999) / 1000); // 5..1
    if (secondsLeft < 1) secondsLeft = 1;
    if (secondsLeft > 9) secondsLeft = 9;
    return secondsLeft;
}

void Game::run(bool showStartScreen) {
    // 统一由 reset() 初始化新局的默认状态；如果用户选择读档，会覆盖这些默认值。
    reset();
    if (showStartScreen) {
        startScreen(); // 显示开始界面
    }

    isRunning = true;

    constexpr auto kNormalFrame = std::chrono::milliseconds(120);
    constexpr auto kBoostFrame = std::chrono::milliseconds(50);
    constexpr auto kBoostWarmupFrame = std::chrono::milliseconds(35);

    while (isRunning) {
        const bool boostingThisTick = boostNextTick;
        boostNextTick = false;
        boostWarmupThisTick = false;

        auto start = std::chrono::high_resolution_clock::now(); // 开始计时

        handleInput();
        update();
        render();

        const auto targetFrame = boostingThisTick ? kBoostFrame
                                : (boostWarmupThisTick ? kBoostWarmupFrame : kNormalFrame);

        auto end = std::chrono::high_resolution_clock::now(); // 结束计时
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        if (elapsed < targetFrame) {
            std::this_thread::sleep_for(targetFrame - elapsed);
        }
    }

    endScreen(); // 显示结算界面
}

void Game::update() {
    // 特殊食物即使蛇没动也会过期
    (void)getSpecialFoodDisplayValue();

    if (!isSnakeMoving) {
        return; // 如果蛇未开始移动，跳过更新逻辑
    }

    const bool wrapThroughWalls = (gameMode == GameMode::Classic);
    if (!snake.move(input.getDirection(), wrapThroughWalls)) {
        isRunning = false;
        endScreen(); // 确保进入结算界面
        return;
    }

    const bool checkWalls = (gameMode == GameMode::Limited);
    if (snake.checkCollision(checkWalls)) {
        isRunning = false;
        endScreen(); // 确保进入结算界面
        return;
    }

    // 吃到特殊食物：加分但不增长蛇身
    if (specialFood.active && snake.getHead() == specialFood.position) {
        const int oldScore = score;
        int displayValue = getSpecialFoodDisplayValue();
        if (displayValue > 0) {
            score += displayValue * 10;
        }
        specialFood.active = false;

        // 若分数跨过 50 分段（蛇变色），固定刷新生成特殊食物
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

        // 每当分数跨过 50 分段（蛇变色），固定刷新生成特殊食物
        const int oldBand = oldScore / 50;
        const int newBand = score / 50;
        if (newBand != oldBand) {
            spawnSpecialFood();
        }
        lastScoreBand = newBand;
    }
}

void Game::render() {
    const int specialValue = getSpecialFoodDisplayValue();
    const bool specialActive = specialFood.active && (specialValue > 0);
    const Point specialPos = specialFood.position;
    renderer.render(snake, food, score, gameMode, specialActive, specialPos, specialValue);
}

void Game::handleInput() {
    if (_kbhit()) {
        char key = _getch();
        switch (key) {
            case 'w':
                if (isSnakeMoving && input.getDirection() == 0) {
                    boostNextTick = true;
                    boostWarmupThisTick = true;
                }
                else input.setDirection(0);
                isSnakeMoving = true;
                break;
            case 's':
                if (isSnakeMoving && input.getDirection() == 1) {
                    boostNextTick = true;
                    boostWarmupThisTick = true;
                }
                else input.setDirection(1);
                isSnakeMoving = true;
                break;
            case 'a':
                if (isSnakeMoving && input.getDirection() == 2) {
                    boostNextTick = true;
                    boostWarmupThisTick = true;
                }
                else input.setDirection(2);
                isSnakeMoving = true;
                break;
            case 'd':
                if (isSnakeMoving && input.getDirection() == 3) {
                    boostNextTick = true;
                    boostWarmupThisTick = true;
                }
                else input.setDirection(3);
                isSnakeMoving = true;
                break;
            case 'p':
                handlePause(); // 调用暂停界面逻辑
                break;
            
        }
    }
}

void Game::handlePause() {
    auto printPauseMenu = []() {
        std::cout << "\n====================\n";
        std::cout << "   Game Paused\n";
        std::cout << "====================\n";
        std::cout << "Press S to Save Game\n";
        std::cout << "Press P to Resume\n";
        std::cout << "Press Q to Return to Start Screen\n";
        std::cout << "====================\n";
    };

    render();
    printPauseMenu();

    while (true) {
        if (_kbhit()) {
            char key = _getch();
            if (key == 'S' || key == 's') {
                saveGame();
                render();
                printPauseMenu();
            } else if (key == 'P' || key == 'p') {
                system("cls"); // 清屏，确保暂停选项被清理
                render(); // 重新绘制游戏界面
                break;
            } else if (key == 'Q' || key == 'q') {
                reset();
                startScreen();
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void Game::saveGame() {
    SaveSystem::displaySaveSlots();
    std::cout << "Select a save slot (1-3): ";
    int slot;
    std::cin >> slot;

    if (slot < 1 || slot > 3) {
        std::cout << "Invalid slot!" << std::endl;
        return;
    }

    std::string filename = "save_slot_" + std::to_string(slot) + ".dat";
    SaveSystem::save(snake, food, score, gameMode, filename);
    std::cout << "Save successful! Returning to game in 3 seconds...\n";
    for (int i = 3; i > 0; --i) {
        std::cout << "\rReturning to game in " << i << " seconds... ";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    system("cls"); // 清屏，确保日志残留被清理
    // 不要在这里调用 handlePause() 或 render()：saveGame() 由暂停循环调用。
    // 递归进入 handlePause() 会造成暂停循环嵌套，导致恢复后按键仍被外层暂停截获。
}

bool Game::loadGame() {
    SaveSystem::displaySaveSlots();
    std::cout << "Select a load slot (1-3): ";
    int slot;
    std::cin >> slot;

    if (slot < 1 || slot > 3) {
        std::cout << "Invalid slot!" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        return false;
    }

    std::string filename = "save_slot_" + std::to_string(slot) + ".dat";

     // 若文件不存在，则该槽位为空：提示并返回开始界面
    {
        std::ifstream inFile(filename, std::ios::binary);
        if (!inFile) {
            std::cout << "Selected slot is empty. Returning to start screen..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));
            system("cls");
            return false;
        }
    }

    if (!SaveSystem::load(snake, food, score, gameMode, filename)) {
        std::cout << "Load failed. Returning to start screen..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        system("cls");
        return false;
    }

    // 读档不恢复特殊食物；按当前分数设置变色段位
    specialFood.active = false;
    lastScoreBand = score / 50;

    system("cls"); // 清屏，确保读档日志被清理
    render(); // 重新绘制游戏界面
    return true;
}