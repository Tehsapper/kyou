#pragma once

#include <stddef.h>
#include <stdint.h>

typedef enum { REG_FIRE, REG_WATER, REG_TREE, REG_METAL, REG_EARTH } register_t;
typedef enum { MOVE_STATEMENT, OPERATOR_STATEMENT } AST_node_type;

typedef struct {
	enum { SOURCE_REGISTER, SOURCE_IMMEDIATE, SOURCE_MEM, SOURCE_FD } type;
	union {
		register_t as_reg;
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
		register_t as_reg;
		void* as_mem;
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
			register_t op_reg;
			enum { OP_ADD, OP_SUB } op_type;
			AST_source op_src;
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
