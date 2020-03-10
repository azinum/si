// file.c

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "mem.h"
#include "file.h"

char* read_file(const char* path) {
	long buffer_size, read_size;
	FILE* file = fopen(path, "rb");

	if (file == NULL) {
		printf("Error: %d (%s)\n", errno, strerror(errno));
		return NULL;
	}

	fseek(file, 0, SEEK_END);

	buffer_size = ftell(file);

	rewind(file);

	char* buffer = (char*)malloc(sizeof(char) * (buffer_size + 1));

	read_size = fread(buffer, sizeof(char), buffer_size, file);
	buffer[read_size] = '\0';

	if (buffer_size != read_size) {
		free(buffer);
		buffer = NULL;
		fclose(file);
		return NULL;
	}
	fclose(file);
	return buffer;
}