#include <string.h>
#include <stdlib.h>
#include "../../_UFA.h"

#include "imobj.c"

UFAMesh UFAGetMesh(char* tag) {
    UFAMesh mesh = {0};
    if (!tag) return mesh;
    if (r3GetHashArrayPtr(tag, UFAInternal.assets.map)) {
        UFAsset* a = r3GetHashArrayPtr(tag, UFAInternal.assets.map);
        if (a->header.type != UFA_MESH) return mesh;
        mesh = a->mesh;
    } return mesh;
}

UFResult UFALoadMesh(UFAMeshDesc desc) {
    if (!desc.tag || !desc.path) return UF_ERROR;
    if (r3GetHashArrayPtr(desc.tag, UFAInternal.assets.map)) return UF_ERROR;

    UFAMeshData meshData = { 0 };
    switch(desc.format) {
        default: return UF_ERROR;
        case UFA_MESH_OBJ: {
            meshData = UFAImportOBJ(desc.path);
        } break;
    } if (!meshData.nvertices) {
        return UF_ERROR;
    }

    u64 dataLen = sizeof(u32) * 4 + sizeof(u32) * meshData.nindices + meshData.nvertices * meshData.stride;
    void* data = r3AllocMemory(dataLen);
    if (!data) {
        r3DelArray(meshData.vertices);
        r3DelArray(meshData.indices);
        return UF_ERROR;
    }

    u8* cursor = data;
    if (!r3WriteMemory(sizeof(u32), &meshData.stride, cursor)) goto fail;
    cursor += sizeof(u32);
    if (!r3WriteMemory(sizeof(u32), &meshData.attribs, cursor)) goto fail;
    cursor += sizeof(u32);
    if (!r3WriteMemory(sizeof(u32), &meshData.nindices, cursor)) goto fail;
    cursor += sizeof(u32);
    if (!r3WriteMemory(sizeof(u32), &meshData.nvertices, cursor)) goto fail;
    cursor += sizeof(u32);
    if (meshData.nindices) {
        if (!r3WriteMemory(sizeof(u32) * meshData.nindices, meshData.indices, cursor)) goto fail;
        cursor += sizeof(u32) * meshData.nindices;
    } if (!r3WriteMemory(meshData.nvertices * meshData.stride, meshData.vertices, cursor)) goto fail;

    UFAMesh mesh = {
        .attribs = meshData.attribs,
        .indices = meshData.nindices,
        .vertices = meshData.nvertices
    };

    UFRIResource vbo = UFRINewVertexBuffer((UFRIVertexBufferDesc){
        .vertices = meshData.vertices,
        .attribs = meshData.attribs,
        .count = meshData.nvertices,
        .memuse = UFRI_STATIC
    });
    if (vbo.handle == UF_INVALID) {
        goto fail;
    }

    if (meshData.indices) {
        UFRIResource ibo = UFRINewIndexBuffer((UFRIIndexBufferDesc){
            .indices = meshData.indices,
            .count = meshData.nindices,
            .memuse = UFRI_STATIC
        });
        if (ibo.handle == UF_INVALID) {
            goto fail;
        } mesh.ibo = ibo;
    } mesh.vbo = vbo;


    UFAsset asset = {
        .header.use = 0,
        .header.type = UFA_MESH,
        .header.dataLen = dataLen,
        .header.tagLen = strlen(desc.tag),

        .data = data,
        .mesh = mesh
    };

    if (!r3SetHashArray(desc.tag, &asset, UFAInternal.assets.map)) {
        goto fail;
    }

    UFAInternal.assets.count++;
    return UF_OK;

    fail:
        r3DelArray(meshData.vertices);
        r3DelArray(meshData.indices);
        r3FreeMemory(data);
        return UF_ERROR;
}
