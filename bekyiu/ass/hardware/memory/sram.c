//
// Created by bekyiu on 2021/8/22.
//

#include "../../header/address.h"
#include <stdint.h>

#define NUM_CACHE_LINE_PER_SET 8

typedef enum {
    CACHE_LINE_INVALID,
    CACHE_LINE_CLEAN,
    CACHE_LINE_DIRTY,
} CacheLineState;

typedef struct {
    CacheLineState state;
    uint64_t tag;
    uint8_t block[1 << CACHE_OFFSET_LENGTH];
} CacheLine;

typedef struct {
    CacheLine lines[NUM_CACHE_LINE_PER_SET];
} CacheSet;

typedef struct {
    CacheSet sets[1 << CACHE_INDEX_LENGTH];
} Cache;

static Cache cache;

uint8_t cacheRead(Addr pAddr) {
    CacheSet set = cache.sets[pAddr.ci];
    for (int i = 0; i < NUM_CACHE_LINE_PER_SET; ++i) {
        CacheLine line = set.lines[i];

        if (line.state != CACHE_LINE_INVALID && line.tag == pAddr.ct) {
            // cache hit
            // TODO: update LRU
            return line.block[pAddr.co];
        }
    }

    // cache miss: load from memory
    // TODO: update LRU
    // TODO: select one victim by replacement policy if set is full

    return 0;
}

void cacheWrite(Addr pAddr, uint8_t data) {
}