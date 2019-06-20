#pragma once

#include <string.h>

#define UTF8_BOM "\xEF\xBB\xBF"
#define IS_UTF8(a) ((a) & (1 << 7))

int utf8_has_bom(const char* data)
{
	if (strncmp(data, UTF8_BOM, sizeof(UTF8_BOM)) == 0)
		return 1;
	return 0;
}

/*int utf8_strncmp(const char* a, const char* b, size_t n)
{
	const char* pa = a;
	const char* pb = b;

	while (pa < a + n && pb < b + n) {
		if 
	}
}*/
