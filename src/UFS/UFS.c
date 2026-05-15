#include "scene.c"

struct UFSInternal {
    UFInterface core;
    UFHandle temp[256];
} UFSInternal = {0};


UFHandle newScene(u32 entityMax) {
    if (!entityMax || entityMax > UFS_ENTITY_MAX) return UF_INVALID;
    return UFSNewScene(entityMax);
}

UFResult delScene(UFHandle handle) {
    return UFSDelScene(handle);
}


UFHandle newEntity(UFHandle scene) {
    if (!UFREResourceValid(scene, &UFSScenePool)) return UF_INVALID;
    return UFSNewEntity(UFREResourcePointer(scene, &UFSScenePool));
}

UFResult delEntity(UFHandle entity, UFHandle scene) {
    if (!UFREResourceValid(scene, &UFSScenePool)) return UF_ERROR;

    UFSScene* s = UFREResourcePointer(scene, &UFSScenePool);
    if (!UFREResourceValid(entity, &s->entityPool)) return UF_ERROR;

    UFSDelEntity(entity, s);
    return UF_OK;
}


UFHandle newComponent(UFSComponentDesc desc, UFHandle scene) {
    if (!UFREResourceValid(scene, &UFSScenePool)) return UF_INVALID;
    return UFSNewComponent(desc, UFREResourcePointer(scene, &UFSScenePool));
}

UFResult delComponent(UFHandle component, UFHandle scene) {
    if (!UFREResourceValid(scene, &UFSScenePool)) return UF_ERROR;

    UFSScene* s = UFREResourcePointer(scene, &UFSScenePool);
    if (!UFREResourceValid(component, &s->componentPool)) return UF_ERROR;

    UFSDelComponent(component, s);
    return UF_OK;
}


UFResult bindComponent(UFHandle entity, UFHandle component, UFHandle scene) {
    if (!UFREResourceValid(scene, &UFSScenePool)) return UF_ERROR;

    UFSScene* s = UFREResourcePointer(scene, &UFSScenePool);
    if (!UFREResourceValid(entity, &s->entityPool)
    ||  !UFREResourceValid(component, &s->componentPool)) return UF_ERROR;

    UFSComponent* c = UFREResourcePointer(component, &s->componentPool);
    UFSEntity* e = UFREResourcePointer(entity, &s->entityPool);

    FOR_I(0, UFS_IDENTITY_MAX, 1) e->identity[i] |= c->identity[i];
    e->ncomponents++;

    // resolve existing archetype
    UFHandle arch = UF_INVALID;
    FOR_J(0, r3ArrayCount(s->liveArchetypes), 1) {
        UFSArchetype* a = UFREResourcePointer(s->liveArchetypes[j], &s->archetypePool);

        u8 match = 1;
        FOR_K(0, UFS_IDENTITY_MAX, 1) {
            if (e->identity[k] != a->identity[k]) {
                match = 0;
                break;
            }
        } if (match) {
            arch = s->liveArchetypes[j];
            break;
        }
    }

    UFHandle old = e->archetype;
    if (arch == UF_INVALID) {
        // resolve components from entity identity
        u8 ncomponents = 0;
        FOR_K(0, r3ArrayCount(s->liveComponents), 1) {
            u8 match = 1;
            UFSComponent* liveComponent = UFREResourcePointer(s->liveComponents[k], &s->componentPool);
            FOR_I(0, UFS_IDENTITY_MAX, 1) {
                if ((e->identity[i] & liveComponent->identity[i]) != liveComponent->identity[i]) {
                    match = 0;
                    break;
                }
            } if (match) UFSInternal.temp[ncomponents++] = s->liveComponents[k];
        }

        arch = UFSNewArchetype(s->entityMax, e->ncomponents, UFSInternal.temp, s);  // no exisitng archetype, create new one
        r3SetMemory(sizeof(UFSInternal.temp), 0, UFSInternal.temp);                 // clear temp buffer
    } if (old != UF_INVALID) UFSUnbindArchetype(entity, old, s);                    // unbind old archetype
    UFSBindArchetype(entity, arch, s);                                              // bind new archetype

    return UF_OK;
}

