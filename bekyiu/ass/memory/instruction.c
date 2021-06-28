//
// Created by bekyiu on 2021/6/27.
//

#include "instruction.h"
#include "../cpu/mmu.h"
#include "../cpu/register.h"

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
    } else if (opd.type == MEM_IMM_REG) {
        vAddr = opd.imm + *(opd.reg1);
    } else if (opd.type == MEM_REG1_REG2) {
        vAddr = *(opd.reg1) + *(opd.reg2);
    } else if (opd.type == MEM_IMM_REG1_REG2) {
        vAddr = opd.imm + *(opd.reg1) + *(opd.reg2);
    } else if (opd.type == MEM_REG2_S) {
        vAddr = *(opd.reg2) * opd.scale;
    } else if (opd.type == MEM_IMM_REG2_S) {
        vAddr = opd.imm + *(opd.reg2) * opd.scale;
    } else if (opd.type == MEM_REG1_REG2_S) {
        vAddr = *(opd.reg1) + *(opd.reg2) * opd.scale;
    } else if (opd.type == MEM_IMM_REG1_REG2_S) {
        vAddr = opd.imm + *(opd.reg1) + *(opd.reg2) * opd.scale;
    }

    return va2pa(vAddr);
}

void instCycle() {
    // rip存的是当前要执行指令的地址
    Inst *inst = (Inst *) reg.rip;

    // imm: imm
    // reg: &value
    // mem: pAddr
    uint64_t src = decodeOpd(inst->src);
    uint64_t dst = decodeOpd(inst->dst);
    InstHandler handler = handlerTable[inst->opt];
    handler(src, dst);

    reg.rip += sizeof(Inst);
}

void initHandlerTable() {
    handlerTable[MOV_REG_REG] = addRegReg;
}

void addRegReg(uint64_t src, uint64_t dst) {
    *((uint64_t *) dst) = *((uint64_t *) dst) + *((uint64_t *) src);
}