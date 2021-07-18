//
// Created by bekyiu on 2021/7/18.
//

#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "../header/common.h"
#include "../header/linker.h"

bool isWhite(const char *str) {
    size_t len = strlen(str);
    if (len == 0) {
        return true;
    }

    if (len == 1 && (str[0] == ' ' || str[0] == '\n' || str[0] == '\r')) {
        return true;
    }

    bool ret = true;
    for (int i = 0; i < len; ++i) {
        char ch = str[i];
        ret = ret && (ch == ' ' || ch == '\t' || ch == '\r');
    }
    return ret;
}

bool isComment(const char *str) {
    size_t len = strlen(str);
    if (len < 2) {
        return false;
    }
    // simple process
    return str[0] == '/' && str[1] == '/';
}

int readElf(const char *filename, uint64_t bufAddr) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        throw("%s\n", strerror(errno));
    }
    char line[MAX_ELF_FILE_WIDTH];
    int lineCount = 0;
    while (fgets(line, MAX_ELF_FILE_WIDTH, f) != NULL) {
        if (isWhite(line)) {
            continue;
        }
        if (isComment(line)) {
            continue;
        }
        lineCount++;
        printf("%s", line);
    }

    fclose(f);
    return lineCount;
}