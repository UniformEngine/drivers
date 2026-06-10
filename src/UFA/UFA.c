#include "pack.c"

#include "asset/mesh/loader.c"
#include "asset/shader/loader.c"
#include "asset/texture/loader.c"


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
            if (data) free(data);
        } break;
    } if (asset.data) r3FreeMemory(asset.data);

    UFAInternal.assets.count--;
    return UF_OK;
}


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
    if (!pack) return;

    if (pack->header.table) {
        r3DelArray(pack->header.table);
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
    if (!asset) return;

    if (asset->data) {
        r3FreeMemory(asset->data);
        asset->data = NULL;
    }

    switch(asset->header.type) {
        default: break;
        case (UFA_MESH): {
            UFRIVertexBufferDesc vdesc;
            UFRIGetDesc(asset->mesh.vbo, &vdesc);
            r3DelArray(vdesc.vertices);

            UFRIIndexBufferDesc idesc;
            UFRIGetDesc(asset->mesh.ibo, &idesc);
            r3DelArray(idesc.indices);

            UFRIDelVertexBuffer(asset->mesh.vbo);
            UFRIDelIndexBuffer(asset->mesh.ibo);
        } break;
        case (UFA_SHADER): {
            UFRIDelShader(asset->shader.shader);
        } break;
        case (UFA_TEXTURE): {
            UFRITextureDesc desc = {0};
            UFRIGetDesc(asset->texture.texture, &desc);
            UFRIDelTexture(asset->texture.texture);
            free(desc.data);
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
    .newPack = UFANewPack,
    .delPack = UFADelPack,
    .loadPack = UFALoadPack,
    .bakePack = UFABakePack,
    .packAsset = UFAPackAsset,

    .getMesh = UFAGetMesh,
    .loadMesh = UFALoadMesh,

    .getShader = UFAGetShader,
    .loadShader = UFALoadShader,

    .getTexture = UFAGetTexture,
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
