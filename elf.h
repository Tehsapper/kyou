#pragma once

#include <stdint.h>

#define ELF_MAGIC "\x7f\x45\x4c\x46"

typedef struct {
	struct
	{
		char ei_mag[4];
		uint8_t ei_class;
		uint8_t ei_data;
		uint8_t ei_version;
		uint8_t ei_osabi;
		uint8_t ei_abiversion;
		uint8_t ei_pad[7];
	} e_ident;

	uint16_t e_type;
	uint16_t e_machine;
	uint32_t e_version;
	size_t e_entry;
	size_t e_phoff;
	size_t e_shoff;
	uint32_t e_flags;
	uint16_t e_ehsize;
	uint16_t e_phentsize;
	uint16_t e_phnum;
	uint16_t e_shentsize;
	uint16_t e_shnum;
	uint16_t e_shstrndx;
}
elf_header;

typedef enum {
	PT_NULL,
	PT_LOAD,
	PT_DYNAMIC,
	PT_INTERP,
	PT_NOTE,
	PT_SHLIB,
	PT_PHDR,
	PT_LOOS   = 0x60000000,
	PT_HIGHOS = 0x6FFFFFFF,
	PT_LOPROC = 0x70000000,
	PT_HIPROC = 0x7FFFFFFF
} elf_ptype;

#define PF_X 0x1
#define PF_W 0x2
#define PF_R 0x4

typedef struct {
	elf_ptype p_type;
	uint32_t p_flags;
	uint64_t p_offset;
	uint64_t p_vaddr;
	uint64_t p_paddr;
	uint64_t p_filesz;
	uint64_t p_memsz;
	uint64_t p_align;
}
elf_program_header;