UFResult unbindComponent(UFHandle entity, UFHandle component, UFHandle scene) {
    if (!UFREResourceValid(scene, &UFSScenePool)) return UF_ERROR;

    UFSScene* s = UFREResourcePointer(scene, &UFSScenePool);
    if (!UFREResourceValid(entity, &s->entityPool)
    ||  !UFREResourceValid(component, &s->componentPool)) return UF_ERROR;

    UFSComponent* c = UFREResourcePointer(component, &s->componentPool);
    UFSEntity* e = UFREResourcePointer(entity, &s->entityPool);

    FOR_I(0, UFS_IDENTITY_MAX, 1) e->identity[i] &= ~c->identity[i];
    e->ncomponents--;

    // resolve existing archetype
    UFHandle arch = UF_INVALID;
    FOR_J(0, r3ArrayCount(s->liveArchetypes), 1) {
        UFSArchetype* a = UFREResourcePointer(s->liveArchetypes[j], &s->archetypePool);

        u8 match = 1;
        FOR_K(0, UFS_IDENTITY_MAX, 1) {
            if (e->identity[k] != a->identity[k]) {
                match = 0;
                break;
            }
        } if (match) {
            arch = s->liveArchetypes[j];
            break;
        }
    }

    UFHandle old = e->archetype;
    if (arch == UF_INVALID) {
        // resolve components from entity identity
        u8 ncomponents = 0;
        FOR_K(0, r3ArrayCount(s->liveComponents), 1) {
            u8 match = 1;
            UFSComponent* liveComponent = UFREResourcePointer(s->liveComponents[k], &s->componentPool);
            FOR_I(0, UFS_IDENTITY_MAX, 1) {
                if ((e->identity[i] & liveComponent->identity[i]) != liveComponent->identity[i]) {
                    match = 0;
                    break;
                }
            } if (match) UFSInternal.temp[ncomponents++] = s->liveComponents[k];
        }

        arch = UFSNewArchetype(s->entityMax, e->ncomponents, UFSInternal.temp, s);  // no exisitng archetype, create new one
        r3SetMemory(sizeof(UFSInternal.temp), 0, UFSInternal.temp);                 // clear temp buffer
    } if (old != UF_INVALID) UFSUnbindArchetype(entity, old, s);                    // unbind old archetype
    UFSBindArchetype(entity, arch, s);                                              // bind new archetype

    return UF_OK;
}


void* getComponent(UFHandle entity, UFHandle component, u32 field, UFHandle scene) {
    if (!UFREResourceValid(scene, &UFSScenePool)) return NULL;

    UFSScene* s = UFREResourcePointer(scene, &UFSScenePool);
    if (!UFREResourceValid(entity, &s->entityPool)
    ||  !UFREResourceValid(component, &s->componentPool)) return NULL;

    return UFSGetComponent(entity, component, field, s);
}

void* viewField(UFHandle component, u32 field, UFSView* view, UFHandle scene) {
    if (!UFREResourceValid(scene, &UFSScenePool)) return NULL;
    UFSScene* s = UFREResourcePointer(scene, &UFSScenePool);
    return UFSViewField(component, field, view, s);
}

UFResult setComponent(UFHandle entity, UFHandle component, u32 field, void* data, UFHandle scene) {
    if (!UFREResourceValid(scene, &UFSScenePool)) return UF_ERROR;

    UFSScene* s = UFREResourcePointer(scene, &UFSScenePool);
    if (!UFREResourceValid(entity, &s->entityPool)
    ||  !UFREResourceValid(component, &s->componentPool)) return UF_ERROR;

    return UFSSetComponent(entity, component, field, data, s);
}


UFHandle newQuery(UFSMonoQueryDesc desc, UFHandle scene) {
    if (!UFREResourceValid(scene, &UFSScenePool)) return UF_INVALID;
    return UFSNewQuery(desc, UFREResourcePointer(scene, &UFSScenePool));
}

UFResult project(UFSProjectionDesc desc, UFHandle query, UFHandle scene) {
    if (!UFREResourceValid(scene, &UFSScenePool)) return UF_ERROR;
    return UFSProjectQuery(desc, query, UFREResourcePointer(scene, &UFSScenePool));
}

UFResult delQuery(UFHandle query, UFHandle scene) {
    if (!UFREResourceValid(scene, &UFSScenePool)) return UF_ERROR;

    UFSScene* s = UFREResourcePointer(scene, &UFSScenePool);
    if (!UFREResourceValid(query, &s->queryPool)) return UF_ERROR;

    UFSDelQuery(query, s);
    return UF_OK;
}


