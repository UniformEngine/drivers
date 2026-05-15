#pragma once

#define __UF_API__
#include <include/uniform/uniform.h>

typedef struct UFSComponentDesc {
    u64 strides[255];
    u32 fields;
    u32 slots;
} UFSComponentDesc;

// Resolves to a single archetype view
typedef struct UFSMonoQueryDesc {
    UFHandle* components;
    u32 count;
} UFSMonoQueryDesc;

// Resolves to multiple archetype views
typedef struct UFSMultiQueryDesc {
    struct {
        UFHandle* components;
        u32 count;
    } include;
    struct {
        UFHandle* components;
        u32 count;
    } exclude;
} UFSMultiQueryDesc;

typedef struct UFSView {
    void* data;
    u64* offsets;
    u64* strides;
    u32* map;
    u32 count;
    u32 fields;
} UFSView;

typedef struct UFSProjectionDesc {
    struct {
        UFHandle component;
        void** dest;
        u32 field;
    } targets[255];
    u32 count;
} UFSProjectionDesc;

typedef struct UFSSystemDesc {
    UFResult (*init)(void* user, UFSView* view);
    UFResult (*main)(void* user, UFSView* view);
    UFResult (*exit)(void* user, UFSView* view);
    void* user;
} UFSSystemDesc;

typedef struct UFS {
    UFHandle (*newScene)(u32 entityMax);
    UFResult (*delScene)(UFHandle scene);

    UFHandle (*newEntity)(UFHandle scene);
    UFResult (*delEntity)(UFHandle entity, UFHandle scene);

    UFHandle (*newComponent)(UFSComponentDesc desc, UFHandle scene);
    UFResult (*delComponent)(UFHandle component, UFHandle scene);

    UFResult (*bindComponent)(UFHandle entity, UFHandle component, UFHandle scene);
    UFResult (*unbindComponent)(UFHandle entity, UFHandle component, UFHandle scene);

    void* (*getComponent)(UFHandle entity, UFHandle component, u32 field, UFHandle scene);
    void* (*viewField)(UFHandle component, u32 field, UFSView* view, UFHandle scene);
    UFResult (*setComponent)(UFHandle entity, UFHandle component, u32 field, void* data, UFHandle scene);

    UFHandle (*newQuery)(UFSMonoQueryDesc desc, UFHandle scene);
    UFResult (*project)(UFSProjectionDesc desc, UFHandle query, UFHandle scene);
    UFResult (*delQuery)(UFHandle query, UFHandle scene);

    UFHandle (*newSystem)(UFSSystemDesc desc, UFHandle query, UFHandle scene);
    UFResult (*runMonoSystem)(UFHandle system, UFHandle scene);
    UFResult (*runMultiSystem)(UFHandle system, UFHandle scene);
    UFResult (*delSystem)(UFHandle system, UFHandle scene);
} UFS;
