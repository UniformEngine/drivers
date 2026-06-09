#include "command.c"

// UFR Passes
UFResult _UFRDepth(UFHandle pipeline) {
    while (UFEXPullPass(&UFRInternal.command, pipeline) == UF_OK) {
        switch (UFRInternal.command.type) {
            case (UFR_CMD_BIND_FBUFFER): {
                UFRIBindFrameBuffer(UFRInternal.command.bind.resource);
            } break;
            case (UFR_CMD_CLEAR_DEPTH): {
                UFRIClearDepth(UFRInternal.command.clear.depth);
            } break;
            default: break;
        }
    } return UF_OK;
}

UFResult _UFROpaque(UFHandle pipeline) {
    while (UFEXPullPass(&UFRInternal.command, pipeline) == UF_OK) {
        switch (UFRInternal.command.type) {
            case (UFR_CMD_BIND_FBUFFER): {
                UFRIBindFrameBuffer(UFRInternal.command.bind.resource);
            } break;
            case (UFR_CMD_CLEAR_COLOR): {
                UFRIClearColor(
                    UFRInternal.command.clear.r,
                    UFRInternal.command.clear.g,
                    UFRInternal.command.clear.b,
                    UFRInternal.command.clear.a
                );
            } break;
            case (UFR_CMD_BIND_SHADER): {
                UFRInternal.shader = UFRInternal.command.bind.resource;
                UFRIBindShader(UFRInternal.shader);
            } break;
            case (UFR_CMD_SEND_UNIFORM): {
                UFRISetUniform(UFRInternal.command.uniform.name, UFRInternal.command.uniform.data, UFRInternal.command.uniform.type, UFRInternal.shader);
                UFRISendUniform(UFRInternal.command.uniform.name, UFRInternal.shader);
            } break;
            case (UFR_CMD_BIND_TEXTURE): {
                UFRInternal.textures[UFRInternal.command.bind.slot] = UFRInternal.command.bind.resource;
                UFRIBindTexture(UFRInternal.command.bind.slot, UFRInternal.textures[UFRInternal.command.bind.slot]);
            } break;
            case (UFR_CMD_BIND_VBUFFER): {
                UFRInternal.vertexBuffer = UFRInternal.command.bind.resource;
                UFRIBindVertexBuffer(UFRInternal.vertexBuffer);
            } break;
            case (UFR_CMD_BIND_IBUFFER): {
                UFRInternal.indexBuffer = UFRInternal.command.bind.resource;
                UFRIBindIndexBuffer(UFRInternal.indexBuffer);
            } break;
            case (UFR_CMD_DRAW): {
                UFRIDraw(UFRInternal.command.draw.first, UFRInternal.command.draw.count);
            } break;
            case (UFR_CMD_DRAWI): {
                UFRIDrawIndexed(UFRInternal.command.draw.count);
            } break;
            default: break;
        }
    }
    return UF_OK;
}

UFResult _UFRLight(UFHandle pipeline) {
    while (UFEXPullPass(&UFRInternal.command, pipeline) == UF_OK) {
        switch (UFRInternal.command.type) {
            case (UFR_CMD_BIND_FBUFFER): {
                UFRIBindFrameBuffer(UFRInternal.command.bind.resource);
            } break;
            default: break;
        }
    } return UF_OK;
}

UFResult _UFRShadow(UFHandle pipeline) {
    while (UFEXPullPass(&UFRInternal.command, pipeline) == UF_OK) {
        switch (UFRInternal.command.type) {
            case (UFR_CMD_BIND_FBUFFER): {
                UFRIBindFrameBuffer(UFRInternal.command.bind.resource);
            } break;
            default: {
                UFEXForwardPhase(UFR_POST, &UFRInternal.command, UFRInternal.pipeline);
            } break;
        }
    } return UF_OK;
}


// UFR Phases
// frame setup and command processing (e.g filtering/expansion)
UFResult _UFRBegin(UFHandle pipeline) {
    UFRIBindFrameBuffer(UFRInternal._offscreenFrameBuffer);

    while (UFEXPullPhase(&UFRInternal.command, UFRInternal.pipeline) == UF_OK) {
        switch (UFRInternal.command.type) {
            case (UFR_CMD_BIND_FBUFFER): {
                UFRIBindFrameBuffer(UFRInternal.command.bind.resource);
            } break;
            default: {
                UFEXForwardPhase(UFR_MAIN, &UFRInternal.command, UFRInternal.pipeline);
            } break;
        }
    } return UF_OK;
}

