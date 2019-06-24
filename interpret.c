#include "interpret.h"

#include "hash.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

const char* ast_names[] = {
	"MOVE_STATEMENT",
	"OPERATOR_STATEMENT",
	"LABEL",
	"BRANCH_STATEMENT",
	"PUSH_STATEMENT",
	"POP_STATEMENT",
	"CALL_STATEMENT",
	"RETURN_STATEMENT",
	"TEMP_STR_PRINT"
};

int64_t regs[7] = { 0 };
struct hash_table* labels;

#define reg_stack_ptr regs[REG_STORAGE]
#define reg_base_stack_ptr regs[REG_STORAGE_BASE]

#define STACK_PUSH(type, value) do { *((type*)(regs[REG_STORAGE])) = (value); regs[REG_STORAGE] += sizeof(type); } while (0)
#define STACK_POP(type, var) do { regs[REG_STORAGE] -= sizeof(type); var = *((type*)(regs[REG_STORAGE])); } while (0)

static int interpret_move(AST_node* node)
{
	int64_t value;

	switch (node->move_src.type) {
		case SOURCE_REGISTER:
			value = regs[node->move_src.as_reg];
			break;
		case SOURCE_IMMEDIATE:
			value = node->move_src.as_immediate;
			break;
		default:
			fprintf(stderr, "error: source type %d is not implemented\n", node->move_src.type);
			return 0;
	}

	switch (node->move_dest.type) {
		case DESTINATION_REGISTER:
			regs[node->move_dest.as_reg] = value;
			break;
		case DESTINATION_FD: 
			if (node->move_dest.as_fd == 1) {
				printf("%lld\n", value);
				break;
			} else {
				fprintf(stderr, "error: destination fd %d is not implemented\n", node->move_dest.as_fd);
				return 0;
			}
		default:
			fprintf(stderr, "error: unknown destination type %d\n", node->move_dest.type);
			return 0;
	}

	return 1;
}

static int interpret_op(AST_node* node)
{
	int64_t* reg = &regs[node->op_reg];
	int64_t value;
				
	switch (node->op_src.type) {
		case SOURCE_REGISTER:
			value = regs[node->op_src.as_reg];
			break;
		case SOURCE_IMMEDIATE:
			value = node->op_src.as_immediate;
			break;
		default:
			fprintf(stderr, "error: source type %d is not implemented\n", node->op_src.type);
			return 0;
	}

	switch (node->op_type) {
		case OP_ADD: *reg += value; break;
		case OP_SUB: *reg -= value; break;
		case OP_MUL: *reg *= value; break;
		case OP_DIV: *reg /= value; break;
		case OP_MOD: *reg %= value; break;
		default:
			fprintf(stderr, "error: unknown operator type %d\n", node->op_type);
			return 0;
	}

	return 1;
}

static int interpret_branch(AST_node* node, AST_node** i)
{
	AST_node* j;
	int64_t a;
	int64_t b;

	switch (node->branch_addr.type) {
		case ADDRESS_LABEL:
			j = (AST_node*)hash_get(labels, node->branch_addr.as_label);
			//fprintf(stderr, "address points to %p node\n", j);
			if (j == NULL) {
				fprintf(stderr, "error: no such label %s\n", node->branch_addr.as_label);
				return 0;
			}
			break;
		case ADDRESS_REGISTER:
			j = (AST_node*)regs[node->branch_addr.as_reg];
			break;
		case ADDRESS_IMMEDIATE:
			j = (AST_node*)node->branch_addr.as_immediate;
			break;
		case ADDRESS_MEM:
			fprintf(stderr, "memory branching is not implemented\n");
			return 0;
		default:
			fprintf(stderr, "unknown address type %d\n", node->branch_addr.type);
			break;
	}

	switch (node->branch_type) {
		case BRANCH_ALWAYS:
			*i = j;
			break;
		default:
			switch (node->branch_a.type) {
				case SOURCE_REGISTER:
					a = regs[node->branch_a.as_reg];
					break;
				case SOURCE_IMMEDIATE:
					a = node->branch_a.as_immediate;
					break;
				default:
					fprintf(stderr, "error: source type %d is not implemented\n", node->branch_a.type);
					return 0;
			}

			switch (node->branch_b.type) {
				case SOURCE_REGISTER:
					b = regs[node->branch_b.as_reg];
					break;
				case SOURCE_IMMEDIATE:
					b = node->branch_b.as_immediate;
					break;
				default:
					fprintf(stderr, "error: source type %d is not implemented\n", node->branch_b.type);
					return 0;
			}

			if ((node->branch_type == BRANCH_EQUALS && a == b) || (node->branch_type == BRANCH_GREATER && a > b) || (node->branch_type == BRANCH_LESS && a < b)) {
				*i = j;
			}
			break;
	}
	return 1;
}

