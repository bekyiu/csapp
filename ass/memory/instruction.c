//
// Created by bekyiu on 2021/6/27.
//

#include "instruction.h"
#include "../cpu/mmu.h"

// 返回真实的操作数
static uint64_t decodeOpd(Opd opd) {
    if (opd.type == IMM) {
        return opd.imm;
    }
    if (opd.type == REG) {
        return (uint64_t) opd.reg1;
    }

    // 此时操作数一定是个虚拟地址
    uint64_t vAddr = 0;
    if (opd.type == MEM_IMM) {
        vAddr = opd.imm;
    } else if (opd.type == MEM_REG) {
        // *reg 得到寄存器的值
        vAddr = *(opd.reg1);
    }

    return va2pa(vAddr);
}

void instCycle() {

}