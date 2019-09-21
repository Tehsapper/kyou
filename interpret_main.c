#include <stdio.h>
#include <stdlib.h>

#include "file.h"
#include "ast.h"
#include "interpret.h"

int main(int argc, char* argv[])
{
	unsigned char* data;
	size_t data_size;

	if (argc < 2) {
		fprintf(stderr, "usage: kyou [file]\n");
		return EXIT_FAILURE;
	}

	if (read_file(argv[1], &data, &data_size) != FILE_IO_SUCCESS) {
		fprintf(stderr, "failed to read data from file %s!\n", argv[1]);
		return EXIT_FAILURE;
	}

	AST ast;
	if (build_ast(&ast, data, data_size) != AST_SUCCESS) {
		return EXIT_FAILURE;
	}

	interpret_ast(ast);

	return EXIT_SUCCESS;
}
