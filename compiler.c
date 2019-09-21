#include "ast.h"
#include "file.h"
#include "elf.h"
#include "hash.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/stat.h>

static unsigned char* data;
static size_t size;
static size_t capacity;

struct hash_table *relocs;

//#define PTR(v) typeof(v*)
#define EMIT(v) do {\
	if (size + sizeof(v) >= capacity) {\
		data = realloc(data, capacity + 4096);\
		capacity += 4096;\
	}\
	*((typeof(&v))(data + size)) = v;\
	size += sizeof(v);\
} while(0)

static uint8_t kyou_reg2x64id(kyou_register_t reg)
{
	switch (reg) {
		case REG_FIRE:
			return 3; //rbx
		case REG_WATER:
			return 12; //r12
		case REG_TREE:
			return 13; //r13
		case REG_METAL:
			return 14; //r14
		case REG_EARTH:
			return 15; //r15
		case REG_STORAGE:
			return 4; //rsp
		case REG_STORAGE_BASE:
			return 5; //rbp
	}
}

static void emit_move_r2r(uint8_t reg1, uint8_t reg2)
{
	// 0100WRXB
	uint8_t rex = 0x48 | (reg1 & 0x8 ? 0x1 : 0) | (reg2 & 0x8 ? 0x4 : 0);
	uint8_t opcode = 0x89;
	uint8_t modRM = 0xC0 | ((reg2 & 0x7) << 3) | (reg1 & 0x7);

	EMIT(rex);
	EMIT(opcode);
	EMIT(modRM);
}

static void emit_move_imm2r(uint8_t reg, uint64_t imm)
{
	uint8_t rex = 0x48 | (reg & 0x8 ? 0x1 : 0);
	uint8_t opcode = 0xB8 + (reg & 0x7);

	EMIT(rex);
	EMIT(opcode);
	EMIT(imm);
}

static int compile_syscall()
{
	emit_move_r2r(0, kyou_reg2x64id(REG_FIRE));
	emit_move_r2r(7, kyou_reg2x64id(REG_WATER));
	emit_move_r2r(6, kyou_reg2x64id(REG_TREE));
	emit_move_r2r(2, kyou_reg2x64id(REG_METAL));
	emit_move_r2r(10, kyou_reg2x64id(REG_EARTH));
	// TODO: push 2 more args to stack if applicable
	uint16_t opcode = 0x050f;
	EMIT(opcode);
	emit_move_r2r(kyou_reg2x64id(REG_FIRE), 0);
}

//rbx, r12, r13, r14, r15, rsp, rbp
static int compile_move(AST_node* node)
{
	if (node->move_dest.type == DESTINATION_REGISTER && node->move_src.type == SOURCE_REGISTER) {
		emit_move_r2r(kyou_reg2x64id(node->move_dest.as_reg), kyou_reg2x64id(node->move_src.as_reg));
		return 1;
	}
	else if (node->move_dest.type == DESTINATION_REGISTER && node->move_src.type == SOURCE_IMMEDIATE) {
		emit_move_imm2r(kyou_reg2x64id(node->move_dest.as_reg), node->move_src.as_immediate);
		return 1;
	}
	else if (node->move_dest.type == DESTINATION_FD && node->move_src.type == SOURCE_REGISTER) {
		emit_move_imm2r(0, 60);
		emit_move_r2r(7, kyou_reg2x64id(node->move_src.as_reg));
		uint16_t opcode = 0x050f;
		EMIT(opcode);
		return 1;
	}
	else {
		fprintf(stderr, "error: moves except register-register are not implemented\n");
		return 0;
	}
}

static void emit_add_r2r(uint8_t reg1, uint8_t reg2)
{
	uint8_t rex = 0x48 | (reg1 & 0x8 ? 0x1 : 0) | (reg2 & 0x8 ? 0x4 : 0);
	uint8_t opcode = 0x01;
	uint8_t modRM = 0xC0 | ((reg2 & 0x7) << 3) | (reg1 & 0x7);
	
	EMIT(rex);
	EMIT(opcode);
	EMIT(modRM);
}

static void emit_add_imm2r(uint8_t reg, uint64_t imm)
{
	uint32_t imm32 = (uint32_t)imm;
	uint8_t rex = 0x48 | (reg & 0x8 ? 0x1 : 0);
	uint8_t opcode = 0x81;
	uint8_t modRM = 0xC0 | (reg & 0x7);

	EMIT(rex);
	EMIT(opcode);
	EMIT(modRM);
	EMIT(imm32);
}

