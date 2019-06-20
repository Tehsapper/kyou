#pragma once

#include <stddef.h>
#include <stdint.h>

#define KANJI_SUN          u8"日"
#define KANJI_MOON         u8"月"
#define KANJI_STARS        u8"星"
#define KANJI_STORAGE_BASE u8"品"
#define KANJI_STORAGE      u8"品台"
#define KANJI_FIRE         u8"火"
#define KANJI_WATER        u8"水"
#define KANJI_TREE         u8"木"
#define KANJI_EARTH        u8"土"
#define KANJI_METAL        u8"金"
#define KANJI_SPRING       u8"春"
#define KANJI_SUMMER       u8"夏"
#define KANJI_AUTUMN       u8"秋"
#define KANJI_WINTER       u8"冬"
#define KANJI_MOVE         u8"動"
#define KANJI_ADD          u8"足"
#define KANJI_ZERO         u8"霊"
#define KANJI_ONE          u8"一"
#define KANJI_TWO          u8"二"
#define KANJI_THREE        u8"三"
#define KANJI_FOUR         u8"四"
#define KANJI_FIVE         u8"五"
#define KANJI_SIX          u8"六"
#define KANJI_SEVEN        u8"七"
#define KANJI_EIGHT        u8"八"
#define KANJI_NINE         u8"九"
#define KANJI_TEN          u8"十"
#define KANJI_HUNDRED      u8"百"
#define KANJI_THOUSAND     u8"千"
#define KANJI_TEN_THOUSAND u8"万"

typedef enum {
	// three lights: stdout, stdin and virtual memory
	TOKEN_SUN,
	TOKEN_MOON,
	TOKEN_STARS,
	
	// additional tokens for stack
	TOKEN_STORAGE,
	TOKEN_STORAGE_BASE,
	
	// five elements
	TOKEN_FIRE,
	TOKEN_WATER,
	TOKEN_TREE,
	TOKEN_METAL,
	TOKEN_EARTH,

	// four seasons
	TOKEN_SPRING,
	TOKEN_SUMMER,
	TOKEN_AUTUMN,
	TOKEN_WINTER,

	// operations
	TOKEN_MOVE,
	TOKEN_ADD,
	TOKEN_SUB,
	TOKEN_MUL,
	TOKEN_DIV,

	TOKEN_IDENTIFIER,
	TOKEN_NUMBER,
	TOKEN_STRING,

	TOKEN_EOF
}
token_type;

typedef struct
{
	token_type type;
	union {
		int8_t as_int8;
		int16_t as_int16;
		int32_t as_int32;
		int64_t as_int64;
		void* as_ptr;
	};
	uint32_t line, col; // hopefully nobody has more than 4 mln lines of code in a file
}
token;

typedef struct
{
	token* data;
	size_t size;
}
tokens;

typedef enum { TOKENIZE_SUCCESS, TOKENIZE_ERROR } tokenize_result_t;

tokenize_result_t tokenize(tokens* output, const char* data, size_t data_size);
