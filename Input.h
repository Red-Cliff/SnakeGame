#ifndef INPUT_H
#define INPUT_H

class Input {
public:
    void setDirection(int dir); // 声明方法，移除内联定义
    int getDirection() const { return direction; }

private:
    int direction = 0;
};

#endif