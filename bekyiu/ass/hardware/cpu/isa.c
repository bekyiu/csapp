//
// Created by bekyiu on 2021/7/4.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../header/cpu.h"
#include "../../header/memory.h"
#include "../../header/common.h"
#include "../../header/instruction.h"


/*======================================*/
/*      parse assembly instruction      */
/*======================================*/

// functions to map the string assembly code to inst_t instance
static void parseInst(const char *str, Inst *inst, Core *cr);

static void parseOpd(const char *str, Opd *opd, Core *cr);

static void parseOpt(const char *str, Opt *opt, Core *cr);

static uint64_t decodeOpd(Opd *opd);

// return the real operand
static uint64_t decodeOpd(Opd *opd) {
    if (opd->type == IMM) {
        return opd->imm;
        // immediate signed number can be negative: convert to bitmap
        // return *((uint64_t *) &opd->imm);
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
    throw("parse register %s error\n", str);
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
            throw("%s is not a legal scale\n", scale);
        }
        opd->scale = s;
    }
    // set opd type
    if (ca == 0) {
        opd->type = MEM_IMM;
        return;
    }
    if (ca == 1) {
        throw("%s is not a legal operand, miss ')'\n", str);
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
    char opt[64] = {'\0'};
    int optLen = 0;
    char src[64] = {'\0'};
    int srcLen = 0;
    char dst[64] = {'\0'};
    int dstLen = 0;

    int state = 0;
    int ca = 0; // number of '(' or ')'

    // status:      0     1     2     3     4         5     6     7
    // inst format: _    opt    _    opd    _    ,    _    opd    _
    // use '_' to represent space
    for (int i = 0; i < strlen(str); ++i) {
        char ch = str[i];
        if (ch == '(' || ch == ')') {
            ca++;
        }
        if (state == 0 && ch != ' ') {
            state = 1;
        } else if (state == 1 && ch == ' ') {
            state = 2;
            continue;
        } else if (state == 2 && ch != ' ') {
            state = 3;
        } else if (state == 3 && ch == ' ') {
            state = 4;
            continue;
        } else if (ch == ',' && (ca == 2 || ca == 0)) {
            state = 5;
            continue;
        } else if (state == 5 && ch != ' ') {
            state = 6;
        } else if (state == 6 && ch == ' ') {
            state = 7;
        }

        // fill in the corresponding str
        if (state == 1) {
            opt[optLen++] = ch;
            continue;
        }
        if (state == 3) {
            src[srcLen++] = ch;
            continue;
        }
        if (state == 6) {
            dst[dstLen++] = ch;
            continue;
        }
    }

    parseOpd(src, &(inst->src), cr);
    parseOpd(dst, &(inst->dst), cr);
    parseOpt(opt, &(inst->opt), cr);

    slog(DEBUG_PARSE_INST, "[%s (%d)] [%s (%d)] [%s (%d)]\n",
         opt, inst->opt, src, inst->src.type, dst, inst->dst.type);
}

// parse an operator string to a integer
static void parseOpt(const char *str, Opt *opt, Core *cr) {
    if (startsWith("mov", str)) {
        *opt = MOV;
        return;
    }
    if (startsWith("push", str)) {
        *opt = PUSH;
        return;
    }
    if (startsWith("pop", str)) {
        *opt = POP;
        return;
    }
    if (startsWith("leave", str)) {
        *opt = LEAVE;
        return;
    }
    if (startsWith("call", str)) {
        *opt = CALL;
        return;
    }
    if (startsWith("ret", str)) {
        *opt = RET;
        return;
    }
    if (startsWith("add", str)) {
        *opt = ADD;
        return;
    }
    if (startsWith("sub", str)) {
        *opt = SUB;
        return;
    }
    if (startsWith("cmp", str)) {
        *opt = CMP;
        return;
    }
    if (startsWith("jne", str)) {
        *opt = JNE;
        return;
    }
    if (startsWith("jmp", str)) {
        *opt = JMP;
        return;
    }
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
static inline void clearFlags(Core *cr) {
    cr->flags._value = 0;
}

static void setFlags(uint64_t srcVal, uint64_t dstVal, uint64_t val, Core *cr) {
    uint8_t srcSign = (srcVal >> 63) & 0x1;
    uint8_t dstSign = (dstVal >> 63) & 0x1;
    uint8_t valSign = (val >> 63) & 0x1;

    // unsigned overflow: two unsigned add but result be smaller
    cr->flags.cf = srcVal > val;
    cr->flags.zf = (val == 0);
    cr->flags.sf = valSign;
    // signed overflow: same sign bit operand add but result get different sign bit
    cr->flags.of = (srcSign == 0 && dstSign == 0 && valSign == 1) ||
                   (srcSign == 1 && dstSign == 1 && valSign == 0);
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
        clearFlags(cr);
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
        clearFlags(cr);
        return;
    }
    if (srcOpd->type >= MEM_IMM && dstOpd->type == REG) {
        // src: virtual address
        // dst: register
        *(uint64_t *) dst = read64Dram(
                va2pa(src, cr),
                cr);
        nextRip(cr);
        clearFlags(cr);
        return;
    }
    if (srcOpd->type == IMM && dstOpd->type == REG) {
        // src: immediate number (uint64_t bit map)
        // dst: register
        *(uint64_t *) dst = src;
        nextRip(cr);
        clearFlags(cr);
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
        clearFlags(cr);
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
        clearFlags(cr);
        return;
    }
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
    clearFlags(cr);
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
    clearFlags(cr);
}

