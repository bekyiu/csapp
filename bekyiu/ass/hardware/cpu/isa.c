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

// lookup table
static const char *regNames[72] = {
        "%rax", "%eax", "%ax", "%ah", "%al",
        "%rbx", "%ebx", "%bx", "%bh", "%bl",
        "%rcx", "%ecx", "%cx", "%ch", "%cl",
        "%rdx", "%edx", "%dx", "%dh", "%dl",
        "%rsi", "%esi", "%si", "%sih", "%sil",
        "%rdi", "%edi", "%di", "%dih", "%dil",
        "%rbp", "%ebp", "%bp", "%bph", "%bpl",
        "%rsp", "%esp", "%sp", "%sph", "%spl",
        "%r8", "%r8d", "%r8w", "%r8b",
        "%r9", "%r9d", "%r9w", "%r9b",
        "%r10", "%r10d", "%r10w", "%r10b",
        "%r11", "%r11d", "%r11w", "%r11b",
        "%r12", "%r12d", "%r12w", "%r12b",
        "%r13", "%r13d", "%r13w", "%r13b",
        "%r14", "%r14d", "%r14w", "%r14b",
        "%r15", "%r15d", "%r15w", "%r15b",
};

// give a reg name, return it's addr
static uint64_t regAddr(const char *str, Core *cr) {
    // lookup table
    Regs *regs = &(cr->regs);
    uint64_t regAddrs[72] = {
            (uint64_t) &(regs->rax), (uint64_t) &(regs->eax), (uint64_t) &(regs->ax), (uint64_t) &(regs->ah),
            (uint64_t) &(regs->al),
            (uint64_t) &(regs->rbx), (uint64_t) &(regs->ebx), (uint64_t) &(regs->bx), (uint64_t) &(regs->bh),
            (uint64_t) &(regs->bl),
            (uint64_t) &(regs->rcx), (uint64_t) &(regs->ecx), (uint64_t) &(regs->cx), (uint64_t) &(regs->ch),
            (uint64_t) &(regs->cl),
            (uint64_t) &(regs->rdx), (uint64_t) &(regs->edx), (uint64_t) &(regs->dx), (uint64_t) &(regs->dh),
            (uint64_t) &(regs->dl),
            (uint64_t) &(regs->rsi), (uint64_t) &(regs->esi), (uint64_t) &(regs->si), (uint64_t) &(regs->sih),
            (uint64_t) &(regs->sil),
            (uint64_t) &(regs->rdi), (uint64_t) &(regs->edi), (uint64_t) &(regs->di), (uint64_t) &(regs->dih),
            (uint64_t) &(regs->dil),
            (uint64_t) &(regs->rbp), (uint64_t) &(regs->ebp), (uint64_t) &(regs->bp), (uint64_t) &(regs->bph),
            (uint64_t) &(regs->bpl),
            (uint64_t) &(regs->rsp), (uint64_t) &(regs->esp), (uint64_t) &(regs->sp), (uint64_t) &(regs->sph),
            (uint64_t) &(regs->spl),
            (uint64_t) &(regs->r8), (uint64_t) &(regs->r8d), (uint64_t) &(regs->r8w), (uint64_t) &(regs->r8b),
            (uint64_t) &(regs->r9), (uint64_t) &(regs->r9d), (uint64_t) &(regs->r9w), (uint64_t) &(regs->r9b),
            (uint64_t) &(regs->r10), (uint64_t) &(regs->r10d), (uint64_t) &(regs->r10w), (uint64_t) &(regs->r10b),
            (uint64_t) &(regs->r11), (uint64_t) &(regs->r11d), (uint64_t) &(regs->r11w), (uint64_t) &(regs->r11b),
            (uint64_t) &(regs->r12), (uint64_t) &(regs->r12d), (uint64_t) &(regs->r12w), (uint64_t) &(regs->r12b),
            (uint64_t) &(regs->r13), (uint64_t) &(regs->r13d), (uint64_t) &(regs->r13w), (uint64_t) &(regs->r13b),
            (uint64_t) &(regs->r14), (uint64_t) &(regs->r14d), (uint64_t) &(regs->r14w), (uint64_t) &(regs->r14b),
            (uint64_t) &(regs->r15), (uint64_t) &(regs->r15d), (uint64_t) &(regs->r15w), (uint64_t) &(regs->r15b),
    };
    for (int i = 0; i < 72; ++i) {
        if (strcmp(str, regNames[i]) == 0) {
            // now we know that i is the index inside regNames
            return regAddrs[i];
        }
    }
    printf("parse register %s error\n", str);
    exit(0);
}