UFHandle newSystem(UFSSystemDesc desc, UFHandle query, UFHandle scene) {
    if (!UFREResourceValid(scene, &UFSScenePool)) return UF_INVALID;

    UFSScene* s = UFREResourcePointer(scene, &UFSScenePool);
    if (!UFREResourceValid(query, &s->queryPool)) return UF_INVALID;

    UFHandle handle = UFRENewResource(NULL, &s->systemPool);
    if (handle == UF_INVALID) return UF_INVALID;

    UFSSystem system = {0};
    system.init = desc.init;
    system.main = desc.main;
    system.exit = desc.exit;
    system.user = desc.user;
    system.query = query;

    UFSView view = {0};
    UFSResolveQuery(query, &view, s);

    UFREWriteResource(handle, &system, &s->systemPool);
    r3PushArray(&handle, s->liveSystems);

    if (system.init) system.init(system.user, &view);

    return handle;
}

// TDOO: runMonoSystem and runMultiSystem for MonoQuery (resolved to UFSView) and MultiQuery (resolved to UFSView* and u32)
UFResult runMonoSystem(UFHandle system, UFHandle scene) {
    if (!UFREResourceValid(scene, &UFSScenePool)) return UF_ERROR;
    UFSScene* s = UFREResourcePointer(scene, &UFSScenePool);

     if (!UFREResourceValid(system, &s->systemPool)) return UF_ERROR;
     UFSSystem* sys = UFREResourcePointer(system, &s->systemPool);

     UFSView view = {0};
     UFSResolveQuery(sys->query, &view, s);

     sys->main(sys->user, &view);

     return UF_OK;
}

UFResult delSystem(UFHandle system, UFHandle scene) {
    if (!UFREResourceValid(scene, &UFSScenePool)) return UF_ERROR;
    UFSScene* s = UFREResourcePointer(scene, &UFSScenePool);

     if (!UFREResourceValid(system, &s->systemPool)) return UF_ERROR;
     UFSSystem* sys = UFREResourcePointer(system, &s->systemPool);

     UFSView view = {0};
     UFSResolveQuery(sys->query, &view, s);
     if (sys->exit) sys->exit(sys->user, &view);

     UFREDelResource(system, &s->systemPool);
     r3SwapPopArray(r3InArray(&system, s->liveSystems), s->liveSystems);

     return UF_OK;
}


none driverInit(void) {
    UFSLiveScenes = r3NewArray(UFS_SCENE_MAX, sizeof(UFHandle));
    if (!UFSLiveScenes || UFRENewPool(UFS_SCENE_MAX, sizeof(UFSScene), 1, &UFSScenePool) != UF_OK) {
        if (UFSLiveScenes) r3DelArray(UFSLiveScenes);
        UFREDelPool(&UFSScenePool);

        r3LogStdOut(R3_LOG_ERROR, "[UFS] Failed to initialize internal resource pool\n");
        return;
    }
}

none driverExit(void) {
    FOR(u32, s, 0, r3ArrayCount(UFSLiveScenes), 1) {
        UFSDelScene(UFSLiveScenes[s]);
    }

    UFREDelPool(&UFSScenePool);
    r3DelArray(UFSLiveScenes);
}

UFS API = {
    .newScene = newScene,
    .delScene = delScene,

    .newEntity = newEntity,
    .delEntity = delEntity,

    .newComponent = newComponent,
    .delComponent = delComponent,

    .bindComponent = bindComponent,
    .unbindComponent = unbindComponent,

    .getComponent = getComponent,
    .setComponent = setComponent,
    .viewField = viewField,

    .newQuery = newQuery,
    .project = project,
    .delQuery = delQuery,

    .newSystem = newSystem,
    .runMonoSystem = runMonoSystem,
    .delSystem = delSystem,
};

UFDriver UFENTRY(UFInterface uniform) {
    UFSInternal.core = uniform;
    return (UFDriver) {
        .interface = &API,
        .init = driverInit,
        .exit = driverExit,
        .info = {
            .caps = 0,
            .flags = 0,
            .ver = "1.0.0",
            .tag = "Uniform Scenes",
        },
    };
}
