//
// Created by bekyiu on 2021/6/27.
//
#ifndef CSAPP_REGISTER_H
#define CSAPP_REGISTER_H
#include <stdint.h>

typedef struct Reg {
    // 一个寄存器
    union {
        struct {
            uint8_t al;
            uint8_t ah;
        };
        uint16_t ax;
        uint32_t eax;
        uint64_t rax;
    };
} Reg;

#endif //CSAPP_REGISTER_H
