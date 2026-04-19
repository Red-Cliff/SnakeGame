#ifndef INPUTSYSTEM_H
#define INPUTSYSTEM_H

#include <vector>

class InputSystem {
public:
    // 一次性读取并清空本帧所有键盘输入（用于“同帧多输入、取最后合法方向”策略）
    std::vector<char> drainKeys() const;

    static bool tryMapKeyToDirection(char key, int& outDir);
    static bool isOpposite(int dir1, int dir2);
};

#endif
