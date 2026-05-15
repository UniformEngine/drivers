#include "_UFS.h"
#include "entity.c"
#include "query.c"
#include "archetype.c"
#include "component.c"

static UFHandle* UFSLiveScenes = NULL;
static UFResourcePool UFSScenePool = {0};

UFHandle UFSNewScene(u32 entityMax) {
    UFHandle handle = UFRENewResource(NULL, &UFSScenePool);
    if (handle == UF_INVALID) return UF_INVALID;

    u32 chunks = 1;
    if (entityMax > 4096) {
        entityMax = entityMax + entityMax % 2;
        chunks = ((entityMax + entityMax % 2) / 4096) + 1;
    }

    UFSScene scene = {0};
    if (UFRENewPool(UFS_ARCHETYPE_MAX, sizeof(UFSArchetype), 2, &scene.archetypePool) != UF_OK
    ||  UFRENewPool(UFS_COMPONENT_MAX, sizeof(UFSComponent), 2, &scene.componentPool) != UF_OK
    ||  UFRENewPool(UFS_SYSTEM_MAX, sizeof(UFSSystem), 2, &scene.systemPool) != UF_OK
    ||  UFRENewPool(UFS_QUERY_MAX, sizeof(UFSQuery), 2, &scene.queryPool) != UF_OK
    ||  UFRENewPool(4096, sizeof(UFSEntity), chunks, &scene.entityPool) != UF_OK) {
        UFREDelPool(&scene.archetypePool);
        UFREDelPool(&scene.componentPool);
        UFREDelPool(&scene.entityPool);
        UFREDelPool(&scene.systemPool);
        UFREDelPool(&scene.queryPool);
        UFREDelResource(handle, &UFSScenePool);
        return UF_INVALID;
    }

    scene.liveArchetypes = r3NewArray(UFS_ARCHETYPE_MAX, sizeof(UFHandle));
    scene.liveComponents = r3NewArray(UFS_COMPONENT_MAX, sizeof(UFHandle));
    scene.liveSystems = r3NewArray(UFS_SYSTEM_MAX, sizeof(UFHandle));
    scene.liveQueries = r3NewArray(UFS_QUERY_MAX, sizeof(UFHandle));
    scene.entityMax = entityMax;

    if (!scene.liveArchetypes || !scene.liveComponents || !scene.liveQueries || !scene.liveSystems) {
        if (scene.liveArchetypes) r3DelArray(scene.liveArchetypes);
        UFREDelPool(&scene.archetypePool);

        if (scene.liveComponents) r3DelArray(scene.liveComponents);
        UFREDelPool(&scene.componentPool);

        UFREDelPool(&scene.entityPool);

        if (scene.liveSystems) r3DelArray(scene.liveSystems);
        UFREDelPool(&scene.systemPool);

        if (scene.liveQueries) r3DelArray(scene.liveQueries);
        UFREDelPool(&scene.queryPool);

        UFREDelResource(handle, &UFSScenePool);
        return UF_INVALID;
    }

    scene.lid = r3ArrayCount(UFSLiveScenes);
    UFREWriteResource(handle, &scene, &UFSScenePool);
    r3PushArray(&handle, UFSLiveScenes);

    return handle;
}

UFResult UFSDelScene(UFHandle handle) {
    if (!UFREResourceValid(handle, &UFSScenePool)) return UF_ERROR;
    UFSScene* s = UFREResourcePointer(handle, &UFSScenePool);

    FOR(u32, a, 0, r3ArrayCount(s->liveArchetypes), 1) {
        UFSDelArchetype(s->liveArchetypes[a], s);
    }
    UFREDelPool(&s->archetypePool);
    r3DelArray(s->liveArchetypes);

    FOR(u32, c, 0, r3ArrayCount(s->liveComponents), 1) {
        UFSDelComponent(s->liveComponents[c], s);
    }
    UFREDelPool(&s->componentPool);
    r3DelArray(s->liveComponents);

    UFREDelPool(&s->entityPool);

    UFREDelPool(&s->queryPool);
    r3DelArray(s->liveQueries);

    UFREDelPool(&s->systemPool);
    r3DelArray(s->liveSystems);

    UFREDelResource(handle, &UFSScenePool);
    r3SwapPopArray(s->lid, UFSLiveScenes);

    return UF_OK;
}
