#include "../_UFS.h"

UFHandle UFSNewQuery(UFSMonoQueryDesc desc, UFSScene* scene) {
    UFHandle handle = UFRENewResource(NULL, &scene->queryPool);
    if (handle == UF_INVALID || !desc.count) return UF_INVALID;

    UFSQuery query = {0};
    FOR_I(0, desc.count, 1) {
        UFSComponent* component = UFREResourcePointer(desc.components[i], &scene->componentPool);
        FOR_J(0, 4, 1) {
            query.identity[j] |= component->identity[j];
        }
    }

    // check if query exists
    FOR_I(0, r3ArrayCount(scene->liveQueries), 1) {
        UFSQuery* q = UFREResourcePointer(scene->liveQueries[i], &scene->queryPool);

        u8 match = 1;
        FOR_J(0, UFS_IDENTITY_MAX, 1) {
            if (q->identity[j] != query.identity[j]) {
                match = 0;
                break;
            }
        } if (match) return scene->liveQueries[i];
    }

    FOR_U(0, r3ArrayCount(scene->liveArchetypes), 1) {
        UFSArchetype* arch = UFREResourcePointer(scene->liveArchetypes[u], &scene->archetypePool);

        u8 match = 1;
        FOR_V(0, UFS_IDENTITY_MAX, 1) {
            if (arch->identity[v] != query.identity[v]) {
                match = 0;
                break;
            }
        } if (match) {
            query.archetype = scene->liveArchetypes[u];
            break;
        }
    } if (query.archetype == UF_INVALID) {
        return UF_INVALID;
    }

    query.lid = r3ArrayCount(scene->liveQueries);
    query.id = UFREResourceIndex(handle, &scene->queryPool);
    UFREWriteResource(handle, &query, &scene->queryPool);

    r3PushArray(&handle, scene->liveQueries);

    return handle;
}

void* UFSViewField(UFHandle component, u32 field, UFSView* view, UFSScene* scene) {
    if (!view) return NULL;

    if (!UFREResourceValid(component, &scene->componentPool)) return NULL;
    if (field >= view->fields) return NULL;

    u32 b = view->map[UFREResourceIndex(component, &scene->componentPool)];
    u64 offset = view->offsets[b + field];
    return (void*)((u8*)view->data + offset);
}


void UFSResolveQuery(UFHandle query, UFSView* view, UFSScene* scene) {
    if (!UFREResourceValid(query, &scene->queryPool)) return;
    UFSQuery* q = UFREResourcePointer(query, &scene->queryPool);

    if (!UFREResourceValid(q->archetype, &scene->archetypePool)) return;
    UFSArchetype* a = UFREResourcePointer(q->archetype, &scene->archetypePool);

    view->offsets = a->offsets;
    view->strides = a->strides;
    view->fields = a->nfields;
    view->count = a->count;
    view->data = a->data;
    view->map = a->cMap;
}

UFResult UFSProjectQuery(UFSProjectionDesc desc, UFHandle query, UFSScene* scene) {
    if (!UFREResourceValid(query, &scene->queryPool)) return UF_ERROR;
    UFSQuery* q = UFREResourcePointer(query, &scene->queryPool);

    if (!UFREResourceValid(q->archetype, &scene->archetypePool)) return UF_ERROR;
    UFSArchetype* a = UFREResourcePointer(q->archetype, &scene->archetypePool);

    FOR_I(0, desc.count, 1) {
        if (!UFREResourceValid(desc.targets[i].component, &scene->componentPool)) return UF_ERROR;
        u32 c = UFREResourceIndex(desc.targets[i].component, &scene->componentPool);
        *(void**)desc.targets[i].dest = (void*)((u8*)a->data + a->offsets[a->cMap[c] + desc.targets[i].field]);
    } return UF_OK;
}

void UFSDelQuery(UFHandle handle, UFSScene* scene) {
    if (!UFREResourceValid(handle, &scene->queryPool)) return;
    UFSQuery* q = UFREResourcePointer(handle, &scene->queryPool);
    r3SwapPopArray(q->lid, scene->liveQueries);
    UFREDelResource(handle, &scene->queryPool);
}
