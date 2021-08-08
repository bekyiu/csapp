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

    // write sht
    dst->sht = malloc(dst->shtCount * sizeof(ShtEntry));
    uint64_t sectionOffset = 1 + 1 + dst->shtCount;
    int shteIdx = 0;
    ShtEntry *shte = NULL;
    // .text
    if (textLineCount > 0) {
        // write sht entry
        shte = &(dst->sht[shteIdx]);
        strcpy(shte->name, ".text");
        shte->shAddr = textRuntimeAddr;
        shte->offset = sectionOffset;
        shte->lineCount = textLineCount;
        // write dst buffer
        sprintf(dst->buffer[2 + shteIdx], "%s,0x%llx,%lld,%lld",
                shte->name, shte->shAddr, shte->offset, shte->lineCount);

        shteIdx++;
        sectionOffset += textLineCount;
    }
    // .rodata
    if (rodataLineCount > 0) {
        shte = &(dst->sht[shteIdx]);
        strcpy(shte->name, ".rodata");
        shte->shAddr = rodataRuntimeAddr;
        shte->offset = sectionOffset;
        shte->lineCount = rodataLineCount;
        sprintf(dst->buffer[2 + shteIdx], "%s,0x%llx,%lld,%lld",
                shte->name, shte->shAddr, shte->offset, shte->lineCount);

        shteIdx++;
        sectionOffset += rodataLineCount;
    }
    // .data
    if (dataLineCount > 0) {
        shte = &(dst->sht[shteIdx]);
        strcpy(shte->name, ".data");
        shte->shAddr = dataRuntimeAddr;
        shte->offset = sectionOffset;
        shte->lineCount = dataLineCount;
        sprintf(dst->buffer[2 + shteIdx], "%s,0x%llx,%lld,%lld",
                shte->name, shte->shAddr, shte->offset, shte->lineCount);

        shteIdx++;
        sectionOffset += dataLineCount;
    }
    // .symtab
    if (*steMapCount > 0) {
        shte = &(dst->sht[shteIdx]);
        strcpy(shte->name, ".symtab");
        shte->shAddr = symtabRuntimeAddr;
        shte->offset = sectionOffset;
        shte->lineCount = *steMapCount;
        sprintf(dst->buffer[2 + shteIdx], "%s,0x%llx,%lld,%lld",
                shte->name, shte->shAddr, shte->offset, shte->lineCount);

        shteIdx++;
        sectionOffset += *steMapCount;
    }

    assert(shteIdx == dst->shtCount);

    // debug log
    if ((DEBUG_VERBOSE_SET & DEBUG_LINKER) != 0) {
//        for (int i = 0; i < dst->shtCount; ++i) {
//            printf("%s\n", dst->buffer[2 + i]);
//        }
    }
}

int findShteIdx(Elf *srcElf, char *name) {
    int srcShteIdx = -1;
    for (int j = 0; j < srcElf->shtCount; ++j) {
        if (strcmp(name, srcElf->sht[j].name) == 0) {
            srcShteIdx = j;
            break;
        }
    }
    return srcShteIdx;
}

char *stBindStr(StBind bind) {
    switch (bind) {
        case STB_GLOBAL:
            return "STB_GLOBAL";
        case STB_LOCAL:
            return "STB_LOCAL";
        case STB_WEAK:
            return "STB_WEAK";
        default:
        throw("incorrect symbol bind\n");
    }
}

char *stTypeStr(StType type) {
    switch (type) {
        case STT_NOTYPE:
            return "STT_NOTYPE";
        case STT_OBJECT:
            return "STT_OBJECT";
        case STT_FUNC:
            return "STT_FUNC";
        default:
        throw("incorrect symbol type\n");
    }
}