int interpret_push(AST_node* node)
{
	int64_t value;

	switch (node->push_from.type) {
		case SOURCE_REGISTER:
			value = regs[node->push_from.as_reg];
			break;
		case SOURCE_IMMEDIATE:
			value = node->push_from.as_immediate;
			break;
		default:
			fprintf(stderr, "error: source type %d is not implemented\n", node->push_from.type);
			return 0;
	}

	STACK_PUSH(int64_t, value);

	return 1;
}

int interpret_pop(AST_node* node)
{
	int64_t value;

	STACK_POP(int64_t, value);

	switch (node->pop_to.type) {
		case DESTINATION_REGISTER:
			regs[node->pop_to.as_reg] = value;
			break;
		case DESTINATION_FD: 
			if (node->pop_to.as_fd == 1) {
				printf("%lld\n", value);
				break;
			} else {
				fprintf(stderr, "error: destination fd %d is not implemented\n", node->pop_to.as_fd);
				return 0;
			}
		default:
			fprintf(stderr, "error: unknown destination type %d\n", node->pop_to.type);
			return 0;
	}

	return 1;
}

int interpret_call(AST_node* node, AST_node** i)
{
	AST_node* j;

	switch (node->call_to.type) {
		case ADDRESS_LABEL:
			j = (AST_node*)hash_get(labels, node->call_to.as_label);
			//fprintf(stderr, "address points to %p node\n", j);
			if (j == NULL) {
				fprintf(stderr, "error: no such label %s\n", node->call_to.as_label);
				return 0;
			}
			break;
		case ADDRESS_REGISTER:
			j = (AST_node*)regs[node->call_to.as_reg];
			break;
		case ADDRESS_IMMEDIATE:
			j = (AST_node*)node->call_to.as_immediate;
			break;
		case ADDRESS_MEM:
			fprintf(stderr, "memory branching is not implemented\n");
			return 0;
		default:
			fprintf(stderr, "unknown address type %d\n", node->call_to.type);
			break;
	}

	STACK_PUSH(AST_node*, node);
	*i = j;
	return 1;
}

int interpret_return(AST_node* node, AST_node** i)
{
	STACK_POP(AST_node*, *i);
	return 1;
}

int interpret_ast(AST ast)
{
	//int64_t fire = 0, water = 0, tree = 0, metal = 0, earth = 0;
	labels = hash_create(djb2, string_equals, 16);

	for (size_t i = 0; i < ast.size; ++i) {
		fprintf(stderr, "%s\n", ast_names[ast.nodes[i].type]);
		if (ast.nodes[i].type == LABEL) {
			if (hash_get(labels, ast.nodes[i].id) == NULL) {
				fprintf(stderr, "added label %s with ptr %p\n", ast.nodes[i].id, &ast.nodes[i]);
				hash_add(labels, ast.nodes[i].id, &ast.nodes[i]);
			} else {
				fprintf(stderr, "error: same label %s declared twice\n", ast.nodes[i].id);
				return 0;
			}
		}
	}

	int64_t *stack = malloc(sizeof(int64_t) * 32);
	//*((int64_t**)(&regs[REG_STORAGE])) = stack;
	//*((int64_t**)(&regs[REG_STORAGE_BASE])) = stack;
	regs[REG_STORAGE] = (int64_t)stack;
	regs[REG_STORAGE_BASE] = (int64_t)stack;

	for (AST_node* node = &ast.nodes[0]; node != &ast.nodes[ast.size]; ++node) {
		//fprintf(stderr, "will execute node type %d\n", node->type);
		switch (node->type) {
			case MOVE_STATEMENT:
				if (!interpret_move(node))
					return 0;
				break;
			case OPERATOR_STATEMENT:
				if (!interpret_op(node))
					return 0;
				break;
			case LABEL:
				//fprintf(stderr, "skipped label\n");
				break;
			case BRANCH_STATEMENT:
				if (!interpret_branch(node, &node))
					return 0;
				break;
			case PUSH_STATEMENT:
				if (!interpret_push(node))
					return 0;
				break;
			case POP_STATEMENT:
				if (!interpret_pop(node))
					return 0;
				break;
			case CALL_STATEMENT:
				if (!interpret_call(node, &node))
					return 0;
				break;
			case RETURN_STATEMENT:
				if (!interpret_return(node, &node))
					return 0;
				break;
			case TEMP_STR_PRINT:
				printf("%s\n", node->id);
				break;
			default:
				fprintf(stderr, "error: unknown statement type %d\n", node->type);
				return 0;
		}
	}
	return 1;
}
