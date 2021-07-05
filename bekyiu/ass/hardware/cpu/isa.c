//
// Created by bekyiu on 2021/7/4.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../header/cpu.h"
#include "../../header/memory.h"
#include "../../header/common.h"

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
    int64_t imm;
    int64_t scale;
    uint64_t reg1;
    uint64_t reg2;
} Opd;

// instruction
typedef struct Inst {
    Opt opt;
    Opd src;
    Opd dst;

} Inst;

/*======================================*/
/*      parse assembly instruction      */
/*======================================*/

// functions to map the string assembly code to inst_t instance
static void parseInst(const char *str, Inst *inst, Core *cr);

static void parseOpd(const char *str, Opd *opd, Core *cr);

static uint64_t decodeOpd(Opd *opd);

// return the real operand
static uint64_t decodeOpd(Opd *opd) {
    if (opd->type == IMM) {
        // immediate signed number can be negative: convert to bitmap
        return *((uint64_t *) &opd->imm);
    }
    if (opd->type == REG) {
        return opd->reg1;
    }

    // now the operand must be a virtual address
    uint64_t vAddr = 0;
    if (opd->type == MEM_IMM) {
        vAddr = opd->imm;
    } else if (opd->type == MEM_REG) {
        // opd->reg store a pointer point to the value in register
        vAddr = *((uint64_t *) opd->reg1);
    } else if (opd->type == MEM_IMM_REG) {
        vAddr = opd->imm + *((uint64_t *) opd->reg1);
    } else if (opd->type == MEM_REG1_REG2) {
        vAddr = *((uint64_t *) opd->reg1) + *((uint64_t *) opd->reg2);
    } else if (opd->type == MEM_IMM_REG1_REG2) {
        vAddr = opd->imm + *((uint64_t *) opd->reg1) + *((uint64_t *) opd->reg2);
    } else if (opd->type == MEM_REG2_S) {
        vAddr = *((uint64_t *) opd->reg2) * opd->scale;
    } else if (opd->type == MEM_IMM_REG2_S) {
        vAddr = opd->imm + *((uint64_t *) opd->reg2) * opd->scale;
    } else if (opd->type == MEM_REG1_REG2_S) {
        vAddr = *((uint64_t *) opd->reg1) + *((uint64_t *) opd->reg2) * opd->scale;
    } else if (opd->type == MEM_IMM_REG1_REG2_S) {
        vAddr = opd->imm + *((uint64_t *) opd->reg1) + *((uint64_t *) opd->reg2) * opd->scale;
    }

    return vAddr;
}

static void parseInst(const char *str, Inst *inst, Core *cr) {

}

// parse an operand string to an Opd object
static void parseOpd(const char *str, Opd *opd, Core *cr) {
    // str: eg %rax, $123 ...
    opd->type = EMPTY;
    opd->reg1 = 0;
    opd->reg2 = 0;
    opd->imm = 0;
    opd->scale = 0;

    size_t len = strlen(str);
    if (len == 0) {
        return;
    }
    // immediate number
    if (str[0] == '$') {
        opd->type = IMM;
        // skip '$'
        opd->imm = str2uintRange(str, 1, -1);
        return;
    }
    // register
    if (str[0] == '%') {

    }
    // memory

}


/*======================================*/
/*      instruction handlers            */
/*======================================*/

// instruction (sub)set
// In this simulator, the instructions have been decoded and fetched
// so there will be no page fault during fetching
// otherwise the instructions must handle the page fault (swap in from disk) first
// and then re-fetch the instruction and do decoding
// and finally re-run the instruction

static void movHandler(Opd *srcOpd, Opd *dstOpd, Core *cr);

static void pushHandler(Opd *srcOpd, Opd *dstOpd, Core *cr);

static void popHandler(Opd *srcOpd, Opd *dstOpd, Core *cr);

static void leaveHandler(Opd *srcOpd, Opd *dstOpd, Core *cr);

static void callHandler(Opd *srcOpd, Opd *dstOpd, Core *cr);

static void retHandler(Opd *srcOpd, Opd *dstOpd, Core *cr);

static void addHandler(Opd *srcOpd, Opd *dstOpd, Core *cr);

