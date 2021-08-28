//
// Created by bekyiu on 2021/7/4.
//

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../../header/cpu.h"
#include "../../header/memory.h"
#include "../../header/common.h"
#include "../../header/address.h"

void cacheWrite(uint64_t pAddrValue, uint8_t data);
uint8_t cacheRead(uint64_t pAddrValue);

// little-endian
void write64Dram(uint64_t pAddr, uint64_t data, Core *cr) {
    if (DEBUG_ENABLE_SRAM_CACHE == 1) {
        // try to write uint64_t to SRAM cache
        for (int i = 0; i < 8; ++i) {
            cacheWrite(pAddr + i, (data >> (8 * i)) & 0xff);
        }
    } else {
        // little-endian
        for (int i = 0; i < 8; ++i) {
            pm[pAddr + i] = (data >> (8 * i)) & 0xff;
        }
    }

}

uint64_t read64Dram(uint64_t pAddr, Core *cr) {
    if (DEBUG_ENABLE_SRAM_CACHE == 1) {
        // try to load uint64_t from SRAM cache
        uint64_t val = 0;
        for (int i = 0; i < 8; ++i) {
            uint64_t byte = cacheRead(pAddr + i);
            val += (byte << (8 * i));
        }
        return val;
    } else {
        // little-endian
        uint64_t val = 0;
        for (int i = 0; i < 8; ++i) {
            uint64_t byte = (uint64_t) pm[pAddr + i];
            val += (byte << (8 * i));
        }
        return val;
    }
}


void writeInstDram(uint64_t pAddr, const char *instStr, Core *cr) {
    size_t len = strlen(instStr);
    if (len > MAX_INSTRUCTION_CHAR) {
        throw("instruction error: %s\n", instStr);
    }
    for (int i = 0; i < MAX_INSTRUCTION_CHAR; ++i) {
        if (i < len) {
            pm[pAddr + i] = instStr[i];
        } else {
            pm[pAddr + i] = '\0';
        }
    }
}

void readInstDram(uint64_t pAddr, char *buf, Core *cr) {
    for (int i = 0; i < MAX_INSTRUCTION_CHAR; ++i) {
        buf[i] = pm[pAddr + i];
    }
}

// read a cache line from dram
void readCacheLine(uint64_t pAddr, uint8_t *block) {
    uint64_t base = (pAddr >> CACHE_OFFSET_LENGTH) << CACHE_OFFSET_LENGTH;
    for (int i = 0; i < (1 << CACHE_OFFSET_LENGTH); ++i) {
        block[i] = pm[base + i];
    }
}

// write a cache line to dram
void writeCacheLine(uint64_t pAddr, uint8_t *block) {
    // write hole block
    uint64_t base = (pAddr >> CACHE_OFFSET_LENGTH) << CACHE_OFFSET_LENGTH;
    for (int i = 0; i < (1 << CACHE_OFFSET_LENGTH); ++i) {
        pm[base + i] = block[i];
    }
}