#pragma once

#include "entity.h"

struct GameState
{
    EntityRef player;
};

extern GameState game_state;

void init_game();
void update_game(float dt);
void render_game();
