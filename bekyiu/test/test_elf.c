//
// Created by bekyiu on 2021/7/18.
//
#include <stdint.h>
#include <stdio.h>
#include "../ass/header/common.h"
#include "../ass/header/linker.h"


int main() {
    // './' 是指cmake-build-debug这个目录
    char file[] = "../bekyiu/file/sum.elf.txt";
    Elf elf;
    parseElf(file, &elf);
    logElf(&elf);
    freeElf(&elf);
    return 0;
}
