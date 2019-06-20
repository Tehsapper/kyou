#include "file.h"

#include <stdio.h>
#include <stdlib.h>

static size_t file_length(FILE* file)
{
	size_t cur = ftell(file);
	fseek(file, 0, SEEK_END);
	size_t result = ftell(file);
	fseek(file, cur, SEEK_SET);
	return result;
}

file_io_result_t read_file(const char* filename, unsigned char** buffer, size_t* size)
{
	FILE* file = fopen(filename, "rb");

	if (file == NULL)
		return FILE_IO_NOT_FOUND;

	*size = file_length(file);
	*buffer = malloc(*size);

	fread(*buffer, 1, *size, file);

	if (ferror(file)) {
		free(*buffer);
		*buffer = NULL;
		*size = 0;
		fclose(file);
		return FILE_IO_READ_FAILURE;
	}

	fclose(file);
	return FILE_IO_SUCCESS;
}
