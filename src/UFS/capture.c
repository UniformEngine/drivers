#include "_UFS.h"
#include "scene.c"

UFResult UFSDelCapture(UFSCapture* capture) {
    FOR_I(0, capture->header.narchetypes, 1) {
        UFSSerialArchetype* archetypes = capture->data.archetype;
        if (archetypes[i].data) r3FreeMemory(archetypes[i].data);
        archetypes[i].data = NULL;
        archetypes[i].sMap = NULL;
    } FOR_J(0, capture->header.ncomponents, 1) {
        UFSSerialComponent* components = capture->data.component;
        if (components[j].strides) r3FreeMemory(components[j].strides);
        components[j].strides = NULL;
    }
    r3FreeMemory(capture->data.archetype);
    r3FreeMemory(capture->data.component);
    r3FreeMemory(capture->data.entity);
    return UF_OK;
}

UFSCapture UFSNewCapture(UFHandle handle) {
    u64 size = sizeof(UFSCapture);
    UFSScene* s = UFREResourcePointer(handle, &UFSScenePool);

    UFSSerialArchetype* archetypes = r3AllocMemory(r3ArrayCount(s->liveArchetypes) * sizeof(UFSSerialArchetype));
    UFSSerialComponent* components = r3AllocMemory(r3ArrayCount(s->liveComponents) * sizeof(UFSSerialComponent));
    UFSSerialEntity* entities = r3AllocMemory(r3ArrayCount(s->liveEntities) * sizeof(UFSSerialEntity));
    if (!archetypes || !components || !entities) {
        if (archetypes) r3FreeMemory(archetypes);
        if (components) r3FreeMemory(components);
        if (entities) r3FreeMemory(entities);
        return (UFSCapture){0};
    }
    size += r3ArrayCount(s->liveArchetypes) * sizeof(UFSSerialArchetype);
    size += r3ArrayCount(s->liveComponents) * sizeof(UFSSerialComponent);
    size += r3ArrayCount(s->liveEntities) * sizeof(UFSSerialEntity);

    UFSCapture capture = {
        .header.narchetypes = r3ArrayCount(s->liveArchetypes),
        .header.ncomponents = r3ArrayCount(s->liveComponents),
        .header.nentities = r3ArrayCount(s->liveEntities),
        .header.max = s->entityMax,
        .header.version = UFS_VERSION,
        .header.magic = UFS_MAGIC,

        .data.archetype = archetypes,
        .data.component = components,
        .data.entity = entities,
    };

    FOR_I(0, capture.header.narchetypes, 1) {
        UFSArchetype* a = UFREResourcePointer(s->liveArchetypes[i], &s->archetypePool);

        void* start = r3AllocMemory(a->dataSize + a->capacity * sizeof(u32));
        size += a->dataSize + a->capacity * sizeof(u32);
        if (!start) {
            UFSDelCapture(&capture);
            return (UFSCapture){0};
        }

        UFSSerialArchetype archetype = {
            .data = start,
            .sMap = (u8*)start + a->dataSize,

            .count = a->count,
            .size = a->dataSize,
            .capacity = a->capacity,
        };
        r3WriteMemory(a->dataSize, a->data, archetype.data);
        r3WriteMemory(a->capacity * sizeof(u32), a->sMap, archetype.sMap);
        r3WriteMemory(sizeof(archetype.identity), a->identity, archetype.identity);

        ((UFSSerialArchetype*)capture.data.archetype)[i] = archetype;
    }

    FOR_J(0, capture.header.ncomponents, 1) {
        UFSComponent* c = UFREResourcePointer(s->liveComponents[j], &s->componentPool);

        void* strides = r3AllocMemory(c->fields * sizeof(u64));
        size += c->fields * sizeof(u64);
        if (!strides) {
            UFSDelCapture(&capture);
            return (UFSCapture){0};
        }

        UFSSerialComponent component = {
            .strides = strides,
            .fields = c->fields,
            .slots = c->slots,
        };
        r3WriteMemory(c->fields * sizeof(u64), c->strides, component.strides);
        r3WriteMemory(sizeof(component.identity), c->identity, component.identity);

        ((UFSSerialComponent*)capture.data.component)[j] = component;
    }

    FOR_K(0, capture.header.nentities, 1) {
        UFSEntity* e = UFREResourcePointer(s->liveEntities[k], &s->entityPool);

        UFSSerialEntity entity ={ .ncomponents = e->ncomponents };
        r3WriteMemory(sizeof(entity.identity), e->identity, entity.identity);

        ((UFSSerialEntity*)capture.data.entity)[k] = entity;
    }

    capture.header.size = size;
    return capture;
}


