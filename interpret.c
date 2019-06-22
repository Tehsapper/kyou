#include "interpret.h"

#include "hash.h"

#include <stdint.h>
#include <stdio.h>

const char* ast_names[] = {
	"MOVE_STATEMENT",
	"OPERATOR_STATEMENT",
	"LABEL",
	"BRANCH_STATEMENT",
	"TEMP_STR_PRINT"
};

int interpret_ast(AST ast)
{
	//int64_t fire = 0, water = 0, tree = 0, metal = 0, earth = 0;
	int64_t regs[5] = { 0 };

	int64_t value;
	int64_t* reg;
	AST_node* j;
	int64_t a;
	int64_t b;

	struct hash_table* labels = hash_create(djb2, string_equals, 16);

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

	for (AST_node* node = &ast.nodes[0]; node != &ast.nodes[ast.size]; ++node) {
		//fprintf(stderr, "will execute node type %d\n", node->type);
		switch (node->type) {
			case MOVE_STATEMENT:

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
				break;
			case OPERATOR_STATEMENT:
				reg = &regs[node->op_reg];
				
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
				break;
			case LABEL:
				//fprintf(stderr, "skipped label\n");
				break;
			case BRANCH_STATEMENT:
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
						node = j;
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
							node = j;
						}
						break;
				}
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
