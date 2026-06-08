#include "pack.c"
#include "asset.c"


none driverInit(void) {
    UFAInternal.packs.map = r3NewHashArray(1024, sizeof(UFAPack));
    UFAInternal.assets.map = r3NewHashArray(1024, sizeof(UFAsset));
    if (!UFAInternal.packs.map || !UFAInternal.assets.map) {
        if (UFAInternal.packs.map) r3DelHashArray(UFAInternal.packs.map);
        if (UFAInternal.assets.map) r3DelHashArray(UFAInternal.assets.map);
        r3LogStdOut(R3_LOG_ERROR, "Failed to allocate hash array for UFA registry\n");
        return;
    }
}

static void _delPacks(ptr value) {
    UFAPack* pack = (UFAPack*)value;
    if (pack->header.table) {
        r3FreeMemory(pack->header.table);
        pack->header.table = NULL;
    } if (pack->map) {
        r3DelHashArray(pack->map);
        pack->map = NULL;
    } if (pack->data) {
        r3FreeMemory(pack->data);
        pack->data = NULL;
    }
}

static void _delAssets(ptr value) {
    UFAsset* asset = (UFAsset*)value;
    if (asset->data) {
        r3FreeMemory(asset->data);
        asset->data = NULL;
    }

    switch(asset->header.type) {
        default: break;
        case (UFA_MESH): {
            UFRIDelVertexBuffer(asset->mesh.vertex.vbo);
            UFRIDelIndexBuffer(asset->mesh.index.ibo);
        } break;
        case (UFA_SHADER): {
            UFRIDelShader(asset->shader.shader);
        } break;
        case (UFA_TEXTURE): {
            UFRIDelTexture(asset->texture.texture);
        } break;
    }
}

none driverExit(void) {
    r3IterHashArray(_delAssets, UFAInternal.assets.map);
    r3IterHashArray(_delPacks, UFAInternal.packs.map);
    r3DelHashArray(UFAInternal.assets.map);
    r3DelHashArray(UFAInternal.packs.map);
}

static UFA API = {
    ._loadTexture = _lt,

    .newPack = UFANewPack,
    .delPack = UFADelPack,
    .packAsset = UFAPackAsset,

    .loadMesh = UFALoadMesh,
    .loadShader = UFALoadShader,
    .loadTexture = UFALoadTexture,
    .unloadAsset = UFAUnloadAsset,
};

UFDriver UFENTRY(UFInterface uniform) {
    UFAInternal.core = uniform;
    return (UFDriver) {
        .interface = &API,
        .init = driverInit,
        .exit = driverExit,
        .info = {
            .ver = "1.0.0",
            .tag = "Uniform Assets",
            .flags = 0,
            .caps = 0
        },
    };
}
