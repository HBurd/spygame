#pragma once

struct SDL_Renderer;

void init_game(SDL_Renderer* renderer);
void update_game(float dt);
void render_game();
