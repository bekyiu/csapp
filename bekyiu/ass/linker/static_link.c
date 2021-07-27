//
// Created by bekyiu on 2021/7/25.
//

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../header/linker.h"
#include "../header/common.h"
#include "../header/cpu.h"

#define MAX_SYMBOL_MAP_LENGTH 64
#define MAX_SECTION_BUFFER_LENGTH 64

// internal mapping between source and destination symbol entries
typedef struct {
    // src elf file
    Elf *srcElf;
    // src symbol belong the srcElf
    StEntry *srcSte;
    StEntry *dstSte;
} SteMap;


// return the symbol precedence
int symbolPrecedence(StEntry *ste) {
    /*  we do not consider weak because it's very rare
        and we do not consider local because it's not conflicting
            bind        type        shndx               prec
            --------------------------------------------------
            global      notype      undef               0 - undefined
            global      object      common              1 - tentative
            global      object      data,bss,rodata     2 - defined
            global      func        text                2 - defined
    */
    assert(ste->bind == STB_GLOBAL);
    if (strcmp(ste->inSecName, "SHN_UNDEF") == 0 && ste->type == STT_NOTYPE) {
        // Undefined: symbols referenced but not assigned a storage address
        return 0;
    }
    if (strcmp(ste->inSecName, "COMMON") == 0 && ste->type == STT_OBJECT) {
        // Tentative: section to be decided after symbol resolution
        return 1;
    }
    if ((strcmp(ste->inSecName, ".text") == 0 && ste->type == STT_FUNC) ||
        (strcmp(ste->inSecName, ".data") == 0 && ste->type == STT_OBJECT) ||
        (strcmp(ste->inSecName, ".rodata") == 0 && ste->type == STT_OBJECT) ||
        (strcmp(ste->inSecName, ".bss") == 0 && ste->type == STT_OBJECT)) {
        // Defined
        return 2;
    }
    throw("symbol resolution: cannot determine the precedence: %s", ste->name);
}

void simpleResolution(StEntry *srcSte, Elf *srcElf, SteMap *steMap) {
    // choose a best matching symbol from srcSte and steMap->srcSte need to follow 3 rules:
    // rule1: multiple strong symbols with the same name are not allowed
    // rule2: given a strong symbol and multiple weak symbols with the same name, choose the strong symbol
    // rule3: given multiple weak symbols with the same name, choose any of the weak symbols

    int pre1 = symbolPrecedence(srcSte);
    int pre2 = symbolPrecedence(steMap->srcSte);

    // rule1
    if (pre1 == 2 && pre2 == 2) {
        throw("multiple strong symbols with the same name are not allowed: %s", srcSte->name);
    }

    // rule3
    if (pre1 != 2 && pre2 != 2) {
        // use the stronger symbol
        if (pre1 > pre2) {
            steMap->srcSte = srcSte;
            steMap->srcElf = srcElf;
        }
        return;
    }

    // rule2
    if (pre1 == 2) {
        steMap->srcSte = srcSte;
        steMap->srcElf = srcElf;
    }

}

