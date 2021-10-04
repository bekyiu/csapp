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

void pageFaultHandler(Pte4 *pte, Addr addr);

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
                    // process page fault
                    pageFaultHandler(&pt[vAddr.vpn4], vAddr);
                }
            } else {
                throw("pmd page fault!");
            }

        } else {
            throw("pud page fault!");
        }
    } else {
        // 4kb
        int pteSize = PAGE_TABLE_ENTRY_NUM * sizeof(Pte123);
        Pte123 *pte = malloc(pteSize);
        memset(pte, 0, pteSize);
        pgd[vAddr.vpn1].present = 1;
        pgd[vAddr.vpn1].pAddr = (uint64_t) pte;
        throw("pgd page fault!");
    }

}

void pageFaultHandler(Pte4 *pte, Addr addr) {
    // 1. try to find a free physical page from dram
    int ppn = -1;
    for (int i = 0; i < MAX_NUM_PHYSICAL_PAGE; ++i) {
        Ppd *ppd = &reversePageMap[i];
        if (ppd->pte4->present == 0) {
            printf("PageFault: use free ppn %d\n", i);
            ppn = i;

            ppd->allocated = 1;
            ppd->dirty = 0;
            ppd->time = 0;
            ppd->pte4 = pte;

            pte->present = 1;
            pte->ppn = ppn;
            pte->dirty = 0;
            return;
        }
    }
    // 2. if no free physical page, select a clean page and overwrite
    // in this case, there is no dram - disk transaction

    // 3. if no free nor clean physical page, select a victim swap out
}


uint64_t va2pa(uint64_t vAddr, Core *cr) {
//    return vAddr % PHYSICAL_MEMORY_SPACE;
    return pageWalk(vAddr);
}