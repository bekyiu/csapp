//
// Created by bekyiu on 2021/7/4.
//

#include <stdint.h>
#include <stdio.h>
#include "../../header/cpu.h"
#include "../../header/memory.h"
#include "../../header/common.h"

// little-endian
void write64Dram(uint64_t pAddr, uint64_t data, Core *cr) {
    if (DEBUG_ENABLE_SRAM_CACHE == 1) {
        // try to write uint64_t to SRAM cache
        // little-endian
    } else {
        for (int i = 0; i < 8; ++i) {
            pm[pAddr + i] = (data >> (8 * i)) & 0xff;
        }
    }

}

uint64_t read64Dram(uint64_t pAddr, Core *cr) {
    if (DEBUG_ENABLE_SRAM_CACHE == 1) {
        // try to load uint64_t from SRAM cache
        // little-endian
    } else {
        uint64_t val = 0;
        for (int i = 0; i < 8; ++i) {
            uint64_t byte = (uint64_t) pm[pAddr + i];
            val += (byte << (8 * i));
        }
        return val;
    }
}