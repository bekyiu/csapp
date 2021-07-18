//
// Created by bekyiu on 2021/7/18.
//
#include <stdint.h>
#include <stdio.h>
#include "../ass/header/common.h"
int readElf(const char *filename, uint64_t bufAddr);

int main() {
    // './' 是指cmake-build-debug这个目录
    char file[] = "../bekyiu/file/sum.elf.txt";
    readElf(file, 0);
    return 0;
}
