#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define BUFFERSIZE 64
#define KEYSIZE 64


encrypt_buffer(unsigned char key[], unsigned char source[], unsigned char cipher[], unsigned char previous[]) { //simple encryption using CBC to be upgraded
	for (int i = 0; i < BUFFERSIZE; i++) {
		cipher[i] = source[i] ^ previous[i];
		cipher[i] ^= key[i];
		previous[i] = cipher[i];
		//printf("cypher: %c\n", cipher[i]);
	}
}

decrypt_buffer(unsigned char key[], unsigned char result[], unsigned char cipher[], unsigned char previous[]) { //simple decryption using CBC to be upgraded
	for (int i = 0; i < BUFFERSIZE; i++) {
		result[i] = key[i]^cipher[i];
		result[i] ^= previous[i];
		previous[i] = cipher[i];
	}
}

generate_key(unsigned char key[]) {
	FILE *newkeyfile;
	errno_t err;
	time_t now = time(0);
	char* filename = "testkey.tck";

	err = fopen_s(&newkeyfile, filename, "wb");
	if (err != 0) {
		fprintf(stderr, "problem with generating key file");
	}
	else {
		srand(now);
		for (int i = 0; i < BUFFERSIZE; i++) {
			key[i] = rand() % 256;
			printf("%c\n", key[i]);
			fprintf(newkeyfile, "%c", key[i]);
		}
		fclose(newkeyfile);
	}

	printf("generated new key\n");
}


encrypt(FILE *source, char *filename, unsigned char key[]) {
	printf("encrypting file %s\n", filename);
	FILE *cipherfile;
	errno_t write_err;
	int block_count = 0;
	write_err = fopen_s(&cipherfile, "cipher.txt", "wb");
	if (write_err != 0) {
		fprintf(stderr, "write error");
		exit(1);
	}
	//prepend random buffer to cipher file
	unsigned char previous_buffer[BUFFERSIZE];
	time_t now = time(0);
	srand(now);
	for (int i = 0; i < BUFFERSIZE; i++) {
		previous_buffer[i] = rand() % 256;
	}
	fwrite(previous_buffer, 1, BUFFERSIZE, cipherfile);
	while (!feof(source)) {
		unsigned char source_buffer[BUFFERSIZE];
		unsigned char cipher_buffer[BUFFERSIZE];
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

decrypt(FILE *encrypted, FILE* key) {
	printf("decrypting file... \n");
	
	FILE *result_file;
	errno_t write_err;
	int block_count = 0;
	//add creating new file name
	write_err = fopen_s(&result_file, "result.txt", "wb");
	if (write_err != 0) {
		fprintf(stderr, "write error");
		exit(1);
	}
	unsigned char key_buffer[BUFFERSIZE];
	size_t read_result = fread(key_buffer, 1, BUFFERSIZE, key);
	if (read_result != BUFFERSIZE) {
		printf("drama, can't read the key\n");
	}
	else {
		printf("key read successfully\n");
	}

	unsigned char previous_buffer[BUFFERSIZE];
	size_t read_iv = fread(previous_buffer, 1, BUFFERSIZE, encrypted);
	if (read_result != BUFFERSIZE) {
		printf("drama, can't read the first block!\n");
	}
	else {
		printf("first block read successfully\n");
	}
	int c;
	while ( (c = fgetc(encrypted)) != EOF ) {
		
		unsigned char source_buffer[BUFFERSIZE];
		unsigned char cipher_buffer[BUFFERSIZE];
		int i;
		cipher_buffer[0] = c;
		for (i = 1; i < BUFFERSIZE; i++) {
			c = fgetc(encrypted);
			if (c == EOF) {
				break;
				printf("value of i %d", i);
			}
			cipher_buffer[i] = c;
			//printf( "cipher buffer %d %c\n", i, cipher_buffer[i] );
		}
		if (i == 0) break;
		decrypt_buffer(key_buffer, source_buffer, cipher_buffer, previous_buffer);
		write_err = fwrite(source_buffer, 1, BUFFERSIZE, result_file);
		block_count++;
		printf("write result %d\n", write_err);
		if (write_err == 0) {
			fprintf(stderr, "write error");
			exit(1);
		}
	}
	printf("total block count: %d\n", block_count);
	fclose(result_file);
	fclose(key);
	fclose(encrypted);
}




int main(int argc, char* argv[]) {
	
	int c;
	FILE *sourcefile;
	FILE *keyfile;
	FILE *encryptedfile;
	unsigned char key_buffer[KEYSIZE];
	errno_t err;
	errno_t key_err;
	errno_t encrypt_err;

	if (argc <= 2 || argc > 4) {
		fprintf(stderr, "Usage: -e (encrypt) sourcefilename | -d (decrypt) encryptedfilename keyfilename");
		exit(1);
	}

	if ( argc == 3 ) {
		if (strcmp(argv[1], "-e") != 0) {
			if (strcmp(argv[1], "-d") == 0) {
				fprintf(stderr, "Missing file name");
				exit(1);
			}
			else {
				fprintf(stderr, "Invalid argument");
				exit(1);
			}
		}
		err = fopen_s(&sourcefile, argv[2], "rb");
		if (err != 0) {
			fprintf(stderr, "%s: can't open %s\n", argv[0], argv[2]);
			exit(1);
		}
		else {
			printf("successfully opened %s \n", argv[2]);
			generate_key(key_buffer);
			encrypt(sourcefile, argv[2], key_buffer);
			printf("successfully encrypted %s \n", argv[2]);
			exit(0);
		}
	}

	if (argc == 4) {
		if (strcmp(argv[1], "-d") != 0) {
			if (strcmp(argv[1], "-e") == 0) {
				fprintf(stderr, "Too many arguments");
				exit(1);
			}
			else {
				fprintf(stderr, "Invalid argument");
				exit(1);
			}
		}
		encrypt_err = fopen_s(&encryptedfile, argv[2], "rb");
		key_err = fopen_s(&keyfile, argv[3], "rb");
		if (encrypt_err != 0) {
			fprintf(stderr, "%s: can't open encrypted file %s\n", argv[0], argv[2]);
			exit(1);
		}
		else {
			if (key_err != 0) {
				fprintf(stderr, "%s: can't open key file %s\n", argv[0], argv[3]);
				exit(1);
			}
			else {
				printf("successfully opened encrypted file %s\n", argv[2]);
				decrypt(encryptedfile, keyfile);
				printf("successfully decrypted file %s\n", argv[2]);
				exit(0);
			}
		}
	}

	return 0;
}