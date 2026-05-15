#ifndef __R3_IO_TEST_H__
#define __R3_IO_TEST_H__

#include <include/libR3/r3def.h>

#define R3_TEST_METHOD_MAX ((1 << 8) - 1)

typedef struct R3TestMethod {
    R3Result (*entry)(ptr data);
    R3Result result;
    char* tag;
    ptr data;
} R3TestMethod;

typedef struct R3TestSuite {
    R3TestMethod methodv[R3_TEST_METHOD_MAX];
    none (*cleanup)(none);
    none (*setup)(none);
    u8 methods;
    char* tag;
} R3TestSuite;

typedef struct R3TestResult {
    u32 passed;
    u32 failed;
} R3TestResult;

R3_PUBLIC_API R3TestResult r3RunTests(u32 tests, R3TestSuite* testv);

#endif // __R3_IO_TEST_H__