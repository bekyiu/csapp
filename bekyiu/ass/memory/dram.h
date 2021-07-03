//
// Created by bekyiu on 2021/6/27.
//

#ifndef CSAPP_DRAM_H
#define CSAPP_DRAM_H

#include <stdint.h>
#define MEM_LEN 1000
// 物理内存
uint8_t mem[MEM_LEN];

void write64Dram(uint64_t pAddr, uint64_t val);
uint64_t read64Dram(uint64_t pAddr);

void logRes();
void logStack();
#endif //CSAPP_DRAM_H
