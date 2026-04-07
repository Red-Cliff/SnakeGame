#include "SaveSystem.h"
#include <fstream>
#include <iostream>
#include <cstdint>
#include "Food.h" // 确保包含定义 Position 的头文件

namespace {
    constexpr std::uint32_t kSaveMagic = 0x56534753; // 'S' 'G' 'S' 'V'
    constexpr std::uint32_t kSaveVersion = 2;
}

void SaveSystem::save(const Snake& snake, const Food& food, int score, GameMode mode, const std::string& filename) {
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile) {
        std::cerr << "Failed to open file for saving: " << filename << std::endl;
        return;
    }

    // Header
    std::uint32_t magic = kSaveMagic;
    std::uint32_t version = kSaveVersion;
    std::int32_t modeValue = static_cast<std::int32_t>(mode);
    outFile.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
    outFile.write(reinterpret_cast<const char*>(&version), sizeof(version));
    outFile.write(reinterpret_cast<const char*>(&modeValue), sizeof(modeValue));

    // Save snake body
    const auto& body = snake.getBody();
    size_t bodySize = body.size();
    outFile.write(reinterpret_cast<const char*>(&bodySize), sizeof(bodySize));
    for (const auto& segment : body) {
        outFile.write(reinterpret_cast<const char*>(&segment), sizeof(segment));
    }

    // Save food position
    Point foodPosition = food.getPosition();
    outFile.write(reinterpret_cast<const char*>(&foodPosition), sizeof(foodPosition));

    // Save score
    outFile.write(reinterpret_cast<const char*>(&score), sizeof(score));

    outFile.close();
}

bool SaveSystem::load(Snake& snake, Food& food, int& score, GameMode& mode, const std::string& filename) {
    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile) {
        std::cerr << "Failed to open file for loading: " << filename << std::endl;
        return false; // Load failed
    }

    // Detect header (new format) vs legacy format
    std::uint32_t magic = 0;
    inFile.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    if (inFile.fail()) return false;

    if (magic == kSaveMagic) {
        std::uint32_t version = 0;
        inFile.read(reinterpret_cast<char*>(&version), sizeof(version));
        if (inFile.fail()) return false;

        if (version >= 2) {
            std::int32_t modeValue = static_cast<std::int32_t>(GameMode::Limited);
            inFile.read(reinterpret_cast<char*>(&modeValue), sizeof(modeValue));
            if (inFile.fail()) return false;
            mode = sanitizeGameMode(modeValue);
        } else {
            mode = GameMode::Limited;
        }
    } else {
        // Legacy format: rewind and parse from the beginning
        inFile.clear();
        inFile.seekg(0, std::ios::beg);
        mode = GameMode::Limited;
    }

    // Load snake body
    std::deque<Point> body;
    size_t bodySize;
    inFile.read(reinterpret_cast<char*>(&bodySize), sizeof(bodySize));
    if (inFile.fail()) return false; // Check read success

    body.resize(bodySize);
    for (auto& segment : body) {
        inFile.read(reinterpret_cast<char*>(&segment), sizeof(segment));
        if (inFile.fail()) return false; // Check read success
    }
    snake.setBody(body);

    // Load food position
    Point foodPosition;
    inFile.read(reinterpret_cast<char*>(&foodPosition), sizeof(foodPosition));
    if (inFile.fail()) return false; // Check read success
    food.setPosition(foodPosition);

    // Load score
    inFile.read(reinterpret_cast<char*>(&score), sizeof(score));
    if (inFile.fail()) return false; // Check read success

    inFile.close();
    return true; // Load successful
}

void SaveSystem::displaySaveSlots() {
    for (int slot = 1; slot <= 3; ++slot) {
        std::string filename = "save_slot_" + std::to_string(slot) + ".dat";
        std::ifstream inFile(filename, std::ios::binary);
        if (!inFile) {
            std::cout << "Slot " << slot << ": Empty\n";
            continue;
        }

        // Read mode if available (new format)
        GameMode mode = GameMode::Limited;
        {
            std::uint32_t magic = 0;
            inFile.read(reinterpret_cast<char*>(&magic), sizeof(magic));
            if (!inFile.fail() && magic == kSaveMagic) {
                std::uint32_t version = 0;
                inFile.read(reinterpret_cast<char*>(&version), sizeof(version));
                if (!inFile.fail() && version >= 2) {
                    std::int32_t modeValue = static_cast<std::int32_t>(GameMode::Limited);
                    inFile.read(reinterpret_cast<char*>(&modeValue), sizeof(modeValue));
                    if (!inFile.fail()) {
                        mode = sanitizeGameMode(modeValue);
                    }
                }
            }
        }

        int score;
        inFile.clear();
        inFile.seekg(-static_cast<int>(sizeof(score)), std::ios::end); // 定位到文件末尾的积分位置
        inFile.read(reinterpret_cast<char*>(&score), sizeof(score));

        if (inFile.fail()) {
            std::cout << "Slot " << slot << ": Corrupted\n";
        } else {
            std::cout << "Slot " << slot << ": Score = " << score << ", Mode = " << gameModeToString(mode) << "\n";
        }
    }
}