#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include "../ass/header/cpu.h"
#include "../ass/header/common.h"
#include "../ass/header/memory.h"

static void testAddFunctionCallAndComputation();

static void testStr2Uint();

// symbols from isa and sram
void logReg(Core *cr);

void logStack(Core *cr);

void testParsingOperand();

void testParsingInstruction();

void testSumRecursiveCondition();


int mainHardware() {
//    testAddFunctionCallAndComputation();
//    testStr2Uint();
//    testParsingOperand();
//    testParsingInstruction();
    testSumRecursiveCondition();
    return 0;
}

void testSumRecursiveCondition() {
    ACTIVE_CORE = 0x0;
    Core *cr = (Core *) &cores[ACTIVE_CORE];

    // init state
    cr->regs.rax = 0x8000630;
    cr->regs.rbx = 0x0;
    cr->regs.rcx = 0x8000650;
    cr->regs.rdx = 0x7ffffffee328;
    cr->regs.rsi = 0x7ffffffee318;
    cr->regs.rdi = 0x1;
    cr->regs.rbp = 0x7ffffffee230;
    cr->regs.rsp = 0x7ffffffee220;

    cr->flags._value = 0;

    write64Dram(va2pa(0x7ffffffee230, cr), 0x0000000008000650, cr);    // rbp
    write64Dram(va2pa(0x7ffffffee228, cr), 0x0000000000000000, cr);
    write64Dram(va2pa(0x7ffffffee220, cr), 0x00007ffffffee310, cr);    // rsp

    char assembly[19][MAX_INSTRUCTION_CHAR] = {
            "push   %rbp",              // 0                0x400000
            "mov    %rsp,%rbp",         // 1                0x400040
            "sub    $0x10,%rsp",        // 2                0x400080
            "mov    %rdi,-0x8(%rbp)",   // 3                0x4000c0
            "cmpq   $0x0,-0x8(%rbp)",   // 4                0x400100
            "jne    0x400200",          // 5: jump to 8     0x400140
            "mov    $0x0,%eax",         // 6                0x400180
            "jmp    0x400380",          // 7: jump to 14    0x4001c0
            "mov    -0x8(%rbp),%rax",   // 8                0x400200
            "sub    $0x1,%rax",         // 9
            "mov    %rax,%rdi",         // 10
            "callq  0x00400000",        // 11
            "mov    -0x8(%rbp),%rdx",   // 12
            "add    %rdx,%rax",         // 13
            "leaveq ",                  // 14
            "retq   ",                  // 15
            "mov    $0x3,%edi",         // 16
            "callq  0x00400000",        // 17
            "mov    %rax,-0x8(%rbp)",   // 18
    };

    // copy to physical memory
    for (int i = 0; i < 19; ++i) {
        writeInstDram(va2pa(i * 0x40 + 0x00400000, cr), assembly[i], cr);
    }
    cr->rip = 0x40 * 16 + 0x00400000;

    printf("begin\n");
    int time = 0;
    while ((cr->rip <= 18 * 0x40 + 0x00400000) && time < 100) {
        instCycle(cr);
        logReg(cr);
        logStack(cr);
        time++;
    }

    // gdb state ret from func
    int match = 1;
    match = match && cr->regs.rax == 0x6;
    match = match && cr->regs.rbx == 0x0;
    match = match && cr->regs.rcx == 0x8000650;
    match = match && cr->regs.rdx == 0x3;
    match = match && cr->regs.rsi == 0x7ffffffee318;
    match = match && cr->regs.rdi == 0x0;
    match = match && cr->regs.rbp == 0x7ffffffee230;
    match = match && cr->regs.rsp == 0x7ffffffee220;

    if (match) {
        printf("register match\n");
    } else {
        printf("register mismatch\n");
    }

    match = match && (read64Dram(va2pa(0x7ffffffee230, cr), cr) == 0x0000000008000650); // rbp
    match = match && (read64Dram(va2pa(0x7ffffffee228, cr), cr) == 0x0000000000000006);
    match = match && (read64Dram(va2pa(0x7ffffffee220, cr), cr) == 0x00007ffffffee310); // rsp

    if (match) {
        printf("memory match\n");
    } else {
        printf("memory mismatch\n");
    }
}

void testStr2Uint() {
    char *nums[] = {
            "0",
            "-0",
            "0x0",
            "1234",
            "0x1234",
            "0xabcd",
            "-0xabcd",
            "-1234",
            "2147483647",
            "-2147483648",
            "0x8000000000000000",
            "0xffffffffffffffff",
            "0x1ffffffffffffffff", // bug, no hint error, but has been overflow
    };

    for (int i = 0; i < 13; ++i) {
        printf("%s => %llx\n", nums[i], str2uint(nums[i]));
    }
}

void testAddFunctionCallAndComputation() {

    ACTIVE_CORE = 0x0;

    Core *ac = (Core *) &cores[ACTIVE_CORE];
    Regs *reg = &(ac->regs);

    // init
    reg->rax = 0x12340000;
    reg->rbx = 0x0;
    reg->rcx = 0x8000660;
    reg->rdx = 0xabcd;
    reg->rsi = 0x7ffffffee2f8;
    reg->rdi = 0x1;
    reg->rbp = 0x7ffffffee210;
    reg->rsp = 0x7ffffffee1f0;

    ac->flags._value = 0;

    write64Dram(va2pa(0x7ffffffee210, ac), 0x08000660, ac); // rbp
    write64Dram(va2pa(0x7ffffffee208, ac), 0x0, ac);
    write64Dram(va2pa(0x7ffffffee200, ac), 0xabcd, ac);
    write64Dram(va2pa(0x7ffffffee1f8, ac), 0x12340000, ac);
    write64Dram(va2pa(0x7ffffffee1f0, ac), 0x08000660, ac); // rsp


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
            "callq  0x00400000",        // 13
            "mov    %rax,-0x8(%rbp)",   // 14
    };


    // copy to physical memory
    for (int i = 0; i < 15; ++i) {
        writeInstDram(va2pa(i * 0x40 + 0x00400000, ac), assembly[i], ac);
    }
    ac->rip = 0x40 * 11 + 0x00400000;


    // run
    for (int i = 0; i < 15; ++i) {
        logReg(ac);
        logStack(ac);
        instCycle(ac);
//        puts("");
    }
    logReg(ac);
    logStack(ac);

    // verify
    int match = 1;

    match = match && (reg->rax == 0x1234abcd);
    match = match && (reg->rbx == 0x0);
    match = match && (reg->rcx == 0x8000660);
    match = match && (reg->rdx == 0x12340000);
    match = match && (reg->rsi == 0xabcd);
    match = match && (reg->rdi == 0x12340000);
    match = match && (reg->rbp == 0x7ffffffee210);
    match = match && (reg->rsp == 0x7ffffffee1f0);

    if (match == 1) {
        printf("register match\n");
    } else {
        printf("register not match\n");
    }

    match = match && (read64Dram(va2pa(0x7ffffffee210, ac), ac) == 0x08000660);     // rbp
    match = match && (read64Dram(va2pa(0x7ffffffee208, ac), ac) == 0x1234abcd);
    match = match && (read64Dram(va2pa(0x7ffffffee200, ac), ac) == 0xabcd);
    match = match && (read64Dram(va2pa(0x7ffffffee1f8, ac), ac) == 0x12340000);
    match = match && (read64Dram(va2pa(0x7ffffffee1f0, ac), ac) == 0x08000660);

    if (match == 1) {
        printf("memory match\n");
    } else {
        printf("memory not match\n");
    }
}