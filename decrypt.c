
#include "decrypt.h"

static void decrypt_buffer(unsigned char key[], unsigned char result[], unsigned char cipher[], unsigned char previous[]) { //simple decryption using CBC to be upgraded
	for (int i = 0; i < BUFFERSIZE; i++) {
		result[i] = key[i] ^ cipher[i];
		result[i] ^= previous[i];
		previous[i] = cipher[i];
		cipher[i] = result[i];
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

static void save_decrypted(unsigned char *file_name, unsigned char *result, long int file_size) {
	FILE *result_file;
	errno_t write_err;
	int name_length = 0;
	while (file_name[name_length++]);
	char new_file_name[80];
	strncpy(new_file_name, file_name, name_length - 4);
	new_file_name[name_length - 5] = '\0';
	printf("new file name: %s\n", new_file_name);

	write_err = fopen_s(&result_file, "result1.txt", "wb");
	if (write_err != 0) {
		fprintf(stderr, "write error");
		exit(1);
	}
	write_err = fwrite(result, 1, file_size, result_file);
	printf("write result %d\n", write_err);
}

void decrypt(unsigned char *encrypted, unsigned char key[], int blocks_number) {
	
	printf("decrypting file... \n");
		
	int block_count = 0;
	
	unsigned char previous_block[KEYSIZE];
	for (int i = 0; i < KEYSIZE; i++) {
		previous_block[i] = encrypted[i];
	}

	unsigned char *encrypted_buffer;
	encrypted_buffer = encrypted + KEYSIZE;
	
	while (block_count < blocks_number) {
				
		unsigned char result_buffer[BUFFERSIZE];
		decrypt_buffer(key, result_buffer, encrypted_buffer, previous_block);
		
		if (block_count <= blocks_number) {
			encrypted_buffer += BUFFERSIZE;
		}
		block_count++;
	}
	printf("total block count: %d\n", block_count);
	
}

void decrypt_file(char* encrypted_name, char* keypath) {

	FILE *encrypted_file;
	FILE *keyfile;
	errno_t encrypted_error;
	errno_t key_error;
	long int filesize = get_file_size(encrypted_name);

	printf("File %s size: %d\n", encrypted_name, filesize);

	encrypted_error = fopen_s(&encrypted_file, encrypted_name, "rb");
	if (encrypted_error != 0) {
		fprintf(stderr, "Problem with opening enrypted file");
		exit(1);
	}

	key_error = fopen_s(&keyfile, keypath, "rb");
	if (key_error != 0) {
		fprintf(stderr, "Problem with opening key file");
		exit(1);
	}

	unsigned char key_buffer[KEYSIZE];
	size_t read_result = fread(key_buffer, 1, KEYSIZE, keyfile);
	if (read_result != KEYSIZE) {
		printf("Incomplete key\n");
		fclose(encrypted_file);
		fclose(keyfile);
		exit(1);
	}
	else {
		printf("key read successfully\n");
	}
	
	if (filesize != -1) {
		unsigned char *encrypted_buffer;
		int blocks_number = (filesize - KEYSIZE) / BUFFERSIZE;
		encrypted_buffer = malloc(filesize);
		if (encrypted_buffer != NULL) {
			fread(encrypted_buffer, filesize, 1, encrypted_file);
			fclose(encrypted_file);
			encrypted_file = NULL;

			printf("decrypting file %s\n", encrypted_name);
			decrypt(encrypted_buffer, key_buffer, blocks_number);
			
			int padding = remove_padding(encrypted_buffer + KEYSIZE + (blocks_number - 1) * BUFFERSIZE);
			int final_size = blocks_number * BUFFERSIZE - padding;
			save_decrypted(encrypted_name, encrypted_buffer + KEYSIZE, final_size);
			free(encrypted_buffer);

			fclose(keyfile);
			keyfile = NULL;
		}
	}
	else {
		exit(1);
	}

}
