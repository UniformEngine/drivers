#pragma once

#include <include/UFA/UFA.h>

#define UFA_VERSION 1
#define UFA_PACK_MAX (1 << 12)
#define UFA_ASSET_MAX (1 << 16)
#define UFA_PACK_DATA_MAX 64 * MiB
#define UFA_SIGNATURE (0x55 << 24)|(0x46 << 16)|(0x41 << 8)|0   // U|F|A|0

typedef enum UFAssetType {
    UFA_MESH,
    UFA_SHADER,
    UFA_TEXTURE,
    UFA_ASSETS
} UFAssetType;

typedef struct UFAsset {
    struct {
        u32 use;
        u32 type;
        u64 tagLen;
        u64 dataLen;
    } header;
    void* data;

    union {
        UFAMesh mesh;
        UFAShader shader;
        UFATexture texture;
    };
} UFAsset;

typedef struct UFAPack {
    struct {
        u32 sig;
        u32 ver;
        u32 count;
        u64* table;
    } header;
    void* data;
    void* map;

    u64 capacity;
    u64 used;

    char* path;
    char* tag;
} UFAPack;

typedef struct UFARegistry {
    u32 count;
    void* map;
} UFARegistry;

typedef struct UFAPRegistry {
    u32 count;
    void* map;
} UFAPRegistry;

static struct {
    UFInterface core;
    UFARegistry assets;
    UFAPRegistry packs;
} UFAInternal = {0};

static inline u32 _fnv1a(const char* k) {
	if (!k) return I32_BIT;
	u32 h = 2166136261u; 	// FNV-1a offset basis
	while (*k) {
		h ^= (char)*k++;	// XOR with current character
		h *= 16777619;		// multiply by FNV-1a prime
	}; return h;
}
