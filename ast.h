#pragma once

#include <stddef.h>
#include <stdint.h>

typedef enum { POWER_SPRING, POWER_SUMMER, POWER_AUTUMN, POWER_WINTER, POWER_STRING, POWER_CHAR } kyou_power_t;
typedef enum { REG_FIRE, REG_WATER, REG_TREE, REG_METAL, REG_EARTH, REG_STORAGE, REG_STORAGE_BASE } kyou_register_t;
typedef enum {
	MOVE_STATEMENT,
	OPERATOR_STATEMENT,
	LABEL,
	BRANCH_STATEMENT,
	PUSH_STATEMENT,
	POP_STATEMENT,
	CALL_STATEMENT,
	RETURN_STATEMENT,
	STORE,
	TEMP_STR_PRINT
} AST_node_type;

typedef struct {
	kyou_power_t power;
	union {
		int8_t as_int8;
		int16_t as_int16;
		int32_t as_int32;
		int64_t as_int64;
		const char* as_string;
		char as_char;
	};
}
AST_value;

typedef struct {
	enum { ADDRESS_LABEL, ADDRESS_IMMEDIATE, ADDRESS_REGISTER } type;
	union {
		const char* as_label;
		size_t as_immediate;
		kyou_register_t as_reg;
	};
}
AST_address;

typedef struct {
	enum { SOURCE_REGISTER, SOURCE_IMMEDIATE, SOURCE_MEM, SOURCE_FD, SOURCE_LABEL } type;
	kyou_power_t power;
	union {
		kyou_register_t as_reg;
		int64_t as_immediate;
		AST_address as_mem;
		int as_fd;
		const char* as_label;
	};
}
AST_source;

typedef struct {
	enum { DESTINATION_REGISTER, DESTINATION_MEM, DESTINATION_FD } type;
	kyou_power_t power;
	union
	{
		kyou_register_t as_reg;
		AST_address as_mem;
		int as_fd;
	};
}
AST_destination;

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
			kyou_power_t op_power;
			enum { OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD, OP_OR, OP_AND, OP_XOR } op_type;
			AST_source op_src;
		};
		struct {
			union {
				const char* id;
				size_t as_size_t;
			};
		};
		struct {
			enum { BRANCH_ALWAYS, BRANCH_GREATER, BRANCH_LESS, BRANCH_EQUALS, BRANCH_GREATER_OR_EQ, BRANCH_LESS_OR_EQ } branch_type;
			AST_address branch_addr;
			AST_source branch_a;
			AST_source branch_b;
		};
		struct {
			AST_source push_from;
		};
		struct {
			AST_destination pop_to;
		};
		struct {
			AST_address call_to;
		};
		struct {
			AST_value value;
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
