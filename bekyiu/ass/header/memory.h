//
// Created by bekyiu on 2021/7/4.
//

#ifndef CSAPP_MEMORY_H
#define CSAPP_MEMORY_H


#include <stdint.h>
#include "cpu.h"

/*======================================*/
/*      physical memory on dram chips   */
/*======================================*/

// physical memory space is decided by the physical address
// in this simulator, there are 4 + 6 + 6 = 16 bit physical address
// then the physical space is (1 << 16) = 65536 / 2 ^ 16
// total 16 physical memory
#define PHYSICAL_MEMORY_SPACE   65536
#define MAX_INDEX_PHYSICAL_PAGE 15

// physical memory
// only used for user process
uint8_t pm[PHYSICAL_MEMORY_SPACE];

#define PAGE_TABLE_ENTRY_NUM 512


// page table entry struct
// 8 bytes = 64 bits
typedef union {
    uint64_t _pteValue;

    struct {
        uint64_t present: 1;
        uint64_t readonly: 1;
        uint64_t userMode: 1;
        uint64_t writeThough: 1;
        uint64_t cacheDisabled: 1;
        uint64_t reference: 1;
        uint64_t unused6: 1;
        uint64_t smallPage: 1;
        uint64_t global: 1;
        uint64_t unused9_11: 3;
        /*

        next level page table

        uint64_t pAddr              : 40;
        uint64_t unused52_62        : 10;
        for malloc, a virtual address on heap is 48 bits
        for real world, a physical page number is 40 bits
        */
        uint64_t pAddr: 50;   // virtual address (48 bits) on simulator's heap
        uint64_t xDisabled: 1;
    };

    struct {
        uint64_t _present: 1;
        uint64_t swapId: 63;   // disk address
    };
} Pte123; // PGD, PUD, PMD

// 8 bytes = 64 bits
typedef union {
    uint64_t _pteValue;

    struct {
        uint64_t present: 1;
        uint64_t readonly: 1;
        uint64_t userMode: 1;
        uint64_t writeThough: 1;
        uint64_t cacheDisabled: 1;
        uint64_t reference: 1;
        uint64_t dirty: 1;    // dirty bit - 1: dirty; 0: clean
        uint64_t zero7: 1;
        uint64_t global: 1;
        uint64_t unused9_11: 3;

        // physical page number
        uint64_t ppn: 40;

        uint64_t unused52_62: 10;
        uint64_t xDisabled: 1;
    };

    struct {
        uint64_t _present: 1;
        uint64_t swapId: 63;   // disk address
    };
} Pte4;   // PT

/*======================================*/
/*      memory R/W                      */
/*======================================*/

// used by instructions: read or write uint64_t to DRAM
uint64_t read64Dram(uint64_t pAddr, Core *cr);

void write64Dram(uint64_t pAddr, uint64_t data, Core *cr);

void writeInstDram(uint64_t pAddr, const char *instStr, Core *cr);

void readInstDram(uint64_t pAddr, char *buf, Core *cr);

void readCacheLine(uint64_t pAddr, uint8_t *block);

void writeCacheLine(uint64_t pAddr, uint8_t *block);

#endif //CSAPP_MEMORY_H
