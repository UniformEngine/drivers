#include "graph.c"

void _pushCommand(UFRCommand* cmd) {
    if (r3ArrayFull(UFRInternal.buffer)) return;
    cmd->index = r3ArrayCount(UFRInternal.buffer);

    switch (cmd->type) {
        default: break;
        case UFR_CMD_CLEAR_COLOR: {
            _UFLDGAddNode(cmd->index, UF_INVALID, LDG_COLOR_NODE);
        } break;
        case UFR_CMD_CLEAR_DEPTH: {
            _UFLDGAddNode(cmd->index, UF_INVALID, LDG_DEPTH_NODE);
        } break;

        case UFR_CMD_BIND_SHADER: {
            _UFLDGAddNode(cmd->index, cmd->bind.resource.handle, LDG_SHADER_NODE);
        } break;

        case UFR_CMD_SEND_UNIFORM: {
            _UFLDGAddNode(cmd->index, UF_INVALID, LDG_UNIFORM_NODE);
        } break;

        case UFR_CMD_BIND_FBUFFER:
        case UFR_CMD_BIND_IBUFFER:
        case UFR_CMD_BIND_VBUFFER:
        case UFR_CMD_BIND_TEXTURE: {
            _UFLDGAddNode(cmd->index, cmd->bind.resource.handle, LDG_BIND_NODE);
        } break;

        case UFR_CMD_DRAW:
        case UFR_CMD_DRAWI: {
            _UFLDGAddNode(cmd->index, UF_INVALID, LDG_DRAW_NODE);
        } break;
    } r3PushArray(cmd, UFRInternal.buffer);
    UFEXForwardPhase(UFR_BEGIN, cmd, UFRInternal.pipeline);
}

void _render(void) {
    _UFLDGBuild(UFRInternal.buffer, r3ArrayCount(UFRInternal.buffer));
    UFEXRunPipeline(UFRInternal.pipeline);
}

void commandViewPort(u32 x, u32 y, u32 w, u32 h) {
    UFRCommand cmd = {
        .type = UFR_CMD_VIEWPORT,
        .viewport.x = x,
        .viewport.y = y,
        .viewport.w = w,
        .viewport.h = h
    };
    _pushCommand(&cmd);
}

void _commandClearDepth(f32 depth) {
    UFRCommand cmd = {
        .type = UFR_CMD_CLEAR_DEPTH,
        .clear.depth = depth,
    };
    _pushCommand(&cmd);
}

void _commandClearColor(f32 r, f32 g, f32 b, f32 a) {
    UFRCommand cmd = {
        .type = UFR_CMD_CLEAR_COLOR,
        .clear.r = r,
        .clear.g = g,
        .clear.b = b,
        .clear.a = a
    };
    _pushCommand(&cmd);
}

void _commandDrawIndexed(u32 count) {
    UFRCommand cmd = {
        .type = UFR_CMD_DRAWI,
        .draw.count = count
    };
    _pushCommand(&cmd);
}

void _commandDraw(u32 first, u32 count) {
    UFRCommand cmd = {
        .type = UFR_CMD_DRAW,
        .draw.first = first,
        .draw.count = count
    };
    _pushCommand(&cmd);
}

void _commandBindShader(UFRIResource shader) {
    UFRCommand cmd = {
        .type= UFR_CMD_BIND_SHADER,
        .bind.resource = shader
    };
    _pushCommand(&cmd);
}

void _commandSendUniform(char* name, void* data, UFRIShaderUniformType type) {
    UFRCommand cmd = {
        .type = UFR_CMD_SEND_UNIFORM,
        .uniform.name = name,
        .uniform.data = data,
        .uniform.type = type
    };
    _pushCommand(&cmd);
}

void _commandBindIndexBuffer(UFRIResource buffer) {
    UFRCommand cmd = {
        .type = UFR_CMD_BIND_IBUFFER,
        .bind.resource = buffer
    };
    _pushCommand(&cmd);
}

void _commandBindFrameBuffer(UFRIResource buffer) {
    UFRCommand cmd = {
        .type = UFR_CMD_BIND_FBUFFER,
        .bind.resource = buffer
    };
    _pushCommand(&cmd);
}

void _commandBindVertexBuffer(UFRIResource buffer) {
    UFRCommand cmd = {
        .type = UFR_CMD_BIND_VBUFFER,
        .bind.resource = buffer
    };
    _pushCommand(&cmd);
}

void _commandBindTexture(u32 slot, UFRIResource texture) {
    UFRCommand cmd = {
        .type = UFR_CMD_BIND_TEXTURE,
        .bind.resource = texture,
        .bind.slot = slot
    };
    _pushCommand(&cmd);
}
