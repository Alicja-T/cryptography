#include <stdio.h>
#include <ctype.h>
#include "encrypt.h"
#include "decrypt.h"

/*
Main parses command line arguments and redirects control to encrypting or decrypting modules
depending on the mode. 
*/


int main(int argc, char* argv[]) {
		
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
		encrypt_file(argv[2]);
		printf("successfully encrypted %s \n", argv[2]);
		exit(0);
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
		decrypt_file(argv[2], argv[3]);
		printf("successfully decrypted file %s\n", argv[2]);
		exit(0);
	}
	return 0;
}