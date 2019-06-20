#include "ast.h"

#include "tokens.h"

#include <stdio.h>
#include <stdlib.h>

#define IS_REGISTER(t) ((t) == TOKEN_FIRE || (t) == TOKEN_WATER || (t) == TOKEN_TREE || (t) == TOKEN_METAL || (t) == TOKEN_EARTH)
#define IS_NUMBER(t) ((t) == TOKEN_NUMBER)

#define IS_OUTPUT(t) ((t) == TOKEN_SUN || (t) == TOKEN_STARS)
#define IS_INPUT(t) ((t) == TOKEN_MOON || (t) == TOKEN_STARS)

#define IS_DESTINATION(t) (IS_REGISTER(t) || IS_OUTPUT(t))
#define IS_SOURCE(t) (IS_REGISTER(t) || IS_INPUT(t) || IS_NUMBER(t))

#define IS_OP(t) ((t) == TOKEN_ADD)

/*token get_next_token(token* cur, tokens toks)
{
	return cur[1];
}*/

static int add_ast_node(AST* ast, AST_node node)
{
	ast->nodes = realloc(ast->nodes, sizeof(AST_node) * ++ast->size);
	if (ast->nodes == NULL)
		return 0;
	ast->nodes[ast->size - 1] = node;
	return 1;
}


static int register_from_token(register_t* dest, token tok)
{
	switch (tok.type) {
		case TOKEN_FIRE: *dest = REG_FIRE; return 1;
		case TOKEN_WATER: *dest = REG_WATER; return 1;
		case TOKEN_TREE: *dest = REG_TREE; return 1;
		case TOKEN_EARTH: *dest = REG_EARTH; return 1;
		case TOKEN_METAL: *dest = REG_METAL; return 1;
		default: return 0;
	}
}

static int source_from_token(AST_source* src, token tok)
{
	switch (tok.type) {
		case TOKEN_FIRE:
		case TOKEN_WATER:
		case TOKEN_TREE:
		case TOKEN_EARTH:
		case TOKEN_METAL:
			src->type = SOURCE_REGISTER;
			return register_from_token(&src->as_reg, tok);
		
		case TOKEN_NUMBER: src->type = SOURCE_IMMEDIATE; src->as_immediate = tok.as_int64; return 1;

		case TOKEN_MOON:
		case TOKEN_STARS:
			fprintf(stderr, "sorry, moon-and-star is not implemented yet\n");
		default:
			return 0;
	}
}

static int destination_from_token(AST_destination* dest, token tok)
{
	switch (tok.type) {
		case TOKEN_FIRE:
		case TOKEN_WATER:
		case TOKEN_TREE:
		case TOKEN_EARTH:
		case TOKEN_METAL:
			dest->type = DESTINATION_REGISTER;
			return register_from_token(&dest->as_reg, tok);

		case TOKEN_SUN: dest->type = DESTINATION_FD; dest->as_fd = 1; return 1;

		case TOKEN_STARS:
			fprintf(stderr, "sorry, star is not implemented yet\n");
		default:
			return 0;
	}
}

ast_result_t build_ast(AST* ast, unsigned char* data, size_t data_size)
{
	ast->nodes = NULL;
	ast->size = 0;

	tokens toks;
	if (tokenize(&toks, (const char*)data, data_size) == TOKENIZE_ERROR) {
		fprintf(stderr, "failed to tokenize, aborting AST building\n");
		return AST_ERROR;
	}

	for (token* t = toks.data; t < toks.data + toks.size;)
	{	
		token* st = t;
		
#define NEXT_TOKEN *st++
#define EXPECTED(new_token, predicate) token new_token = NEXT_TOKEN; if (! (predicate) ) goto error;
#define ACCEPT t = st; continue
		// arithmetic operation statement
		if (IS_REGISTER(t->type))
		{
			token reg_tok = *st;
			EXPECTED(op_tok, IS_OP(op_tok.type))
			EXPECTED(src_tok, IS_SOURCE(src_tok.type))

			AST_node node;
			node.type = OPERATOR_STATEMENT;
			node.op_type = OP_ADD;
			if (!register_from_token(&node.op_reg, reg_tok))
				goto error;
			if (!source_from_token(&node.op_src, src_tok))
				goto error;
			add_ast_node(ast, node);
			ACCEPT;
		}

		// move statement
		if (IS_SOURCE(t->type))
		{
			token source_tok = *st;
			EXPECTED(move_tok, move_tok.type == TOKEN_MOVE)
			EXPECTED(dest_tok, IS_DESTINATION(dest_tok.type))

			AST_node node;
			node.type = MOVE_STATEMENT;
			if (!source_from_token(&node.move_src, source_tok))
				goto error;
			if (!destination_from_token(&node.move_dest, dest_tok))
				goto error;

			add_ast_node(ast, node);
			ACCEPT;
		}
error:
		fprintf(stderr, "syntax error at line %u, %u\n", t->line, t->col);
		return AST_ERROR;
	}
	return AST_SUCCESS;

}