// parse access memory operand, build it's Opd object
static void parseAccessMemoryOpd(const char *str, Opd *opd, Core *cr, size_t len) {
    // format: imm(reg1,reg2,scale)
    char imm[64] = {'\0'};
    int immLen = 0;
    char scale[64] = {'\0'};
    int scaleLen = 0;
    char reg1[64] = {'\0'};
    int reg1Len = 0;
    char reg2[64] = {'\0'};
    int reg2Len = 0;

    // number of '(' or ')'
    int ca = 0;
    // number of ','
    int cb = 0;

    for (size_t i = 0; i < len; i++) {
        char ch = str[i];
        if (ch == '(' || ch == ')') {
            ca++;
            continue;
        }
        if (ch == ',') {
            cb++;
            continue;
        }

        // x(... now parsing x
        if (ca == 0) {
            imm[immLen++] = ch;
            continue;
        }
        if (ca == 1) {
            // a(x,...
            if (cb == 0) {
                reg1[reg1Len++] = ch;
                continue;
            }
            // a(a,x,...
            if (cb == 1) {
                reg2[reg2Len++] = ch;
                continue;
            }
            // a(a,a,x...
            if (cb == 2) {
                scale[scaleLen++] = ch;
                continue;
            }
        }
    }

    // set opd value
    if (immLen > 0) {
        opd->imm = str2uint(imm);
    }
    if (reg1Len > 0) {
        opd->reg1 = regAddr(reg1, cr);
    }
    if (reg2Len > 0) {
        opd->reg2 = regAddr(reg2, cr);
    }
    if (scaleLen > 0) {
        uint64_t s = str2uint(scale);
        if (s != 0x1 && s != 0x2 && s != 0x4 && s != 0x8) {
            printf("%s is not a legal scale\n", scale);
            exit(0);
        }
        opd->scale = s;
    }
    // set opd type
    if (ca == 0) {
        opd->type = MEM_IMM;
        return;
    }
    if (ca == 1) {
        printf("%s is not a legal operand, miss ')'\n", str);
        exit(0);
    }
    if (cb == 0) {
        if (immLen > 0) {
            opd->type = MEM_IMM_REG;
            return;
        }
        opd->type = MEM_REG;
        return;
    }
    // now ca == 2
    if (cb == 1) {
        if (immLen > 0) {
            opd->type = MEM_IMM_REG1_REG2;
            return;
        }
        opd->type = MEM_REG1_REG2;
        return;
    }
    // now cb == 2
    if (immLen > 0) {
        if (reg1Len > 0) {
            opd->type = MEM_IMM_REG1_REG2_S;
            return;
        }
        opd->type = MEM_IMM_REG2_S;
        return;
    }
    // now cb == 2 and immLen == 0
    if (reg1Len > 0) {
        opd->type = MEM_REG1_REG2_S;
        return;
    }
    opd->type = MEM_REG2_S;
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
        opd->type = REG;
        opd->reg1 = regAddr(str, cr);
        return;
    }
    // memory
    parseAccessMemoryOpd(str, opd, cr, len);
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
    cr->flags._value = 0;
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
    Flags flags = cr->flags;
    printf("rax = %16llx\trbx = %16llx\trcx = %16llx\trdx = %16llx\n",
           regs.rax, regs.rbx, regs.rcx, regs.rdx);
    printf("rsi = %16llx\trdi = %16llx\trbp = %16llx\trsp = %16llx\n",
           regs.rsi, regs.rdi, regs.rbp, regs.rsp);
    printf("rip = %16llx\n", cr->rip);
    printf("CF = %u\tZF = %u\tSF = %u\tOF = %u\n",
           flags.cf, flags.zf, flags.sf, flags.of);
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


void testParsingOperand() {
    ACTIVE_CORE = 0x0;
    Core *ac = (Core *) &cores[ACTIVE_CORE];

    const char *strs[11] = {
            "$0x1234",
            "%rax",
            "0xabcd",
            "(%rsp)",
            "0xabcd(%rsp)",
            "(%rsp,%rbx)",
            "0xabcd(%rsp,%rbx)",
            "(,%rbx,8)",
            "0xabcd(,%rbx,8)",
            "(%rsp,%rbx,8)",
            "0xabcd(%rsp,%rbx,8)",
    };

    printf("rax %p\n", &(ac->regs.rax));
    printf("rsp %p\n", &(ac->regs.rsp));
    printf("rbx %p\n", &(ac->regs.rbx));

    for (int i = 0; i < 11; ++i) {
        Opd opd;
        parseOpd(strs[i], &opd, ac);

        printf("\n%s\n", strs[i]);
        printf("od enum type: %d, is correct: %d\n", opd.type, opd.type == i + 1);
        printf("od imm: %llx\n", opd.imm);
        printf("od reg1: %llx\n", opd.reg1);
        printf("od reg2: %llx\n", opd.reg2);
        printf("od scale: %llx\n", opd.scale);
    }
}