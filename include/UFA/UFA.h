#pragma once

#define __UF_API__
#include <include/uniform/uniform.h>

typedef struct UFAMeshDesc {
    char* tag;
    char* path;
} UFAMeshDesc;

typedef struct UFAMesh {
    UFRIResource vbo;
    UFRIResource ibo;
    u32 nvertices;
} UFAMesh;

typedef struct UFATextureDesc {
    char* tag;
    char* path;
    u32 format;
    u32 memuse;
    u32 use;
} UFATextureDesc;

typedef struct UFATexture {
    UFRIResource texture;
    u32 height;
    u32 width;
} UFATexture;

typedef struct UFAShaderDesc {
    char* fragment;
    char* vertex;
    char* tag;
    u32 type;
} UFAShaderDesc;

typedef struct UFAShader {
    UFRIResource shader;
} UFAShader;

typedef struct UFA {
    UFResult (*newPack)(char* tag);
    UFResult (*delPack)(char* pack);
    UFResult (*loadPack)(char* path);
    UFResult (*packAsset)(char* asset, char* pack);

    UFResult (*loadTexture)(UFATextureDesc desc);
    UFResult (*loadShader)(UFAShaderDesc desc);
    UFResult (*loadMesh)(UFAMeshDesc desc);
    UFResult (*unloadAsset)(char* tag);

    UFATexture (*getTexture)(char* tag);
    UFAShader (*getShader)(char* tag);
    UFAMesh (*getMesh)(char* tag);
} UFA;
