ifeq ($(OS),Windows_NT) 
RM = del /q /f
else 
RM = rm -rf
endif

CC = gcc
CFLAGS = -std=c99 -g -Wall -pedantic

default: t_crypt

t_crypt: utils.o encrypt.o decrypt.o t_crypt.o
	$(CC) $(CFLAGS) -o t_crypt utils.o encrypt.o decrypt.o t_crypt.o

utils.o: utils.h utils.c
	$(CC) $(CFLAGS) -c utils.c

encrypt.o: encrypt.h encrypt.c
	$(CC) $(CFLAGS) -c encrypt.c

decrypt.o: decrypt.h decrypt.c
	$(CC) $(CFLAGS) -c decrypt.c

t_crypt.o: t_crypt.c
	$(CC) $(CFLAGS) -c t_crypt.c

clean: 
	@echo cleaning project
	-$(RM) *.o
	@echo clean completed
