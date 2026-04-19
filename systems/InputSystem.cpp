#include "systems/InputSystem.h"
#include <conio.h>

std::vector<char> InputSystem::drainKeys() const {
    std::vector<char> keys;
    while (_kbhit()) {
        keys.push_back(_getch());
    }
    return keys;
}

bool InputSystem::tryMapKeyToDirection(char key, int& outDir) {
    switch (key) {
        case 'w':
        case 'W':
            outDir = 0;
            return true;
        case 's':
        case 'S':
            outDir = 1;
            return true;
        case 'a':
        case 'A':
            outDir = 2;
            return true;
        case 'd':
        case 'D':
            outDir = 3;
            return true;
        default:
            return false;
    }
}

bool InputSystem::isOpposite(int dir1, int dir2) {
    return (dir1 == 0 && dir2 == 1) || (dir1 == 1 && dir2 == 0) || (dir1 == 2 && dir2 == 3) ||
           (dir1 == 3 && dir2 == 2);
}
