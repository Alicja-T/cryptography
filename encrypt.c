#include "encrypt.h"

void generate_key(unsigned char key[]) {
	FILE *newkeyfile;
	errno_t err;
	time_t now = time(0);
	char* filename = "testkey.tck";

	err = fopen_s(&newkeyfile, filename, "wb");
	if (err != 0) {
		fprintf(stderr, "problem with generating key file");
	}
	else {
		get_random_buffer(key, BUFFERSIZE);
		fwrite(key, sizeof(char), BUFFERSIZE, newkeyfile);
		fclose(newkeyfile);
		printf("generated new key\n");
	}
}



static void encrypt_buffer(unsigned char key[], unsigned char source[], unsigned char cipher[], unsigned char previous[]) { //simple encryption using CBC to be upgraded
	for (int i = 0; i < BUFFERSIZE; i++) {
		cipher[i] = source[i] ^ previous[i];
		cipher[i] ^= key[i];
		previous[i] = cipher[i];
	}
}

void encrypt(FILE *source, unsigned char key[], long int file_size) {
	
	FILE *cipherfile;
	errno_t write_err;
	int block_count = 0;
	int max_block_count = file_size / BUFFERSIZE - 1;
	write_err = fopen_s(&cipherfile, "cipher.txt", "wb");
	if (write_err != 0) {
		fprintf(stderr, "write error");
		exit(1);
	}
	//prepend random buffer to cipher file
	unsigned char previous_buffer[BUFFERSIZE];
	get_random_buffer(previous_buffer, BUFFERSIZE);
	fwrite(previous_buffer, 1, BUFFERSIZE, cipherfile);
	while (!feof(source)) {
		unsigned char source_buffer[BUFFERSIZE];
		unsigned char cipher_buffer[BUFFERSIZE];
		if (block_count < max_block_count) {
			size_t read_result = fread(source_buffer, 1, BUFFERSIZE, source);
		}
		else {
			int i;
			for (i = 0; i < BUFFERSIZE; i++) {
				int c = fgetc(source);
				if (c == EOF) {
					break;
				}
				source_buffer[i] = c;
			}
			if (i == 0) break;
			int padding = BUFFERSIZE - i;
			for (; i < BUFFERSIZE; i++) {
				source_buffer[i] = padding;
			}
		}
		encrypt_buffer(key, source_buffer, cipher_buffer, previous_buffer);
		write_err = fwrite(cipher_buffer, sizeof(char), BUFFERSIZE, cipherfile);
		block_count++;
		printf("write result %d\n", write_err);
		if (write_err == 0) {
			fprintf(stderr, "write error");
			fclose(cipherfile);
			fclose(source);
			exit(1);
		}
	}
	printf("total block count: %d\n", block_count);
	fclose(cipherfile);
	fclose(source);
}

void encrypt_file(char *filepath) {
	FILE *sourcefile;
	errno_t source_error;
	long int filesize = get_file_size(filepath);

	printf("File %s size: %d\n", filepath, filesize);
	
	source_error = fopen_s(&sourcefile, filepath, "rb");
	if (source_error != 0) {
		fprintf(stderr, "Problem with opening source file");
		exit(1);
	}
		
	unsigned char key_buffer[BUFFERSIZE];
	generate_key(key_buffer);

	if (filesize != -1) {
		printf("encrypting file %s\n", filepath);
		encrypt(sourcefile, key_buffer, filesize);
		fclose(sourcefile);
	}
	else {
		exit(1);
	}

}