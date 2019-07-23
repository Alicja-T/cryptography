#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include "utils.h"


void get_random_buffer(unsigned char buffer[], int size) {
	time_t now = time(0);
	srand(now);
	for (int i = 0; i < size; i++) {
		buffer[i] = rand() % 256;
	}
}

long int get_file_size(char* filepath) {
	struct _stat buffer;
	long int size = -1;
	int result = _stat(filepath, &buffer);
	
	if (result != 0) {
		if (errno == ENOENT) {
			printf(stderr, "File %s not found\n", filepath);
		}
	}
	else {
		size = buffer.st_size;
	}
	return size;
}