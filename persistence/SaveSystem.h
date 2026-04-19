#ifndef SAVESYSTEM_H
#define SAVESYSTEM_H

#include "gameplay/Snake.h"
#include "gameplay/Food.h"
#include "GameMode.h"
#include <string>

class SaveSystem {
public:
    static void save(const Snake& snake, const Food& food, int score, GameMode mode, const std::string& filename); // 保存到指定文件
    static bool load(Snake& snake, Food& food, int& score, GameMode& mode, const std::string& filename); // 返回加载状态
    static void displaySaveSlots(); // 显示存档积分信息
};

#endif