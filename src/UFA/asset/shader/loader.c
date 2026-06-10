#include "../../_UFA.h"
#include <string.h>
#include <stdlib.h>


UFAShader UFAGetShader(char* tag) {
    UFAShader shader = {0};
    if (!tag) return shader;
    if (r3GetHashArrayPtr(tag, UFAInternal.assets.map)) {
        UFAsset* a = r3GetHashArrayPtr(tag, UFAInternal.assets.map);
        if (a->header.type != UFA_SHADER) return shader;
        shader = a->shader;
    } return shader;
}

UFResult UFALoadShader(UFAShaderDesc desc) {
    if (!desc.tag || !desc.vertex || !desc.fragment) return UF_ERROR;
    if (r3GetHashArrayPtr(desc.tag, UFAInternal.assets.map)) return UF_ERROR;

    char* vs = r3LoadFile(desc.vertex);
    char* fs = r3LoadFile(desc.fragment);
    if (!vs || !fs) {
        if (vs) r3DelFile(vs);
        if (fs) r3DelFile(fs);
        return UF_ERROR;
    }

    UFRIResource shader = UFRINewShader((UFRIShaderDesc){
        .vertex = vs,
        .fragment = fs,
        .use = desc.use,
    });
    if (shader.handle == UF_INVALID) {
        r3DelFile(vs);
        r3DelFile(fs);
        return UF_ERROR;
    }

    u64 vsize = r3FileSize(vs);
    u64 fsize = r3FileSize(fs);

    u64 dataLen = vsize + fsize + sizeof(u64) * 2;
    void* data = r3AllocMemory(dataLen);
    if (!data) {
        r3DelFile(vs);
        r3DelFile(fs);
        return UF_ERROR;
    }

    if (!r3WriteMemory(sizeof(u64), &vsize, data)
    ||  !r3WriteMemory(vsize, vs, (u8*)data + sizeof(u64))
    ||  !r3WriteMemory(sizeof(u64), &fsize, (u8*)data + vsize + sizeof(u64))
    ||  !r3WriteMemory(fsize, fs, (u8*)data + vsize + sizeof(u64) * 2)) {
        r3FreeMemory(data);
        r3DelFile(vs);
        r3DelFile(fs);
        return UF_ERROR;
    }
    r3DelFile(vs);
    r3DelFile(fs);

    UFAsset asset = {
        .header.tagLen = strlen(desc.tag),
        .header.dataLen = dataLen,
        .header.type = UFA_SHADER,
        .header.use = desc.use,
        .data = data,

        .shader.shader = shader,
    };

    if (!r3SetHashArray(desc.tag, &asset, UFAInternal.assets.map)) {
        r3FreeMemory(data);
        return UF_ERROR;
    }

    UFAInternal.assets.count++;
    return UF_OK;
}
