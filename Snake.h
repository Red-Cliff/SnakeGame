#ifndef SNAKE_H
#define SNAKE_H

#include <deque>

struct Point {
    int x, y;

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

class Snake {
public:
    Snake() : direction(3) { // 默认向右移动
        body.push_back({10, 10}); // 初始位置在 (10, 10)
    }

    bool move(int newDirection, bool wrapThroughWalls); // 返回 false 表示有限模式下撞墙
    void grow();
    void clear();
    void addSegment(const Point& segment);
    bool checkCollision(bool checkWalls) const;

    const std::deque<Point>& getBody() const; // 添加 const 修饰符
    const Point& getHead() const; // 添加 getHead 声明
    void setBody(const std::deque<Point>& newBody); // 设置蛇的身体

private:
    std::deque<Point> body;
    int direction;
};

#endif