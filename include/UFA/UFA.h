#pragma once

#define __UF_API__
#include <include/uniform/uniform.h>

/**
 * UFA is an asset management driver over UFRI.
 * UFA supports various asset types, and various formats for those assets.
 *
 * UFA assets can be loaded from and stored as .UFA files. (.obj/.gltf/.png/.jpg/.mp3 converted to .UFA on load)
 * .UFA is a file that stores asset data in a compact text-based format, able to be easily edited by hand
 * and then baked into a binary format for faster loading/smaller file size (.UFB).
 *
 * UFA Format
 * ----------
 * [HEAD]
 * // version
 * // num assets
 *
 * [ASSET]
 * // asset desc
 * [DATA]
 * // asset data
 */

typedef struct UFA {
    void* (*loadTexture)(i32 width, i32 height, char* path);

    UFResult (*load)(char* target);                     // load a .UFA bundle
    UFResult (*compile)(char* target, char* dest);      // compile a .UFA bundle to .UFB format
    UFResult (*decompile)(char* target, char* dest);    // decompile a .UFB bundle to .UFA format

    UFHandle (*loadAsset)(char* tag);                   // parse an asset from the current bundle or in-memory array
    UFResult (*unoadAsset)(UFHandle asset);             // unload an asset from in-memory array
    UFResult (*reloadAsset)(UFHandle asset);            // re-parse an asset from the current bundle and store in-memory
} UFA;
