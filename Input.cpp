#include "Input.h"

void Input::setDirection(int newDirection) {
    // 防止蛇直接反向移动
    if ((direction == 0 && newDirection == 1) || (direction == 1 && newDirection == 0) ||
        (direction == 2 && newDirection == 3) || (direction == 3 && newDirection == 2)) {
        return;
    }
    direction = newDirection;
}