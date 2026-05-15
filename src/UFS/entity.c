#include "_UFS.h"

UFHandle UFSNewEntity(UFSScene* scene) {
    UFHandle handle = UFRENewResource(NULL, &scene->entityPool);
    if (handle == UF_INVALID) return UF_INVALID;

    UFSEntity entity = {0};
    entity.id = UFREResourceIndex(handle, &scene->entityPool);
    entity.archetype = UF_INVALID;

    UFREWriteResource(handle, &entity, &scene->entityPool);

    return handle;
}

void UFSDelEntity(UFHandle handle, UFSScene* scene) {
    if (!UFREResourceValid(handle, &scene->entityPool)) return;
    UFREDelResource(handle, &scene->entityPool);
}
