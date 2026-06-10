#include "../../_UFA.h"


void _UFAParseTexture(UFAsset* asset) {
    if (!asset || !asset->data) return;
    if (asset->header.type != UFA_TEXTURE) return;

    u32 use, memuse, format;
    r3ReadMemory(sizeof(u32), (u8*)asset->data, &use);
    r3ReadMemory(sizeof(u32), (u8*)asset->data + sizeof(u32), &memuse);
    r3ReadMemory(sizeof(u32), (u8*)asset->data + sizeof(u32) * 2, &format);

    u64 textureSize = asset->header.dataLen - sizeof(u32) * 3;
    void* data = r3AllocMemory(textureSize);
    if (!data) return;

    r3ReadMemory(textureSize, (u8*)asset->data + sizeof(u32) * 3, data);

    i32 channels = 0;
    i32 width, height;
    stbi_set_flip_vertically_on_load(1);
    void* textureData = stbi_load_from_memory(data, textureSize, &width, &height, &channels, 0);
    if (!textureData) {
        r3FreeMemory(data);
        return;
    }

    asset->texture.texture = UFRINewTexture((UFRITextureDesc){
        .use = use,
        .width = width,
        .height = height,
        .memuse = memuse,
        .format = format,
        .data = textureData
    });

    if (!asset->texture.texture.handle) {
        free(textureData);
    } r3FreeMemory(data);
}
