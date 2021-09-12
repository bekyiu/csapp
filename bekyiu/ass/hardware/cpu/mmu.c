//
// Created by bekyiu on 2021/7/4.
//

#include <stdio.h>
#include <stdlib.h>
#include "../../header/cpu.h"
#include "../../header/memory.h"
#include "../../header/address.h"


uint64_t va2pa(uint64_t vAddr, Core *cr) {
    // return vAddr & (0xffffffffffffffff >> (64 - MAX_INDEX_PHYSICAL_PAGE));
    return vAddr % PHYSICAL_MEMORY_SPACE;
}

// map vaddr to paddr
uint64_t pageWalk(uint64_t vAddrValue) {
    Addr vAddr = {
            ._vAddrValue = vAddrValue
    };

    // remember cr3 point to the simulator's heap
    Pte123 *pgd = (Pte123*) crs.cr3;

    Pte123 *pmd = (Pte123*) pgd[vAddr.vpn1].pAddr;

}