static void subHandler(Opd *srcOpd, Opd *dstOpd, Core *cr);

static void cmpHandler(Opd *srcOpd, Opd *dstOpd, Core *cr);

static void jneHandler(Opd *srcOpd, Opd *dstOpd, Core *cr);

static void jmpHandler(Opd *srcOpd, Opd *dstOpd, Core *cr);


// handler table storing the handlers to different instruction types
typedef void (*InstHandler)(Opd *, Opd *, Core *);

// look-up table of pointers to function
static InstHandler handlerTable[NUM_INSTRUCTION_TYPE] = {
        movHandler,               // 0
        pushHandler,              // 1
        popHandler,               // 2
        leaveHandler,             // 3
        callHandler,              // 4
        retHandler,               // 5
        addHandler,               // 6
        subHandler,               // 7
        cmpHandler,               // 8
        jneHandler,               // 9
        jmpHandler,               // 10
};

// reset the condition flags
// inline to reduce cost
static inline void resetCFlags(Core *cr) {
    cr->cf = 0;
    cr->zf = 0;
    cr->sf = 0;
    cr->of = 0;
}

// update the rip pointer to the next instruction sequentially
static inline void nextRip(Core *cr) {
    // we are handling the fixed-length of assembly string here
    // but their size can be variable as true X86 instructions
    // that's because the operands' sizes follow the specific encoding rule
    // the risc-v is a fixed length ISA
    cr->rip = cr->rip + sizeof(char) * MAX_INSTRUCTION_CHAR;
}

// instruction handlers

static void movHandler(Opd *srcOpd, Opd *dstOpd, Core *cr) {
    uint64_t src = decodeOpd(srcOpd);
    uint64_t dst = decodeOpd(dstOpd);

    if (srcOpd->type == REG && dstOpd->type == REG) {
        // src: register
        // dst: register
        *(uint64_t *) dst = *(uint64_t *) src;
        nextRip(cr);
        resetCFlags(cr);
        return;
    }
    if (srcOpd->type == REG && dstOpd->type >= MEM_IMM) {
        // src: register
        // dst: virtual address
        write64Dram(
                va2pa(dst, cr),
                *(uint64_t *) src,
                cr
        );
        nextRip(cr);
        resetCFlags(cr);
        return;
    }
    if (srcOpd->type >= MEM_IMM && dstOpd->type == REG) {
        // src: virtual address
        // dst: register
        *(uint64_t *) dst = read64Dram(
                va2pa(src, cr),
                cr);
        nextRip(cr);
        resetCFlags(cr);
        return;
    }
    if (srcOpd->type == IMM && dstOpd->type == REG) {
        // src: immediate number (uint64_t bit map)
        // dst: register
        *(uint64_t *) dst = src;
        nextRip(cr);
        resetCFlags(cr);
        return;
    }
}

static void pushHandler(Opd *srcOpd, Opd *dstOpd, Core *cr) {
    uint64_t src = decodeOpd(srcOpd);
    Regs *regs = &(cr->regs);

    if (srcOpd->type == REG) {
        // src: register
        // dst: empty
        regs->rsp = regs->rsp - 8;
        write64Dram(
                va2pa(regs->rsp, cr),
                *(uint64_t *) src,
                cr
        );
        nextRip(cr);
        resetCFlags(cr);
        return;
    }
}

static void popHandler(Opd *srcOpd, Opd *dstOpd, Core *cr) {
    uint64_t src = decodeOpd(srcOpd);
    Regs *regs = &(cr->regs);
    if (srcOpd->type == REG) {
        // src: register
        // dst: empty
        uint64_t val = read64Dram(
                va2pa(regs->rsp, cr),
                cr
        );
        regs->rsp = regs->rsp + 8;
        *(uint64_t *) src = val;
        nextRip(cr);
        resetCFlags(cr);
        return;
    }
}

static void leaveHandler(Opd *srcOpd, Opd *dstOpd, Core *cr) {
}

