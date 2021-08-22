//
// Created by bekyiu on 2021/8/22.
//

#ifndef CSAPP_ADDRESS_H
#define CSAPP_ADDRESS_H

#include <stdint.h>

#define CACHE_TAG_LENGTH 40
#define CACHE_INDEX_LENGTH 6
#define CACHE_OFFSET_LENGTH 6


#define PHYSICAL_PAGE_OFFSET_LENGTH 12
#define PHYSICAL_PAGE_NUMBER_LENGTH 40
#define PHYSICAL_ADDRESS_LENGTH 52

typedef union {
    uint64_t _addrValue;

    // physical addr
    struct {
        union {
            uint64_t _pAddrValue: PHYSICAL_ADDRESS_LENGTH;
            struct {
                uint64_t ppo: PHYSICAL_PAGE_OFFSET_LENGTH;
                uint64_t ppn: PHYSICAL_PAGE_NUMBER_LENGTH;
            };
        };
    };

    // sram cache: 52
    struct {
        uint64_t co: CACHE_OFFSET_LENGTH;
        uint64_t ci: CACHE_INDEX_LENGTH;
        uint64_t ct: CACHE_TAG_LENGTH;
    };
} Addr;

#endif //CSAPP_ADDRESS_H
