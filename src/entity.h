#pragma once

#include "util.h"
#include "shapes.h"

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

    EntityRef ref;
    
    Entity() = default;
    Entity(Transform2d transform_);
};

extern Array<Entity> entities;

void init_entities();

EntityRef create_entity(Entity entity);

Entity* lookup_entity(EntityRef entity_ref);
