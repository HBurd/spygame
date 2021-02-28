#pragma once

#include "util.h"
#include "shapes.h"
#include "rendering.h"

#define MAX_ENTITIES 65536

struct EntityRef
{
    size_t index = 0;
    uint version = 0;  // The default version is invalid

    bool operator==(const EntityRef& rhs);
};

struct Entity
{
    Transform2d transform;

    render::RenderObjectIndex render_object = render::cube;

    EntityRef ref;
    
    Entity() = default;
    Entity(Transform2d transform_);
};

struct EntityRecord
{
    uint version;   // Compared with EntityRef to check if ref is valid

    union
    {
        size_t index;
        size_t next_free;
    };

    static size_t first_free;

    static EntityRecord records[MAX_ENTITIES];

    static EntityRef create(size_t index);
    static void destroy(EntityRef ref);
};

extern Array<Entity> entities;

void init_entities();

EntityRef create_entity(Entity entity);

Entity* lookup_entity(EntityRef entity_ref);
