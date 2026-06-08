#pragma once
#include "_UFA.h"
#include <string.h>

UFResult UFALoadMesh(UFAMeshDesc desc) {
    if (!desc.tag || !desc.path) return UF_ERROR;
    if (r3GetHashArrayPtr(desc.tag, UFAInternal.assets.map)) return UF_ERROR;

    u8* file = r3LoadFile(desc.path);
    if (!file) return UF_ERROR;

    UFAsset asset = {0};

    r3DelFile(file);
    if (!r3SetHashArray(desc.tag, &asset, UFAInternal.assets.map)) {
        return UF_ERROR;
    } else return UF_OK;
}

static u8 temp[6] = {"Hello!"};
UFResult UFALoadShader(UFAShaderDesc desc) {
    if (!desc.tag || !desc.vertex || !desc.fragment) return UF_ERROR;
    if (r3GetHashArrayPtr(desc.tag, UFAInternal.assets.map)) return UF_ERROR;

    u8* vfile = r3LoadFile(desc.vertex);
    u8* ffile = r3LoadFile(desc.fragment);
    if (!vfile || !ffile) {
        if (vfile) r3DelFile(vfile);
        if (ffile) r3DelFile(ffile);
        return UF_ERROR;
    }

    UFAsset asset = {
        .header.tagLen = strlen(desc.tag),
        .header.dataLen = 6,
        .data = temp
    };

    r3DelFile(vfile);
    r3DelFile(ffile);
    if (!r3SetHashArray(desc.tag, &asset, UFAInternal.assets.map)) {
        return UF_ERROR;
    } else return UF_OK;
}


#define STB_IMAGE_IMPLEMENTATION
#include <include/STB/stb_image.h>
void* _lt(i32 width, i32 height, char* path) {
    i32 channels = 0;
    stbi_set_flip_vertically_on_load(1);
    ptr data = stbi_load(path, &width, &height, &channels, 0);
    if (!data) {
        r3LogStdOut(R3_LOG_ERROR, "`stbi_load` failed\n");
        return NULL;
    } return data;
}

UFResult UFALoadTexture(UFATextureDesc desc) {
    if (!desc.tag || !desc.path) return UF_ERROR;
    if (r3GetHashArrayPtr(desc.tag, UFAInternal.assets.map)) return UF_ERROR;

    u8* file = r3LoadFile(desc.path);
    if (!file) return UF_ERROR;

    UFAsset asset = {0};

    r3DelFile(file);
    if (!r3SetHashArray(desc.tag, &asset, UFAInternal.assets.map)) {
        return UF_ERROR;
    } else return UF_OK;
}

UFResult UFAUnloadAsset(char* tag) {
    if (!tag) return UF_ERROR;
    if (!r3RemHashArray(tag, UFAInternal.assets.map, &(UFAsset){0})) {
        return UF_ERROR;
    } else return UF_OK;
}
