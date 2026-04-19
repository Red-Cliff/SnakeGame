#ifndef STATEMACHINE_H
#define STATEMACHINE_H

enum class GameState {
    MENU,
    RUNNING,
    PAUSED,
    GAME_OVER,
    EXIT,
};

enum class MenuPhase {
    MAIN,
    MODE_SELECT,
};

class StateMachine {
public:
    GameState state = GameState::MENU;
    MenuPhase menuPhase = MenuPhase::MAIN;
};

#endif
