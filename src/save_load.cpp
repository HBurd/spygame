#include "save_load.h"

#include "game.h"
#include "entity.h"
#include "util.h"

#include <cstdio>

struct SaveHeader
{
    // Save file is ordered:
    // - header
    // - game state
    // - entity records
    // - entities

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

    entities.size = header.num_entities;
    
    fclose(save_file);
    return true;
}
