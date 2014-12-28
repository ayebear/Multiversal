// Copyright (C) 2014 Eric Hebert (ayebear)
// This code is licensed under GPLv3, see LICENSE.txt for details.

#include "gameobjects.h"
#include "statemanager.h"
#include "gamestate.h"

int main()
{
    GameObjects objects("Puzzle Game v0.1.0 Dev");
    StateManager states;
    states.add<GameState>("Game", objects);
    states.start("Game");
    return 0;
}
