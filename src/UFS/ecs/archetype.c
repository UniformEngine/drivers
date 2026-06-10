#include "../_UFS.h"
#include <stdio.h>


UFHandle UFSNewArchetype(u32 capacity, u8 ncomponents, UFHandle* components, UFSScene* scene) {
    if (!capacity || !ncomponents || !components) return UF_INVALID;

    UFHandle handle = UFRENewResource(NULL, &scene->archetypePool);
    if (handle == UF_INVALID) {
        return (u32)UF_INVALID;
    }

    UFSArchetype arch = {0};

    u64 data = 0;
    u32 fields = 0;
    FOR_I(0, ncomponents, 1) {
        if (!UFREResourceValid(components[i], &scene->componentPool)) return UF_INVALID;
        UFSComponent* component = UFREResourcePointer(components[i], &scene->componentPool);

        FOR_J(0, component->fields, 1) {
            data += capacity * component->strides[j];
        } FOR_K(0, UFS_IDENTITY_MAX, 1) {
            arch.identity[k] |= component->identity[k];
        } fields += component->fields;
    }

    u64 metadata =
        (sizeof(UFHandle) * ncomponents) +
        (sizeof(u64) * fields) +
        (sizeof(u64) * fields) +
        (sizeof(u32) * capacity) +
        (sizeof(u32) * scene->entityMax) +
        (sizeof(u32) * UFS_COMPONENT_MAX);

    arch.dataSize = data;
    arch.totalSize = metadata + data;
    arch.start = r3AllocMemory(arch.totalSize);
    if (!arch.start) return UF_INVALID;

    arch.components = (UFHandle*)arch.start;

    arch.offsets = (u64*)((u8*)arch.components + (sizeof(UFHandle) * ncomponents));

    arch.strides = (u64*)((u8*)arch.offsets + (sizeof(u64) * fields));

    arch.sMap = (u32*)((u8*)arch.strides + (sizeof(u64) * fields));
    r3SetMemory(sizeof(u32) * capacity, 0xFF, arch.sMap);

    arch.eMap = (u32*)((u8*)arch.sMap + (sizeof(u32) * capacity));
    r3SetMemory(sizeof(u32) * scene->entityMax, 0xFF, arch.eMap);

    arch.cMap = (u32*)((u8*)arch.eMap + (sizeof(u32) * scene->entityMax));
    r3SetMemory(sizeof(u32) * UFS_COMPONENT_MAX, 0xFF, arch.cMap);

    arch.data = (void*)((u8*)arch.cMap + (sizeof(u32) * UFS_COMPONENT_MAX));

    u32 field = 0;
    u64 offset = 0;
    FOR_I(0, ncomponents, 1) {
        UFSComponent* component = UFREResourcePointer(components[i], &scene->componentPool);

        arch.components[i] = components[i];
        arch.cMap[component->id] = field;   // component base index

        FOR_J(0, component->fields, 1) {
            arch.offsets[field] = offset;
            arch.strides[field] = component->strides[j];
            offset += capacity * component->strides[j];
            field++;
        }
    }

    arch.count = 0;
    arch.nfields = fields;
    arch.capacity = capacity;
    arch.ncomponents = ncomponents;
    arch.lid = r3ArrayCount(scene->liveArchetypes);
    arch.id = UFREResourceIndex(handle, &scene->archetypePool);

    UFREWriteResource(handle, &arch, &scene->archetypePool);
    r3PushArray(&handle, scene->liveArchetypes);

    return handle;
}

void UFSDelArchetype(UFHandle handle, UFSScene* scene) {
    if (!UFREResourceValid(handle, &scene->archetypePool)) return;

    UFSArchetype* a = UFREResourcePointer(handle, &scene->archetypePool);
    r3FreeMemory(a->start);

    UFREDelResource(handle, &scene->archetypePool);
    r3SwapPopArray(a->lid, scene->liveArchetypes);
}


