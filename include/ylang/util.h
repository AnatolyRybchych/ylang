#ifndef YLANG_UTIL_H_
#define YLANG_UTIL_H_

#include <stdio.h>
#include <assert.h>

#define Y_BLOCK(...) do {__VA_ARGS__} while(0)
#define Y_EPRT_FMT(FMT, ...) fprintf(stderr, FMT,##__VA_ARGS__)

#define Y_TODO(FMT, ...) Y_BLOCK(\
    while(1){\
        Y_EPRT_FMT("%s:%i TODO(" FMT ");\n", __FILE__, (int)__LINE__,##__VA_ARGS__);\
    }\
)

#define Y_UNREACHABLE(FMT, ...) Y_BLOCK(\
    while(1){\
        Y_EPRT_FMT("%s:%i UNREACHABLE(" FMT ");\n", __FILE__, (int)__LINE__,##__VA_ARGS__);\
    }\
)

#define Y_FAULT(FMT, ...) Y_BLOCK(\
    while(1){\
        Y_EPRT_FMT("%s:%i FAULT(" FMT ")\n", __FILE__, (int)__LINE__,##__VA_ARGS__);\
    }\
)

#define Y_ARRLEN(ARRAY) (sizeof (ARRAY) / sizeof (*ARRAY))


#endif //YLANG_UTIL_H_
