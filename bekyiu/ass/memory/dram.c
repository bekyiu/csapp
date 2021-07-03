//
// Created by bekyiu on 2021/6/27.
//
#include <stdint.h>
#include <stdio.h>
#include "dram.h"
#include "../cpu/register.h"
#include "../cpu/mmu.h"

// 小端
void write64Dram(uint64_t pAddr, uint64_t val) {
    for (int i = 0; i < 8; ++i) {
        mem[pAddr + i] = (val >> (8 * i)) & 0xff;
    }
}

uint64_t read64Dram(uint64_t pAddr) {
    uint64_t val = 0;
    for (int i = 0; i < 8; ++i) {
        uint64_t byte = (uint64_t) mem[pAddr + i];
        val += (byte << (8 * i));
    }
    return val;
}

void logRes() {
    printf("rax = %16llx\trbx = %16llx\trcx = %16llx\trdx = %16llx\n",
           reg.rax, reg.rbx, reg.rcx, reg.rdx);
    printf("rsi = %16llx\trdi = %16llx\trbp = %16llx\trsp = %16llx\n",
           reg.rsi, reg.rdi, reg.rbp, reg.rsp);
    printf("rip = %16llx\n", reg.rip);
}

void logStack() {
    int n = 10;

    uint64_t *low = (uint64_t *) &mem[va2pa(reg.rsp)];
    uint64_t *high = &low[n];

    // 初始化为栈底
    uint64_t vAddrBottom = reg.rsp + n * 8;

    for (int i = 0; i < 2 * n; ++i) {
        uint64_t *ptr = (uint64_t *) (high - i);
        printf("0x%016llx : %16llx", vAddrBottom, (uint64_t) *ptr);

        if (i == n) {
            printf(" <== rsp");
        }

        vAddrBottom = vAddrBottom - 8;

        printf("\n");
    }
}