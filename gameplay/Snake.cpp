#include "Snake.h"
#include <algorithm>

bool Snake::move(int newDirection, bool wrapThroughWalls) {
    constexpr int kWidth = 20;
    constexpr int kHeight = 20;
    constexpr int kMinX = 1;
    constexpr int kMaxX = kWidth - 2;
    constexpr int kMinY = 1;
    constexpr int kMaxY = kHeight - 2;

    direction = newDirection;
    Point newHead = body.front();
    switch (direction) {
        case 0: newHead.y--; break;
        case 1: newHead.y++; break;
        case 2: newHead.x--; break;
        case 3: newHead.x++; break;
    }

    // 有限模式：撞墙直接失败；经典模式：穿墙到对侧
    if (wrapThroughWalls) {
        if (newHead.x <= 0) newHead.x = kMaxX;
        else if (newHead.x >= kWidth - 1) newHead.x = kMinX;

        if (newHead.y <= 0) newHead.y = kMaxY;
        else if (newHead.y >= kHeight - 1) newHead.y = kMinY;
    } else {
        if (newHead.x <= 0 || newHead.x >= kWidth - 1 || newHead.y <= 0 || newHead.y >= kHeight - 1) {
            return false;
        }
    }

    body.push_front(newHead);
    body.pop_back();
    return true; // 正常移动，返回 true
}

void Snake::grow() {
    body.push_back(body.back());
}

void Snake::clear() {
    body.clear();
}

void Snake::addSegment(const Point& segment) {
    body.push_back(segment);
}

void Snake::setBody(const std::deque<Point>& newBody) {
    body = newBody;
}

bool Snake::checkCollision(bool checkWalls) const {
    const Point& head = body.front();

    // 检查是否碰到围墙
    if (checkWalls) {
        if (head.x <= 0 || head.x >= 19 || head.y <= 0 || head.y >= 19) {
            return true;
        }
    }

    // 检查是否碰到自己
    return std::any_of(body.begin() + 1, body.end(), [&head](const Point& p) {
        return p.x == head.x && p.y == head.y;
    });
}

const Point& Snake::getHead() const {
    return body.front();
}

const std::deque<Point>& Snake::getBody() const {
    return body;
}