static void addHandler(Opd *srcOpd, Opd *dstOpd, Core *cr) {
    uint64_t src = decodeOpd(srcOpd);
    uint64_t dst = decodeOpd(dstOpd);

    if (srcOpd->type == REG && dstOpd->type == REG) {
        // src: register (value: int64_t bit map)
        // dst: register (value: int64_t bit map)
        uint64_t dstVal = *(uint64_t *) dst;
        uint64_t srcVal = *(uint64_t *) src;
        uint64_t val = dstVal + srcVal;

        // set condition flags
        setFlags(srcVal, dstVal, val, cr);

        // update registers
        *(uint64_t *) dst = val;
        // signed and unsigned value follow the same addition. e.g.
        // 5 = 0000000000000101, 3 = 0000000000000011, -3 = 1111111111111101, 5 + (-3) = 0000000000000010
        nextRip(cr);
        return;
    }
}

static void subHandler(Opd *srcOpd, Opd *dstOpd, Core *cr) {
    uint64_t src = decodeOpd(srcOpd);
    uint64_t dst = decodeOpd(dstOpd);

    if (srcOpd->type == IMM && dstOpd->type == REG) {
        uint64_t dstVal = *(uint64_t *) dst;
        uint64_t srcVal = ~src + 1;
        // dst = dst - src = dst + (-src)
        uint64_t val = dstVal + srcVal;

        // set condition flags
        setFlags(dstVal, srcVal, val, cr);

        // update registers
        *(uint64_t *) dst = val;
        nextRip(cr);
        return;
    }
}

static void cmpHandler(Opd *srcOpd, Opd *dstOpd, Core *cr) {
    uint64_t src = decodeOpd(srcOpd);
    uint64_t dst = decodeOpd(dstOpd);

    if (srcOpd->type == IMM && dstOpd->type >= MEM_IMM) {
        uint64_t dstVal = ~read64Dram(va2pa(dst, cr), cr) + 1;
        uint64_t srcVal = src;
        // dst - src = dst + (-src)
        uint64_t val = dstVal + srcVal;

        // set condition flags
        setFlags(dstVal, srcVal, val, cr);

        // update registers
        nextRip(cr);
        return;
    }
}

static void jneHandler(Opd *srcOpd, Opd *dstOpd, Core *cr) {
    // jne    0x400200
    // now 0x400200 will be parsed as a memory instruction
    // but it is actually a immediate number, so we do not check the instruction type
    uint64_t src = decodeOpd(srcOpd);

    if (cr->flags.zf) {
        nextRip(cr);
    } else {
        cr->rip = src;
    }
    clearFlags(cr);
}

static void jmpHandler(Opd *srcOpd, Opd *dstOpd, Core *cr) {
    // also do not check the instruction type
    uint64_t src = decodeOpd(srcOpd);
    cr->rip = src;
    clearFlags(cr);
}

static void leaveHandler(Opd *srcOpd, Opd *dstOpd, Core *cr) {
    Regs *regs = &(cr->regs);
    // mov %rbp, %rsp
    regs->rsp = regs->rbp;
    // pop %rbp
    uint64_t val = read64Dram(
            va2pa(regs->rsp, cr),
            cr
    );
    regs->rsp = regs->rsp + 8;
    regs->rbp = val;
    nextRip(cr);
    clearFlags(cr);
}

// instruction cycle is implemented in CPU
// the only exposed interface outside CPU
void instCycle(Core *cr) {
    // fetch: get the instruction string by program counter
    char instStr[MAX_INSTRUCTION_CHAR];
    readInstDram(va2pa(cr->rip, cr), instStr, cr);
    slog(DEBUG_INSTRUCTION_CYCLE, "%lx    %s\n", cr->rip, instStr);

    // decode: decode the run-time instruction operands
    Inst inst;
    parseInst(instStr, &inst, cr);

    // execute: get the function pointer or handler by the operator
    InstHandler handler = handlerTable[inst.opt];
    // update CPU and memory according the instruction
    handler(&(inst.src), &(inst.dst), cr);
}

void testParsingInstruction() {
    ACTIVE_CORE = 0x0;
    Core *ac = (Core *) &cores[ACTIVE_CORE];

    char assembly[15][MAX_INSTRUCTION_CHAR] = {
            "push   %rbp",              // 0
            "mov    %rsp,%rbp",         // 1
            "mov    %rdi,-0x18(%rbp)",  // 2
            "mov    %rsi,-0x20(%rbp)",  // 3
            "mov    -0x18(%rbp),%rdx",  // 4
            "mov    -0x20(%rbp),%rax",  // 5
            "add    %rdx,%rax",         // 6
            "mov    %rax,-0x8(%rbp)",   // 7
            "mov    -0x8(%rbp),%rax",   // 8
            "pop    %rbp",              // 9
            "retq",                     // 10
            "mov    %rdx,%rsi",         // 11
            "mov    %rax,%rdi",         // 12
            "callq  0",                 // 13
            "mov    %rax,-0x8(%rbp)",   // 14
    };

    Inst inst;
    for (int i = 0; i < 15; ++i) {
        parseInst(assembly[i], &inst, ac);
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