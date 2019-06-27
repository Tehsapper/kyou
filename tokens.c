#include "tokens.h"

#include "utf8.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int add_token(tokens* toks, token t)
{
	toks->data = realloc(toks->data, sizeof(token) * ++toks->size);
	if (toks->data == NULL)
		return 0;
	toks->data[toks->size-1] = t;
	return 1;
}

tokenize_result_t tokenize(tokens* toks, const char* data, size_t data_size)
{
	//tokens* toks = malloc(sizeof(tokens));
	toks->data = NULL;
	toks->size = 0;

	uint32_t line = 1;
	uint32_t col = 1;

	if (utf8_has_bom(data)) {
		fprintf(stderr, "had bom\n");
		data += 3;
	}

	for (const char* p = data; p < data + data_size;) {
		if (*p == '#') {
			while (*p != '\n' && *p++ != '\0');
			++p;
			++line;
			col = 1;
			continue;
		}

		if (isspace(*p)) {
			if (*p == '\n') {
				++line;
				col = 1;
			} else {
				++col;
			}
			++p;
			continue;
		} else {

#define strlit_eq(who, str) (strncmp((who), (str), utf8_size(*(str))) == 0)
#define IS_NUMBER(p) (strlit_eq(p, KANJI_ZERO) || strlit_eq(p, KANJI_ONE) || strlit_eq(p, KANJI_TWO) || strlit_eq(p, KANJI_THREE) || strlit_eq(p, KANJI_FOUR) || strlit_eq(p, KANJI_FIVE) || strlit_eq(p, KANJI_SIX) || strlit_eq(p, KANJI_SEVEN) || strlit_eq(p, KANJI_EIGHT) || strlit_eq(p, KANJI_NINE) || strlit_eq(p, KANJI_TEN))
#define CHECK_KANJI(_kanji, _token)	if (strlit_eq(p, _kanji)) {\
				add_token(toks, (token) { .type = (_token), .line = line, .col = col });\
				p += utf8_size(*(_kanji));\
				++col;\
				continue;\
			}
			CHECK_KANJI(KANJI_SUN, TOKEN_SUN);
			CHECK_KANJI(KANJI_MOON, TOKEN_MOON);
			CHECK_KANJI(KANJI_STARS, TOKEN_STARS);

			CHECK_KANJI(KANJI_STORAGE_BASE, TOKEN_STORAGE_BASE);
			CHECK_KANJI(KANJI_STORAGE, TOKEN_STORAGE);

			CHECK_KANJI(KANJI_FIRE, TOKEN_FIRE);
			CHECK_KANJI(KANJI_WATER, TOKEN_WATER);
			CHECK_KANJI(KANJI_TREE, TOKEN_TREE);
			CHECK_KANJI(KANJI_EARTH, TOKEN_EARTH);
			CHECK_KANJI(KANJI_METAL, TOKEN_METAL);

			CHECK_KANJI(KANJI_SPRING, TOKEN_SPRING);
			CHECK_KANJI(KANJI_SUMMER, TOKEN_SUMMER);
			CHECK_KANJI(KANJI_AUTUMN, TOKEN_AUTUMN);
			CHECK_KANJI(KANJI_WINTER, TOKEN_WINTER);
			CHECK_KANJI(KANJI_STRING, TOKEN_STRING_TYPE);
			CHECK_KANJI(KANJI_CHAR, TOKEN_CHAR);
			
			CHECK_KANJI(KANJI_MOVE, TOKEN_MOVE);
			CHECK_KANJI(KANJI_PUSH, TOKEN_PUSH);
			CHECK_KANJI(KANJI_POP, TOKEN_POP);
			CHECK_KANJI(KANJI_CALL, TOKEN_CALL);
			CHECK_KANJI(KANJI_RETURN, TOKEN_RETURN);

			CHECK_KANJI(KANJI_ADD, TOKEN_ADD);
			CHECK_KANJI(KANJI_SUBSTRACT, TOKEN_SUB);
			CHECK_KANJI(KANJI_MULTIPLY, TOKEN_MUL);
			CHECK_KANJI(KANJI_DIVIDE, TOKEN_DIV);
			CHECK_KANJI(KANJI_MODULO, TOKEN_MOD);
			CHECK_KANJI(KANJI_OR, TOKEN_OR);
			CHECK_KANJI(KANJI_AND, TOKEN_AND);
			CHECK_KANJI(KANJI_XOR, TOKEN_XOR);

			CHECK_KANJI(KANJI_LABEL, TOKEN_LABEL);
			CHECK_KANJI(KANJI_BRANCH, TOKEN_BRANCH);
			CHECK_KANJI(KANJI_ALWAYS, TOKEN_ALWAYS);
			CHECK_KANJI(KANJI_EQUALS, TOKEN_EQUALS);
			CHECK_KANJI(KANJI_GREATER, TOKEN_GREATER);
			CHECK_KANJI(KANJI_LESS, TOKEN_LESS);

			if (IS_NUMBER(p)) {
				int64_t result = 0;
				int64_t curr = 0;
				int lvl = 0;
				
				const char* d = p;
				while (IS_NUMBER(d)) {
#define CHECK_KANJI_NUMBER(kanji, l, expr) if (strlit_eq(d, (kanji))) {\
					if ((l) == lvl) {\
						fprintf(stderr, "malformed number at %u, %u (l %d lvl %d)\n", line, col, (l), lvl);\
						return TOKENIZE_ERROR;\
					}\
					if ((l) < lvl) {\
						result += curr;\
						curr = 0;\
					}\
					expr;\
					lvl = (l);\
					d += utf8_size(*kanji);\
					++col;\
					continue;\
				}

					CHECK_KANJI_NUMBER(KANJI_ZERO, 1, curr += 0);
					CHECK_KANJI_NUMBER(KANJI_ONE, 1, curr += 1);
					CHECK_KANJI_NUMBER(KANJI_TWO, 1, curr += 2);
					CHECK_KANJI_NUMBER(KANJI_THREE, 1, curr += 3);
					CHECK_KANJI_NUMBER(KANJI_FOUR, 1, curr += 4);
					CHECK_KANJI_NUMBER(KANJI_FIVE, 1, curr += 5);
					CHECK_KANJI_NUMBER(KANJI_SIX, 1, curr += 6);
					CHECK_KANJI_NUMBER(KANJI_SEVEN, 1, curr += 7);
					CHECK_KANJI_NUMBER(KANJI_EIGHT, 1, curr += 8);
					CHECK_KANJI_NUMBER(KANJI_NINE, 1, curr += 9);
					CHECK_KANJI_NUMBER(KANJI_TEN, 2, curr = 10 * (curr ? curr : 1));
					CHECK_KANJI_NUMBER(KANJI_HUNDRED, 3, curr = 100 * (curr ? curr : 1));
					CHECK_KANJI_NUMBER(KANJI_THOUSAND, 4, curr = 1000 * (curr ? curr : 1));
					CHECK_KANJI_NUMBER(KANJI_TEN_THOUSAND, 5, curr = 10000 * (curr ? curr : 1));
				}
				result += curr;
				add_token(toks, (token){ .type = TOKEN_NUMBER, .as_int64 = result, .line = line, .col = col});
				p = d;
				continue;
			}

			if (isalpha(*p)) {
				const char* d = p;
				while (isalnum(*d)) ++d;
				
				char* str = malloc(d - p + 1);
				str[d - p] = 0;
				strncpy(str, p, d - p);

				add_token(toks, (token) { .type = TOKEN_IDENTIFIER, .as_cstr = str, .line = line, .col = col});
				p = d;
				col += d - p;
				continue;
			}

			if (strlit_eq(p, KANJI_OPEN_QUOTE)) {
				const char* d = p + utf8_size(*KANJI_OPEN_QUOTE);
				while (isascii(*d)) {//(isalnum(*d) || isspace(*d)) {	
					if (*d == '\n') { ++line; col = 1; } else ++col;
					//fprintf(stderr, "added %c to str\n", *d);
					++d;
				}
				if (!strlit_eq(d, KANJI_CLOSE_QUOTE)) {
					fprintf(stderr, "did not close string literal properly at line %u, %u\n", line, col);
					return TOKENIZE_ERROR;
				}// else fprintf(stderr, "closed string\n");

				char* str = malloc(d - p - utf8_size(*KANJI_OPEN_QUOTE));
				str[d - p - utf8_size(*KANJI_OPEN_QUOTE)] = 0;
				strncpy(str, p + utf8_size(*KANJI_OPEN_QUOTE), d - p - utf8_size(*KANJI_OPEN_QUOTE));

				add_token(toks, (token) { .type = TOKEN_STRING, .as_cstr = str, .line = line, .col = col});
				//fprintf(stderr, "resulted in %s\n", str);
				p = d + utf8_size(*KANJI_CLOSE_QUOTE); // d should point to CLOSE_QUOTE
				continue;
			}
			fprintf(stderr, "unknown symbol at line %u, %u\n", line, col);
			return TOKENIZE_ERROR;
		}
	}
	add_token(toks, (token) { .type = TOKEN_EOF, .line = line, .col = col });
	return TOKENIZE_SUCCESS;
}
