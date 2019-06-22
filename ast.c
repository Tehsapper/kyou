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

#define IS_ADDRESS(t) (IS_REGISTER(t) || IS_NUMBER(t) || (t) == TOKEN_STARS || (t) == TOKEN_LABEL)
#define IS_BRANCH_TYPE(t) ((t) == TOKEN_EQUALS || (t) == TOKEN_LESS || (t) == TOKEN_GREATER)

#define IS_OP(t) ((t) == TOKEN_ADD || (t) == TOKEN_SUB || (t) == TOKEN_MUL || (t) == TOKEN_DIV || (t) == TOKEN_MOD)

const char* token_str[] = {
	// three lights: stdout, stdin and virtual memory
	"TOKEN_SUN",
	"TOKEN_MOON",
	"TOKEN_STARS",
	
	// additional tokens for stack
	"TOKEN_STORAGE",
	"TOKEN_STORAGE_BASE",
	
	// five elements
	"TOKEN_FIRE",
	"TOKEN_WATER",
	"TOKEN_TREE",
	"TOKEN_METAL",
	"TOKEN_EARTH",

	// four seasons
	"TOKEN_SPRING",
	"TOKEN_SUMMER",
	"TOKEN_AUTUMN",
	"TOKEN_WINTER",

	// operations
	"TOKEN_MOVE",
	"TOKEN_ADD",
	"TOKEN_SUB",
	"TOKEN_MUL",
	"TOKEN_DIV",
	"TOKEN_MOD",

	"TOKEN_IDENTIFIER",
	"TOKEN_NUMBER",
	"TOKEN_STRING",

	"TOKEN_LABEL",
	"TOKEN_BRANCH",
	"TOKEN_EQUALS",
	"TOKEN_LESS",
	"TOKEN_GREATER",

	"TOKEN_EOF",
	"TOKEN_NONE"
};

token *t, *st;

