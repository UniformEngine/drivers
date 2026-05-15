#pragma once

#include <include/UFR/UFR.h>
#include <stdio.h>

#define UFR_CMD_MAX      ((1 << 16) - 1)

typedef enum UFRPhase {
    UFR_BEGIN,
    UFR_MAIN,
    UFR_POST,
    UFR_END,
    UFR_PHASES
} UFRPhase;

typedef enum UFRPass {
    UFR_DEPTH,
    UFR_OPAQUE,
    UFR_LIGHT,
    UFR_SHADOW,
    UFR_PASSES
} UFRPass;

typedef enum UFRCommandType {
    UFR_CMD_NONE,
    UFR_CMD_DRAW,
    UFR_CMD_DRAWI,
    UFR_CMD_VIEWPORT,
    UFR_CMD_CLEAR_COLOR,
    UFR_CMD_CLEAR_DEPTH,
    UFR_CMD_BIND_SHADER,
    UFR_CMD_SEND_UNIFORM,
    UFR_CMD_BIND_VBUFFER,
    UFR_CMD_BIND_IBUFFER,
    UFR_CMD_BIND_FBUFFER,
    UFR_CMD_BIND_TEXTURE,
} UFRCommandType;

typedef struct UFRCommand {
    u64 index;
    UFRCommandType type;

    union {
        struct {
            UFRIResource resource;
            u32 slot;
        } bind;
        struct {
            u32 first, count;
        } draw;
        struct {
            f32 r, g, b, a;
            f32 depth;
        } clear;
        struct {
            u32 x, y, w, h;
        } viewport;
        struct {
            UFRIShaderUniformType type;
            char* name;
            void* data;
        } uniform;
    };
} UFRCommand;

typedef enum UFRMode {
    UFR_MODE_EXECUTE,
    UFR_MODE_RECORD,
} UFRMode;

static struct UFRInternal {
    UFRMode mode;
    UFInterface core;
    UFHandle pipeline;

    UFRCommand command;
    UFRCommand* buffer;

    UFRIResource shader;
    UFRIResource indexBuffer;
    UFRIResource frameBuffer;
    UFRIResource vertexBuffer;
    UFRIResource textures[16];

    UFRIResource _offscreenShader;
    UFRIResource _offscreenFrameBuffer;
    UFRIResource _offscreenDepthBuffer;
    UFRIResource _offscreenColorBuffer;
    UFRIResource _offscreenVertexBuffer;
} UFRInternal = {0};


// Uniform's LDG (live dependency graph) is a runtime-built graph of command relationships
// computed from an adjacency list representation.

#define UFLDG_INVALID (u32)UF_INVALID

typedef enum UFLDGNodeType {
    LDG_DRAW_NODE,
    LDG_BIND_NODE,      // buffer/texture binds
    LDG_COLOR_NODE,     // color buffer mutations
    LDG_DEPTH_NODE,     // depth buffer mutations
    LDG_SHADER_NODE,    // shader binds/mutations
    LDG_UNIFORM_NODE,   // uniform binds/mutations
} UFLDGNodeType;

typedef struct UFLDGNode {
    UFLDGNodeType type;
    UFHandle resource;
    u32 cmd;
} UFLDGNode;

typedef struct UFLDGEdge {
    u32 src;
    u32 dst;
} UFLDGEdge;

static struct {
    UFLDGNode* nodes;
    UFLDGEdge** edges;
} UFLDG = {0};
