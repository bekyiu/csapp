//
// Created by bekyiu on 2021/6/27.
//

#include "instruction.h"
#include "../cpu/mmu.h"
#include "../cpu/register.h"
#include "stdio.h"
#include "dram.h"

// 返回真实的操作数
static uint64_t decodeOpd(Opd opd) {
    if (opd.type == IMM) {
        return *((uint64_t *) &opd.imm);
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

    return vAddr;
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

    printf("[%s]\n", inst->asmCode);

}

void initHandlerTable() {
    handlerTable[ADD_REG_REG] = addRegReg;
    handlerTable[MOV_REG_REG] = movRegReg;
    handlerTable[CALL] = call;
    handlerTable[PUSH_REG] = pushReg;
    handlerTable[MOV_REG_MEM] = movRegMem;
    handlerTable[MOV_MEM_REG] = movMemReg;
    handlerTable[POP_REG] = popReg;
    handlerTable[RET] = ret;
}

void addRegReg(uint64_t src, uint64_t dst) {
    *((uint64_t *) dst) = *((uint64_t *) dst) + *((uint64_t *) src);
    reg.rip += sizeof(Inst);
}

void movRegReg(uint64_t src, uint64_t dst) {
    *((uint64_t *) dst) = *((uint64_t *) src);
    reg.rip += sizeof(Inst);
}

void call(uint64_t src, uint64_t dst) {
    // 写入返回地址到栈顶
    reg.rsp -= 8;
    write64Dram(va2pa(reg.rsp), reg.rip + sizeof(Inst));
    // 跳转到目标位置执行
    reg.rip = src;
}

void pushReg(uint64_t src, uint64_t dst) {
    reg.rsp -= 8;
    write64Dram(va2pa(reg.rsp), *(uint64_t *) src);
    reg.rip += sizeof(Inst);
}

void movRegMem(uint64_t src, uint64_t dst) {
    write64Dram(va2pa(dst), *(uint64_t *) src);
    reg.rip += sizeof(Inst);
}

void movMemReg(uint64_t src, uint64_t dst) {
    *(uint64_t *) dst = read64Dram(va2pa(src));
    reg.rip += sizeof(Inst);
}

void ret(uint64_t src, uint64_t dst) {
    // 从栈顶取出返回地址
    uint64_t retAddr = read64Dram(va2pa(reg.rsp));
    reg.rsp += 8;

    reg.rip = retAddr;
}

void popReg(uint64_t src, uint64_t dst) {
    *(uint64_t *) src = read64Dram(va2pa(reg.rsp));
    reg.rsp += 8;
    reg.rip += sizeof(Inst);
}



