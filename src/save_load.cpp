#include "save_load.h"

#include "game.h"
#include "entity.h"
#include "util.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>

struct SaveHeader
{
    // Save file is ordered:
    // - header
    // - game state
    // - entity records
    // - entities
    // - RenderObject filename count
    // - RenderObject filenames

    u32 num_entities;
};

void save_scene(const char* filename)
{
    FILE* save_file = fopen(filename, "w");
    
    SaveHeader header;
    header.num_entities = entities.size;

    fwrite(&header, sizeof(SaveHeader), 1, save_file);
    fwrite(&game_state, sizeof(GameState), 1, save_file);
    fwrite(EntityRecord::records, sizeof(EntityRecord), ARRAY_LENGTH(EntityRecord::records), save_file);
    fwrite(entities.data, sizeof(Entity), entities.size, save_file);

    for (uint i = 0; i < render::render_objects.size; ++i)
    {
        u32 filename_length = strlen(render::render_objects[i].filename);
        if (filename_length)
        {
            fwrite(&filename_length, sizeof(u32), 1, save_file);
            fwrite(render::render_objects[i].filename, 1, filename_length, save_file);
        }
    }

    fclose(save_file);
}

bool load_scene(const char* filename)
{
    FILE* save_file = fopen(filename, "r");
    if (!save_file)
    {
        return false;
    }

    SaveHeader header;
    fread(&header, sizeof(SaveHeader), 1, save_file);
    fread(&game_state, sizeof(GameState), 1, save_file);
    fread(EntityRecord::records, sizeof(EntityRecord), ARRAY_LENGTH(EntityRecord::records), save_file);
    fread(entities.data, sizeof(Entity), header.num_entities, save_file);
    
    u32 filename_length;
    while (fread(&filename_length, sizeof(u32), 1, save_file))
    {
        // TODO: memory leak (not freeing because the loaded RenderObject will point to filename ...
        // not sure what to do here).
        char* filename = (char*) malloc(filename_length + 1);
        fread(filename, 1, filename_length, save_file);
        filename[filename_length] = 0;
        render::load_obj(filename);
    }

    entities.size = header.num_entities;
    
    fclose(save_file);
    return true;
}
