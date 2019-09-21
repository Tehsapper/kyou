#include "ast.h"

#include "tokens.h"

#include <stdio.h>
#include <stdlib.h>

#define IS_REGISTER(t) ((t) == TOKEN_FIRE || (t) == TOKEN_WATER || (t) == TOKEN_TREE || (t) == TOKEN_METAL || (t) == TOKEN_EARTH || (t) == TOKEN_STORAGE || (t) == TOKEN_STORAGE_BASE)
#define IS_NUMBER(t) ((t) == TOKEN_NUMBER)

#define IS_OUTPUT(t) ((t) == TOKEN_SUN || (t) == TOKEN_STARS)
#define IS_INPUT(t) ((t) == TOKEN_MOON || (t) == TOKEN_STARS)

#define IS_POWER(t) ((t) == TOKEN_SPRING || (t) == TOKEN_SUMMER || (t) == TOKEN_AUTUMN || (t) == TOKEN_WINTER || (t) == TOKEN_STRING_TYPE || (t) == TOKEN_CHAR)
#define IS_DESTINATION(t) (IS_REGISTER(t) || IS_OUTPUT(t))
#define IS_SOURCE(t) (IS_REGISTER(t) || IS_INPUT(t) || IS_NUMBER(t))

#define IS_ADDRESS(t) (IS_REGISTER(t) || IS_NUMBER(t) || (t) == TOKEN_STARS || (t) == TOKEN_LABEL)
#define IS_BRANCH_TYPE(t) ((t) == TOKEN_EQUALS || (t) == TOKEN_LESS || (t) == TOKEN_GREATER)

#define IS_OP(t) ((t) == TOKEN_ADD || (t) == TOKEN_SUB || (t) == TOKEN_MUL || (t) == TOKEN_DIV || (t) == TOKEN_MOD || (t) == TOKEN_OR || (t) == TOKEN_AND || (t) == TOKEN_XOR)

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
	"TOKEN_STRING_TYPE",
	"TOKEN_CHAR",

	// operations
	"TOKEN_MOVE",
	"TOKEN_PUSH",
	"TOKEN_CALL",
	"TOKEN_RETURN",
	"TOKEN_POP",

	"TOKEN_ADD",
	"TOKEN_SUB",
	"TOKEN_MUL",
	"TOKEN_DIV",
	"TOKEN_MOD",
	"TOKEN_OR",
	"TOKEN_AND",
	"TOKEN_XOR",

	"TOKEN_IDENTIFIER",
	"TOKEN_NUMBER",
	"TOKEN_STRING",

	"TOKEN_LABEL",
	"TOKEN_BRANCH",
	"TOKEN_ALWAYS",
	"TOKEN_EQUALS",
	"TOKEN_LESS",
	"TOKEN_GREATER",

	"TOKEN_EOF",
	"TOKEN_NONE"
};

token *t, *st;

typedef enum { RULE_PASS, RULE_ACCEPT, RULE_ERROR } rule_result_t;

#define ACCEPT do { t = st; return RULE_ACCEPT; } while (0)
#define PASS do { return RULE_PASS; } while(0)
#define NEXT_TOKEN *st++
#define ROLLBACK_ONCE do { --st; } while (0)
#define ROLLBACK_TOKEN do { st = t; PASS; } while (0)
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
#define MAYBE_TOKEN(new_token, predicate) token new_token = NEXT_TOKEN; if (! (predicate) ) { ROLLBACK_TOKEN; }
#define OPTIONAL(new_token, predicate) token new_token = NEXT_TOKEN; if (! (predicate)) { --st; new_token.type = TOKEN_NONE; }
#define OPTIONAL_IF(new_token, first_predicate, token_predicate) token new_token = NEXT_TOKEN; if (token_predicate) {\
		if (!(first_predicate)) { fprintf(stderr, "optional token encountered while 1st predicate failed\n"); return RULE_ERROR; }\
	} else { --st; new_token.type = TOKEN_NONE; }

static int add_ast_node(AST* ast, AST_node node)
{
	ast->nodes = realloc(ast->nodes, sizeof(AST_node) * ++ast->size);
	if (ast->nodes == NULL)
		return 0;
	ast->nodes[ast->size - 1] = node;
	return 1;
}


static int register_from_token(kyou_register_t* dest)
{
	token reg_tok = NEXT_TOKEN;

	switch (reg_tok.type) {
		case TOKEN_FIRE: *dest = REG_FIRE; return 1;
		case TOKEN_WATER: *dest = REG_WATER; return 1;
		case TOKEN_TREE: *dest = REG_TREE; return 1;
		case TOKEN_EARTH: *dest = REG_EARTH; return 1;
		case TOKEN_METAL: *dest = REG_METAL; return 1;
		case TOKEN_STORAGE: *dest = REG_STORAGE; return 1;
		case TOKEN_STORAGE_BASE: *dest = REG_STORAGE_BASE; return 1;
		default: ROLLBACK_ONCE; return 0;
	}
}

