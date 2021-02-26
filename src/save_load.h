#pragma once

// Note: there are no guarantees that save files will be compatible between different
// compilations of the application.

void save_scene(const char* filename);
bool load_scene(const char* filename);
