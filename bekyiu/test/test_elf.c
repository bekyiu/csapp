//
// Created by bekyiu on 2021/7/18.
//
#include <stdint.h>
#include <stdio.h>
#include "../ass/header/common.h"
#include "../ass/header/linker.h"


int main() {
    Elf elf[2];
    // './' 是指cmake-build-debug这个目录
    parseElf("../bekyiu/file/rel_sum.elf.txt", &elf[0]);
    parseElf("../bekyiu/file/rel_main.elf.txt", &elf[1]);
    logElf(&elf[0]);
    logElf(&elf[1]);

//    Elf *srcElfs[2];
//    srcElfs[0] = &elf[0];
//    srcElfs[1] = &elf[1];
//
//    Elf dst;
//    linkElf(srcElfs, 2, &dst);


    freeElf(&elf[0]);
    freeElf(&elf[1]);
    return 0;
}
