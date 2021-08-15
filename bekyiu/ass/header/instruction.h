//
// Created by bekyiu on 2021/8/15.
//

#ifndef CSAPP_INSTRUCTION_H
#define CSAPP_INSTRUCTION_H
/*======================================*/
/*      instruction set architecture    */
/*======================================*/

// operator
typedef enum Opt {
    MOV,           // 0
    PUSH,          // 1
    POP,           // 2
    LEAVE,         // 3
    CALL,          // 4
    RET,           // 5
    ADD,           // 6
    SUB,           // 7
    CMP,           // 8
    JNE,           // 9
    JMP,           // 10
} Opt;


// operand type
typedef enum OpdType {
    EMPTY,
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
    uint64_t imm;
    uint64_t scale;
    uint64_t reg1;
    uint64_t reg2;
} Opd;

// instruction
typedef struct Inst {
    Opt opt;
    Opd src;
    Opd dst;

} Inst;

#endif //CSAPP_INSTRUCTION_H
