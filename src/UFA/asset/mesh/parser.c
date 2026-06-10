#include "../../_UFA.h"


void _UFAParseMesh(UFAsset* asset) {
    if (!asset || !asset->data) return;
    if (asset->header.type != UFA_MESH) return;

    u8* cursor = (u8*)asset->data;
    u32 stride, attribs, nindices, nvertices;

    r3ReadMemory(sizeof(u32), cursor, &stride); cursor += sizeof(u32);
    r3ReadMemory(sizeof(u32), cursor, &attribs); cursor += sizeof(u32);
    r3ReadMemory(sizeof(u32), cursor, &nindices); cursor += sizeof(u32);
    r3ReadMemory(sizeof(u32), cursor, &nvertices); cursor += sizeof(u32);

    u32* indices = NULL;
    f32* vertices = NULL;

    if (nindices) {
        indices = r3NewArray(nindices, sizeof(u32));
        if (!indices) return;
        FOR_I(0, nindices, 1) {
            r3PushArray(cursor, indices);
            cursor += sizeof(u32);
        }
    }

    vertices = r3NewArray(nvertices * stride / sizeof(f32), sizeof(f32));
    if (!vertices) {
        r3DelArray(indices);
        return;
    }
    FOR_I(0, nvertices * stride / sizeof(f32), 1) {
        r3PushArray(cursor, vertices);
        cursor += sizeof(f32);
    }

    UFRIResource vbo = UFRINewVertexBuffer((UFRIVertexBufferDesc){
        .vertices = (f32*)vertices,
        .attribs = attribs,
        .count = nvertices,
        .memuse = UFRI_STATIC
    });

    if (vbo.handle == UF_INVALID) {
        r3DelArray(vertices);
        r3DelArray(indices);
        return;
    }

    UFRIResource ibo = {0};
    if (indices) {
        ibo = UFRINewIndexBuffer((UFRIIndexBufferDesc){
            .indices = indices,
            .count = nindices,
            .memuse = UFRI_STATIC
        });

        if (ibo.handle == UF_INVALID) {
            UFRIDelVertexBuffer(vbo);
            r3DelArray(vertices);
            r3DelArray(indices);
            return;
        }
    }

    asset->mesh.vbo = vbo;
    asset->mesh.ibo = ibo;
    asset->mesh.attribs = attribs;
    asset->mesh.indices = nindices;
    asset->mesh.vertices = nvertices;
}