u8 UFSCompareCapture(UFSCapture* a, UFSCapture* b) {
    if (!a || !b) return 0;

    // header
    if (a->header.max != b->header.max) return 0;
    if (a->header.size != b->header.size) return 0;
    if (a->header.magic != b->header.magic) return 0;
    if (a->header.version != b->header.version) return 0;
    if (a->header.nentities != b->header.nentities) return 0;
    if (a->header.ncomponents != b->header.ncomponents) return 0;
    if (a->header.narchetypes != b->header.narchetypes) return 0;

    UFSSerialArchetype* aa = a->data.archetype;
    UFSSerialArchetype* ba = b->data.archetype;

    UFSSerialComponent* ac = a->data.component;
    UFSSerialComponent* bc = b->data.component;

    UFSSerialEntity* ae = a->data.entity;
    UFSSerialEntity* be = b->data.entity;

    // entities
    FOR(u32, i, 0, a->header.nentities, 1) {
        if (ae[i].ncomponents != be[i].ncomponents) {
            return 0;
        }

        if (r3CompareMemory(
            sizeof(ae[i].identity),
            ae[i].identity,
            be[i].identity
        ) != R3_OK) {
            return 0;
        }
    }

    // components
    FOR(u32, i, 0, a->header.ncomponents, 1) {
        if (ac[i].fields != bc[i].fields) {
            return 0;
        }

        if (ac[i].slots != bc[i].slots) {
            return 0;
        }

        if (r3CompareMemory(
            sizeof(ac[i].identity),
            ac[i].identity,
            bc[i].identity
        ) != R3_OK) {
            return 0;
        }

        if (r3CompareMemory(
            ac[i].fields * sizeof(u64),
            ac[i].strides,
            bc[i].strides
        ) != R3_OK) {
            return 0;
        }
    }

    // archetypes
    FOR(u32, i, 0, a->header.narchetypes, 1) {
        if (aa[i].count != ba[i].count) {
            return 0;
        }

        if (aa[i].capacity != ba[i].capacity) {
            return 0;
        }

        if (aa[i].size != ba[i].size) {
            return 0;
        }

        if (r3CompareMemory(
            sizeof(aa[i].identity),
            aa[i].identity,
            ba[i].identity
        ) != R3_OK) {
            return 0;
        }

        if (aa[i].count) {
            if (r3CompareMemory(
                aa[i].size,
                aa[i].data,
                ba[i].data
            ) != R3_OK) {
                return 0;
            }

            if (r3CompareMemory(
                aa[i].capacity * sizeof(u32),
                aa[i].sMap,
                ba[i].sMap
            ) != R3_OK) {
                return 0;
            }
        }
    }

    return 1;
}