void symbolProcessing(Elf **srcElfs, int srcNum, Elf *dstElf, SteMap *steMaps, int *steMapCount) {
    for (int i = 0; i < srcNum; ++i) {
        Elf *srcElf = srcElfs[i];
        for (int j = 0; j < srcElf->stCount; ++j) {
            StEntry *ste = &srcElf->st[j];
            if (ste->bind == STB_LOCAL) {
                // insert the static (local) symbol to new elf with confidence
                // compiler would check if the symbol is re-declared in one *.c file
                assert(*steMapCount < MAX_SYMBOL_MAP_LENGTH);
                // even if local symbol has the same name, just insert it into dst
                steMaps[*steMapCount].srcElf = srcElf;
                steMaps[*steMapCount].srcSte = ste;
                (*steMapCount)++;
            } else if (ste->bind == STB_GLOBAL) {
                // for other bind: STB_GLOBAL, etc. it's possible to have name conflict
                // check if this symbol has been cached in the map
                for (int k = 0; k < *steMapCount; ++k) {
                    StEntry *candidate = steMaps[k].srcSte;
                    // name conflict
                    if (candidate->bind == STB_GLOBAL && strcmp(candidate->name, ste->name) == 0) {
                        simpleResolution(ste, srcElf, &steMaps[k]);
                        goto NEXT_SYMBOL_PROCESS;
                    }
                }
                // no conflict
                assert(*steMapCount <= MAX_SYMBOL_MAP_LENGTH);
                steMaps[*steMapCount].srcElf = srcElf;
                steMaps[*steMapCount].srcSte = ste;
                (*steMapCount)++;
            }
            NEXT_SYMBOL_PROCESS:;
        }
    }

    // check if there is any undefined or common symbols in the map
    for (int i = 0; i < *steMapCount; ++i) {
        StEntry *ste = steMaps[i].srcSte;
        assert(strcmp(ste->inSecName, "SHN_UNDEF") != 0);
        assert(ste->type != STT_NOTYPE);
        if (strcmp(ste->inSecName, "COMMON") == 0) {
            // still has common symbol, put it in .bss
            strcpy(ste->inSecName, ".bss");
            ste->inSecOffset = 0;
        }
    }
}

void computeSectionHeader(Elf *dst, SteMap *steMaps, int *steMapCount) {
    // compute section line count
    uint64_t textLineCount = 0;
    uint64_t dataLineCount = 0;
    uint64_t rodataLineCount = 0;

    for (int i = 0; i < *steMapCount; ++i) {
        StEntry *ste = steMaps[i].srcSte;
        if (strcmp(ste->inSecName, ".text") == 0) {
            textLineCount += ste->lineCount;
        } else if (strcmp(ste->inSecName, ".rodata") == 0) {
            rodataLineCount += ste->lineCount;
        } else if (strcmp(ste->inSecName, ".data") == 0) {
            dataLineCount += ste->lineCount;
        }
    }

    // plus one means add .symtab
    dst->shtCount = (textLineCount != 0) + (dataLineCount != 0) + (rodataLineCount != 0) + 1;
    dst->lineCount = 1 + 1 + dst->shtCount + textLineCount + dataLineCount + rodataLineCount + *steMapCount;
    dst->stCount = *steMapCount;
    sprintf(dst->buffer[0], "%lld", dst->lineCount);
    sprintf(dst->buffer[1], "%lld", dst->shtCount);

    // compute the run-time address of the sections: compact in memory
    uint64_t textRuntimeAddr = 0x00400000;
    uint64_t rodataRuntimeAddr = textRuntimeAddr + textLineCount * MAX_INSTRUCTION_CHAR * sizeof(char);
    uint64_t dataRuntimeAddr = rodataRuntimeAddr + rodataLineCount * sizeof(uint64_t);
    uint64_t symtabRuntimeAddr = 0; // For EOF, .symtab is not loaded into run-time memory but still on disk
}

void linkElf(Elf **srcElfs, int srcNum, Elf *dstElf) {
    memset(dstElf, 0, sizeof(Elf));

    int steMapCount = 0;
    SteMap steMaps[MAX_SYMBOL_MAP_LENGTH];
    // update the steMaps - symbol processing
    symbolProcessing(srcElfs, srcNum, dstElf, steMaps, &steMapCount);
    puts("=================");
    for (int i = 0; i < steMapCount; ++i) {
        StEntry *e = steMaps[i].srcSte;
        printf("%s\t%d\t%d\t%s\t%llu\t%llu\n", e->name, e->bind, e->type,
               e->inSecName, e->inSecOffset, e->lineCount);
    }

    // compute and write dst EOF file header: include sht, line count, sht count
    computeSectionHeader(dstElf, steMaps, &steMapCount);
}




