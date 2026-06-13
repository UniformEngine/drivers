#include <string.h>
#include <stdlib.h>
#include "../../_UFA.h"


typedef struct UFAMeshVertexKey {
    u32 vi;
    u32 ti;
    u32 ni;
} UFAMeshVertexKey;

// This structure must match the `UFRIVertexAttrib` enum
typedef struct UFAMeshVertex {
    f32 pos[3];
    f32 nrm[3];
    f32 uv[2];
} UFAMeshVertex;

typedef struct UFAMeshEntry {
    UFAMeshVertexKey key;
    u32 index;
    u8 used;
} UFAMeshEntry;

typedef struct UFAMeshData {
    u32 stride;
    u32 attribs;
    u32 nindices;
    u32 nvertices;
    u32* indices;
    f32* vertices;
} UFAMeshData;


static inline u8 _startswith(char* str, char* prefix) {
    if (!str || !prefix) return 0;
    while (*prefix) {
        if (*str++ != *prefix++) return 0;
    } return 1;
}

static inline u8 _strinstr(char* str, char* sub) {
    if (!str || !sub) return 0;

    u8 m = 1;
    u32 l = strlen(sub);
    FOR_I(0, strlen(str), 1) {
        FOR_J(0, l, 1) {
            if (str[i + j] != sub[j]) {
                m = 0;
                break;
            }
        } if (m) {
            return 1;
        }
    } return 0;
}

static inline u32 _hashedMeshKey(UFAMeshVertexKey k) {
    return k.vi * 73856093u ^ k.ti * 19349663u ^ k.ni * 83492791u;
}

static inline u32 _hashedMeshSlot(UFAMeshVertexKey k, u32 mask) {
    return _hashedMeshKey(k) & mask;
}

static inline u32 _hashedMeshLookup(u32 mask, UFAMeshVertexKey key, UFAMeshEntry* map) {
    u32 h = _hashedMeshSlot(key, mask);
    while (map[h].used) {
        if (memcmp(&map[h].key, &key, sizeof(key)) == 0) {
            return h; // matched slot
        } h = (h + 1) & mask; // a fast modulo since map size is 2^n (x % 2^n == x & (2^n - 1))
    } return h; // empty slot
}


UFResult _parseOBJFace(char* cursor, void* keys) {
    while (*cursor) {
        while (*cursor == ' ') cursor++;
        if (*cursor == '\n' || *cursor == '\r' || *cursor == '\0') break;
        UFAMeshVertexKey key = {
            .vi = strtoul(cursor, &cursor, 10) - 1,
            .ti = 0xFFFFFFFF,
            .ni = 0xFFFFFFFF,
        };

        if (*cursor == '/') {
            cursor++;
            if (*cursor != '/') {
                key.ti = strtoul(cursor, &cursor, 10) - 1;
            } if (*cursor == '/') {
                cursor++;
                key.ni = strtoul(cursor, &cursor, 10) - 1;
            }
        } r3PushArray(&key, keys);
    } return UF_OK;
}

void _triangulateOBJFace(
    UFAMeshEntry* map,
    UFAMeshVertexKey* keys,
    f32* uvs,
    f32* normals,
    u32* indices,
    UFAMeshVertex* vertices,
    f32* positions
) {
    for(u32 i = 1; i + 1 < r3ArrayCount(keys); i++) {
        // triangle fan
        // a quad (0, 1, 2, 3) becomes two triangles (0, 1, 2) and (0, 2, 3)
        UFAMeshVertexKey verts[3] = {
            keys[0],
            keys[i],
            keys[i + 1],
        };

        for(u32 j = 0; j < 3; j++) {
            UFAMeshVertexKey key = verts[j];

            u32 h = _hashedMeshLookup(r3ArraySlots(map) - 1, key, map);
            u32 idx = 0;

            if (map[h].used) {
                idx = map[h].index;
            } else {
                UFAMeshVertex vert = {0};
                idx = r3ArrayCount(vertices);

                vert.pos[0] = positions[key.vi * 3 + 0];
                vert.pos[1] = positions[key.vi * 3 + 1];
                vert.pos[2] = positions[key.vi * 3 + 2];

                if (key.ti != 0xFFFFFFFF) {
                    vert.uv[0] = uvs[key.ti * 2 + 0];
                    vert.uv[1] = uvs[key.ti * 2 + 1];
                } if (key.ni != 0xFFFFFFFF) {
                    vert.nrm[0] = normals[key.ni * 3 + 0];
                    vert.nrm[1] = normals[key.ni * 3 + 1];
                    vert.nrm[2] = normals[key.ni * 3 + 2];
                }

                r3PushArray(&vert, vertices);

                map[h] = (UFAMeshEntry){
                    .used = 1,
                    .key = key,
                    .index = idx,
                };
            } r3PushArray(&idx, indices);
        }
    } r3ClearArray(keys);
    return;
}

