#pragma once

#define IS_UTF8(a) ((a) & (1 << 7))

int utf8_has_bom(const char* data);
int utf8_size(char ch);