void UFSBindArchetype(UFHandle entity, UFHandle arch, UFSScene* scene) {
    UFSArchetype* a = UFREResourcePointer(arch, &scene->archetypePool);
    if (a->count == a->capacity) return;

    UFSEntity* e = UFREResourcePointer(entity, &scene->entityPool);
    u32 id = UFREResourceIndex(entity, &scene->entityPool);
    u32 slot = a->count++;

    e->ncomponents = a->ncomponents;
    e->archetype = arch;
    a->sMap[slot] = id;
    a->eMap[id] = slot;
}

void UFSUnbindArchetype(UFHandle entity, UFHandle arch, UFSScene* scene) {
    UFSArchetype* a = UFREResourcePointer(arch, &scene->archetypePool);
    UFSEntity* e = UFREResourcePointer(entity, &scene->entityPool);
    u32 deadId = UFREResourceIndex(entity, &scene->entityPool);

    // if (a->count - 1 == 0) {
    // }

    u32 lastSlot = a->count - 1;
    u32 deadSlot = a->eMap[deadId];
    if (deadSlot == UFS_INVALID) return;

    // migrate per-field data
    FOR(u32, f, 0, a->nfields, 1) {
        void* base = (u8*)a->data + a->offsets[f];
        u64 stride = a->strides[f];

        void* dead = (u8*)base + deadSlot * stride;
        void* last = (u8*)base + lastSlot * stride;

        r3WriteMemory(stride,  last, dead);
    }

    u32 movedId = a->sMap[lastSlot];
    a->sMap[deadSlot] = movedId;
    a->eMap[movedId] = deadSlot;

    a->sMap[lastSlot] = UFS_INVALID;
    a->eMap[deadId] = UFS_INVALID;
    e->archetype =  UFS_INVALID;
    e->ncomponents = 0;
    a->count--;
}


void* UFSGetComponent(UFHandle entity, UFHandle component, u32 field, UFSScene* scene) {
    UFSEntity* e = UFREResourcePointer(entity, &scene->entityPool);
    if (!e || e->archetype == UF_INVALID) return NULL;

    UFSArchetype* a = UFREResourcePointer(e->archetype, &scene->archetypePool);
    UFSComponent* c = UFREResourcePointer(component, &scene->componentPool);
    if (!a || !c) return NULL;

    u32 slot = a->eMap[e->id];
    if (slot >= a->count || field >= c->fields) return NULL;

    u32 componentBase = a->cMap[c->id];
    if (componentBase + field >= a->nfields) return NULL;

    void* componentField = (u8*)a->data + a->offsets[componentBase + field];

    return (u8*)componentField + slot * c->strides[field];
}

UFResult UFSSetComponent(UFHandle entity, UFHandle component, u32 field, void* data, UFSScene* scene) {
    if (!data) return UF_ERROR;

    UFSEntity* e = UFREResourcePointer(entity, &scene->entityPool);
    if (!e || e->archetype == UF_INVALID) {
        printf("entity %llu is invalid\n", entity);
        return UF_ERROR;
    }

    UFSArchetype* a = UFREResourcePointer(e->archetype, &scene->archetypePool);
    UFSComponent* c = UFREResourcePointer(component, &scene->componentPool);
    if (!a || !c) {
        printf("archetype %llu or component %llu is invalid\n", e->archetype, component);
        return UF_ERROR;
    }

    u32 slot = a->eMap[e->id];
    if (slot >= a->count || field >= c->fields) {
        printf("slot %u or field %u is out of bounds\n", slot, field);
        return UF_ERROR;
    }

    u32 componentBase = a->cMap[c->id];
    if (componentBase + field >= a->nfields) {
        printf("component base %u or field %u is out of bounds\n", componentBase, field);
        return UF_ERROR;
    }

    void* componentField = (u8*)a->data + a->offsets[componentBase + field];

    u64 stride = c->strides[field];
    return (!r3WriteMemory(stride, data, (u8*)componentField + slot * stride)) ? UF_ERROR: UF_OK;
}