// pass construction and command routing (opaque/light/shadow etc...)
UFResult _UFRMain(UFHandle pipeline) {
    while (UFEXPullPhase(&UFRInternal.command, UFRInternal.pipeline) == UF_OK) {
        switch (UFRInternal.command.type) {
            case (UFR_CMD_CLEAR_DEPTH): {
                UFEXForwardPass(UFR_DEPTH, &UFRInternal.command, UFRInternal.pipeline);
            } break;
            case (UFR_CMD_DRAW):
            case (UFR_CMD_DRAWI):
            case (UFR_CMD_CLEAR_COLOR):
            case (UFR_CMD_BIND_SHADER):
            case (UFR_CMD_BIND_TEXTURE):
            case (UFR_CMD_SEND_UNIFORM): {
                UFEXForwardPass(UFR_OPAQUE, &UFRInternal.command, UFRInternal.pipeline);
            } break;
            case (UFR_CMD_BIND_IBUFFER):
            case (UFR_CMD_BIND_VBUFFER): {
                UFEXForwardPass(UFR_OPAQUE, &UFRInternal.command, UFRInternal.pipeline);
                UFEXForwardPass(UFR_DEPTH, &UFRInternal.command, UFRInternal.pipeline);
                UFEXForwardPass(UFR_LIGHT, &UFRInternal.command, UFRInternal.pipeline);
                UFEXForwardPass(UFR_SHADOW, &UFRInternal.command, UFRInternal.pipeline);
            } break;
            case (UFR_CMD_BIND_FBUFFER):
            default: {
                UFEXForwardPhase(UFR_POST, &UFRInternal.command, UFRInternal.pipeline);
            } break;
        }
    } return UF_OK;
}

// post processing
UFResult _UFRPost(UFHandle pipeline) {
    while (UFEXPullPhase(&UFRInternal.command, UFRInternal.pipeline) == UF_OK) {
        switch (UFRInternal.command.type) {
            default: {
                UFEXForwardPhase(UFR_END, &UFRInternal.command, UFRInternal.pipeline);
            } break;
        }
    }

    UFRIBindFrameBuffer((UFRIResource){ .type = UFRI_FRAME_BUFFER, .handle = 0 });
    UFRIBindVertexBuffer(UFRInternal._offscreenVertexBuffer);
    UFRIBindTexture(0, UFRInternal._offscreenColorBuffer);
    UFRIBindShader(UFRInternal._offscreenShader);
    // UFRIToggle(UFRI_TOGGLE_DEPTH);
    UFRIDraw(0, 6);

    return UF_OK;
}

// frame cleanup
UFResult _UFREnd(UFHandle pipeline) {
    while (UFEXPullPhase(&UFRInternal.command, UFRInternal.pipeline) == UF_OK) {
        switch (UFRInternal.command.type) {
            default: break;
        }
    }

    // _UFLDGPrint();
    _UFLDGReset();
    r3ClearArray(UFRInternal.buffer);

    return UF_OK;
}


UFEXHook _UFRPasses[UFR_PASSES] = {
    _UFRDepth,
    _UFROpaque,
    _UFRLight,
    _UFRShadow
};

