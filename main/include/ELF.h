#pragma once

#define EI_NIDENT        16

typedef struct {
    unsigned char    e_ident[EI_NIDENT];
    unsigned short    e_type;
    unsigned short    e_machine;
    unsigned int    e_version;
    unsigned int    e_entry;
    unsigned int    e_phoff;
    unsigned int    e_shoff;
    unsigned int    e_flags;
    unsigned short    e_ehsize;
    unsigned short    e_phentsize;
    unsigned short    e_phnum;
    unsigned short    e_shentsize;
    unsigned short    e_shnum;
    unsigned short    e_shstrndx;
 } Elf32_Ehdr;

typedef struct {
    unsigned int    sh_name;
    unsigned int    sh_type;
    unsigned int    sh_flags;
    unsigned int    sh_addr;
    unsigned int    sh_offset;
    unsigned int    sh_size;
    unsigned int    sh_link;
    unsigned int    sh_info;
    unsigned int    sh_addralign;
    unsigned int    sh_entsize;
} Elf32_Shdr;

typedef struct {
    unsigned int    p_type;
    unsigned int    p_offset;
    unsigned int    p_vaddr;
    unsigned int    p_paddr;
    unsigned int    p_filesz;
    unsigned int    p_memsz;
    unsigned int    p_flags;
    unsigned int    p_align;
} Elf32_Phdr;

#define EI_MAG0        0
#define EI_MAG1        1
#define EI_MAG2        2
#define EI_MAG3        3
#define EI_CLASS    4
#define EI_DATA        5
#define EI_VERSION    6
#define EI_PAD        7
#define EI_NIDENT    16    //size of ident

#define PT_NULL        0
#define PT_LOAD        1
#define PT_DYNAMIC    2
#define PT_INTERP    3
#define PT_NOTE        4
#define PT_SHLIB    5
#define PT_PHDR        6
#define PT_LOOS        0x60000000
#define PT_HIOS        0x6fffffff
#define PT_LOPROC    0x70000000
#define PT_HIPROC    0x7fffffff
