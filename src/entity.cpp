#include "entity.h"
#include <cassert>

MAKE_ARRAY(entities, Entity, MAX_ENTITIES);

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

size_t EntityRecord::first_free;

EntityRecord EntityRecord::records[MAX_ENTITIES];

EntityRef EntityRecord::create(size_t index)
{
    // Check if there is room for another record
    assert(first_free < ARRAY_LENGTH(records));

    // Pick the first record in the free list
    EntityRef ref;
    ref.index = first_free;
    ref.version = records[first_free].version;

    first_free = records[first_free].next_free;

    // Point the record to the given entity index
    records[ref.index].index = index;

    return ref;
}

void EntityRecord::destroy(EntityRef ref)
{
    // Fail on double free
    assert(ref.version == records[ref.index].version);

    ++records[ref.index].version;

    // Prepend to free list
    records[ref.index].next_free = first_free;
    first_free = ref.index;
}

void init_entities()
{
    EntityRecord::first_free = 0;
    for (uint i = 0; i < ARRAY_LENGTH(EntityRecord::records); ++i)
    {
        // The default value for version in EntityRef is 0, so initialize all versions
        // to 1 in the record so that a default-constructed EntityRef is invalid.
        EntityRecord::records[i].version = 1;
        EntityRecord::records[i].next_free = i + 1;
    }
}

EntityRef create_entity(Entity entity)
{
    size_t index = entities.size;
    entities.push(entity);
    EntityRef ref = EntityRecord::create(index);

    // Initialize the entity's ref field so that the ref can be looked up from the entity.
    lookup_entity(ref)->ref = ref;

    return ref;
}

Entity::Entity(Transform2d transform_)
    : transform(transform_)
{}

Entity* lookup_entity(EntityRef ref)
{
    if (EntityRecord::records[ref.index].version == ref.version)
    {
        return &entities[EntityRecord::records[ref.index].index];
    }
    else
    {
        // The reference is no longer valid (entity was probably deleted)
        return nullptr;
    }
}

bool EntityRef::operator==(const EntityRef& rhs)
{
    return index == rhs.index && version == rhs.version;
}