static int immediate_from_token(int64_t* dest)
{
	token num_tok = NEXT_TOKEN;

	if (num_tok.type == TOKEN_NUMBER) {
		*dest = num_tok.as_int64;
		return 1;
	}
	else {
		ROLLBACK_ONCE;
		return 0;
	}
}

static int label_from_token(const char** name)
{
	token label_tok = NEXT_TOKEN;
	if (label_tok.type != TOKEN_LABEL) {
		ROLLBACK_ONCE;
		return 0;
	}

	token id_tok = NEXT_TOKEN;
	if (id_tok.type != TOKEN_IDENTIFIER)  {
		ROLLBACK_ONCE;
		return 0;
	}

	*name = id_tok.as_cstr;
	return 1;
}

static int address_from_token(AST_address* dest)
{
	if (register_from_token(&dest->as_reg)) {
		dest->type = ADDRESS_REGISTER;
		return 1;
	}
	else if (label_from_token(&dest->as_label)) {
		dest->type = ADDRESS_LABEL;
		return 1;
	}
	else if (immediate_from_token((size_t*)&dest->as_immediate)) {
		dest->type = ADDRESS_IMMEDIATE;
		return 1;
	}

	return 0;
}

static int mem_from_token(AST_address* dest)
{
	token star_tok = NEXT_TOKEN;
	if (star_tok.type != TOKEN_STARS) {
		ROLLBACK_ONCE;
		return 0;
	}

	return address_from_token(dest);
}

static int fd_from_token(int* fd)
{
	token fd_tok = NEXT_TOKEN;

	if (fd_tok.type != TOKEN_SUN) {
		ROLLBACK_ONCE;
		return 0;
	}

	*fd = 1;

	return 1;
}

static int power_from_token(kyou_power_t* power, token_type type)
{
	switch (type) {
		case TOKEN_SPRING: *power = POWER_SPRING; return 1;
		case TOKEN_SUMMER: *power = POWER_SUMMER; return 1;
		case TOKEN_AUTUMN: *power = POWER_AUTUMN; return 1;
		case TOKEN_WINTER: *power = POWER_WINTER; return 1;
		case TOKEN_STRING_TYPE: *power = POWER_STRING; return 1;
		case TOKEN_CHAR: *power = POWER_CHAR; return 1;
		default: fprintf(stderr, "unknown power type %d\n", type); return 0;
	}
}

static int source_from_token(AST_source* src)
{
	if (register_from_token(&src->as_reg)) {
		src->type = SOURCE_REGISTER;
	}
	else if (immediate_from_token(&src->as_immediate)) {
		src->type = SOURCE_IMMEDIATE;
	}
	else if (mem_from_token(&src->as_mem)) {
		src->type = SOURCE_MEM;
	}
	else if (label_from_token(&src->as_label)) {
		src->type = SOURCE_LABEL;
	}
	else {
		return 0;
	}

	token power_tok = NEXT_TOKEN;
	if (IS_POWER(power_tok.type)) {
		power_from_token(&src->power, power_tok.type);
	} else {
		ROLLBACK_ONCE;
		src->power = POWER_WINTER;
	}

	return 1;
}

static int destination_from_token(AST_destination* dest)
{
	if (register_from_token(&dest->as_reg)) {
		dest->type = DESTINATION_REGISTER;
	}
	else if (fd_from_token(&dest->as_fd)) {
		dest->type = DESTINATION_FD;
	}
	else if (mem_from_token(&dest->as_mem)) {
		dest->type = DESTINATION_MEM;
	}
	else return 0;

	token power_tok = NEXT_TOKEN;
	if (IS_POWER(power_tok.type)) {
		power_from_token(&dest->power, power_tok.type);
	} else {
		ROLLBACK_ONCE;
		dest->power = POWER_WINTER;
	}

	if (dest->type == DESTINATION_FD && IS_POWER(power_tok.type)) {
		fprintf(stderr, "error: sun can't have power\n");
		return 0;
	}

	return 1;
}

int arithm_op_rule(AST* ast)
{
	AST_node node;

	if (!register_from_token(&node.op_reg)) {
		return RULE_PASS;
	}

	token power_tok = NEXT_TOKEN;
	if (IS_POWER(power_tok.type)) {
		power_from_token(&node.op_power, power_tok.type);
	} else {
		ROLLBACK_ONCE;
		node.op_power = POWER_WINTER;
	}

	MAYBE_TOKEN(op_tok, IS_OP(op_tok.type))
	node.type = OPERATOR_STATEMENT;

	switch (op_tok.type) {
		case TOKEN_ADD: node.op_type = OP_ADD; break;
		case TOKEN_SUB: node.op_type = OP_SUB; break;
		case TOKEN_MUL: node.op_type = OP_MUL; break;
		case TOKEN_DIV: node.op_type = OP_DIV; break;
		case TOKEN_MOD: node.op_type = OP_MOD; break;
		default: {
			fprintf(stderr, "unimplemented operator token %s\n", token_str[op_tok.type]);
			return RULE_ERROR;
		}
	}

	if (!source_from_token(&node.op_src)) {
		fprintf(stderr, "failed at unknown source %s\n", token_str[st->type]);
		return RULE_ERROR;
	}
	add_ast_node(ast, node);
	ACCEPT;
}

