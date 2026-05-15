#include "_UFS.h"

void UFSDelComponent(UFHandle handle, UFSScene* scene) {
    if (!UFREResourceValid(handle, &scene->componentPool)) return;

    UFSComponent* c = UFREResourcePointer(handle, &scene->componentPool);
    r3FreeMemory(c->strides);

    r3SwapPopArray(c->lid, scene->liveComponents);
    UFREDelResource(handle, &scene->componentPool);
}

UFHandle UFSNewComponent(UFSComponentDesc desc, UFSScene* scene) {
    UFHandle handle = UFRENewResource(NULL, &scene->componentPool);
    if (handle == UF_INVALID) {
        return UF_INVALID;
    }

    FOR(u8, f, 0, desc.fields, 1) {
        if (!desc.slots ||  !desc.strides[f]) {
            return UF_INVALID;
        }
    }

    UFSComponent component = {0};
    component.strides = r3AllocMemory(sizeof(u64) * desc.fields);
    if (!component.strides) return UF_INVALID;

    FOR(u8, f, 0, desc.fields, 1) component.strides[f] = desc.strides[f];
    component.fields = desc.fields;
    component.slots = desc.slots;

    u32 id = UFREResourceIndex(handle, &scene->componentPool);
    u32 chunk = id / 64;
    u32 bit = id % 64;

    component.identity[chunk] |= (1ULL << bit);
    component.lid = r3ArrayCount(scene->liveComponents);
    component.id = id;

    UFREWriteResource(handle, &component, &scene->componentPool);
    r3PushArray(&handle, scene->liveComponents);

    return handle;
}