#define NEXT_TOKEN *st++
#define ROLLBACK_TOKEN do { st = t; return 0; } while (0)
#define EXPECTED(new_token, predicate) token new_token = NEXT_TOKEN;\
if (! (predicate) ) {\
	fprintf(stderr, "predicate " #predicate " failed for %s\n", token_str[new_token.type]);\
	return 2;\
}
#define EXPECTED_IF(new_token, first_predicate, token_predicate) token new_token = NEXT_TOKEN;\
if ( ! (first_predicate) ) {\
	--st;\
	new_token.type = TOKEN_NONE;\
} else if (! (token_predicate) ) {\
	return 2;\
}
#define MAYBE(new_token, predicate) token new_token = NEXT_TOKEN; if (! (predicate) ) { fprintf(stderr, "predicate " #predicate " did not work for %s\n", token_str[new_token.type]); ROLLBACK_TOKEN; }
#define OPTIONAL(new_token, predicate) token new_token = NEXT_TOKEN; if (! (predicate)) { --st; new_token.type = TOKEN_NONE; }
#define ACCEPT do { t = st; return 1; } while (0)

static int add_ast_node(AST* ast, AST_node node)
{
	ast->nodes = realloc(ast->nodes, sizeof(AST_node) * ++ast->size);
	if (ast->nodes == NULL)
		return 0;
	ast->nodes[ast->size - 1] = node;
	return 1;
}


static int register_from_token(kyou_register_t* dest, token tok)
{
	switch (tok.type) {
		case TOKEN_FIRE: *dest = REG_FIRE; return 1;
		case TOKEN_WATER: *dest = REG_WATER; return 1;
		case TOKEN_TREE: *dest = REG_TREE; return 1;
		case TOKEN_EARTH: *dest = REG_EARTH; return 1;
		case TOKEN_METAL: *dest = REG_METAL; return 1;
		default: { fprintf(stderr, "%s is not a register token\n", token_str[tok.type]); return 0; }
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
		
		case TOKEN_NUMBER:
			src->type = SOURCE_IMMEDIATE;
			src->as_immediate = tok.as_int64;
			return 1;

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

		case TOKEN_SUN:
			dest->type = DESTINATION_FD;
			dest->as_fd = 1;
			return 1;

		case TOKEN_STARS:
			fprintf(stderr, "sorry, star is not implemented yet\n");
		default:
			return 0;
	}
}


static int address_from_token(AST_address* addr, token tok, token id)
{
	switch (tok.type) {
		case TOKEN_FIRE:
		case TOKEN_WATER:
		case TOKEN_TREE:
		case TOKEN_EARTH:
		case TOKEN_METAL:
			addr->type = ADDRESS_REGISTER;
			return register_from_token(&addr->as_reg, tok);
		case TOKEN_LABEL:
			addr->type = ADDRESS_LABEL;
			addr->as_label = id.as_cstr;
			return 1;
		case TOKEN_NUMBER:
			addr->type = ADDRESS_IMMEDIATE;
			addr->as_immediate = (size_t)tok.as_int64;
			return 1;
		case TOKEN_STARS:
			fprintf(stderr, "sorry, star is not implemented yet\n");
		default:
			return 0;
	}
}

int arithm_op_rule(AST* ast)
{
	// arithmetic operation statement
	if (IS_REGISTER(st->type))
	{
		token reg_tok = NEXT_TOKEN;
		MAYBE(op_tok, IS_OP(op_tok.type))
		EXPECTED(src_tok, IS_SOURCE(src_tok.type))

		AST_node node;
		node.type = OPERATOR_STATEMENT;

		switch (op_tok.type) {
			case TOKEN_ADD: node.op_type = OP_ADD; break;
			case TOKEN_SUB: node.op_type = OP_SUB; break;
			case TOKEN_MUL: node.op_type = OP_MUL; break;
			case TOKEN_DIV: node.op_type = OP_DIV; break;
			case TOKEN_MOD: node.op_type = OP_MOD; break;
			default: return 2;
		}
		if (!register_from_token(&node.op_reg, reg_tok))
			return 2;
		if (!source_from_token(&node.op_src, src_tok))
			return 2;
		add_ast_node(ast, node);
		ACCEPT;
	}
	return 0;
}

int move_rule(AST* ast)
{
	// move statement
	if (IS_SOURCE(t->type))
	{
		token source_tok = NEXT_TOKEN;
		MAYBE(move_tok, move_tok.type == TOKEN_MOVE)
		EXPECTED(dest_tok, IS_DESTINATION(dest_tok.type))

		AST_node node;
		node.type = MOVE_STATEMENT;
		if (!source_from_token(&node.move_src, source_tok))
			return 2;
		if (!destination_from_token(&node.move_dest, dest_tok))
			return 2;

		add_ast_node(ast, node);
		ACCEPT;
	}
	return 0;
}

int label_rule(AST* ast)
{
	MAYBE(label_tok, label_tok.type == TOKEN_LABEL)
	EXPECTED(id_tok, id_tok.type == TOKEN_IDENTIFIER)

	AST_node node;
	node.type = LABEL;
	node.id = id_tok.as_cstr;
	add_ast_node(ast, node);
	ACCEPT;
}

int branch_rule(AST* ast)
{
	MAYBE(branch_tok, branch_tok.type == TOKEN_BRANCH)
	EXPECTED(addr_tok, IS_ADDRESS(addr_tok.type))
	EXPECTED_IF(id_tok, addr_tok.type == TOKEN_LABEL, id_tok.type == TOKEN_IDENTIFIER)
	OPTIONAL(a_tok, IS_SOURCE(a_tok.type))

	if (a_tok.type != TOKEN_NONE) {
		EXPECTED(type_tok, IS_BRANCH_TYPE(type_tok.type))
		EXPECTED(b_tok, IS_SOURCE(b_tok.type))

		AST_node node;
		node.type = BRANCH_STATEMENT;
		node.branch_type = (type_tok.type == TOKEN_EQUALS ? BRANCH_EQUALS : (type_tok.type == TOKEN_GREATER ? BRANCH_GREATER : BRANCH_LESS ));
		if (!address_from_token(&node.branch_addr, addr_tok, id_tok))
			return 2;

		if (!source_from_token(&node.branch_a, a_tok))
			return 2;
		if (!source_from_token(&node.branch_b, b_tok))
			return 2;

		add_ast_node(ast, node);
		ACCEPT;
	} else {
		// absolute branch
		AST_node node;
		node.type = BRANCH_STATEMENT;
		node.branch_type = BRANCH_ALWAYS;
		if (!address_from_token(&node.branch_addr, addr_tok, id_tok))
			return 2;
		add_ast_node(ast, node);
		ACCEPT;
	}
}

int temp_str_print(AST* ast)
{
	MAYBE(str_tok, str_tok.type == TOKEN_STRING)
	EXPECTED(mov_tok, mov_tok.type == TOKEN_MOVE)
	EXPECTED(sun_tok, sun_tok.type == TOKEN_SUN)

	AST_node node;
	node.type = TEMP_STR_PRINT;
	node.id = str_tok.as_cstr;
	add_ast_node(ast, node);
	ACCEPT;
}

#undef ACCEPT
#undef OPTIONAL
#undef MAYBE
#undef EXPECT
#undef ROLLBACK_TOKEN
#undef NEXT_TOKEN

ast_result_t build_ast(AST* ast, unsigned char* data, size_t data_size)
{
	ast->nodes = NULL;
	ast->size = 0;

	tokens toks;
	if (tokenize(&toks, (const char*)data, data_size) == TOKENIZE_ERROR) {
		fprintf(stderr, "failed to tokenize, aborting AST building\n");
		return AST_ERROR;
	}

	for (size_t i = 0; i < toks.size; ++i) {
		fprintf(stderr, "%s\n", token_str[toks.data[i].type]);
	}

	for (t = toks.data; t < toks.data + toks.size;)
	{	
		st = t;
		if (st->type == TOKEN_EOF)
			break;
		int rule_result;
#define CHECK_RULE(func) rule_result = func(ast); if (rule_result == 1) continue; if (rule_result == 2) goto error;

		CHECK_RULE(arithm_op_rule)
		CHECK_RULE(move_rule)
		CHECK_RULE(label_rule)
		CHECK_RULE(branch_rule)
		CHECK_RULE(temp_str_print)

error:
		fprintf(stderr, "syntax error at line %u, %u\n", t->line, t->col);
		return AST_ERROR;
	}
	return AST_SUCCESS;

}
