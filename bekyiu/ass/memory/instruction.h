//
// Created by bekyiu on 2021/6/27.
//

#ifndef CSAPP_INSTRUCTION_H
#define CSAPP_INSTRUCTION_H

#include <stdint.h>

#define NUM_OPT

// operator
typedef enum Opt {
    MOV_REG_REG,    // 0
    PUSH,   // 1
    CALL,   // 2
} Opt;

// operand type
typedef enum OpdType {
    // imm
    IMM,
    // R[ra]
    REG,
    // M[imm]
    MEM_IMM,
    // M[R[ra]]
    MEM_REG,
    // M[imm + R[rb]]
    MEM_IMM_REG,
    // M[R[rb] + R[ri]]
    MEM_REG1_REG2,
    // M[imm + R[rb] + R[ri]]
    MEM_IMM_REG1_REG2,
    // M[R[ri] * s]
    MEM_REG2_S,
    // M[imm + R[ri] * s]
    MEM_IMM_REG2_S,
    // M[R[rb] + R[ri] * s]
    MEM_REG1_REG2_S,
    // M[imm + R[rb] + R[ri] * s]
    MEM_IMM_REG1_REG2_S,
} OpdType;

// operand
typedef struct Opd {
    OpdType type;
    int64_t imm;
    int64_t scale;
    uint64_t *reg1;
    uint64_t *reg2;
} Opd;

// instruction
typedef struct Inst {
    Opt opt;
    Opd src;
    Opd dst;

    char asmCode[100];
} Inst;

// 处理指令的函数指针
typedef void(*InstHandler)(uint64_t, uint64_t);
InstHandler handlerTable[NUM_OPT];


// 一个指令周期
void instCycle();
// 初始化
void initHandlerTable();
// 指令操作
void addRegReg(uint64_t src, uint64_t dst);

#endif //CSAPP_INSTRUCTION_H
