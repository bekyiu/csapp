//
// Created by bekyiu on 2021/6/27.
//
#include "mmu.h"
#include "../memory/dram.h"

// 先简单实现
uint64_t va2pa(uint64_t vAddr) {
    return vAddr % MEM_LEN;
}

