#include "../../_UFA.h"


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
