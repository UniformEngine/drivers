#include "_UFA.h"
#include <stdio.h>
#include <string.h>

UFResult UFANewPack(char* tag) {
    if (!tag) return UF_ERROR;

    if (r3ArrayCount(UFAInternal.packs.map) + 1 >= r3ArraySlots(UFAInternal.packs.map)) {
        u32 growth = (UFA_PACK_MAX - r3ArraySlots(UFAInternal.packs.map)) / 4;
        void* nmap = r3ResizeHashArray(growth + r3ArraySlots(UFAInternal.packs.map), UFAInternal.packs.map);
        if (!nmap) return UF_ERROR;
        UFAInternal.packs.map = nmap;
    }

    UFAPack pack = {
        .tag = tag,
        .capacity = 2 * MiB,

        .header.size = 0,
        .header.count = 0,
        .header.ver = UFA_VERSION,
        .header.sig = UFA_SIGNATURE,
    };

    void* map = r3NewHashArray(1024, sizeof(u32));
    u64* table = r3NewArray(1024, sizeof(u64));
    u64* data = r3AllocMemory(pack.capacity);

    if (!table || !data || !map) {
        if (table) r3DelArray(table);
        if (data) r3FreeMemory(data);
        if (map) r3DelHashArray(map);
        return UF_ERROR;
    }

    pack.map = map;
    pack.data = data;
    pack.header.table = table;
    if (!r3SetHashArray(tag, &pack, UFAInternal.packs.map)) {
        if (table) r3DelArray(table);
        if (data) r3FreeMemory(data);
        if (map) r3DelHashArray(map);
        return UF_ERROR;
    }

    UFAInternal.packs.count++;
    return UF_OK;
}

UFResult UFADelPack(char* tag) {
    if (!tag) return UF_ERROR;

    UFAPack pack = {0};
    if (!r3RemHashArray(tag, UFAInternal.packs.map, &pack)) {
        return UF_ERROR;
    }

    if (pack.header.table) r3DelArray(pack.header.table);
    if (pack.data) r3FreeMemory(pack.data);
    if (pack.map) r3DelHashArray(pack.map);

    pack.header.table = NULL;
    pack.data = NULL;
    pack.map = NULL;

    UFAInternal.packs.count--;
    return UF_OK;
}

UFResult UFAPackAsset(char* asset, char* pack) {
    if (!asset || !pack) return UF_ERROR;
    UFAsset* a = r3GetHashArrayPtr(asset, UFAInternal.assets.map);
    UFAPack* p = r3GetHashArrayPtr(pack, UFAInternal.packs.map);

    if (!a || !p) return UF_ERROR;
    if (r3GetHashArrayPtr(asset, p->map)) return UF_ERROR;
    if (p->header.count == UFA_ASSET_MAX) return UF_ERROR;

    u64 asize = sizeof(a->header) + a->header.dataLen + a->header.tagLen;
    if (asize > UFA_PACK_DATA_MAX) return UF_ERROR;

    while (p->header.size + asize > p->capacity) {
        u64 growth = (UFA_PACK_DATA_MAX - p->capacity) / 4;
        if (!growth) return UF_ERROR;

        void* ndata = r3ReallocMemory(growth + p->capacity, p->data);
        if (!ndata) return UF_ERROR;

        p->capacity += growth;
        p->data = ndata;
    }

    if (p->header.count >= r3ArraySlots(p->header.table)) {
        u32 growth = (UFA_ASSET_MAX - r3ArraySlots(p->header.table)) / 4;
        if (!growth) return UF_ERROR;

        void* ntable = r3ResizeArray(growth + r3ArraySlots(p->header.table), r3ArrayStride(p->header.table), p->header.table);
        void* nmap = r3ResizeHashArray(growth + r3ArraySlots(p->map), p->map);
        if (!ntable || !nmap) return UF_ERROR;
        p->header.table = ntable;
        p->map = nmap;
    }

    u32 id = p->header.count++;
    p->header.table[id] = p->header.size;

    void* addr = (u8*)p->data + p->header.size;
    p->header.size += asize;

    if (!r3SetHashArray(asset, &id, p->map)
    ||  !r3WriteMemory(sizeof(a->header), &a->header, addr)
    ||  !r3WriteMemory(a->header.tagLen, asset, (u8*)addr + sizeof(a->header))
    ||  !r3WriteMemory(a->header.dataLen, a->data, (u8*)addr + sizeof(a->header) + a->header.tagLen)) {
        return UF_ERROR;
    }

    return UF_OK;
}

