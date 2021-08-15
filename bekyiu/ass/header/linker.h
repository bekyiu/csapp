//
// Created by bekyiu on 2021/7/18.
//

#ifndef CSAPP_LINKER_H
#define CSAPP_LINKER_H

#include <stdint.h>
#include <stdlib.h>

#define MAX_CHAR_SECTION_NAME (32)

// ================== symbol table ======================
// symbol table st_info enums
typedef enum {
    STB_LOCAL,
    STB_GLOBAL,
    STB_WEAK,
} StBind;

typedef enum {
    STT_NOTYPE,
    STT_OBJECT,
    STT_FUNC,
} StType;

// symbol table entry
typedef struct {
    // symbol name; st_name
    char name[MAX_CHAR_SECTION_NAME];
    // st_info
    StBind bind;
    StType type;
    // current symbol in which section; st_shndx
    char inSecName[MAX_CHAR_SECTION_NAME];
    // in-section offset; st_value
    uint64_t inSecOffset;
    // count of lines of symbol; st_size
    uint64_t lineCount;
} StEntry;

// ================== relocation =====================
typedef enum {
    R_X86_64_32,
    R_X86_64_PC32,
    R_X86_64_PLT32,
} RelType;

typedef struct {
    // these two fields to locate the placeholder which to be relocate
    uint64_t rRow;
    uint64_t rCol;
    // relocation type
    RelType type;
    // associated symtab entry
    uint32_t stIdx;
    int64_t rAddend;
} RelEntry;
// ================== section header table ======================
typedef struct {
    // section name; sh_name
    char name[MAX_CHAR_SECTION_NAME];
    // section virtual addr at execution; sh_addr
    uint64_t shAddr;
    // line offset or effective line index; sh_offset
    uint64_t offset;
    // count of lines of section; sh_size
    uint64_t lineCount;
} ShtEntry;

// ================== elf file ======================

#define MAX_ELF_FILE_ROW (64)    // max 64 effective lines
#define MAX_ELF_FILE_COLUMN (128)    // max 128 chars per line
#define TEXT_ADDR_BASE 0x00400000

typedef struct {
    char buffer[MAX_ELF_FILE_ROW][MAX_ELF_FILE_COLUMN];
    // effective lines
    uint64_t lineCount;
    // section header table start pointer
    ShtEntry *sht;
    uint64_t shtCount;

    // symtab
    StEntry *st;
    uint64_t stCount;

    RelEntry *relText;
    uint64_t relTextCount;

    RelEntry *relData;
    uint64_t relDataCount;
} Elf;

void logElf(Elf *elf);

void freeElf(Elf *elf);

void parseElf(char *filename, Elf *elf);

void linkElf(Elf **srcElfs, int srcNum, Elf *dstElf);

#endif //CSAPP_LINKER_H
