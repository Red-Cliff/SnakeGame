#include "Renderer.h"
#include <iostream>
#include <vector>
#include <windows.h>

void Renderer::render(const Snake& snake,
                      const Food& food,
                      int score,
                      GameMode mode,
                      bool specialFoodActive,
                      const Point& specialFoodPosition,
                      int specialFoodDisplayValue) {
    const int width = 20;
    const int height = 20;
    std::vector<std::vector<char>> buffer(height, std::vector<char>(width, ' '));

    // 设置墙的颜色为白色
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);

    // 绘制围墙：经典模式用 '|' 和 '-'；有限模式沿用 '#'
    const char horizontalWall = (mode == GameMode::Classic) ? '-' : '#';
    const char verticalWall = (mode == GameMode::Classic) ? '|' : '#';
    for (int x = 0; x < width; ++x) {
        buffer[0][x] = horizontalWall;
        buffer[height - 1][x] = horizontalWall;
    }
    for (int y = 1; y < height - 1; ++y) {
        buffer[y][0] = verticalWall;
        buffer[y][width - 1] = verticalWall;
    }

    // 设置食物的颜色为黄色
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14);
    buffer[food.getPosition().y][food.getPosition().x] = 'X';

    // 特殊食物：红色倒计时数字（只占一个格子）
    if (specialFoodActive && specialFoodDisplayValue >= 1 && specialFoodDisplayValue <= 9) {
        buffer[specialFoodPosition.y][specialFoodPosition.x] = static_cast<char>('0' + specialFoodDisplayValue);
    }

    // 设置蛇的颜色，根据分数每 50 分变化一次（循环 1..14，避免黑色/白色）
    int snakeColor = (score / 50) % 14 + 1;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), snakeColor);

    for (const auto& segment : snake.getBody()) {
        buffer[segment.y][segment.x] = (segment == snake.getHead()) ? '@' : 'O'; // 蛇头为 @，身体为 O
    }

    // 确保界面其他部分为白色
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);

    // 使用双缓冲：将内容存储到字符串中
    std::string output;
    for (const auto& row : buffer) {
        for (char cell : row) {
            output += cell;
        }
        output += '\n';
    }
    output += "Score: " + std::to_string(score) + '\n';

    // 隐藏光标
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = false; // 隐藏光标
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    // 移动光标到控制台左上角
    COORD cursorPosition = {0, 0};
    SetConsoleCursorPosition(hConsole, cursorPosition);

    // 输出内容时动态设置颜色
    for (size_t y = 0; y < buffer.size(); ++y) {
        for (size_t x = 0; x < buffer[y].size(); ++x) {
            if (buffer[y][x] == '@' || buffer[y][x] == 'O') {
                SetConsoleTextAttribute(hConsole, snakeColor); // 设置蛇的颜色
            } else if (buffer[y][x] == 'X') {
                SetConsoleTextAttribute(hConsole, 14); // 设置食物的颜色为黄色
            } else if (specialFoodActive && static_cast<int>(x) == specialFoodPosition.x && static_cast<int>(y) == specialFoodPosition.y && buffer[y][x] >= '0' && buffer[y][x] <= '9') {
                SetConsoleTextAttribute(hConsole, 12); // 红色
            } else if (buffer[y][x] == '#' || buffer[y][x] == '|' || buffer[y][x] == '-') {
                SetConsoleTextAttribute(hConsole, 15); // 设置墙的颜色为白色
            } else {
                SetConsoleTextAttribute(hConsole, 15); // 其他部分为白色
            }
            std::cout << buffer[y][x];
        }
        std::cout << '\n';
    }

    // 输出分数
    SetConsoleTextAttribute(hConsole, 15); // 确保分数显示为白色
    std::cout << "Score: " << score << '\n';
    std::cout << "Mode: " << gameModeToString(mode) << '\n';

    // 恢复默认颜色
    SetConsoleTextAttribute(hConsole, 15);
}