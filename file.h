#pragma once

#include <stddef.h>

typedef enum { FILE_IO_SUCCESS, FILE_IO_NOT_FOUND, FILE_IO_OPEN_FAILURE, FILE_IO_READ_FAILURE } file_io_result_t;

file_io_result_t read_file(const char* filename, unsigned char** buffer, size_t* size);
