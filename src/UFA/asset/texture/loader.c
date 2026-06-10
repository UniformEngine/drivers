#include "../../_UFA.h"
#include <string.h>
#include <stdlib.h>


UFATexture UFAGetTexture(char* tag) {
    UFATexture texture = {0};
    if (!tag) return texture;
    if (r3GetHashArrayPtr(tag, UFAInternal.assets.map)) {
        UFAsset* a = r3GetHashArrayPtr(tag, UFAInternal.assets.map);
        if (a->header.type != UFA_TEXTURE) return texture;
        texture = a->texture;
    } return texture;
}

UFResult UFALoadTexture(UFATextureDesc desc) {
    if (!desc.tag || !desc.path) return UF_ERROR;
    if (r3GetHashArrayPtr(desc.tag, UFAInternal.assets.map)) return UF_ERROR;

    u8* file = r3LoadFile(desc.path);
    if (!file) {
        return UF_ERROR;
    }

    u64 dataLen = r3FileSize(file) + sizeof(u32) * 3;
    void* data = r3AllocMemory(dataLen);
    if (!data) {
        r3DelFile(file);
        return UF_ERROR;
    }

    if (!r3WriteMemory(sizeof(u32), &desc.use, data)
    ||  !r3WriteMemory(sizeof(u32), &desc.memuse, (u8*)data + sizeof(u32))
    ||  !r3WriteMemory(sizeof(u32), &desc.format, (u8*)data + sizeof(u32) * 2)
    ||  !r3WriteMemory(r3FileSize(file), file, (u8*)data + sizeof(u32) * 3)) {
        r3FreeMemory(data);
        r3DelFile(file);
        return UF_ERROR;
    } r3DelFile(file);

    i32 channels = 0;
    i32 width, height;
    stbi_set_flip_vertically_on_load(1);
    void* textureData = stbi_load(desc.path, &width, &height, &channels, 0);
    if (!textureData) {
        r3FreeMemory(data);
        return UF_ERROR;
    }

    UFRIResource texture = UFRINewTexture((UFRITextureDesc){
        .use = desc.use,
        .memuse = desc.memuse,
        .format = desc.format,
        .height = height,
        .width = width
        ,
        .data = textureData,
    });
    if (texture.handle == UF_INVALID) {
        free(textureData);
        r3FreeMemory(data);
        return UF_ERROR;
    }

    UFAsset asset = {
        .header.tagLen = strlen(desc.tag),
        .header.dataLen = dataLen,
        .header.type = UFA_TEXTURE,
        .header.use = desc.use,
        .data = data,

        .texture.texture = texture
    };

    if (!r3SetHashArray(desc.tag, &asset, UFAInternal.assets.map)) {
        free(textureData);
        r3FreeMemory(data);
        return UF_ERROR;
    }

    UFAInternal.assets.count++;
    return UF_OK;
}
