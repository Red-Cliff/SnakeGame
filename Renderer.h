#ifndef RENDERER_H
#define RENDERER_H

#include "Snake.h"
#include "Food.h"
#include "GameMode.h"

class Renderer {
public:
    void render(const Snake& snake,
                const Food& food,
                int score,
                GameMode mode,
                bool specialFoodActive,
                const Point& specialFoodPosition,
                int specialFoodDisplayValue);
};

#endif