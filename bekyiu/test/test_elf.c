//
// Created by bekyiu on 2021/7/18.
//
#include <stdint.h>
#include <stdio.h>
#include "../ass/header/common.h"
#include "../ass/header/linker.h"
int readElf(const char *filename, uint64_t bufAddr);

int main() {
    char buf[MAX_ELF_FILE_ROW][MAX_ELF_FILE_COLUMN] = {'\0'};
    // './' 是指cmake-build-debug这个目录
    char file[] = "../bekyiu/file/sum.elf.txt";
    int lineCount = readElf(file, (uint64_t) &buf);

    for (int i = 0; i < lineCount; ++i) {
        printf("%s\n", buf[i]);
    }
    return 0;
}
