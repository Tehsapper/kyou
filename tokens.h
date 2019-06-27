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
#define KANJI_STRING       u8"文"
#define KANJI_CHAR         u8"字"

#define KANJI_MOVE         u8"動"
#define KANJI_PUSH         u8"押"
#define KANJI_CALL         u8"呼"
#define KANJI_RETURN       u8"帰"
#define KANJI_POP          u8"弾"

#define KANJI_ADD          u8"足"
#define KANJI_SUBSTRACT    u8"引"
#define KANJI_MULTIPLY     u8"掛"
#define KANJI_DIVIDE       u8"割"
#define KANJI_MODULO       u8"余"
#define KANJI_OR           u8"或"
#define KANJI_AND          u8"共"
#define KANJI_XOR          u8"排"

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

#define KANJI_LABEL        u8"札"
#define KANJI_SECTION      u8"句"
#define KANJI_EXPORT       u8"輸出"
#define KANJI_IMPORT       u8"輸入"
#define KANJI_STORE        u8"資"
#define KANJI_TIMES        u8"度"
#define KANJI_BRANCH       u8"別"
#define KANJI_ALWAYS       u8"常"
#define KANJI_EQUALS       u8"等"
#define KANJI_GREATER      u8"大"
#define KANJI_LESS         u8"小"

#define KANJI_OPEN_QUOTE   u8"「"
#define KANJI_CLOSE_QUOTE  u8"」"

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

	TOKEN_STRING_TYPE, // not to be confused with spring and string
	TOKEN_CHAR,

	// operations
	TOKEN_MOVE,
	TOKEN_PUSH,
	TOKEN_CALL,
	TOKEN_RETURN,
	TOKEN_POP,

	TOKEN_ADD,
	TOKEN_SUB,
	TOKEN_MUL,
	TOKEN_DIV,
	TOKEN_MOD,
	TOKEN_OR,
	TOKEN_AND,
	TOKEN_XOR,

	TOKEN_IDENTIFIER,
	TOKEN_NUMBER,
	TOKEN_STRING,
	TOKEN_LABEL,
	TOKEN_BRANCH,
	TOKEN_ALWAYS,
	TOKEN_EQUALS,
	TOKEN_LESS,
	TOKEN_GREATER,

	TOKEN_EOF,
	TOKEN_NONE
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
		const char* as_cstr;
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
