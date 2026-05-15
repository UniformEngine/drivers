#pragma once

#include <include/UFS/UFS.h>

#define UFS_INVALID                 0xFFFFFFFF
#define UFS_SCENE_MAX               255
#define UFS_FIELD_MAX               255
#define UFS_QUERY_MAX               1024
#define UFS_SYSTEM_MAX              1024
#define UFS_ENTITY_MAX              5000000
#define UFS_IDENTITY_MAX            4
#define UFS_COMPONENT_MAX           64 * UFS_IDENTITY_MAX
#define UFS_ARCHETYPE_MAX           64 * UFS_IDENTITY_MAX

typedef struct UFSEntity {
    UFHandle archetype;
    u64 identity[4];
    u32 ncomponents;
    u32 id;
} UFSEntity;

typedef struct UFSComponent {
    u64 identity[4];
    u64* strides;
    u32 fields;
    u32 slots;
    u32 lid;
    u32 id;
} UFSComponent;

typedef struct UFSArchetype {
    UFHandle* components;
    u64* offsets;   // [component base + field] = offset
    u64* strides;   // [component base + field] = stride
    u32* sMap;      // [slot] = entity id
    u32* eMap;      // [entity id] = slot
    u32* cMap;      // [component id] = component base
    void* data;     // [offset + slot * stride] = data
    void* start;

    u64 identity[4];
    u32 ncomponents;
    u32 nfields;

    u32 capacity;
    u32 count;
    u32 lid;
    u32 id;
} UFSArchetype;

typedef struct UFSQuery {
    UFHandle archetype;
    u64 identity[4];
    u32 lid;
    u32 id;
} UFSQuery;

typedef struct UFSSystem {
    UFResult (*init)(void* user, UFSView* view);
    UFResult (*main)(void* user, UFSView* view);
    UFResult (*exit)(void* user, UFSView* view);
    void* user;

    UFHandle query;
} UFSSystem;

typedef struct UFSScene {
    UFResourcePool archetypePool;
    UFResourcePool componentPool;
    UFResourcePool entityPool;
    UFResourcePool systemPool;
    UFResourcePool queryPool;
    UFHandle* liveArchetypes;
    UFHandle* liveComponents;
    UFHandle* liveSystems;
    UFHandle* liveQueries;
    u32 entityMax;
    u32 lid;
} UFSScene;