void mergeSection(Elf **srcElfs, int srcNum, Elf *dstElf, SteMap *steMaps, int steMapCount) {
    // index to start write to dst buffer
    int lineWritten = 2 + dstElf->shtCount;
    // index to start write to dst symtab
    int steWritten = 0;
    // offset in hole section, help to merge same type sections into one dst section
    int secOffset = 0;
    // 1. scan sht which in dst
    for (int dstShteIdx = 0; dstShteIdx < dstElf->shtCount; ++dstShteIdx) {
        // section type changed, so secOffset need to reset
        secOffset = 0;
        // for example: this dstShte is .text
        ShtEntry *dstShte = &dstElf->sht[dstShteIdx];
        // 2. scan every elf file and merge their .text sections into dst file
        for (int i = 0; i < srcNum; ++i) {
            Elf *srcElf = srcElfs[i];
            // 3. we need to copy src buffer to dst buffer
            // so we need to known if the srcElf has .text section, and from sht we can get the answer
            int srcShteIdx = findShteIdx(srcElf, dstShte->name);
            if (srcShteIdx == -1) {
                continue;
            }
            // 4. if found, check its symtab
            // it's aim to get src buffer start index
            for (int j = 0; j < srcElf->stCount; ++j) {
                StEntry *srcSte = &srcElf->st[j];
                // 5. find the symbol witch in .text
                if (strcmp(dstShte->name, srcSte->inSecName) == 0) {
                    // we record the correct symbol (like process name conflict) in steMap
                    // we only merge the correct symbol
                    // so we scn the steMap to check it
                    for (int k = 0; k < steMapCount; ++k) {
                        if (srcSte != steMaps[k].srcSte) {
                            continue;
                        }
                        // 6. now can copy src section to dst section line by line
                        for (int l = 0; l < srcSte->lineCount; ++l) {
                            int dstBufferIdx = lineWritten + l;
                            int srcBufferIdx = srcElf->sht[srcShteIdx].offset + srcSte->inSecOffset + l;

                            assert(dstBufferIdx < MAX_ELF_FILE_ROW);
                            assert(srcBufferIdx < MAX_ELF_FILE_ROW);

                            strcpy(dstElf->buffer[dstBufferIdx], srcElf->buffer[srcBufferIdx]);
                        }
                        // 7. now we can fill the dst symtab
                        StEntry *dstSte = &dstElf->st[steWritten];
                        strcpy(dstSte->name, srcSte->name);
                        dstSte->bind = srcSte->bind;
                        dstSte->type = srcSte->type;
                        strcpy(dstSte->inSecName, srcSte->inSecName);
                        dstSte->inSecOffset = secOffset;
                        dstSte->lineCount = srcSte->lineCount;

                        steMaps[k].dstSte = dstSte;

                        steWritten += 1;
                        lineWritten += srcSte->lineCount;
                        secOffset += srcSte->lineCount;
                    }
                }

            }

        }
    }

    // write symtab to buffer
    for (int i = 0; i < dstElf->stCount; ++i) {
        StEntry *ste = &dstElf->st[i];
        sprintf(dstElf->buffer[lineWritten], "%s,%s,%s,%s,%lld,%lld",
                ste->name, stBindStr(ste->bind), stTypeStr(ste->type),
                ste->inSecName, ste->inSecOffset, ste->lineCount);
        lineWritten++;
    }
    assert(lineWritten == dstElf->lineCount);

}

// relocating handlers
void r_x86_64_32_handler(Elf *dstElf, ShtEntry *shte,
                         int rowReferencing, int colReferencing, int addend,
                         StEntry *eofReferencedSymbol) {
    printf("row = %d, col = %d, symbol referenced = %s\n",
           rowReferencing, colReferencing, eofReferencedSymbol->name
    );
}

void r_x86_64_pc32_handler(Elf *dstElf, ShtEntry *shte,
                           int rowReferencing, int colReferencing, int addend,
                           StEntry *eofReferencedSymbol) {
    printf("row = %d, col = %d, symbol referenced = %s\n",
           rowReferencing, colReferencing, eofReferencedSymbol->name
    );
}

void r_x86_64_plt32_handler(Elf *dstElf, ShtEntry *shte,
                            int rowReferencing, int colReferencing, int addend,
                            StEntry *eofReferencedSymbol) {
    printf("row = %d, col = %d, symbol referenced = %s\n",
           rowReferencing, colReferencing, eofReferencedSymbol->name
    );
}

typedef void (*RelHandler)(Elf *dstElf, ShtEntry *shte,
                           int rowReferencing, int colReferencing, int addend,
                           StEntry *eofReferencedSymbol);

RelHandler relHandlers[3] = {
        &r_x86_64_32_handler,       // 0
        &r_x86_64_pc32_handler,     // 1
        // linux commit b21ebf2: x86: Treat R_X86_64_PLT32 as R_X86_64_PC32
        &r_x86_64_pc32_handler,     // 2
};


// find the symbol which is referencing re from the elf
StEntry *findElfReferencingSymbol(RelEntry *re, Elf *elf, char *sectionName) {

    for (int k = 0; k < elf->stCount; ++k) {
        StEntry *ste = &elf->st[k];
        if (strcmp(ste->inSecName, sectionName) != 0) {
            continue;
        }
        int symTextStart = ste->inSecOffset;
        int symTextEnd = ste->inSecOffset + ste->lineCount;
        // if true, we find the referencing symbol in elf
        if (re->rRow >= symTextStart && re->rRow <= symTextEnd) {
            return ste;
        }
    }
    throw("can not find referencing symbol in elf");
}

// find the referencing symbol mapping in eof(dst elf)
StEntry *findEofReferencingSymbol(StEntry *elfReferencingSymbol, SteMap *steMaps, int steMapCount) {
    for (int l = 0; l < steMapCount; ++l) {
        SteMap *steMap = &steMaps[l];
        if (elfReferencingSymbol == steMap->srcSte) {
            return steMap->dstSte;
        }
    }
    throw("can not find referencing symbol in eof");
}

