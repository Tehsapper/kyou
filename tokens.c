#include "tokens.h"

#include "utf8.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

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
		data += 3;
	}

	for (const char* p = data; p < data + data_size;) {
		if (!IS_UTF8(*p)) {
			if (isspace(*p)) {
				if (*p == '\n') {
					++line;
					col = 1;
				} else
					++col;
				++p;
				continue;
			} else {
				fprintf(stderr, "unknown identifier at line %u, %u: %c\n", line, col, *p);
				return TOKENIZE_ERROR;
			}
		} else {

#define strlit_eq(who, str) (strncmp((who), (str), sizeof(str)) == 0)
#define IS_NUMBER(p) (strlit_eq(p, KANJI_ONE) || strlit_eq(p, KANJI_TWO) || strlit_eq(p, KANJI_THREE) || strlit_eq(p, KANJI_FOUR) || strlit_eq(p, KANJI_FIVE) || strlit_eq(p, KANJI_SIX) || strlit_eq(p, KANJI_SEVEN) || strlit_eq(p, KANJI_EIGHT) || strlit_eq(p, KANJI_NINE) || strlit_eq(p, KANJI_TEN))
#define CHECK_KANJI(_kanji, _token) if (strncmp(p, (_kanji), sizeof(_kanji)) == 0) {\
				add_token(toks, (token) { .type = (_token), .line = line, .col = col });\
				p += sizeof(_kanji);\
				++col;\
				continue;\
			}
			CHECK_KANJI(KANJI_SUN, TOKEN_SUN);
			CHECK_KANJI(KANJI_MOON, TOKEN_MOON);
			CHECK_KANJI(KANJI_STARS, TOKEN_STARS);

			// have to check this one first, because the next one will also pass
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
			
			CHECK_KANJI(KANJI_MOVE, TOKEN_MOVE);
			CHECK_KANJI(KANJI_ADD, TOKEN_ADD);

			if (IS_NUMBER(p)) {
				int64_t result = 0;
				int64_t curr = 0;
				int lvl = 0;
				
				const char* d = p;
				while (IS_NUMBER(d)) {
#define CHECK_KANJI_NUMBER(kanji, l, expr) if (strlit_eq(d, (kanji))) {\
					fputs(#kanji, stderr);\
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
					d += sizeof(kanji);\
					fprintf(stderr, "added %zu to d\n", sizeof(kanji));\
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
#undef CHECK_KANJI_NUMBER
				result += curr;
				add_token(toks, (token){ .type = TOKEN_NUMBER, .as_int64 = result, .line = line, .col = col});
				p = d;
				continue;
			}

#undef CHECK_KANJI
			fprintf(stderr, "unknown utf8 identifier at line %u, %u\n", line, col);
			return TOKENIZE_ERROR;
		}
	}
	add_token(toks, (token) { .type = TOKEN_EOF, .line = line, .col = col });
	return TOKENIZE_SUCCESS;
}
