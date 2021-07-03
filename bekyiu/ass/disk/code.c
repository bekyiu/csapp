//
// Created by bekyiu on 2021/6/27.
//

#include <stdlib.h>
#include "elf.h"
#include "../cpu/register.h"

Inst program[INST_LEN] = {
        // uint64_t add(uint64_t, uint64_t)
        {
                PUSH_REG,
                {REG,         0,                        0, (uint64_t *) &reg.rbp, NULL},
                {EMPTY,       0,     0, NULL,                  NULL},
                "push \%rbp",
        },
        {
                MOV_REG_REG,
                {REG,         0,                        0, (uint64_t *) &reg.rsp, NULL},
                {REG,         0,     0, (uint64_t *) &reg.rbp, NULL},
                "mov \%rsp, \%rbp",
        },
        {
                MOV_REG_MEM,
                {REG,         0,                        0, (uint64_t *) &reg.rdi, NULL},
                {MEM_IMM_REG, -0x18, 0, (uint64_t *) &reg.rbp, NULL},
                "mov    \%rdi,-0x18(\%rbp)"
        },
        {
                MOV_REG_MEM,
                {REG,         0,                        0, (uint64_t *) &reg.rsi, NULL},
                {MEM_IMM_REG, -0x20, 0, (uint64_t *) &reg.rbp, NULL},
                "mov    \%rsi,-0x20(\%rbp)"
        },
        {
                MOV_MEM_REG,
                {MEM_IMM_REG, -0x18,                    0, (uint64_t *) &reg.rbp, NULL},
                {REG,         0,     0, (uint64_t *) &reg.rdx, NULL},
                "mov    -0x18(\%rbp),\%rdx"
        },
        {
                MOV_MEM_REG,
                {MEM_IMM_REG, -0x20,                    0, (uint64_t *) &reg.rbp, NULL},
                {REG,         0,     0, (uint64_t *) &reg.rax, NULL},
                "mov    -0x20(\%rbp),\%rax"
        },
        {
                ADD_REG_REG,
                {REG,         0,                        0, (uint64_t *) &reg.rdx, NULL},
                {REG,         0,     0, (uint64_t *) &reg.rax, NULL},
                "add    \%rdx,\%rax"
        },
        {
                MOV_REG_MEM,
                {REG,         0,                        0, (uint64_t *) &reg.rax, NULL},
                {MEM_IMM_REG, -0x8,  0, (uint64_t *) &reg.rbp, NULL},
                "mov    \%rax,-0x8(\%rbp)"
        },
        {
                MOV_MEM_REG,
                {MEM_IMM_REG, -0x8,                     0, (uint64_t *) &reg.rbp, NULL},
                {REG,         0,     0, (uint64_t *) &reg.rax, NULL},
                "mov    -0x8(\%rbp),\%rax"
        },
        {
                POP_REG,
                {REG,         0,                        0, (uint64_t *) &reg.rbp, NULL},
                {EMPTY,       0,     0, NULL,                  NULL},
                "pop   \%rbp"
        },
        {
                RET,
                {EMPTY,       0,                        0, NULL,                  NULL},
                {EMPTY,       0,     0, NULL,                  NULL},
                "retq"
        },
        // main entry point
        {
                MOV_REG_REG,
                {REG,         0,                        0, (uint64_t *) &reg.rdx, NULL},
                {REG,         0,     0, (uint64_t *) &reg.rsi, NULL},
                "mov    \%rdx,\%rsi"
        },
        {
                MOV_REG_REG,
                {REG,         0,                        0, (uint64_t *) &reg.rax, NULL},
                {REG,         0,     0, (uint64_t *) &reg.rdi, NULL},
                "mov    \%rax,\%rdi"
        },
        // reg.rip
        {
                CALL,
                {IMM,         (uint64_t) &(program[0]), 0, NULL,                  NULL},
                {EMPTY,       0,     0, NULL,                  NULL},
                "callq  <add>"
        },
        // return address
        {
                MOV_REG_MEM,
                {REG,         0,                        0, (uint64_t *) &reg.rax, NULL},
                {MEM_IMM_REG, -0x8,  0, (uint64_t *) &reg.rbp, NULL},
                "mov    \%rax,-0x8(\%rbp)"
        },
};


