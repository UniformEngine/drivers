#pragma once

#define __UF_API__
#include <include/uniform/uniform.h>

typedef struct UFAMeshDesc {
    char* tag;
    char* path;
} UFAMeshDesc;

typedef struct UFAMesh {
    struct {
        u32 count;
        f32* data;
        UFRIResource vbo;
    } vertex;
    struct {
        u32 count;
        f32* data;
    } normal;
    struct {
        u32 count;
        f32* data;
        UFRIResource ibo;
    } index;
    struct {
        u32 count;
        f32* data;
    } uv;
} UFAMesh;

typedef struct UFATextureDesc {
    char* tag;
    char* path;
} UFATextureDesc;

typedef struct UFATexture {
    u32 use;
    u32 mem;
    u32 width;
    u32 height;
    u32 format;
    char* data;
    UFRIResource texture;
} UFATexture;

// TODO: convert this to a tagged union to support UFRI shader types
typedef struct UFAShaderDesc {
    char* tag;
    char* vertex;
    char* fragment;
} UFAShaderDesc;

typedef struct UFAShader {
    char* tag;
    char* vertex;
    char* fragment;
    UFRIResource shader;
} UFAShader;

typedef struct UFA {
    void* (*_loadTexture)(i32 width, i32 height, char* path);

    UFResult (*newPack)(char* tag);
    UFResult (*delPack)(char* pack);
    UFResult (*loadPack)(char* path);
    UFResult (*packAsset)(char* asset, char* pack);

    UFResult (*getMesh)(char* tag);
    UFResult (*getShader)(char* tag);
    UFResult (*getTexture)(char* tag);
    UFResult (*unloadAsset)(char* tag);
    UFResult (*loadMesh)(UFAMeshDesc desc);
    UFResult (*loadShader)(UFAShaderDesc desc);
    UFResult (*loadTexture)(UFATextureDesc desc);

} UFA;
