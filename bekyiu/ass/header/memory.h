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
// 16 physical memory pages
uint8_t pm[PHYSICAL_MEMORY_SPACE];

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
