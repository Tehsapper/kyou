#include "utf8.h"

#include <string.h>

#define UTF8_BOM "\xEF\xBB\xBF"

int utf8_has_bom(const char* data)
{
	if (strncmp(data, UTF8_BOM, sizeof(UTF8_BOM)) == 0)
		return 1;
	return 0;
}

int utf8_size(char ch)
{
	if (!IS_UTF8(ch))
		return 1;
	if ((ch & 0xE0) == 0xC0)
		return 2;
	if ((ch & 0xF0) == 0xE0)
		return 3;
	if ((ch & 0xF8) == 0xF0)
		return 4;

	// probably should throw an error
	return 1;
}