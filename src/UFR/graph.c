#include "_UFR.h"
#include "_UFR.h"

void _UFLDGExit(void) {
    FOR_I(0, r3ArraySlots(UFLDG.edges), 1) {
        if (UFLDG.edges[i]) r3DelArray(UFLDG.edges[i]);
    } r3DelArray(UFLDG.edges);
    r3DelArray(UFLDG.nodes);
}

void _UFLDGReset(void) {
    r3ClearArray(UFLDG.nodes);
    FOR_I(0, r3ArraySlots(UFLDG.nodes), 1) {
        UFLDG.nodes[i].resource = UF_INVALID;
        UFLDG.nodes[i].cmd = UFLDG_INVALID;

        if (UFLDG.edges[i]) {
            r3ClearArray(UFLDG.edges[i]);
            FOR_J(0, r3ArraySlots(UFLDG.edges[i]), 1) {
                UFLDG.edges[i][j].src = UFLDG_INVALID;
                UFLDG.edges[i][j].dst = UFLDG_INVALID;
            }
        }
    }
}

UFResult _UFLDGInit(u32 maxNodes) {
    if (!maxNodes) return UF_ERROR;

    UFLDG.edges = r3NewArray(maxNodes, sizeof(UFLDGEdge*));
    UFLDG.nodes = r3NewArray(maxNodes, sizeof(UFLDGNode));
    if (!UFLDG.edges || !UFLDG.nodes) {
        if (UFLDG.edges) r3DelArray(UFLDG.edges);
        if (UFLDG.nodes) r3DelArray(UFLDG.nodes);
        return UF_ERROR;
    }

    FOR_I(0, maxNodes, 1) {
        UFLDG.edges[i] = r3NewArray(8, sizeof(UFLDGEdge));
        if (!UFLDG.edges[i]) {
            _UFLDGExit();
            return UF_ERROR;
        } FOR_J(0, r3ArraySlots(UFLDG.edges[i]), 1) {
            UFLDG.edges[i][j].src = UFLDG_INVALID;
            UFLDG.edges[i][j].dst = UFLDG_INVALID;
        } UFLDG.nodes[i].cmd = UFLDG_INVALID;
    }

    return UF_OK;
}


void _UFLDGAddEdge(u32 src, u32 dst) {
    if (src >= r3ArraySlots(UFLDG.nodes)
    ||  src >= r3ArraySlots(UFLDG.edges)
    ||  dst >= r3ArraySlots(UFLDG.nodes)) return;

    UFLDGEdge* edges = UFLDG.edges[src];
    if (!edges) return;

    // FOR_I(0, r3ArrayCount(edges), 1) {
    //     if (edges[i].dst == dst) {
    //         return;
    //     }
    // }

    if (r3ArrayFull(edges)) {
        edges = r3ResizeArray( r3ArraySlots(edges) * 2, sizeof(UFLDGEdge), edges);
        if (!edges) return;
        UFLDG.edges[src] = edges;
    }

    r3PushArray(&(UFLDGEdge){
        .src = src,
        .dst = dst,
    }, edges);
}

void _UFLDGAddNode(u32 cmd, UFHandle resource, UFLDGNodeType type) {
    if (cmd == UFLDG_INVALID || r3ArrayFull(UFLDG.nodes)) return;
    r3PushArray(&(UFLDGNode){
        .type = type,
        .resource = resource,
        .cmd = cmd,
    }, UFLDG.nodes);
}


void _UFLDGBuild(UFRCommand* cmds, u32 count) {
    struct {
        u32 u;
        u32 s;
        u32 vb;
        u32 ib;
        u32 fb;
        u32 tx[16];
    } state = {
        .u = UFLDG_INVALID,
        .s = UFLDG_INVALID,
        .vb = UFLDG_INVALID,
        .ib = UFLDG_INVALID,
        .fb = UFLDG_INVALID
    }; FOR_I(0, 16, 1) state.tx[i] = UFLDG_INVALID;

    FOR_I(0, count, 1) {

        UFLDGNode* node = &UFLDG.nodes[i];
        UFRCommand* cmd = &cmds[i];

        switch (node->type) {
            case LDG_SHADER_NODE: {
                state.s = i;
            } break;

            case LDG_UNIFORM_NODE: {
                state.u = i;
                if (state.s != UFLDG_INVALID) {
                    _UFLDGAddEdge(state.s, i);
                }
            } break;

            case LDG_BIND_NODE: {
                switch (cmd->bind.resource.type) {
                    case UFRI_FRAME_BUFFER: {
                        state.fb = i;
                    } break;
                    case UFRI_VERTEX_BUFFER: {
                        state.vb = i;
                    } break;
                    case UFRI_INDEX_BUFFER: {
                        state.ib = i;
                    } break;
                    case UFRI_TEXTURE: {
                        state.tx[cmd->bind.slot] = i;
                    } break;
                    default: break;
                }
            } break;

            case LDG_COLOR_NODE:
            case LDG_DEPTH_NODE: {
                if (state.fb != UFLDG_INVALID) {
                    _UFLDGAddEdge(state.fb, i);
                }
            } break;


            case LDG_DRAW_NODE: {
                if (state.s != UFLDG_INVALID) {
                    _UFLDGAddEdge(state.s, i);
                } if (state.fb != UFLDG_INVALID) {
                    _UFLDGAddEdge(state.fb, i);
                } if (state.vb != UFLDG_INVALID) {
                    _UFLDGAddEdge(state.vb, i);
                } if (state.ib != UFLDG_INVALID) {
                    _UFLDGAddEdge(state.ib, i);
                }

                FOR_J(0, 16, 1) {
                    if (state.tx[j] != UFLDG_INVALID) {
                        _UFLDGAddEdge(state.tx[j], i);
                    }
                }

            } break;

            default: break;
        }
    }
}


char* _UFLDGNodeTypeStr(UFLDGNodeType type) {
    switch (type) {
        default: return "UNKNOWN";
        case LDG_DRAW_NODE: return "DRAW";
        case LDG_BIND_NODE: return "BIND";
        case LDG_COLOR_NODE: return "COLOR";
        case LDG_DEPTH_NODE: return "DEPTH";
        case LDG_SHADER_NODE: return "SHADER";
        case LDG_UNIFORM_NODE: return "UNIFORM";
    }
}

void _UFLDGPrint() {
    printf("NODES\n");
    FOR_I(0, r3ArrayCount(UFLDG.nodes), 1) {
        UFLDGNode* node = &UFLDG.nodes[i];
        if (node->cmd == UFLDG_INVALID) continue;

        char* type = _UFLDGNodeTypeStr(node->type);
        if (node->resource != UF_INVALID) {
            u64 resource = node->resource;
            printf("[%u]%s: cmd=%u resource=%llu\n", i, type, node->cmd, resource);
        } else {
            printf("[%u]%s: cmd=%u\n", i, type, node->cmd);
        }
    } printf("\n");

    printf("EDGES\n");
    FOR_I(0, r3ArrayCount(UFLDG.nodes), 1) {
        UFLDGEdge* edges = UFLDG.edges[i];
        if (!edges) continue;

        FOR_J(0, r3ArrayCount(edges), 1) {
            UFLDGEdge* edge = &edges[j];
            if (edge->src == UFLDG_INVALID || edge->dst == UFLDG_INVALID) continue;
            printf("\t%u -> %u\n", edge->src, edge->dst);
        }
    } printf("\n");
}
