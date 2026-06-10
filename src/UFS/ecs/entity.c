#include "../_UFS.h"


UFHandle UFSNewEntity(UFSScene* scene) {
    UFHandle handle = UFRENewResource(NULL, &scene->entityPool);
    if (handle == UF_INVALID) return UF_INVALID;

    UFSEntity entity = {0};
    entity.id = UFREResourceIndex(handle, &scene->entityPool);
    entity.lid = r3ArrayCount(scene->liveEntities);
    entity.archetype = UF_INVALID;

    UFREWriteResource(handle, &entity, &scene->entityPool);
    r3PushArray(&handle, scene->liveEntities);

    return handle;
}

void UFSDelEntity(UFHandle handle, UFSScene* scene) {
    if (!UFREResourceValid(handle, &scene->entityPool)) return;
    UFSEntity* e = UFREResourcePointer(handle, &scene->entityPool);
    r3SwapPopArray(e->lid, scene->liveComponents);
    UFREDelResource(handle, &scene->entityPool);
}
