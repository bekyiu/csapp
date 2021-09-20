//
// Created by bekyiu on 2021/7/4.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../header/cpu.h"
#include "../../header/memory.h"
#include "../../header/address.h"
#include "../../header/common.h"

// map vaddr to paddr
uint64_t pageWalk(uint64_t vAddrValue) {
    Addr vAddr = {
            ._vAddrValue = vAddrValue
    };

    // remember cr3 point to the simulator's heap
    Pte123 *pgd = (Pte123 *) crs.cr3;

    if (pgd[vAddr.vpn1].present == 1) {
        Pte123 *pud = (Pte123 *) pgd[vAddr.vpn1].pAddr;
        if (pud[vAddr.vpn2].present == 1) {
            Pte123 *pmd = (Pte123 *) pud[vAddr.vpn2].pAddr;
            if (pmd[vAddr.vpn3].present == 1) {
                Pte4 *pt = (Pte4 *) pmd[vAddr.vpn3].pAddr;
                if (pt[vAddr.vpn4].present == 1) {
                    Addr pAddr = {
                            .ppn = pt[vAddr.vpn4].ppn,
                            .ppo = vAddr.ppo,
                    };
                    return pAddr._addrValue;
                } else {
                    throw("page fault!");
                }
            } else {
                throw("pmd page fault!");
            }

        } else {
            throw("pud page fault!");
        }
    } else {
        // todo page fault

        // 4kb
        int pteSize = PAGE_TABLE_ENTRY_NUM * sizeof(Pte123);
        Pte123 *pte = malloc(pteSize);
        memset(pte, 0, pteSize);
        pgd[vAddr.vpn1].present = 1;
        pgd[vAddr.vpn1].pAddr = (uint64_t) pte;
        throw("pgd page fault!");
    }

}


uint64_t va2pa(uint64_t vAddr, Core *cr) {
//    return vAddr % PHYSICAL_MEMORY_SPACE;
    return pageWalk(vAddr);
}