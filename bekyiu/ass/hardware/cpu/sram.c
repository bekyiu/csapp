//
// Created by bekyiu on 2021/8/22.
//

#include "../../header/address.h"
#include "../../header/memory.h"
#include "../../header/common.h"
#include <stdint.h>
#include <stdio.h>

#define NUM_CACHE_LINE_PER_SET 1

typedef enum {
    CACHE_LINE_INVALID,
    CACHE_LINE_CLEAN,
    CACHE_LINE_DIRTY,
} CacheLineState;

typedef struct {
    CacheLineState state;
    uint64_t tag;
    uint8_t block[1 << CACHE_OFFSET_LENGTH];
    // for lru, the max one will to be replaced
    int time;
} CacheLine;

typedef struct {
    CacheLine lines[NUM_CACHE_LINE_PER_SET];
} CacheSet;

typedef struct {
    CacheSet sets[1 << CACHE_INDEX_LENGTH];
} Cache;

static Cache cache;

uint8_t cacheRead(uint64_t pAddrValue) {
    Addr pAddr = {
            ._addrValue = pAddrValue,
    };

    CacheSet *set = &(cache.sets[pAddr.ci]);
    // update LRU
    // find candidate victim line and invalid line
    CacheLine *victim = NULL;
    CacheLine *invalid = NULL;
    int maxTime = -1;

    for (int i = 0; i < NUM_CACHE_LINE_PER_SET; ++i) {
        CacheLine *line = &(set->lines[i]);
        line->time++;

        if (line->time > maxTime) {
            victim = line;
            maxTime = line->time;
        }
        if (line->state == CACHE_LINE_INVALID) {
            invalid = line;
        }
    }

    for (int i = 0; i < NUM_CACHE_LINE_PER_SET; ++i) {
        CacheLine *line = &(set->lines[i]);

        if (line->state != CACHE_LINE_INVALID && line->tag == pAddr.ct) {
            // cache hit
            // update LRU
            blog("read hit");
            line->time = 0;
            return line->block[pAddr.co];
        }
    }

    // cache miss
    blog("read miss");
    // if found free cache line, load data from dram to it
    if (invalid != NULL) {
        readCacheLine(pAddrValue, invalid->block);
        invalid->state = CACHE_LINE_CLEAN;
        invalid->time = 0;
        invalid->tag = pAddr.ct;
        return invalid->block[pAddr.co];
    }

    // if no free cache line, replace the victim
    if (victim != NULL) {
        blog("read replace victim");
        if (victim->state == CACHE_LINE_DIRTY) {
            // todo write back addr is correct ?
            // write back the dirt line to dram
            writeCacheLine(pAddrValue, victim->block);
        }
        // if CACHE_LINE_CLEAN discard this victim directly
        victim->state = CACHE_LINE_INVALID;
        // reload from dram
        readCacheLine(pAddrValue, victim->block);
        victim->state = CACHE_LINE_CLEAN;
        victim->time = 0;
        victim->tag = pAddr.ct;
        return victim->block[pAddr.co];
    }
    throw("cacheRead error");
}

void cacheWrite(uint64_t pAddrValue, uint8_t data) {
    Addr pAddr = {
            ._addrValue = pAddrValue,
    };

    CacheSet *set = &(cache.sets[pAddr.ci]);
    // update LRU
    // find candidate victim line and invalid line
    CacheLine *victim = NULL;
    CacheLine *invalid = NULL;
    int maxTime = -1;

    for (int i = 0; i < NUM_CACHE_LINE_PER_SET; ++i) {
        CacheLine *line = &(set->lines[i]);
        line->time++;

        if (line->time > maxTime) {
            victim = line;
            maxTime = line->time;
        }
        if (line->state == CACHE_LINE_INVALID) {
            invalid = line;
        }
    }

    for (int i = 0; i < NUM_CACHE_LINE_PER_SET; ++i) {
        CacheLine *line = &(set->lines[i]);

        if (line->state != CACHE_LINE_INVALID && line->tag == pAddr.ct) {
            // cache hit
            // update LRU
            blog("write hit");
            line->time = 0;
            line->block[pAddr.co] = data;
            line->state = CACHE_LINE_DIRTY;
            return;
        }
    }

    blog("write miss");
    // cache miss
    // if found free cache line, load data from dram to it
    if (invalid != NULL) {
        readCacheLine(pAddrValue, invalid->block);
        invalid->block[pAddr.co] = data;
        invalid->state = CACHE_LINE_DIRTY;
        invalid->time = 0;
        invalid->tag = pAddr.ct;
        return;
    }

    // if no free cache line, replace the victim
    if (victim != NULL) {
        blog("write replace victim");
        if (victim->state == CACHE_LINE_DIRTY) {
            // write back the dirt line to dram
            writeCacheLine(pAddrValue, victim->block);
        }
        // if CACHE_LINE_CLEAN discard this victim directly
        victim->state = CACHE_LINE_INVALID;
        // reload from dram
        readCacheLine(pAddrValue, victim->block);
        victim->block[pAddr.co] = data;
        victim->state = CACHE_LINE_DIRTY;
        victim->time = 0;
        victim->tag = pAddr.ct;
        return;
    }
    throw("cacheWrite error");
}