// find the referenced symbol in eof
StEntry *findEofReferencedSymbol(Elf *elf, RelEntry *re, SteMap *steMaps, int steMapCount) {
    for (int m = 0; m < steMapCount; ++m) {
        StEntry *dstSte = steMaps[m].dstSte;
        if (strcmp(elf->st[re->stIdx].name, dstSte->name) == 0 && dstSte->bind == STB_GLOBAL) {
            return dstSte;
        }
    }
    throw("can not find referenced symbol in eof");
}

// recalculate rRow and replace the placeholder
void recalculateAndReplace(Elf *elf, char *sectionName, ShtEntry *dstShte, Elf *dstElf, SteMap *steMaps,
                           int steMapCount) {
    uint64_t relCount = 0;
    RelEntry *relTable = NULL;

    if (strcmp(sectionName, ".text") == 0) {
        relCount = elf->relTextCount;
        relTable = elf->relText;
    } else if (strcmp(sectionName, ".data") == 0) {
        relCount = elf->relDataCount;
        relTable = elf->relData;
    } else {
        throw("error section name: %s, expected .text or .data", sectionName);
    }

    // scan every rel entry
    for (int j = 0; j < relCount; ++j) {
        RelEntry *re = &relTable[j];

        // 1: find the symbol which is referencing re
        StEntry *elfReferencingSymbol = findElfReferencingSymbol(re, elf, sectionName);
        // 2: now we need to find the referencing symbol mapping in eof(dst elf) to help us calculate the new rRow
        StEntry *eofReferencingSymbol = findEofReferencingSymbol(elfReferencingSymbol, steMaps, steMapCount);
        // 3: now we need to find the referenced symbol in eof
        StEntry *eofReferencedSymbol = findEofReferencedSymbol(elf, re, steMaps, steMapCount);

        // 4: do replace
        (relHandlers[(int) re->type])(
                dstElf,
                dstShte,
                re->rRow - elfReferencingSymbol->inSecOffset + eofReferencingSymbol->inSecOffset,
                re->rCol,
                re->rAddend,
                eofReferencedSymbol
        );

    }
}

void relocationProcessing(Elf **srcElfs, int srcNum, Elf *dstElf, SteMap *steMaps, int steMapCount) {
    ShtEntry *dstShteText = NULL;
    ShtEntry *dstShteData = NULL;

    for (int i = 0; i < dstElf->shtCount; ++i) {
        if (strcmp(dstElf->sht[i].name, ".text") == 0) {
            dstShteText = &(dstElf->sht[i]);
        } else if (strcmp(dstElf->sht[i].name, ".data") == 0) {
            dstShteData = &(dstElf->sht[i]);
        }
    }


    // we can find the relocation place from the rel entry in every elf files
    // but after merge sections, the rRow may have been changed
    // so we have to recalculate it and then replace the placeholder
    for (int i = 0; i < srcNum; ++i) {
        Elf *elf = srcElfs[i];
        // process .rel.text
        recalculateAndReplace(elf, ".text", dstShteText, dstElf, steMaps, steMapCount);
        // process .rel.data
        recalculateAndReplace(elf, ".data", dstShteData, dstElf, steMaps, steMapCount);
    }

}


void linkElf(Elf **srcElfs, int srcNum, Elf *dstElf) {
    memset(dstElf, 0, sizeof(Elf));

    int steMapCount = 0;
    SteMap steMaps[MAX_SYMBOL_MAP_LENGTH];
    // update the steMaps - symbol processing
    symbolProcessing(srcElfs, srcNum, dstElf, steMaps, &steMapCount);
    blog("=================");
//    for (int i = 0; i < steMapCount; ++i) {
//        StEntry *e = steMaps[i].srcSte;
//        printf("%s\t%d\t%d\t%s\t%llu\t%llu\n", e->name, e->bind, e->type,
//               e->inSecName, e->inSecOffset, e->lineCount);
//    }
    blog("=================");
    // compute and write dst EOF file header: include line count, sht count, sht
    computeSectionHeader(dstElf, steMaps, &steMapCount);

    dstElf->stCount = steMapCount;
    dstElf->st = malloc(steMapCount * sizeof(StEntry));
    // merge the left sections and relocate the entries in .text and .data
    // merge the symbol content from ELF src into dst sections
    mergeSection(srcElfs, srcNum, dstElf, steMaps, steMapCount);
    blog("=================");
    for (int i = 0; i < dstElf->lineCount; ++i) {
        printf("%s\n", dstElf->buffer[i]);
    }
    // relocating: update the relocation entries from ELF files into EOF buffer
    relocationProcessing(srcElfs, srcNum, dstElf, steMaps, steMapCount);
}