static void callHandler(Opd *srcOpd, Opd *dstOpd, Core *cr) {
    uint64_t src = decodeOpd(srcOpd);
    Regs *regs = &(cr->regs);

    // src: immediate number: virtual address of target function starting
    // dst: empty
    // push the return address
    regs->rsp = regs->rsp - 8;
    write64Dram(
            va2pa(regs->rsp, cr),
            cr->rip + sizeof(char) * MAX_INSTRUCTION_CHAR,
            cr
    );
    // jump to target function address
    cr->rip = src;
    resetCFlags(cr);
}

static void retHandler(Opd *srcOpd, Opd *dstOpd, Core *cr) {
    Regs *regs = &(cr->regs);

    // src: empty
    // dst: empty
    // pop rsp
    uint64_t retAddr = read64Dram(
            va2pa(regs->rsp, cr),
            cr
    );
    regs->rsp = regs->rsp + 8;
    // jump to return address
    cr->rip = retAddr;
    resetCFlags(cr);
}

static void addHandler(Opd *srcOpd, Opd *dstOpd, Core *cr) {
    uint64_t src = decodeOpd(srcOpd);
    uint64_t dst = decodeOpd(dstOpd);

    if (srcOpd->type == REG && dstOpd->type == REG) {
        // src: register (value: int64_t bit map)
        // dst: register (value: int64_t bit map)
        uint64_t val = *(uint64_t *) dst + *(uint64_t *) src;

        // set condition flags

        // update registers
        *(uint64_t *) dst = val;
        // signed and unsigned value follow the same addition. e.g.
        // 5 = 0000000000000101, 3 = 0000000000000011, -3 = 1111111111111101, 5 + (-3) = 0000000000000010
        nextRip(cr);
        return;
    }
}

static void subHandler(Opd *srcOpd, Opd *dstOpd, Core *cr) {
}

static void cmpHandler(Opd *srcOpd, Opd *dstOpd, Core *cr) {
}

static void jneHandler(Opd *srcOpd, Opd *dstOpd, Core *cr) {
}

static void jmpHandler(Opd *srcOpd, Opd *dstOpd, Core *cr) {
}

// instruction cycle is implemented in CPU
// the only exposed interface outside CPU
void instCycle(Core *cr) {
    // fetch: get the instruction string by program counter
    const char *instStr = (const char *) cr->rip;
    slog(DEBUG_INSTRUCTION_CYCLE, "%lx    %s\n", cr->rip, instStr);

    // decode: decode the run-time instruction operands
    Inst inst;
    parseInst(instStr, &inst, cr);

    // execute: get the function pointer or handler by the operator
    InstHandler handler = handlerTable[inst.opt];
    // update CPU and memory according the instruction
    handler(&(inst.src), &(inst.dst), cr);
}


void logReg(Core *cr) {
    if ((DEBUG_VERBOSE_SET & DEBUG_REGISTERS) == 0x0) {
        return;
    }

    Regs regs = cr->regs;

    printf("rax = %16llx\trbx = %16llx\trcx = %16llx\trdx = %16llx\n",
           regs.rax, regs.rbx, regs.rcx, regs.rdx);
    printf("rsi = %16llx\trdi = %16llx\trbp = %16llx\trsp = %16llx\n",
           regs.rsi, regs.rdi, regs.rbp, regs.rsp);
    printf("rip = %16llx\n", cr->rip);
    printf("CF = %u\tZF = %u\tSF = %u\tOF = %u\n",
           cr->cf, cr->zf, cr->sf, cr->of);
}

void logStack(Core *cr) {
    if ((DEBUG_VERBOSE_SET & DEBUG_PRINT_STACK) == 0x0) {
        return;
    }
    Regs regs = cr->regs;

    int n = 10;
    // physical address of the top of stack
    uint64_t *low = (uint64_t *) &pm[va2pa(regs.rsp, cr)];
    // bias to high address
    uint64_t *high = &low[n];

    uint64_t va = regs.rsp + n * 8;

    for (int i = 0; i < 2 * n; ++i) {
        uint64_t *ptr = (uint64_t *) (high - i);
        printf("0x%016llx : %16llx", va, (uint64_t) *ptr);

        if (i == n) {
            printf(" <== rsp");
        }

        va -= 8;

        printf("\n");
    }
}