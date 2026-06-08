#include "_UFA.h"
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include <include/STB/stb_image.h>

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


UFAShader UFAGetShader(char* tag) {
    UFAShader shader = {0};
    if (!tag) return shader;
    if (r3GetHashArrayPtr(tag, UFAInternal.assets.map)) {
        shader = ((UFAsset*)r3GetHashArrayPtr(tag, UFAInternal.assets.map))->shader;
    } return shader;
}

UFResult UFALoadShader(UFAShaderDesc desc) {
    if (!desc.tag || !desc.vertex || !desc.fragment) return UF_ERROR;
    if (r3GetHashArrayPtr(desc.tag, UFAInternal.assets.map)) return UF_ERROR;

    char* vfile = r3LoadFile(desc.vertex);
    char* ffile = r3LoadFile(desc.fragment);
    if (!vfile || !ffile) {
        if (vfile) r3DelFile(vfile);
        if (ffile) r3DelFile(ffile);
        return UF_ERROR;
    }

    UFRIResource shader = UFRINewShader((UFRIShaderDesc){
        .vertex = vfile,
        .fragment = ffile,
        .type = desc.type,
    });
    if (shader.handle == UF_INVALID) {
        r3DelFile(vfile);
        r3DelFile(ffile);
        return UF_ERROR;
    }

    u64 vsize = r3FileSize(vfile);
    u64 fsize = r3FileSize(ffile);

    u64 dataLen = vsize + fsize + sizeof(u64) * 2;
    void* data = r3AllocMemory(dataLen);
    if (!data) {
        r3DelFile(vfile);
        r3DelFile(ffile);
        return UF_ERROR;
    }

    if (!r3WriteMemory(sizeof(u64), &vsize, data)
    ||  !r3WriteMemory(vsize, vfile, data + sizeof(u64))
    ||  !r3WriteMemory(sizeof(u64), &fsize, data + vsize + sizeof(u64))
    ||  !r3WriteMemory(fsize, ffile, data + vsize + sizeof(u64) * 2)) {
        r3FreeMemory(data);
        r3DelFile(vfile);
        r3DelFile(ffile);
        return UF_ERROR;
    }
    r3DelFile(vfile);
    r3DelFile(ffile);

    UFAsset asset = {
        .header.tagLen = strlen(desc.tag),
        .header.dataLen = dataLen,
        .header.type = UFA_SHADER,
        .header.use = desc.type,
        .data = data,

        .shader.shader = shader,
    };

    if (!r3SetHashArray(desc.tag, &asset, UFAInternal.assets.map)) {
        r3FreeMemory(data);
        return UF_ERROR;
    } return UF_OK;
}


UFATexture UFAGetTexture(char* tag) {
    UFATexture texture = {0};
    if (!tag) return texture;
    if (r3GetHashArrayPtr(tag, UFAInternal.assets.map)) {
        texture = ((UFAsset*)r3GetHashArrayPtr(tag, UFAInternal.assets.map))->texture;
    } return texture;
}

UFResult UFALoadTexture(UFATextureDesc desc) {
    if (!desc.tag || !desc.path) return UF_ERROR;
    if (r3GetHashArrayPtr(desc.tag, UFAInternal.assets.map)) return UF_ERROR;

    u8* file = r3LoadFile(desc.path);
    if (!file) {
        return UF_ERROR;
    }

    u64 dataLen = r3FileSize(file);
    void* data = r3AllocMemory(dataLen);
    if (!data) {
        r3DelFile(file);
        return UF_ERROR;
    }

    // TODO: prefix asset data with descriptor use, memuse, and format (12 bytes)
    if (!r3WriteMemory(dataLen, file, data)) {
        r3FreeMemory(data);
        r3DelFile(file);
        return UF_ERROR;
    } r3DelFile(file);

    i32 wh[2] = {0};
    i32 channels = 0;
    stbi_set_flip_vertically_on_load(1);
    void* textureData = stbi_load(desc.path, &wh[0], &wh[1], &channels, 0);
    if (!textureData) {
        r3FreeMemory(data);
        return UF_ERROR;
    }

    UFRIResource texture = UFRINewTexture((UFRITextureDesc){
        .use = desc.use,
        .memuse = desc.memuse,
        .format = desc.format,
        .height = wh[1],
        .width = wh[0],
        .data = textureData,
    });
    if (texture.handle == UF_INVALID) {
        stbi_image_free(textureData);
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
        stbi_image_free(textureData);
        r3FreeMemory(data);
        return UF_ERROR;
    } return UF_OK;
}


UFResult UFAUnloadAsset(char* tag) {
    if (!tag) return UF_ERROR;

    UFAsset asset = {0};
    if (!r3RemHashArray(tag, UFAInternal.assets.map, &asset)) {
        return UF_ERROR;
    }

    switch (asset.header.type) {
        default: break;
        case UFA_SHADER: {
            UFRIDelShader(asset.shader.shader);
        } break;
        case UFA_TEXTURE: {
            void* data = NULL;
            UFRIGetData(asset.texture.texture, &data);
            UFRIDelShader(asset.texture.texture);
            if (data) stbi_image_free(data);
        } break;
    } if (asset.data) r3FreeMemory(asset.data);

    return UF_OK;
}
