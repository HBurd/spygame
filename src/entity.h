#pragma once

#include "util.h"
#include "shapes.h"
#include "rendering.h"

#define MAX_ENTITIES 65536

struct EntityRef
{
    u32 index = 0;
    u32 version = 0;  // The default version is invalid

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
        u32 index;
        u32 next_free;
    };

    static u32 first_free;

    static EntityRecord records[MAX_ENTITIES];

    static EntityRef create(u32 index);
    static void destroy(EntityRef ref);
};

extern Array<Entity> entities;

void init_entities();

EntityRef create_entity(Entity entity);
void delete_entity(EntityRef entity);

Entity* lookup_entity(EntityRef entity_ref);
