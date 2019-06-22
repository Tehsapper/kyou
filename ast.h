#pragma once

#include <stddef.h>
#include <stdint.h>

typedef enum { REG_FIRE, REG_WATER, REG_TREE, REG_METAL, REG_EARTH } kyou_register_t;
typedef enum { MOVE_STATEMENT, OPERATOR_STATEMENT, LABEL, BRANCH_STATEMENT, TEMP_STR_PRINT } AST_node_type;

typedef struct {
	enum { SOURCE_REGISTER, SOURCE_IMMEDIATE, SOURCE_MEM, SOURCE_FD } type;
	union {
		kyou_register_t as_reg;
		int64_t as_immediate;
		void* as_mem;
		int as_fd;
	};
}
AST_source;

typedef struct {
	enum { DESTINATION_REGISTER, DESTINATION_MEM, DESTINATION_FD } type;
	union
	{
		kyou_register_t as_reg;
		void* as_mem;
		int as_fd;
	};
}
AST_destination;

typedef struct {
	enum { ADDRESS_LABEL, ADDRESS_IMMEDIATE, ADDRESS_REGISTER, ADDRESS_MEM } type;
	union {
		const char* as_label;
		size_t as_immediate;
		kyou_register_t as_reg;
		void* as_mem;
	};
}
AST_address;

typedef struct
{
	AST_node_type type;
	union {
		struct {
			AST_source move_src;
			AST_destination move_dest;
		};
		struct {
			kyou_register_t op_reg;
			enum { OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD } op_type;
			AST_source op_src;
		};
		struct {
			union {
				const char* id;
				size_t as_size_t;
			};
		};
		struct {
			enum { BRANCH_ALWAYS, BRANCH_GREATER, BRANCH_LESS, BRANCH_EQUALS } branch_type;
			AST_address branch_addr;
			AST_source branch_a;
			AST_source branch_b;
		};
	};
}
AST_node;

typedef struct
{
	AST_node* nodes;
	size_t size;
}
AST;

typedef enum { AST_SUCCESS, AST_ERROR } ast_result_t;

ast_result_t build_ast(AST* ast, unsigned char* data, size_t data_size);
