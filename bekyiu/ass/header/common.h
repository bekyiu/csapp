//
// Created by bekyiu on 2021/7/4.
//

#ifndef CSAPP_COMMON_H
#define CSAPP_COMMON_H

#include <stdint.h>
#include <stdbool.h>

#define DEBUG_INSTRUCTION_CYCLE      0x1
#define DEBUG_REGISTERS              0x2
#define DEBUG_PRINT_STACK            0x4
#define DEBUG_PRINT_CACHE_SET        0x8
#define DEBUG_CACHEDE_TAILS          0x10
#define DEBUG_MMU                    0x20
#define DEBUG_LINKER                 0x40
#define DEBUG_LOADER                 0x80
#define DEBUG_PARSE_INST             0x100

#define DEBUG_VERBOSE_SET            DEBUG_INSTRUCTION_CYCLE | DEBUG_PARSE_INST

// do page walk
#define DEBUG_ENABLE_PAGE_WALK      0

// use sram cache for memory access
#define DEBUG_ENABLE_SRAM_CACHE     0

// printf wrapper
uint64_t slog(uint64_t openSet, const char *format, ...);

// type converter
// uint32 to its equivalent float with rounding
uint32_t uint2float(uint32_t u);

// convert string dec or hex to the integer bitmap
uint64_t str2uint(const char *str);

uint64_t str2uintRange(const char *str, int start, int end);

bool startsWith(const char *pre, const char *str);

#endif //CSAPP_COMMON_H
