//
// Created by bekyiu on 2021/6/27.
//

#include <stdlib.h>
#include "elf.h"
#include "../cpu/register.h"

Inst program[INST_LEN] = {
    {
        MOV_REG_REG,
        {REG, 0, 0, (uint64_t*) &reg.rax, NULL},
        {REG, 0, 0, (uint64_t*) &reg.rbx, NULL},
        "mov rax rbx",
    },
};