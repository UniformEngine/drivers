#pragma once

#define __UF_API__
#include <include/uniform/uniform.h>

typedef struct UFR {
    void (*render)(void);

    void (*commandViewPort)(u32 x, u32 y, u32 w, u32 h);

    void (*commandClearDepth)(f32 depth);
    void (*commandClearColor)(f32 r, f32 g, f32 b, f32 a);

    void (*commandDrawIndexed)(u32 count);
    void (*commandDraw)(u32 first, u32 count);

    void (*commandBindShader)(UFRIResource shader);
    void (*commandBindIndexBuffer)(UFRIResource buffer);
    void (*commandBindFrameBuffer)(UFRIResource buffer);
    void (*commandBindVertexBuffer)(UFRIResource buffer);
    void (*commandBindTexture)(u32 slot, UFRIResource texture);
    void (*commandSendUniform)(char* name, void* data, UFRIShaderUniformType type);
} UFR;
