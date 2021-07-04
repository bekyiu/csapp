//
// Created by bekyiu on 2021/7/4.
//

#include <stdio.h>
#include <stdlib.h>
#include "../../header/cpu.h"
#include "../../header/memory.h"

uint64_t va2pa(uint64_t vAddr, Core *cr) {
    // return vAddr & (0xffffffffffffffff >> (64 - MAX_INDEX_PHYSICAL_PAGE));
    return vAddr % PHYSICAL_MEMORY_SPACE;
}