// restores a scene from a capture
UFHandle UFSRestoreCapture(UFSCapture* capture) {
    if (capture->header.magic != UFS_MAGIC || capture->header.version != UFS_VERSION) return UF_ERROR;

    UFHandle* eMap = r3AllocMemory(capture->header.max * sizeof(UFHandle));
    UFHandle* cMap = r3AllocMemory(UFS_COMPONENT_MAX * sizeof(UFHandle));
    if (cMap == NULL || eMap == NULL) {
        r3FreeMemory(cMap);
        r3FreeMemory(eMap);
        return UF_INVALID;
    }

    UFHandle handle = UFSNewScene(capture->header.max);
    if (handle == UF_INVALID) {
        r3FreeMemory(cMap);
        r3FreeMemory(eMap);
        return UF_INVALID;
    }

    UFSSerialArchetype* archetypes = (UFSSerialArchetype*)capture->data.archetype;
    UFSSerialComponent* components = (UFSSerialComponent*)capture->data.component;
    UFSSerialEntity* entities = (UFSSerialEntity*)capture->data.entity;
    UFSScene* s = UFREResourcePointer(handle, &UFSScenePool);

    // restore entities
    FOR(u32, e, 0, capture->header.nentities, 1) {
        UFHandle h = UFSNewEntity(s);
        UFSEntity* entity = UFREResourcePointer(h, &s->entityPool);

        r3WriteMemory(sizeof(entities[e].identity), entities[e].identity, entity->identity);
        entity->ncomponents = entities[e].ncomponents;
        eMap[entity->id] = h;
    }

    // restore components
    FOR(u32, c, 0, capture->header.ncomponents, 1) {
        UFHandle h = UFSNewComponent((UFSComponentDesc){
            .strides = components[c].strides,
            .fields = components[c].fields,
            .slots = components[c].slots,
        }, s);

        UFSComponent* component = UFREResourcePointer(h, &s->componentPool);
        r3WriteMemory(sizeof(components[c].identity), components[c].identity, component->identity);
        cMap[c] = h;
    }

    // restore archetypes
    UFHandle temp[UFS_COMPONENT_MAX] = {0};
    FOR(u32, a, 0, capture->header.narchetypes, 1) {
        u32 count = 0;
        FOR(u32, c, 0, capture->header.ncomponents, 1) {
            UFHandle h = cMap[c];
            UFSComponent* component = UFREResourcePointer(h, &s->componentPool);

            u8 match = 1;
            FOR_I(0, 4, 1) {
                if ((archetypes[a].identity[i] & component->identity[i]) != component->identity[i]) {
                    match = 0;
                    break;
                }
            } if (match) temp[count++] = h;
        }

        UFHandle h = UFSNewArchetype(archetypes[a].capacity, count, temp, s);
        UFSArchetype* archetype = UFREResourcePointer(h, &s->archetypePool);

        archetype->count = archetypes[a].count;
        r3WriteMemory(archetypes[a].size, archetypes[a].data, archetype->data);
        r3WriteMemory(archetypes[a].capacity * sizeof(u32), archetypes[a].sMap, archetype->sMap);

        FOR(u32, slot, 0, archetypes[a].count, 1) {
            u32 entityID = archetype->sMap[slot];

            // restore entity-archetype references
            UFHandle entity = eMap[entityID];
            UFSEntity* e = UFREResourcePointer(entity, &s->entityPool);
            e->archetype = h;

            archetype->eMap[entityID] = slot;
        }

        r3SetMemory(sizeof(UFHandle) * count, 0, temp);
    }

    r3FreeMemory(cMap);
    r3FreeMemory(eMap);
    return handle;
}

// compiles a capture to a file on disk
UFResult UFSCompileCapture(char* path, UFSCapture* capture) {
    if (capture->header.magic != UFS_MAGIC || capture->header.version != UFS_VERSION) return UF_ERROR;

    u8* file = r3NewFile(capture->header.size);
    if (!file) return UF_ERROR;

    r3RemFileFlags(R3_FILE_WRITE, file);

    r3WriteFile(sizeof(capture->header), &capture->header, file);

    UFSSerialComponent* components = capture->data.component;
    UFSSerialArchetype* archetypes = capture->data.archetype;
    UFSSerialEntity* entities = capture->data.entity;
    FOR_J(0, capture->header.nentities, 1) {
        r3WriteFile(sizeof(UFSSerialEntity), &entities[j], file);
    }

    FOR_I(0, capture->header.ncomponents, 1) {
        struct {
            u32 slots;
            u32 fields;
            u64 identity[4];
        } data = {
            .slots = components[i].slots,
            .fields = components[i].fields
        };
        r3WriteMemory(sizeof(data.identity), components[i].identity, data.identity);

        r3WriteFile(sizeof(data), &data, file);
        r3WriteFile(sizeof(u64) * components[i].fields, components[i].strides, file);
    }

    FOR_K(0, capture->header.narchetypes, 1) {
        struct {
            u64 size;
            u32 count;
            u64 capacity;
            u64 identity[4];
        } record = {
            .size = archetypes[k].size,
            .count = archetypes[k].count,
            .capacity = archetypes[k].capacity
        };
        r3WriteMemory(sizeof(record.identity), archetypes[k].identity, record.identity);

        r3WriteFile(sizeof(record), &record, file);
        r3WriteFile(archetypes[k].size, archetypes[k].data, file);
        r3WriteFile(sizeof(u32) * record.capacity, archetypes[k].sMap, file);
    }

    if (!r3SaveFile(r3FileCursor(file), path, file)) {
        r3DelFile(file);
        return UF_ERROR;
    }

    r3DelFile(file);
    return UF_OK;
}

