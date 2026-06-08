#include "_UFA.h"

UFResult UFANewPack(char* tag) {
    if (!tag) return UF_ERROR;

    if (r3ArrayCount(UFAInternal.packs.map) + 1 >= r3ArraySlots(UFAInternal.packs.map)) {
        u32 growth = (UFA_PACK_MAX - r3ArraySlots(UFAInternal.packs.map)) / 4;
        void* nmap = r3ResizeHashArray(growth + r3ArraySlots(UFAInternal.packs.map), UFAInternal.packs.map);
        if (!nmap) return UF_ERROR;
        UFAInternal.packs.map = nmap;
    }

    UFAPack pack = {
        .used = 0,
        .capacity = 2 * MiB,

        .tag = tag,
        .path = ".UF\\assets",

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

    u64 asize = (sizeof(a->header) + a->header.dataLen);
    if (asize > UFA_PACK_DATA_MAX) return UF_ERROR;

    while (p->used + asize > p->capacity) {
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
    p->header.table[id] = p->used;

    void* addr = (u8*)p->data + p->used;
    p->used += asize;

    if (!r3SetHashArray(asset, &id, p->map)
    ||  !r3WriteMemory(sizeof(a->header), &a->header, addr)
    ||  !r3WriteMemory(a->header.tagLen, asset, (u8*)addr + sizeof(a->header))
    ||  !r3WriteMemory(a->header.dataLen, a->data, (u8*)addr + sizeof(a->header) + a->header.tagLen)) {
        return UF_ERROR;
    }

    return UF_OK;
}
