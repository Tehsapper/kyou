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
	"STORE",
	"TEMP_STR_PRINT"
};

int64_t regs[7] = { 0 };
struct hash_table* labels;
struct hash_table* strings;

#define reg_stack_ptr regs[REG_STORAGE]
#define reg_base_stack_ptr regs[REG_STORAGE_BASE]

#define STACK_PUSH(type, value) do { *((type*)(regs[REG_STORAGE])) = (value); regs[REG_STORAGE] += sizeof(type); } while (0)
#define STACK_POP(type, var) do { regs[REG_STORAGE] -= sizeof(type); var = *((type*)(regs[REG_STORAGE])); } while (0)

static int evaluate_address(AST_address* addr, void** value)
{
	switch (addr->type) {
		case ADDRESS_REGISTER:
			*value = &regs[addr->as_reg];
			return 1;
		case ADDRESS_LABEL:
			*value = hash_get(labels, addr->as_label);
			//fprintf(stderr, "address points to %p node\n", j);
			if (*value == NULL) {
				fprintf(stderr, "error: no such label %s\n", addr->as_label);
				return 0;
			} else {
				*value += sizeof(AST_node);
			}
			return 1;
		case ADDRESS_IMMEDIATE:
			*value = (void*)addr->as_immediate;
			return 1;
		default:
			fprintf(stderr, "error: unknown address type %d\n", addr->type);
			return 0;
	}
}

static int evaluate_source(AST_source* src, int64_t* value)
{
	switch (src->type) {
		case SOURCE_REGISTER:
			*value = regs[src->as_reg];
			return 1;
		case SOURCE_IMMEDIATE:
			*value = src->as_immediate;
			return 1;
		case SOURCE_MEM:
			if (!evaluate_address(&src->as_mem, (void**)&value))
				return 0;
			*value = *((int64_t*)(*value));
			return 1;
		case SOURCE_LABEL:
			*value = (int64_t)hash_get(labels, src->as_label);
			if (*((int64_t*)value) == 0) {
				fprintf(stderr, "error: no such label %s\n", src->as_label);
				return 0;
			}
			return 1;
		default:
			fprintf(stderr, "error: source type %d is not implemented\n", src->type);
			return 0;
	}
}

static int evaluate_destination(AST_destination* dest, int64_t value, kyou_power_t power)
{
	if (power <= POWER_WINTER && dest->power <= POWER_WINTER && power > dest->power) {
		fprintf(stderr, "error: source has bigger power than destination\n");
		return 0;
	}

	switch (dest->type) {
		case DESTINATION_REGISTER:
			if (dest->as_reg > 7) {
				fprintf(stderr, "error: bad register id %d\n", dest->as_reg);
				return 0;
			}

			regs[dest->as_reg] = value;
			return 1;
		case DESTINATION_FD: 
			if (dest->as_fd == 1) {
				if (power <= POWER_WINTER) printf("%lld\n", value);
				if (power == POWER_STRING) printf("%s\n", (const char*)value);
				if (power == POWER_CHAR) printf("%c\n", (char)value);
				return 1;
			} else {
				fprintf(stderr, "error: destination fd %d is not implemented\n", dest->as_fd);
				return 0;
			}
		case DESTINATION_MEM: {
			int64_t* addr;
			if (!evaluate_address(&dest->as_mem, (void**)&addr))
				return 0;
			*addr = value;
			}
			return 1;
		default:
			fprintf(stderr, "error: unknown destination type %d\n", dest->type);
			return 0;
	}
}

static int interpret_move(AST_node* node)
{
	int64_t value;

	if (!evaluate_source(&node->move_src, &value))
		return 0;

	if (!evaluate_destination(&node->move_dest, value, node->move_src.power))
		return 0;

	return 1;
}

static int interpret_op(AST_node* node)
{
	int64_t* reg = &regs[node->op_reg];
	int64_t value;

	if (!evaluate_source(&node->op_src, &value))
		return 0;

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

	if (!evaluate_address(&node->branch_addr, (void**)&j))
		return 0;

	if (node->branch_addr.type == ADDRESS_LABEL)
		j -= 1; // the loop will advance the node, TODO: maybe continue; if branch is successful?

	if (node->branch_type == BRANCH_ALWAYS) {
		*i = j;
	} else {
		if (!evaluate_source(&node->branch_a, &a) || !(evaluate_source(&node->branch_b, &b)))
			return 0;

		switch (node->branch_type) {
			case BRANCH_EQUALS:
				if (a == b) *i = j;
				break;
			case BRANCH_GREATER:
				if (a > b) *i = j;
				break;
			case BRANCH_GREATER_OR_EQ:
				if (a >= b) *i = j;
				break;
			case BRANCH_LESS:
				if (a < b) *i = j;
				break;
			case BRANCH_LESS_OR_EQ:
				if (a <= b) *i = j;
				break;
			default:
				fprintf(stderr, "unknown branch type %d\n", node->branch_type);
				return 0;
		}
	}

	return 1;
}

int interpret_push(AST_node* node)
{
	int64_t value;

	if (!evaluate_source(&node->push_from, &value))
		return 0;

	STACK_PUSH(int64_t, value);

	return 1;
}

int interpret_pop(AST_node* node)
{
	int64_t value;

	STACK_POP(int64_t, value);

	if (!evaluate_destination(&node->pop_to, value, node->pop_to.power))
		return 0;

	return 1;
}

int interpret_call(AST_node* node, AST_node** i)
{
	AST_node* j;

	if (!evaluate_address(&node->call_to, (void**)&j))
		return 0;

	if (node->call_to.type == ADDRESS_LABEL)
		--j;

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
	labels = hash_create(djb2, string_equals, 16);

	for (size_t i = 0; i < ast.size; ++i) {
		//fprintf(stderr, "%s\n", ast_names[ast.nodes[i].type]);
		if (ast.nodes[i].type == LABEL) {
			if (hash_get(labels, ast.nodes[i].id) == NULL) {
				//fprintf(stderr, "added label %s with ptr %p\n", ast.nodes[i].id, &ast.nodes[i]);
				hash_add(labels, ast.nodes[i].id, &ast.nodes[i]);
			} else {
				fprintf(stderr, "error: same label %s declared twice\n", ast.nodes[i].id);
				return 0;
			}
		}
	}

	int64_t *stack = malloc(sizeof(int64_t) * 32);
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
