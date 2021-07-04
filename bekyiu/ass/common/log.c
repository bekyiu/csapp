//
// Created by bekyiu on 2021/7/4.
//

#include <stdarg.h>
#include <stdio.h>
#include "../header/common.h"

// switch log, wrapper of stdio printf
// controlled by the debug verbose bit set
uint64_t slog(uint64_t openSet, const char *format, ...) {
    if ((openSet & DEBUG_VERBOSE_SET) == 0x0) {
        return 0x1;
    }

    // implementation of std printf()
    va_list argPtr;
    va_start(argPtr, format);
    vfprintf(stderr, format, argPtr);
    va_end(argPtr);

    return 0x0;
}