static int compile_add(AST_node* node)
{
	if (node->op_src.type == SOURCE_REGISTER) {
		emit_add_r2r(kyou_reg2x64id(node->op_reg), kyou_reg2x64id(node->op_src.as_reg));
		return 1;
	}
	else if (node->op_src.type == SOURCE_IMMEDIATE) {
		emit_add_imm2r(kyou_reg2x64id(node->op_reg), node->op_src.as_immediate);
		return 1;
	}
	else {
		fprintf(stderr, "error: only reg-2-reg addition is supported\n");
		return -1;
	}
}

static int compile_op(AST_node* node)
{
	switch (node->op_type) {
		case OP_ADD: return compile_add(node);
		default: fprintf(stderr, "error: only add is supported\n"); return -1;
	}
}

static int compile_print(AST_node* node)
{

}

static int compile_push(AST_node* node)
{
	
}

static int compile_start()
{

}

static int compile_end()
{
	emit_move_imm2r(0, 60);
	emit_move_imm2r(7, 0x0);
	uint16_t opcode = 0x050f;
	EMIT(opcode);
}

int compile(AST* ast, const char* filename)
{
	elf_header header;

	memset(&header, 0, sizeof(elf_header));

	memcpy(header.e_ident.ei_mag, ELF_MAGIC, sizeof(ELF_MAGIC));
	header.e_ident.ei_class = 2; //x64 format
	header.e_ident.ei_data = 1;  // little endian
	header.e_ident.ei_version = 1; // current ELF version
	header.e_ident.ei_osabi = 0; // Sys V ABI

	header.e_type = 0x2; // executable, dyn notes that code is PI and can be ASLR'ed
	header.e_machine = 0x3E; // x86_64
	header.e_version = 1; // current ELF version
	header.e_entry = 0x8048000 + sizeof(elf_header) + sizeof(elf_program_header);
	header.e_phoff = 0x40;
	header.e_shoff = 0;
	header.e_flags = 0;
	header.e_ehsize = sizeof(elf_header);
	header.e_phentsize = sizeof(elf_program_header);
	header.e_phnum = 1;
	header.e_shentsize = 0;
	header.e_shnum = 0;
	header.e_shstrndx = 0;

	elf_program_header text_header;
	memset(&text_header, 0, sizeof(elf_program_header));

	text_header.p_type = PT_LOAD;
	text_header.p_flags = PF_X | PF_R;
	text_header.p_offset = 0;//sizeof(elf_header) + sizeof(elf_program_header);
	text_header.p_vaddr = 0x8048000;// + sizeof(elf_header) + sizeof(elf_program_header);
	text_header.p_paddr = 0x8048000;// + sizeof(elf_header) + sizeof(elf_program_header);
	text_header.p_align = 0x1000;

	capacity = 4096;
	size = 0;
	data = malloc(capacity);

	compile_start();

	for (size_t i = 0; i < ast->size; ++i) {
		switch (ast->nodes[i].type) {
			case MOVE_STATEMENT:
				compile_move(&ast->nodes[i]);
				break;
			case OPERATOR_STATEMENT:
				compile_op(&ast->nodes[i]);
				break;
			default:
				fprintf(stderr, "unimplemented statement\n");
				break;
		}
	}

	compile_end();

	text_header.p_filesz = size + sizeof(elf_header) + sizeof(elf_program_header);
	text_header.p_memsz = size + sizeof(elf_header) + sizeof(elf_program_header);

	FILE* file = fopen(filename, "wb");
	if (file) {
		fwrite(&header, sizeof(elf_header), 1, file);
		fwrite(&text_header, sizeof(elf_program_header), 1, file);
		fwrite(data, 1, size, file);
	} else {
		return -1;
	}

	fclose(file);

	chmod(filename, 00777);

	return EXIT_SUCCESS;
}

int main(int argc, const char* argv[])
{
	unsigned char* data;
	size_t data_size;

	if (argc < 3) {
		fprintf(stderr, "usage: kyouc [input] [output]\n");
		return EXIT_FAILURE;
	}

	if (read_file(argv[1], &data, &data_size) != FILE_IO_SUCCESS) {
		fprintf(stderr, "failed to read data from file %s!\n", argv[1]);
		return EXIT_FAILURE;
	}

	AST ast;
	if (build_ast(&ast, data, data_size) != AST_SUCCESS) {
		return EXIT_FAILURE;
	}

	return compile(&ast, argv[2]);
}