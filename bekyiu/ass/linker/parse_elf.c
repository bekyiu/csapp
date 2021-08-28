//
// Created by bekyiu on 2021/7/18.
//

#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
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
    char (*buf)[MAX_ELF_FILE_COLUMN] = (char (*)[MAX_ELF_FILE_COLUMN]) baseAddr;
    if (str2uint(buf[0]) != lineCount) {
        throw("read elf error!");
    }
    return lineCount;
}


// str: a string like "c0,c1,c2,c3"
// entryAddr: an address which a value on stack
// *entryAddr: a pointer point to a string array
// *(*entryAddr): the first value of string array
int parseTableEntry(char *str, char ***entryAddr) {
    int colNum = 1;
    size_t len = strlen(str);
    for (int i = 0; i < len; ++i) {
        if (str[i] == ',') {
            colNum++;
        }
    }

    // malloc the string array first address
    char **arr = malloc(colNum * sizeof(char *));
    *entryAddr = arr;
    // put every column into string array
    char col[32] = {'\0'};
    char colIdx = 0;
    char arrIdx = 0;
    for (int i = 0; i < len + 1; ++i) {
        // find a column
        if (str[i] == ',' || str[i] == '\0') {
            char *ele = malloc((colIdx + 1) * sizeof(char));
            memcpy(ele, col, colIdx);
            ele[colIdx] = '\0';
            arr[arrIdx] = ele;

            arrIdx++;
            colIdx = 0;
        } else {
            assert(colIdx < 32);
            col[colIdx] = str[i];
            colIdx++;
        }
    }
    return colNum;
}

void freeTableEntry(char **entry, int n) {
    for (int i = 0; i < n; ++i) {
        free(entry[i]);
    }
    free(entry);
}

// str: entry str like ".text,0x0,4,22"
// shtEntry:
void parseShtEntry(char *str, ShtEntry *shtEntry) {
    char **entry;
    int entryNum = parseTableEntry(str, &entry);

    strcpy(shtEntry->name, entry[0]);
    shtEntry->shAddr = str2uint(entry[1]);
    shtEntry->offset = str2uint(entry[2]);
    shtEntry->lineCount = str2uint(entry[3]);

    freeTableEntry(entry, entryNum);
}

int parseStBind(char *bind) {
    if (strcmp(bind, "STB_LOCAL") == 0) {
        return STB_LOCAL;
    }
    if (strcmp(bind, "STB_GLOBAL") == 0) {
        return STB_GLOBAL;
    }
    if (strcmp(bind, "STB_WEAK") == 0) {
        return STB_WEAK;
    }
    throw("unknown symbol table bind: %s", bind);
}

int parseStType(char *type) {
    if (strcmp(type, "STT_NOTYPE") == 0) {
        return STT_NOTYPE;
    }
    if (strcmp(type, "STT_OBJECT") == 0) {
        return STT_OBJECT;
    }
    if (strcmp(type, "STT_FUNC") == 0) {
        return STT_FUNC;
    }
    throw("unknown symbol table type: %s", type);
}

int parseRelType(char *type) {
    if (strcmp(type, "R_X86_64_32") == 0) {
        return R_X86_64_32;
    }
    if (strcmp(type, "R_X86_64_PC32") == 0) {
        return R_X86_64_PC32;
    }
    if (strcmp(type, "R_X86_64_PLT32") == 0) {
        return R_X86_64_PLT32;
    }
    throw("unknown rel type: %s", type);
}

// str: sum,STB_GLOBAL,STT_FUNC,.text,0,22
void parseStEntry(char *str, StEntry *stEntry) {
    char **entry;
    int entryNum = parseTableEntry(str, &entry);

    strcpy(stEntry->name, entry[0]);
    stEntry->bind = parseStBind(entry[1]);
    stEntry->type = parseStType(entry[2]);
    strcpy(stEntry->inSecName, entry[3]);
    stEntry->inSecOffset = str2uint(entry[4]);
    stEntry->lineCount = str2uint(entry[5]);

    freeTableEntry(entry, entryNum);
}

