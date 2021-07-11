//
// Created by bekyiu on 2021/7/4.
//

#include <stdarg.h>
#include <stdio.h>
#include "../header/common.h"
#include "../header/cpu.h"
#include "../header/memory.h"

// switch log, wrapper of stdio printf
// controlled by the debug verbose bit set
uint64_t slog(uint64_t openSet, const char *format, ...) {
    if ((openSet & DEBUG_VERBOSE_SET) == 0x0) {
        return 0x1;
    }

    // implementation of std printf()
    va_list argPtr;
    va_start(argPtr, format);
    vfprintf(stdout, format, argPtr);
    va_end(argPtr);

    return 0x0;
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