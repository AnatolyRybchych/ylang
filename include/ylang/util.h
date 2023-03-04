#ifndef YLANG_UTIL_H_
#define YLANG_UTIL_H_

#include <stdio.h>
#include <assert.h>

#define Y_BLOCK(...) do {__VA_ARGS__} while(0)
#define Y_EPRT_FMT(FMT, ...) fprintf(stderr, FMT,##__VA_ARGS__)

#define Y_TODO(WHAT_TO_DO) Y_BLOCK(\
    assert("TODO:" WHAT_TO_DO);\
)

#define Y_UNREACHABLE(FMT, ...) Y_BLOCK(\
    while(1){\
        Y_EPRT_FMT("%s:%i UNREACHABLE(" FMT ");\n", __func__, (int)__LINE__,##__VA_ARGS__);\
    }\
)


#endif //YLANG_UTIL_H_