none driverInit(void) {
    UFRInternal.buffer = r3NewArray(UFR_CMD_MAX, sizeof(UFRCommand));
    if (!UFRInternal.buffer || _UFLDGInit(4096) != UF_OK) {
        r3LogStdOut(R3_LOG_ERROR, "[UFR] Error during init -- failed to allocate command buffer\n");
        return;
    }

    UFRInternal._offscreenShader = UFRINewShader((UFRIShaderDesc){
        .use = UFRI_SHADER_PIXEL,
        .vertex =   "#version 460\n"
                    "layout(location=0) in vec3 _position;\n"
                    "layout(location=3) in vec2 _texture;\n"
                    "out vec2 _uv;\n"
                    "void main() {\n"
                    "\tgl_Position = vec4(_position, 1.0);\n"
                    "\t_uv = _texture;\n"
                    "}\n",

        .fragment = "#version 460\n"
                    "in vec2 _uv;\n"
                    "out vec4 _color;\n"
                    "uniform sampler2D _texture;\n"
                    "void main() {\n"
                    "\t_color = texture(_texture, _uv);\n"
                    "}\n"
    });

    UFRInternal._offscreenDepthBuffer = UFRINewTexture((UFRITextureDesc){
        .data = NULL,
        .width = 1280,
        .height = 720,
        .use = UFRI_USE_2D,
        .memuse = UFRI_STATIC,
        .format = UFRI_FORMAT_DEPTH24,
    });

    UFRInternal._offscreenColorBuffer = UFRINewTexture((UFRITextureDesc){
        .data = NULL,
        .width = 1280,
        .height = 720,
        .use = UFRI_USE_2D,
        .memuse = UFRI_STATIC,
        .format = UFRI_FORMAT_RGBA8,
    });

    UFRInternal._offscreenFrameBuffer = UFRINewFrameBuffer((UFRIFrameBufferDesc){
        .color[0] = UFRInternal._offscreenColorBuffer,
        .depth = UFRInternal._offscreenDepthBuffer,
        .attachments = 1,
    });
    UFRInternal.frameBuffer = UFRInternal._offscreenFrameBuffer;

    UFRInternal._offscreenVertexBuffer = UFRINewVertexBuffer((UFRIVertexBufferDesc){
        .count = 6,
        .memuse = UFRI_STATIC,
        .attribs = UFRI_ATTRIB_POSITION|UFRI_ATTRIB_UV,
        .vertices = (f32[]){
            -1.0f,  1.0f, 0.0,  0.0f, 1.0f,
            -1.0f, -1.0f, 0.0,  0.0f, 0.0f,
             1.0f, -1.0f, 0.0,  1.0f, 0.0f,

            -1.0f,  1.0f, 0.0,  0.0f, 1.0f,
             1.0f, -1.0f, 0.0,  1.0f, 0.0f,
             1.0f,  1.0f, 0.0,  1.0f, 1.0f
        }
    });

    UFRInternal.pipeline = UFEXNewPipeline((UFEXPipelineDesc){
        .hookMax = 1,
        .stepMax = 1,
        .passMax = UFR_PASSES,
        .phaseMax = UFR_PHASES,
        .forwardMax = UFR_CMD_MAX,
        .longStride = sizeof(UFRCommand),
        .shortStride = sizeof(UFRCommand)
    });

    UFEXNewPhase(UFRInternal.pipeline);
    UFEXHookPhase(_UFRBegin, UFRInternal.pipeline);

    UFEXNewPhase(UFRInternal.pipeline);
    UFEXHookPhase(_UFRMain, UFRInternal.pipeline);

    FOR_I(0, UFR_PASSES, 1) {
        UFEXNewPass(UFRInternal.pipeline);
        UFEXHookPass(_UFRPasses[i], UFRInternal.pipeline);
    }

    UFEXNewPhase(UFRInternal.pipeline);
    UFEXHookPhase(_UFRPost, UFRInternal.pipeline);

    UFEXNewPhase(UFRInternal.pipeline);
    UFEXHookPhase(_UFREnd, UFRInternal.pipeline);
}

none driverExit(void) {
    if (UFRInternal.buffer) r3DelArray(UFRInternal.buffer);
    UFEXDelPipeline(UFRInternal.pipeline);
}


UFR API = {
    .render = _render,

    .commandClearDepth = _commandClearDepth,
    .commandClearColor = _commandClearColor,

    .commandDrawIndexed = _commandDrawIndexed,
    .commandDraw = _commandDraw,

    .commandBindShader = _commandBindShader,
    .commandSendUniform = _commandSendUniform,
    .commandBindTexture = _commandBindTexture,
    .commandBindIndexBuffer = _commandBindIndexBuffer,
    .commandBindFrameBuffer = _commandBindFrameBuffer,
    .commandBindVertexBuffer = _commandBindVertexBuffer,
};

UFDriver UFENTRY(UFInterface uniform) {
    UFRInternal.core = uniform;
    return (UFDriver) {
        .interface = &API,
        .init = driverInit,
        .exit = driverExit,
        .info = {
            .caps = 0,
            .flags = 0,
            .ver = "1.0.0",
            .tag = "Uniform Renderer",
        },
    };
}
