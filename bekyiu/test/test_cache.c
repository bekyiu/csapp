//
// Created by bekyiu on 2021/8/29.
//

#include <stdio.h>
#include <stdint.h>

void cacheWrite(uint64_t pAddrValue, uint8_t data);
uint8_t cacheRead(uint64_t pAddrValue);
int main() {
    // L f0 4
    cacheRead(0xf0);
    // L f1 1
    cacheRead(0xf1);
    // L f2 2
    cacheRead(0xf2);
    // S f2 3
    cacheWrite(0xf2, 77);
    // L f2 77
    printf("%d", cacheRead(0xf2));
}

