#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include "ass/header/cpu.h"
#include "ass/header/common.h"
#include "ass/header/memory.h"


static void testAddFunctionCallAndComputation();

static void testStr2Uint();

// symbols from isa and sram
void logReg(Core *cr);

void logStack(Core *cr);

int main() {
//    testAddFunctionCallAndComputation();
    testStr2Uint();
    return 0;
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
    Reg *reg = &(ac->reg);

    // init
    reg->rax = 0x12340000;
    reg->rbx = 0x0;
    reg->rcx = 0x8000660;
    reg->rdx = 0xabcd;
    reg->rsi = 0x7ffffffee2f8;
    reg->rdi = 0x1;
    reg->rbp = 0x7ffffffee210;
    reg->rsp = 0x7ffffffee1f0;

    ac->cf = 0;
    ac->zf = 0;
    ac->sf = 0;
    ac->of = 0;

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
            "callq  0",                 // 13, the addr will be replace during the run time
            "mov    %rax,-0x8(%rbp)",   // 14
    };

    ac->rip = (uint64_t) &assembly[11];
    sprintf(assembly[13], "callq  $%p", &assembly[0]);

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