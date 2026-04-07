#ifndef FOOD_H
#define FOOD_H

#include "Snake.h" // 引入完整的 Point 定义

struct Point;

class Food {
public:
    Food() : position({5, 5}) {} // 默认位置为 (5, 5)
    void generate(const Snake& snake); // 修改 generate 方法，接受蛇的引用
    const Point& getPosition() const;
    void setPosition(const Point& p); // 添加 setPosition 声明

private:
    Point position;
};

#endif