UFResult UFABakePack(char* path, char* pack) {
    if (!path || !pack) return UF_ERROR;

    UFAPack* p = r3GetHashArrayPtr(pack, UFAInternal.packs.map);
    if (!p) return UF_ERROR;

    u64 fsize = p->header.size + sizeof(u64) * p->header.count + sizeof(u32) * 4;
    char* file = r3NewFile(fsize);
    if (!file) return UF_ERROR;

    r3WriteFile(sizeof(u32) * 4, &p->header, file);
    r3SeekFile(sizeof(u32) * 4, file);

    r3WriteFile(sizeof(u64) * p->header.count, p->header.table, file);
    r3SeekFile(sizeof(u64) * p->header.count, file);

    r3WriteFile(p->header.size, p->data, file);
    r3SeekFile(p->header.size, file);

    if (!r3SaveFile(fsize, path, file)) return UF_ERROR;
    return UF_OK;
}


void _UFAParseShader(UFAsset* asset) {
    if (!asset || !asset->data) return;
    if (asset->header.type != UFA_SHADER) return;

    u64 vsize = 0;
    u64 fsize = 0;
    r3ReadMemory(sizeof(u64), asset->data, &vsize);
    r3ReadMemory(sizeof(u64), (u8*)asset->data + vsize + sizeof(u64), &fsize);

    char* vs = r3AllocMemory(vsize + 1);
    char* fs = r3AllocMemory(fsize + 1);
    if (!vs || !fs) return;
    vs[vsize] = '\0';
    fs[fsize] = '\0';

    r3ReadMemory(vsize, (u8*)asset->data + sizeof(u64), vs);
    r3ReadMemory(fsize, (u8*)asset->data + vsize + sizeof(u64) * 2, fs);

    asset->shader.shader = UFRINewShader((UFRIShaderDesc){
        .vertex = vs,
        .fragment = fs,
        .use = asset->header.use,
    });

    r3FreeMemory(vs);
    r3FreeMemory(fs);
}

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

UFResult UFALoadPack(char* tag, char* path) {
    if (!tag || !path) return UF_ERROR;

    char* file = r3LoadFile(path);
    if (!file) return UF_ERROR;

    UFAPack p = {0};

    r3ReadFile(sizeof(u32) * 4, file, &p.header);
    r3SeekFile(sizeof(u32) * 4, file);
    if (p.header.sig != UFA_SIGNATURE
    ||  p.header.ver != UFA_VERSION) {
        r3DelFile(file);
        return UF_ERROR;
    }

    void* map = r3NewHashArray(p.header.count, sizeof(u32));
    u64* table = r3NewArray(p.header.count, sizeof(u64));
    if (!map || !table) {
        if (map) r3FreeMemory(map);
        if (table) r3DelArray(table);
        r3DelFile(file);
        return UF_ERROR;
    }

    r3ReadFile(sizeof(u64) * p.header.count, file, table);
    r3SeekFile(sizeof(u64) * p.header.count, file);

    void* data = r3AllocMemory(p.header.size);
    if (!data) {
        if (map) r3FreeMemory(map);
        if (table) r3DelArray(table);
        r3DelFile(file);
        return UF_ERROR;
    }

    r3ReadFile(p.header.size, file, data);
    r3DelFile(file);

    p.capacity = p.header.size;
    p.header.table = table;
    p.data = data;
    p.map = map;
    p.tag = tag;

    if (!r3SetHashArray(tag, &p, UFAInternal.packs.map)) {
        if (map) r3FreeMemory(map);
        if (data) r3FreeMemory(data);
        if (table) r3DelArray(table);
        return UF_ERROR;
    }

    char temp[1024] = {0};
    temp[1023] = '\0';
    FOR_I(0, p.header.count, 1) {
        UFAsset a = {0};
        u64 offset = table[i];
        r3ReadMemory(sizeof(a.header), (u8*)p.data + offset, &a.header);

        a.data = r3AllocMemory(a.header.dataLen);
        if (!a.data) {
            if (map) r3FreeMemory(map);
            if (data) r3FreeMemory(data);
            if (table) r3DelArray(table);
            return UF_ERROR;
        }
        r3ReadMemory(a.header.tagLen, (u8*)p.data + offset + sizeof(a.header), temp);
        r3ReadMemory(a.header.dataLen, (u8*)p.data + offset + sizeof(a.header) + a.header.tagLen, a.data);

        switch(a.header.type) {
            default: break;
            case UFA_SHADER: {
                _UFAParseShader(&a);
            } break;
            case UFA_TEXTURE: {
                _UFAParseTexture(&a);
            } break;
        }

        if (!r3SetHashArray(temp, &a, UFAInternal.assets.map)
        ||  !r3SetHashArray(temp, &i, p.map)) {
            if (map) r3FreeMemory(map);
            if (data) r3FreeMemory(data);
            if (table) r3DelArray(table);
            if (a.data) r3FreeMemory(a.data);
            return UF_ERROR;
        }

        r3SetMemory(sizeof(temp), 0, temp);
        UFAInternal.assets.count++;
    } UFAInternal.packs.count++;
    return UF_OK;
}
