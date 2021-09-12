//
// Created by bekyiu on 2021/8/22.
//

#ifndef CSAPP_ADDRESS_H
#define CSAPP_ADDRESS_H

#include <stdint.h>

// 40
#define CACHE_TAG_LENGTH 4
#define CACHE_INDEX_LENGTH 6
#define CACHE_OFFSET_LENGTH 6


#define PHYSICAL_PAGE_OFFSET_LENGTH 12
// 40
#define PHYSICAL_PAGE_NUMBER_LENGTH 4
// 52
#define PHYSICAL_ADDRESS_LENGTH 16

#define VIRTUAL_PAGE_OFFSET_LENGTH 12 // 4kb
#define VIRTUAL_PAGE_NUMBER_LENGTH 9 // 9 + 9 + 9 + 9
#define VIRTUAL_ADDRESS_LENGTH 48

/*
+--------+--------+--------+--------+---------------+
|  VPN3  |  VPN2  |  VPN1  |  VPN0  |               |
+--------+--------+--------+-+------+      VPO      |
|    TLBT                    | TLBI |               |
+---------------+------------+------+---------------+
                |        PPN        |      PPO      |
                +-------------------+--------+------+
                |        CT         |   CI   |  CO  |
                +-------------------+--------+------+
*/

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
    // virtual addr
    struct {
        union {
            uint64_t _vAddrValue: VIRTUAL_ADDRESS_LENGTH;
            struct {
                uint64_t vpo: VIRTUAL_PAGE_OFFSET_LENGTH;
                uint64_t vpn4: VIRTUAL_PAGE_NUMBER_LENGTH;
                uint64_t vpn3: VIRTUAL_PAGE_NUMBER_LENGTH;
                uint64_t vpn2: VIRTUAL_PAGE_NUMBER_LENGTH;
                uint64_t vpn1: VIRTUAL_PAGE_NUMBER_LENGTH;
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
