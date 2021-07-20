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

int readElf(const char *filename, uint64_t baseAddr) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        throw("%s\n", strerror(errno));
    }
    char line[MAX_ELF_FILE_COLUMN];
    int lineCount = 0;
    while (fgets(line, MAX_ELF_FILE_COLUMN, f) != NULL) {
        if (isWhite(line)) {
            continue;
        }
        if (isComment(line)) {
            continue;
        }
        if (lineCount > MAX_ELF_FILE_ROW) {
            throw("elf file [%s] is to large, expected max size is %d lines",
                  filename, MAX_ELF_FILE_ROW);
        }

        // store into buffer
        size_t len = strlen(line);
        uint64_t bufAddr = baseAddr + lineCount * MAX_ELF_FILE_COLUMN * sizeof(char);
        char *buffer = (char *) bufAddr;
        int i;
        for (i = 0; i < len; ++i) {
            if (line[i] == '\n' || line[i] == '\r') {
                break;
            }
            // in-line comment
            if ((i + 1 < len) && (i + 1 < MAX_ELF_FILE_COLUMN) && line[i] == '/' && line[i + 1] == '/') {
                break;
            }
            buffer[i] = line[i];
        }
        buffer[i] = '\0';
        lineCount++;
    }

    fclose(f);
    // check first line is correct
    char (*buf)[MAX_ELF_FILE_COLUMN] = (char (*)[MAX_ELF_FILE_COLUMN])baseAddr;
    if (str2uint(buf[0]) != lineCount) {
        throw("read elf error!");
    }
    return lineCount;
}