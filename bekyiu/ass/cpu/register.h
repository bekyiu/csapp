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

    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t rsp;

    uint64_t rip;
} Reg;

Reg reg;
#endif //CSAPP_REGISTER_H
