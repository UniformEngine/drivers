#include <string.h>
#include <stdlib.h>
#include "../../_UFA.h"


typedef struct UFAMeshKey {
    u32 vi;
    u32 ti;
    u32 ni;
} UFAMeshKey;

// This structure must match the `UFRIVertexAttrib` enum
typedef struct UFAMeshVertex {
    f32 pos[3];
    f32 nrm[3];
    f32 uv[2];
} UFAMeshVertex;

typedef struct UFAMeshEntry {
    UFAMeshKey key;
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

static inline u32 _hashedMeshKey(UFAMeshKey k) {
    return k.vi * 73856093u ^ k.ti * 19349663u ^ k.ni * 83492791u;
}

static inline u32 _hashedMeshSlot(UFAMeshKey k, u32 mask) {
    return _hashedMeshKey(k) & mask;
}

static inline u32 _hashedMeshLookup(u32 mask, UFAMeshKey key, UFAMeshEntry* map) {
    u32 h = _hashedMeshSlot(key, mask);
    while (map[h].used) {
        if (memcmp(&map[h].key, &key, sizeof(key)) == 0) {
            return h; // matched slot
        } h = (h + 1) & mask; // a fast modulo since map size is 2^n (x % 2^n == x & (2^n - 1))
    } return h; // empty slot
}


// TODO: implement 3-phase pipeline
//  parse -> triangulation (fan for now ear clip later) -> packing
UFAMeshData UFAImportOBJ(char* path) {
    if (!path) return (UFAMeshData){0};

    char* file = r3LoadFile(path);
    if (!file) return (UFAMeshData){0};

    f32* positions = r3NewArray(1024, sizeof(f32));
    f32* uvs       = r3NewArray(1024, sizeof(f32));
    f32* normals   = r3NewArray(1024, sizeof(f32));
    if (!positions || !uvs || !normals) {
        if (positions) r3DelArray(positions);
        if (normals) r3DelArray(normals);
        if (uvs) r3DelArray(uvs);
        r3DelFile(file);
        return (UFAMeshData){0};
    }

    UFAMeshEntry* map = r3NewArray(2048, sizeof(UFAMeshEntry));

    u32* indices = r3NewArray(1024, sizeof(u32));
    UFAMeshVertex* vertices = r3NewArray(1024, sizeof(UFAMeshVertex));
    if (!vertices || !indices || !map) goto fail;

    u32 nindices = 0;
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
            u32 vertex = 0;
            while (*cursor) {
                while (*cursor == ' ') cursor++;
                if (*cursor == '\n' || *cursor == '\r' || *cursor == '\0') break;
                if (vertex >= 3) goto fail;

                UFAMeshKey key = {
                    .vi = strtoul(cursor, &cursor, 10) - 1,
                    .ti = 0xFFFFFFFF,
                    .ni = 0xFFFFFFFF,
                };

                // supports 4 face formats v | v/vt | v//vn | v/vt/vn
                // missing attribs are defaulted to 0xFFFFFFFF in during parsing and 0 during packing
                if (*cursor == '/') {
                    cursor++;
                    if (*cursor != '/') {
                        key.ti = strtoul(cursor, &cursor, 10) - 1;
                    } if (*cursor == '/') {
                        cursor++;
                        key.ni = strtoul(cursor, &cursor, 10) - 1;
                    }
                }

                // lookup or insert unique per-face vertices
                u32 idx = 0;
                u32 h = _hashedMeshLookup(r3ArraySlots(map) - 1, key, map);
                if (map[h].used) {
                    idx = map[h].index;
                } else {
                    UFAMeshVertex v = {0};
                    idx = r3ArrayCount(vertices);

                    v.pos[0] = positions[key.vi * 3];
                    v.pos[1] = positions[key.vi * 3 + 1];
                    v.pos[2] = positions[key.vi * 3 + 2];

                    if (key.ti != 0xFFFFFFFF) {
                        v.uv[0] = uvs[key.ti * 2];
                        v.uv[1] = uvs[key.ti * 2 + 1];
                    } if (key.ni != 0xFFFFFFFF) {
                        v.nrm[0] = normals[key.ni * 3];
                        v.nrm[1] = normals[key.ni * 3 + 1];
                        v.nrm[2] = normals[key.ni * 3 + 2];
                    }

                    if (r3ArrayFull(vertices)) {
                        vertices = r3ResizeArray(r3ArraySlots(vertices)*1.5, sizeof(UFAMeshVertex), vertices);
                    } r3PushArray(&v, vertices);

                    if (r3ArrayFull(map)) {
                        map = r3ResizeArray(r3ArraySlots(map)*2, sizeof(UFAMeshEntry), map);
                    } r3PushArray(&(UFAMeshEntry){
                        .used = 1,
                        .key = key,
                        .index = idx,
                    }, map);
                }

                if (r3ArrayFull(indices)) {
                    indices = r3ResizeArray(r3ArraySlots(indices)*1.5, sizeof(u32), indices);
                } r3PushArray(&idx, indices);

                nindices++;
                vertex++;
            }
        }

        while (*cursor && *cursor != '\n') cursor++;
        if (*cursor == '\n') cursor++;
    }

    r3DelArray(positions);
    r3DelArray(normals);
    r3DelArray(uvs);
    r3DelArray(map);
    r3DelFile(file);

    return (UFAMeshData) {
        .attribs = UFRI_ATTRIB_POSITION | UFRI_ATTRIB_UV | UFRI_ATTRIB_NORMAL,
        .nvertices = r3ArrayCount(vertices),
        .stride = sizeof(UFAMeshVertex),
        .vertices = (f32*)vertices,
        .nindices = nindices,
        .indices = indices,
    };

    fail:
        r3DelArray(positions);
        r3DelArray(normals);
        r3DelArray(uvs);
        r3DelFile(file);
        return (UFAMeshData){0};
}
