#include <include/UFA/UFA.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>


typedef enum UFAssetType {
    UFA_MESH,
    UFA_SHADER,
    UFA_TEXTURE,
    UFA_ASSETS
} UFAssetType;

typedef struct UFAEntry {
    char tag[256];
    UFHandle handle;
    UFAssetType type;
    UFRIMemoryUseage memuse;
} UFAEntry;

static struct UFAInternal {
    UFInterface core;
    void* assets;
} UFAInternal = {0};


i32 _startsWith(char* src, const char* prefix) {
    return strncmp(src, prefix, strlen(prefix)) == 0;
}

i32 _endsWith(char* src, const char* suffix) {
    return strncmp(src + strlen(src) - strlen(suffix), suffix, strlen(suffix)) == 0;
}


static inline UFResult _consume(char* consume, char** src) {
    if (!src || !*src) return UF_ERROR;
    while (**src && strchr(consume, **src)) (*src)++;
    if (!src) return UF_ERROR;
    return UF_OK;
}


static inline void _parseMETA(char** src) {
    if (!src) return;

    struct {
        char version[4];
        u32 assets;
    } meta = { .assets = 0, .version[3] = '\0' };

    char* version = strstr(*src, "version=");
    if (version) {
        u8 digits = 0;
        while (*src && isdigit(**src)) {
            (*src)++;
            digits++;
        } *src += strlen("version=") + digits;
        r3WriteMemory(3, version + strlen("version="), &meta.version);
    }

    char* assets = strstr(*src, "assets=");
    if (assets) {
        u8 digits = 0;
        while (*src && isdigit(**src)) {
            (*src)++;
            digits++;
        } *src += strlen("assets=") + digits;
        meta.assets = (u32)atoi(assets + strlen("assets="));
    }

    r3LogStdOutF(R3_LOG_INFO, "UFA META: version=%s, assets=%u\n", meta.version, meta.assets);
}

static inline void _parseDATA(char** src) {
    if (!src || !*src) return;

}

static inline void _parseASSET(char** src) {
    if (!src || !*src) return;

    UFAEntry entry = {0};

    while (**src) {
        if (_startsWith(*src, "tag=")) {
            *src += strlen("tag=");
            char* start = *src;
            u32 len = 0;
            while (**src && isalnum(**src)) {
                (*src)++;
                len++;
            } r3WriteMemory(len, start, entry.tag);
        } else if (_startsWith(*src, "type=")) {
            *src += strlen("type=");
            entry.type = atoi(*src);
            while (**src && isdigit(**src)) (*src)++;
        } else if (_startsWith(*src, "mem=")) {
            *src += strlen("mem=");
            entry.memuse = atoi(*src);
            while (**src && isdigit(**src)) (*src)++;
        } else if (_startsWith(*src, "[DATA]")) {
            *src += strlen("[DATA]");
            _consume(" \n\t\r", src);
            _parseDATA(src);
        }

        else if (_startsWith(*src, "[ASSET]")) break;
        else while (**src && **src != '\n') (*src)++;
        _consume(" \n\t\r", src);
    }

    r3LogStdOutF(R3_LOG_INFO, "UFA ASSET: tag=%s, type=%u, memuse=%u\n", entry.tag, entry.type, entry.memuse);
}

UFResult _load(char* path) {
    if (!path) return UF_ERROR;

    char* src = r3LoadFile(path);
    if (!src) {
        r3LogStdOutF(R3_LOG_ERROR, "FAILED TO LOAD UFA: %s\n", path);
        return UF_ERROR;
    } r3LogStdOutF(R3_LOG_INFO, "LOADED UFA: %s\n", path);

    _consume(" \n\t\r", &src);
    u8 parsedMETA = 0;
    while(*src) {
        if (!parsedMETA && strncmp(src, "[META]", strlen("[META]")) == 0) {
            src += strlen("[META]");
            _consume(" \n\t\r", &src);
            _parseMETA(&src);
            _consume(" \n\t\r", &src);
            parsedMETA = 1;
        }

        if (strncmp(src, "[ASSET]", strlen("[ASSET]")) == 0) {
            src += strlen("[ASSET]");
            _consume(" \n\t\r", &src);
            _parseASSET(&src);
            _consume(" \n\t\r", &src);
        } else {
            break;
        }
    } return UF_OK;
}


UFHandle _getMesh(char* tag) {
    UFHandle handle = 0;
    //     UFAInternal.core.UFRI.newBuffer((UFRIBufferDesc){
    //     .size = ,
    //     .data = ,
    //     .use = ,
    //     .memuse = ,
    //     .attribs = ,
    // });
    return handle;
}

UFHandle _getShader(char* tag) {
    UFHandle handle = 0;
    //     UFAInternal.core.UFRI.newShader((UFRIShaderDesc){
    //     .type = ,
    //     .vertex = ,
    //     .fragment = ,
    // });
    return handle;
}

UFHandle _getTexture(char* tag) {
    UFHandle handle = 0;
    //     UFAInternal.core.UFRI.newTexture((UFRITextureDesc){
    //         .width = ,
    //         .height = ,
    //         .data = ,
    //         .use = ,
    //         .memuse = ,
    //         .format = ,
    //     });
    return handle;
}



none driverInit(void) {

}

none driverExit(void) {

}

#define STB_IMAGE_IMPLEMENTATION
#include <include/STB/stb_image.h>
void* _lt(i32 width, i32 height, char* path) {
    i32 channels = 0;
    stbi_set_flip_vertically_on_load(1);
    ptr data = stbi_load(path, &width, &height, &channels, 0);
    if (!data) {
        r3LogStdOut(R3_LOG_ERROR, "`stbi_load` failed\n");
        return NULL;
    } return data;
}

static UFA API = {
    .load = _load,
    .loadTexture = _lt
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
