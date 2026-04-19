#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "gameplay/Snake.h"
#include "gameplay/Food.h"
#include "GameMode.h"

class RenderSystem {
public:
    void render(const Snake& snake,
                const Food& food,
                int score,
                GameMode mode,
                bool specialFoodActive,
                const Point& specialFoodPosition,
                int specialFoodDisplayValue,
                int snakeColor);
};

#endif