UFAMeshData UFAImportOBJ(char* path) {
    if (!path) return (UFAMeshData){0};

    char* file = r3LoadFile(path);
    if (!file) return (UFAMeshData){0};

    f32* positions = r3NewArray(16384, sizeof(f32));
    f32* uvs       = r3NewArray(16384, sizeof(f32));
    f32* normals   = r3NewArray(16384, sizeof(f32));
    if (!positions || !uvs || !normals) {
        if (positions) r3DelArray(positions);
        if (normals) r3DelArray(normals);
        if (uvs) r3DelArray(uvs);
        r3DelFile(file);
        return (UFAMeshData){0};
    }

    UFAMeshVertexKey* keys = r3NewArray(16384, sizeof(UFAMeshVertexKey));
    UFAMeshEntry* map = r3NewArray(16384, sizeof(UFAMeshEntry));

    u32* indices = r3NewArray(16384, sizeof(u32));
    UFAMeshVertex* vertices = r3NewArray(16384, sizeof(UFAMeshVertex));
    if (!vertices || !indices || !map || !keys) goto fail;

    char* cursor = file;
    while (*cursor) {
        if (_startswith(cursor, "v ")) {
            cursor += 2;
            FOR_I(0, 3, 1) {
                if (r3ArrayFull(positions)) {
                    positions = r3ResizeArray(r3ArraySlots(positions)*1.5, sizeof(f32), positions);
                } r3PushArray(&(f32){strtof(cursor, &cursor)}, positions);
            }
        } else if (_startswith(cursor, "vt ")) {
            cursor += 3;
            FOR_I(0, 2, 1) {
                if (r3ArrayFull(uvs)) {
                    uvs = r3ResizeArray(r3ArraySlots(uvs)*1.5, sizeof(f32), uvs);
                } r3PushArray(&(f32){strtof(cursor, &cursor)}, uvs);
            }
        } else if (_startswith(cursor, "vn ")) {
            cursor += 3;
            FOR_I(0, 3, 1) {
                if (r3ArrayFull(normals)) {
                    normals = r3ResizeArray(r3ArraySlots(normals)*1.5, sizeof(f32), normals);
                } r3PushArray(&(f32){strtof(cursor, &cursor)}, normals);
            }
        } else if (_startswith(cursor, "f ")) {
            cursor += 2;
            if (_parseOBJFace(cursor, keys) != UF_OK) {
                goto fail;
            } _triangulateOBJFace(map, keys, uvs, normals, indices, vertices, positions);
        }
        while (*cursor && *cursor != '\n') cursor++;
        if (*cursor == '\n') cursor++;
    }

    r3DelArray(positions);
    r3DelArray(normals);
    r3DelArray(keys);
    r3DelArray(uvs);
    r3DelArray(map);
    r3DelFile(file);

    return (UFAMeshData) {
        .attribs = UFRI_ATTRIB_POSITION | UFRI_ATTRIB_UV | UFRI_ATTRIB_NORMAL,
        .nvertices = r3ArrayCount(vertices),
        .nindices = r3ArrayCount(indices),
        .stride = sizeof(UFAMeshVertex),
        .vertices = (f32*)vertices,
        .indices = indices,
    };

    fail:
        r3DelArray(positions);
        r3DelArray(normals);
        r3DelArray(keys);
        r3DelArray(map);
        r3DelArray(uvs);
        r3DelFile(file);
        return (UFAMeshData){0};
}