// 17,7,R_X86_64_PC32,1,-4
void parseRelEntry(char *str, RelEntry *relEntry) {
    char **entry;
    int entryNum = parseTableEntry(str, &entry);
    relEntry->rRow = str2uint(entry[0]);
    relEntry->rCol = str2uint(entry[1]);
    relEntry->type = parseRelType(entry[2]);
    relEntry->stIdx = str2uint(entry[3]);

    uint64_t rAddend = str2uint(entry[4]);
    relEntry->rAddend = *((int64_t *) &rAddend);

    freeTableEntry(entry, entryNum);
}

void parseElf(char *filename, Elf *elf) {
    int lineCount = readElf(filename, (uint64_t) (&elf->buffer));
    elf->lineCount = lineCount;
//    for (int i = 0; i < lineCount; ++i) {
//        printf("[%d]\t%s\n", i, elf->buffer[i]);
//    }

    // build section header table
    int shtEntryNum = (int) str2uint(elf->buffer[1]);
    elf->sht = malloc(shtEntryNum * sizeof(ShtEntry));
    elf->shtCount = shtEntryNum;

    ShtEntry *symShte = NULL;
    ShtEntry *relTextShte = NULL;
    ShtEntry *relDataShte = NULL;
    for (int i = 0; i < shtEntryNum; ++i) {
        ShtEntry *curShte = &elf->sht[i];
        parseShtEntry(elf->buffer[2 + i], curShte);

        if (strcmp(curShte->name, ".symtab") == 0) {
            symShte = curShte;
        } else if (strcmp(curShte->name, ".rel.text") == 0) {
            relTextShte = curShte;
        } else if (strcmp(curShte->name, ".rel.data") == 0) {
            relDataShte = curShte;
        }
    }
    // parse symtab
    elf->stCount = symShte->lineCount;
    elf->st = malloc(elf->stCount * sizeof(StEntry));
    for (int i = 0; i < elf->stCount; ++i) {
        parseStEntry(elf->buffer[symShte->offset + i], &elf->st[i]);
    }
    // .rel.text
    if (relTextShte != NULL) {
        elf->relTextCount = relTextShte->lineCount;
        elf->relText = malloc(elf->relTextCount * sizeof(RelEntry));
        for (int i = 0; i < elf->relTextCount; ++i) {
            parseRelEntry(elf->buffer[relTextShte->offset + i], &elf->relText[i]);
        }
    } else {
        elf->relTextCount = 0;
        elf->relText = NULL;
    }
    // .rel.data
    if (relDataShte != NULL) {
        elf->relDataCount = relDataShte->lineCount;
        elf->relData = malloc(elf->relDataCount * sizeof(RelEntry));
        for (int i = 0; i < elf->relDataCount; ++i) {
            parseRelEntry(elf->buffer[relDataShte->offset + i], &elf->relData[i]);
        }
    } else {
        elf->relDataCount = 0;
        elf->relData = NULL;
    }

}

void freeElf(Elf *elf) {
    free(elf->sht);
    free(elf->st);
}

void logElf(Elf *elf) {
    printf("\nline count: %lld\n", elf->lineCount);
    printf("\nsection header table:\n");
    for (int i = 0; i < elf->shtCount; ++i) {
        ShtEntry e = elf->sht[i];
        printf("%s\t%llu\t%llu\t%llu\n", e.name, e.shAddr, e.offset, e.lineCount);
    }
    printf("\n");
    printf("symbol table:\n");
    for (int i = 0; i < elf->stCount; ++i) {
        StEntry e = elf->st[i];
        printf("%s\t%d\t%d\t%s\t%llu\t%llu\n", e.name, e.bind, e.type, e.inSecName, e.inSecOffset, e.lineCount);
    }
    printf("\n.rel.text:\n");
    for (int i = 0; i < elf->relTextCount; ++i) {
        RelEntry e = elf->relText[i];
        printf("%lld\t%lld\t%d\t%d\t%lld\n", e.rRow, e.rCol, e.type, e.stIdx, e.rAddend);
    }
}