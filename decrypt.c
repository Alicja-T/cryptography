#include "decrypt.h"

static void decrypt_buffer(unsigned char key[], unsigned char result[], unsigned char cipher[], unsigned char previous[]) { //simple decryption using CBC to be upgraded
	for (int i = 0; i < BUFFERSIZE; i++) {
		result[i] = key[i] ^ cipher[i];
		result[i] ^= previous[i];
		previous[i] = cipher[i];
	}
}

int remove_padding(unsigned char source[]) {
	int padding = source[BUFFERSIZE - 1];
	for (int i = BUFFERSIZE - 2; i > BUFFERSIZE - padding - 1; i--) {
		if (source[i] != padding) {
			return 0;
		}
	}
	return padding;
}


void decrypt(FILE *encrypted, unsigned char key[], long int filesize) {
	
	printf("decrypting file... \n");

	FILE *result_file;
	errno_t write_err;
	int block_count = 0;
	int max_block_count = filesize / BUFFERSIZE - 1;

	//add creating new file name
	write_err = fopen_s(&result_file, "result1.txt", "wb");
	if (write_err != 0) {
		fprintf(stderr, "write error");
		exit(1);
	}
	

	unsigned char previous_buffer[BUFFERSIZE];
	size_t read_iv = fread(previous_buffer, 1, BUFFERSIZE, encrypted); //first block is from random IV
	if (read_iv != BUFFERSIZE) {
		printf("drama, can't read the first block!\n");
	}
	else {
		printf("first block read successfully\n");
	}
	
	while (!feof(encrypted)) {

		unsigned char source_buffer[BUFFERSIZE];
		unsigned char cipher_buffer[BUFFERSIZE];
		size_t read_buffer_size = fread(cipher_buffer, 1, BUFFERSIZE, encrypted); //first block is from random IV
		block_count++;
		decrypt_buffer(key, source_buffer, cipher_buffer, previous_buffer);
		if (block_count < max_block_count) {
			write_err = fwrite(source_buffer, 1, BUFFERSIZE, result_file);
		}
		if (block_count == max_block_count) {
			int	padding_size = remove_padding(source_buffer);
			write_err = fwrite(source_buffer, 1, BUFFERSIZE - padding_size, result_file);
		}
		printf("write result %d\n", write_err);
		if (write_err == 0) {
			fprintf(stderr, "write error");
			exit(1);
		}
	}
	printf("total block count: %d\n", block_count);
	fclose(result_file);
}

void decrypt_file(char* cipherpath, char* keypath) {

	FILE *cipherfile;
	FILE *keyfile;
	errno_t cipher_error;
	errno_t key_error;
	long int filesize = get_file_size(cipherpath);

	printf("File %s size: %d\n", cipherpath, filesize);

	cipher_error = fopen_s(&cipherfile, cipherpath, "rb");
	if (cipher_error != 0) {
		fprintf(stderr, "Problem with opening enrypted file");
		exit(1);
	}

	key_error = fopen_s(&keyfile, keypath, "rb");
	if (key_error != 0) {
		fprintf(stderr, "Problem with opening key file");
		exit(1);
	}

	unsigned char key_buffer[BUFFERSIZE];
	size_t read_result = fread(key_buffer, 1, BUFFERSIZE, keyfile);
	if (read_result != BUFFERSIZE) {
		printf("Incomplete key\n");
	}
	else {
		printf("key read successfully\n");
	}
	
	if (filesize != -1) {
		printf("decrypting file %s\n", cipherpath);
		decrypt(cipherfile, key_buffer, filesize);
		fclose(cipherfile);
		fclose(keyfile);
	}
	else {
		exit(1);
	}

}