int move_rule(AST* ast)
{
	AST_node node;

	if (!source_from_token(&node.move_src))
		return RULE_PASS;

	MAYBE_TOKEN(move_tok, move_tok.type == TOKEN_MOVE)
	node.type = MOVE_STATEMENT;

	if (!destination_from_token(&node.move_dest))
		return RULE_ERROR;

	add_ast_node(ast, node);
	ACCEPT;
}

int label_rule(AST* ast)
{
	MAYBE_TOKEN(label_tok, label_tok.type == TOKEN_LABEL)
	EXPECTED(id_tok, id_tok.type == TOKEN_IDENTIFIER)

	add_ast_node(ast, (AST_node){ .type = LABEL, .id = id_tok.as_cstr });
	ACCEPT;
}

int branch_rule(AST* ast)
{
	AST_node node;

	MAYBE_TOKEN(branch_tok, branch_tok.type == TOKEN_BRANCH)
	if (!address_from_token(&node.branch_addr))
		return RULE_ERROR;

	node.type = BRANCH_STATEMENT;

	token always_tok = NEXT_TOKEN;
	if (always_tok.type == TOKEN_ALWAYS) {
		// unconditional jump
		node.branch_type = BRANCH_ALWAYS;
	} else {
		// conditional jump
		ROLLBACK_ONCE;
		
		if (!source_from_token(&node.branch_a))
			return RULE_ERROR;

		EXPECTED(type_tok, IS_BRANCH_TYPE(type_tok.type))
		node.branch_type = (type_tok.type == TOKEN_EQUALS ? BRANCH_EQUALS : (type_tok.type == TOKEN_GREATER ? BRANCH_GREATER : BRANCH_LESS ));

		if (!source_from_token(&node.branch_b))
			return RULE_ERROR;
	}

	add_ast_node(ast, node);
	ACCEPT;
}

int push_rule(AST* ast)
{
	AST_node node;
	
	MAYBE_TOKEN(push_tok, push_tok.type == TOKEN_PUSH)
	if (!source_from_token(&node.push_from))
		return RULE_ERROR;

	node.type = PUSH_STATEMENT;
	
	add_ast_node(ast, node);
	ACCEPT;
}

int pop_rule(AST* ast)
{
	AST_node node;

	MAYBE_TOKEN(pop_tok, pop_tok.type == TOKEN_POP)
	if (!destination_from_token(&node.pop_to))
		return RULE_ERROR;

	node.type = POP_STATEMENT;
	
	add_ast_node(ast, node);
	ACCEPT;
}

int call_rule(AST* ast)
{
	AST_node node;

	MAYBE_TOKEN(call_tok, call_tok.type == TOKEN_CALL)
	if (!address_from_token(&node.call_to))
		return RULE_ERROR;

	node.type = CALL_STATEMENT;
	
	add_ast_node(ast, node);
	ACCEPT;
}

int return_rule(AST* ast)
{
	MAYBE_TOKEN(return_tok, return_tok.type == TOKEN_RETURN)
	add_ast_node(ast, (AST_node) { .type = RETURN_STATEMENT });
	ACCEPT;
}

int temp_str_print(AST* ast)
{
	AST_node node;

	MAYBE_TOKEN(str_tok, str_tok.type == TOKEN_STRING)
	EXPECTED(mov_tok, mov_tok.type == TOKEN_MOVE)
	EXPECTED(sun_tok, sun_tok.type == TOKEN_SUN)
	
	node.type = TEMP_STR_PRINT;
	node.id = str_tok.as_cstr;
	add_ast_node(ast, node);
	ACCEPT;
}

#undef ACCEPT
#undef PASS
#undef OPTIONAL_IF
#undef OPTIONAL
#undef MAYBE_TOKEN
#undef EXPECTED_IF
#undef EXPECTED
#undef ROLLBACK_TOKEN
#undef ROLLBACK_ONCE
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
#define CHECK_RULE(func) rule_result = func(ast); if (rule_result == RULE_ACCEPT) continue; if (rule_result == RULE_ERROR) goto error;
		CHECK_RULE(arithm_op_rule)
		CHECK_RULE(move_rule)
		CHECK_RULE(label_rule)
		CHECK_RULE(branch_rule)
		CHECK_RULE(push_rule)
		CHECK_RULE(pop_rule)
		CHECK_RULE(call_rule)
		CHECK_RULE(return_rule)
		CHECK_RULE(temp_str_print)
#undef CHECK_RULE
error:
		fprintf(stderr, "syntax error at line %u, %u\n", t->line, t->col);
		return AST_ERROR;
	}
	return AST_SUCCESS;

}
