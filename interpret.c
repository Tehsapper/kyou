#include "interpret.h"

#include <stdint.h>
#include <stdio.h>

int interpret_ast(AST ast)
{
	int64_t fire = 0, water = 0, tree = 0, metal = 0, earth = 0;

	int64_t value;
	int64_t* reg;

	for (size_t i = 0; i < ast.size; ++i) {
		switch (ast.nodes[i].type) {
			case MOVE_STATEMENT:
				switch (ast.nodes[i].move_src.type) {
					case SOURCE_REGISTER: value = (&fire)[ast.nodes[i].move_src.as_reg]; break;
					case SOURCE_IMMEDIATE: value = ast.nodes[i].move_src.as_immediate; break;
					default: fprintf(stderr, "error: source type %d is not implemented\n", ast.nodes[i].move_src.type); return 0;
				}

				switch (ast.nodes[i].move_dest.type) {
					case DESTINATION_REGISTER:
						(&fire)[ast.nodes[i].move_dest.as_reg] = value;
						break;
					case DESTINATION_FD: 
						if (ast.nodes[i].move_dest.as_fd == 1) {
							printf("%lld\n", value);
							break;
						} else {
							fprintf(stderr, "error: destination fd %d is not implemented\n", ast.nodes[i].move_dest.as_fd);
							return 0;
						}
					default:
						fprintf(stderr, "error: destination type %d is not implemented\n", ast.nodes[i].move_dest.type);
						return 0;
				}
				break;
			case OPERATOR_STATEMENT:
				reg = &(&fire)[ast.nodes[i].op_reg];
				
				switch (ast.nodes[i].op_src.type) {
					case SOURCE_REGISTER: value = (&fire)[ast.nodes[i].op_src.as_reg]; break;
					case SOURCE_IMMEDIATE: value = ast.nodes[i].op_src.as_immediate; break;
					default: fprintf(stderr, "error: source type %d is not implemented\n", ast.nodes[i].op_src.type); return 0;
				}

				switch (ast.nodes[i].op_type) {
					case OP_ADD: *reg += value; break;
					case OP_SUB: *reg -= value; break;
					default: fprintf(stderr, "error: uknown operator type %d\n", ast.nodes[i].op_type); return 0;
				}
				break;
			default:
				fprintf(stderr, "error: uknown statement type %d\n", ast.nodes[i].type);
				return 0;
		}
	}
	return 1;
}
