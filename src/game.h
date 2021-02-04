#pragma once

struct Renderer;

void init_game();
void update_game(float dt, const Renderer* renderer);
void render_game(Renderer* renderer);