// decompiles a capture from a file on disk
UFSCapture UFSDecompileCapture(char* path) {
    UFSCapture capture = {0};

    u8* file = r3LoadFile(path);
    if (!file) return (UFSCapture){0};

    r3ReadFile(sizeof(capture.header), file, &capture.header);
    r3SeekFile(sizeof(capture.header), file);

    if (capture.header.magic != UFS_MAGIC || capture.header.version != UFS_VERSION) {
        r3DelFile(file);
        return (UFSCapture){0};
    }

    capture.data.archetype = r3AllocMemory(capture.header.narchetypes * sizeof(UFSSerialArchetype));
    capture.data.component = r3AllocMemory(capture.header.ncomponents * sizeof(UFSSerialComponent));
    capture.data.entity = r3AllocMemory(capture.header.nentities * sizeof(UFSSerialEntity));
    if (!capture.data.archetype || !capture.data.component || !capture.data.entity) {
        UFSDelCapture(&capture);
        r3DelFile(file);
        return (UFSCapture){0};
    }

    UFSSerialEntity* entities = capture.data.entity;
    FOR_I(0, capture.header.nentities, 1) {
        r3ReadFile(sizeof(UFSSerialEntity), file, &entities[i]);
        r3SeekFile(sizeof(UFSSerialEntity), file);
    }

    UFSSerialComponent* components = capture.data.component;
    FOR_I(0, capture.header.ncomponents, 1) {
        struct {
            u32 slots;
            u32 fields;
            u64 identity[4];
        } record = {0};
        r3ReadFile(sizeof(record), file, &record);
        r3SeekFile(sizeof(record), file);

        void* strides = r3AllocMemory(record.fields * sizeof(u64));
        if (!strides) {
            UFSDelCapture(&capture);
            r3DelFile(file);
            return (UFSCapture){0};
        }
        r3ReadFile(record.fields * sizeof(u64), file, strides);
        r3SeekFile(record.fields * sizeof(u64), file);

        UFSSerialComponent component = {
            .fields = record.fields,
            .slots = record.slots,
            .strides = strides,
        };
        r3WriteMemory(sizeof(u64) * 4, record.identity, component.identity);
        components[i] = component;
    }

    UFSSerialArchetype* archetypes = capture.data.archetype;
    FOR_I(0, capture.header.narchetypes, 1) {
        struct {
            u64 size;
            u32 count;
            u64 capacity;
            u64 identity[4];
        } record = {0};
        r3ReadFile(sizeof(record), file, &record);
        r3SeekFile(sizeof(record), file);

        void* data = r3AllocMemory(record.size + record.capacity * sizeof(u32));
        if (!data) {
            UFSDelCapture(&capture);
            r3DelFile(file);
            return (UFSCapture){0};
        }
        r3ReadFile(record.size + record.capacity * sizeof(u32), file, data);
        r3SeekFile(record.size + record.capacity * sizeof(u32), file);

        UFSSerialArchetype archetype = {
            .data = data,
            .sMap = (u8*)data + record.size,

            .size = record.size,
            .count = record.count,
            .capacity = record.capacity,
        };
        r3WriteMemory(sizeof(u64) * 4, record.identity, archetype.identity);
        archetypes[i] = archetype;
    }

    r3DelFile(file);
    return capture;
}
