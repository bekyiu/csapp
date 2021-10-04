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
// 65536 / 4kb
#define MAX_NUM_PHYSICAL_PAGE 16

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

// physical page descriptor
typedef struct {
    int allocated;
    int dirty;
    int time;

    Pte4 *pte4
} Ppd;

/*
 * 物理内存作为磁盘的缓存，如果当前的进程[t]发生了缺页，内核决定牺牲进程[j]物理页[i]，
 * 那么就要将物理页[i]的数据写入磁盘，然后才将进程[t]的数据写入物理页[i]。
 * 同时，内核需要修改进程[j]的页表，使得[j]仍能通过页表索引到磁盘上的数据。
 *
 * 也就是说，我们需要通过物理页的页号（Page Number）[i]查找到进程[j]的页表项，
 * 这个过程就是页表的反向映射（Reversed Mapping），是从物理地址到虚拟地址的映射。
 *
 * ppn map to pte
 */
Ppd reversePageMap[MAX_NUM_PHYSICAL_PAGE];

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
