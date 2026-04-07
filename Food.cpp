#include "Food.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>

void Food::generate(const Snake& snake) {
    static bool initialized = false;
    if (!initialized) {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        initialized = true;
    }

    const int width = 20;
    const int height = 20;
    Point newPosition;
    bool validPosition;

    do {
        validPosition = true;
        newPosition.x = std::rand() % width;
        newPosition.y = std::rand() % height;

        // 检查是否生成在围墙上
        if (newPosition.x == 0 || newPosition.x == width - 1 ||
            newPosition.y == 0 || newPosition.y == height - 1) {
            validPosition = false;
            continue;
        }

        // 检查是否生成在蛇身上
        for (const auto& segment : snake.getBody()) {
            if (segment.x == newPosition.x && segment.y == newPosition.y) {
                validPosition = false;
                break;
            }
        }
    } while (!validPosition);

    position = newPosition;
}

void Food::setPosition(const Point& p) {
    position = p;
}

const Point& Food::getPosition() const {
    return position;
}